/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <semaphore.h>

#define NAME "TimsRouterTcp"

#define DEBUG
//#define DEBUG_DETAIL

#include <main/tims/tims.h>
#include <main/tims/router/tims_router.h>

#define TIMS_LEVEL_PRINT          0
#define TIMS_LEVEL_DBG            1
#define TIMS_LEVEL_DBG_DETAIL     2

#define tims_print(fmt, ... )       \
                                do { \
                                  if (loglevel >= TIMS_LEVEL_PRINT) \
                                    printf(NAME": "fmt, ##__VA_ARGS__); \
                                } while(0)

#define tims_dbg(fmt, ... )       \
                                do { \
                                  if (loglevel >= TIMS_LEVEL_DBG) \
                                    printf(NAME": "fmt, ##__VA_ARGS__); \
                                } while(0)

#define tims_dbgdetail(fmt, ... )       \
                                do { \
                                  if (loglevel >= TIMS_LEVEL_DBG_DETAIL) \
                                    printf(NAME": "fmt, ##__VA_ARGS__); \
                                } while(0)

//
// init flags
//
#define TIMS_ROUTER_SEM_LIST          0x0001
#define TIMS_ROUTER_WATCHDOG          0x0002


//
// parameter, constants and variables
//


#define DEFAULT_PORT        2000
#define DEFAULT_MAX         256

static unsigned int         maxMsgSize;
static unsigned int         init_flags;
static unsigned int         sem_flags;
static int                  terminate;
static int                  tcpServerSocket = -1;
static struct sockaddr_in   tcpServerAddr;
static pthread_t            watchdogThread;
static int                  loglevel;
static struct sched_param   sched_param = { .sched_priority = 1 };


typedef struct {
    int                 index;
    int                 socket;
    struct sockaddr_in  addr;
    socklen_t           addrLen;
    pthread_t           conThread;
    int                 watchdog;
    sem_t               sendSem;
} connection_t;

// if MAX_CONNECTIONS > 32 -> look @ sem_flags
#define MAX_CONNECTIONS 8

connection_t conList[MAX_CONNECTIONS];

typedef struct {
    int32_t mbx;
    int32_t conIndex;
} mbx_data_t;

#define MAX_MBX 256

mbx_data_t mbxList[MAX_MBX];
int        mbxNum = 0;
sem_t      mbxListSem;


//
// connection functions
//
void connection_close(connection_t *con)
{
    close(con->socket);
    con->socket = -1;
}

connection_t* connection_getFree(connection_t* newCon)
{
    int i;

    for (i = 0; i < MAX_CONNECTIONS; i++)
    {
        if (conList[i].socket == -1)    // free socket
        {
            memcpy(&conList[i].addr, &newCon->addr, sizeof(newCon->addr));
            conList[i].addrLen  = newCon->addrLen;
            conList[i].socket   = newCon->socket;
            conList[i].index    = i;
            conList[i].watchdog = 0;
            return &conList[i];
        }
    }
    return NULL;
}

void mailbox_delete_all(int conIndex);

void connection_delete(connection_t* con)
{
    mailbox_delete_all(con->index);
    shutdown(con->socket, SHUT_RDWR);
    connection_close(con);

    tims_dbgdetail("connection[%i] closed\n", con->index);
}

//
// TCP send / receive functions
//

int sndTcpTimsMsg(connection_t *con, tims_msg_head* sndMsg)
{
    int idx = con->index;
    int ret;

    sem_wait(&con->sendSem);
    tims_dbgdetail("con[%02d]: %15s: %8x --(%4d)--> %8x, sending %u bytes\n",
                   idx, inet_ntoa(con->addr.sin_addr), sndMsg->src,
                   sndMsg->type, sndMsg->dest, sndMsg->msglen);

    ret = send(con->socket, sndMsg, sndMsg->msglen, 0);
    if ( ret == -1)
    {
        connection_close(con);
        sem_post(&con->sendSem);
        tims_print("con[%02d]: %15s: %8x --(%4d)--> %8x, (%u bytes), "
                   "send ERROR, code = %d\n", idx, inet_ntoa(con->addr.sin_addr),
                   sndMsg->src, sndMsg->type, sndMsg->dest, sndMsg->msglen, errno);

        return -1;
    }
    sem_post(&con->sendSem);
    return 0;
}

int recvTcpTimsMsg(connection_t *con, tims_msg_head* recvMsg)
{
    int          ret, idx;
    unsigned int len = 0;

    idx = con->index;

    // receive head
    while (len < TIMS_HEADLEN)
    {
        ret = recv(con->socket, (void*)recvMsg + len, TIMS_HEADLEN - len, 0);
        if (ret < 0)
        {
            connection_close(con);
            tims_print("con[%02d]: %15s: recv head ERROR, (%s)\n",
                       idx, inet_ntoa(con->addr.sin_addr), strerror(errno));
            return ret;
        }

        if (!ret)
        {
            connection_close(con);
            tims_print("con[%02d]: %15s: recv head ERROR, socket closed\n",
                       idx, inet_ntoa(con->addr.sin_addr));
            return -1;
        }
        len += ret;

        if (terminate)
            return -EINTR;
    }

    tims_parse_head_byteorder(recvMsg);

    if (recvMsg->msglen < TIMS_HEADLEN)
    {
        connection_close(con);
        tims_print("con[%02d]: %15s: recv ERROR, invalid message length: "
                   "%u is smaller than TIMS_HEADLEN\n", idx,
                   inet_ntoa(con->addr.sin_addr), recvMsg->msglen);
        return -1;
    }
    else if (recvMsg->msglen > maxMsgSize)
    {
        connection_close(con);
        tims_print("con[%02d]: %15s: %8x --(%4d)--> %8x, recv ERROR, message "
                   "(%u bytes) is too big for buffer (%u bytes)\n", idx,
                   inet_ntoa(con->addr.sin_addr), recvMsg->src, recvMsg->type,
                   recvMsg->dest, recvMsg->msglen, maxMsgSize);
        return -1;
    }

    if (recvMsg->msglen > TIMS_HEADLEN)
    {
        while (len < recvMsg->msglen)
        {
            ret = recv(con->socket, (void*)recvMsg + len,
                       recvMsg->msglen - len, 0);
            if (ret == -1)
            {
                connection_close(con);
                tims_print("con[%02d]: %15s: %8x --(%4d)--> %8x, recv body "
                           "ERROR, (%s)\n", idx,
                           inet_ntoa(con->addr.sin_addr), recvMsg->src,
                           recvMsg->type, recvMsg->dest, strerror(errno));
                return -1;
            }

            if (!ret)
            {
                connection_close(con);
                tims_print("con[%02d]: %15s: %8x --(%4d)--> %8x, recv body "
                           "ERROR, socket closed\n", idx,
                           inet_ntoa(con->addr.sin_addr), recvMsg->src,
                           recvMsg->type, recvMsg->dest);
                return -1;
            }
            len += ret;

            if (terminate)
                return -EINTR;
        }

        tims_dbgdetail("con[%02d]: %15s: %8x --(%4d)--> %8x, recv head (%u bytes), "
                       "body (%u bytes)\n", idx, inet_ntoa(con->addr.sin_addr),
                       recvMsg->src, recvMsg->type, recvMsg->dest,
                       TIMS_HEADLEN, recvMsg->msglen - TIMS_HEADLEN);

    }
    else
    {
        tims_dbgdetail("con[%02d]: %15s: %8x --(%4d)--> %8x, recv head "
                       "(%u bytes)\n", idx, inet_ntoa(con->addr.sin_addr),
                       recvMsg->src, recvMsg->type, recvMsg->dest,
                       recvMsg->msglen);
    }
    return 0;
}

//
// mailbox functions
//

// returns the connection index of the mbx, -ENODEV if the mbx is unknown
int mailbox_get(int mbx)
{
    int i;
    sem_wait(&mbxListSem);
    for (i = 0; i < mbxNum; i++)
    {
        if (mbxList[i].mbx == mbx)
        {
            sem_post(&mbxListSem);
            return mbxList[i].conIndex;
        }
    }
    sem_post(&mbxListSem);
    return -ENODEV;
}

// adds the mbx to the mbx list, returns -EBUSY if the mbx is allready known
int mailbox_create(int32_t mbx, int conIndex)
{
    int i, j;
    sem_wait(&mbxListSem);

    if (mbxNum >= MAX_MBX)
    {
        sem_post(&mbxListSem);
        return -ENODEV;
    }

    // Find position i to insert the new mbx
    for (i = 0; i < mbxNum; i++)
    {
        if (mbxList[i].mbx > mbx)
        {
            break;
        }
        else if (mbxList[i].mbx == mbx) // mbx is already contained in the list
        {
            sem_post(&mbxListSem);
            return -EBUSY;
        }
    }

    // move the rest of the list
    for (j = mbxNum; j > i; j--)
    {
        mbxList[j].mbx      = mbxList[j-1].mbx;
        mbxList[j].conIndex = mbxList[j-1].conIndex;
    }

    // add new mbx
    mbxList[i].mbx      = mbx;
    mbxList[i].conIndex = conIndex;
    mbxNum++;

    sem_post(&mbxListSem);
    return 0;
}

// remove the mbx from the mbx list
void mailbox_delete(int32_t mbx)
{
    int i, j = 0;
    sem_wait(&mbxListSem);
    for (i = 0; i < mbxNum; i++)
    {
        if (mbxList[i].mbx != mbx)
        {
            mbxList[j].mbx      = mbxList[i].mbx;
            mbxList[j].conIndex = mbxList[i].conIndex;
            j++;
        }
    }
    mbxNum = j;
    sem_post(&mbxListSem);
}

// remove all mbxs with this connection index
void mailbox_delete_all(int conIndex)
{
    int i, j = 0;
    sem_wait(&mbxListSem);
    for (i = 0; i < mbxNum; i++)
    {
        if (mbxList[i].conIndex != conIndex)
        {
            mbxList[j].mbx      = mbxList[i].mbx;
            mbxList[j].conIndex = mbxList[i].conIndex;
            j++;
        }
    }
    mbxNum = j;
    sem_post(&mbxListSem);
}

int mailbox_init(connection_t *con, tims_msg_head* tcpMsg, tims_msg_head *replyMsg)
{
    int ret;
    tims_router_mbx_msg* mbxMsg = NULL;

    if (tcpMsg->msglen < sizeof(tims_router_mbx_msg))
    {
        tims_print("con[%02d]: %s: init mailbox -> message "
                   "length invalid, is: %u bytes, must be %u bytes\n",
                   con->index, inet_ntoa(con->addr.sin_addr),
                   tcpMsg->msglen, sizeof(tims_router_mbx_msg));
        return -EFAULT;
    }

    mbxMsg = tims_router_parse_mbx_msg(tcpMsg);

    ret = mailbox_create(mbxMsg->mbx, con->index);
    if (ret)
    {
        tims_print("con[%02d] %s error: Can't init MBX %x, "
                   "code = %d\n", con->index, inet_ntoa(con->addr.sin_addr),
                   mbxMsg->mbx, ret);
        if (replyMsg)
        {
            tims_fillhead(replyMsg, TIMS_MSG_ERROR, tcpMsg->src, tcpMsg->dest,
                          tcpMsg->priority, tcpMsg->seq_nr, 0, TIMS_HEADLEN);
        }
    }
    else
    {
        tims_print("con[%02d]: %s: init MBX %x\n", con->index,
                   inet_ntoa(con->addr.sin_addr), mbxMsg->mbx);

        if (replyMsg)
        {
            tims_fillhead(replyMsg, TIMS_MSG_OK, tcpMsg->src, tcpMsg->dest,
                          tcpMsg->priority, tcpMsg->seq_nr, 0, TIMS_HEADLEN);
        }

//TODO: Register mailbox @ next level TCP Router

    }

    if (replyMsg)
        sndTcpTimsMsg(con, replyMsg);

    return 0;
}

void mailbox_cleanup(connection_t *con, tims_msg_head* tcpMsg,
                     tims_msg_head *replyMsg)
{
    tims_router_mbx_msg* mbxMsg = NULL;

    if (tcpMsg->msglen < sizeof(tims_router_mbx_msg))
    {
        tims_print("con[%02d]: %s: delete mailbox -> message length invalid, "
                   "is: %u bytes, must be %u bytes\n", con->index,
                   inet_ntoa(con->addr.sin_addr), tcpMsg->msglen,
                   sizeof(tims_router_mbx_msg));
        return;
    }

    mbxMsg = tims_router_parse_mbx_msg(tcpMsg);

    mailbox_delete(mbxMsg->mbx);

    tims_print("con[%02d]: %s: delete MBX %x\n", con->index,
               inet_ntoa(con->addr.sin_addr), mbxMsg->mbx);

    if (replyMsg)
    {
        tims_fillhead(replyMsg, TIMS_MSG_OK, tcpMsg->src, tcpMsg->dest,
                      tcpMsg->priority, tcpMsg->seq_nr, 0, TIMS_HEADLEN);

        sndTcpTimsMsg(con, replyMsg);
    }

//TODO: delete mailbox @ next level TCP Router

}

void mailbox_purge(connection_t *con)
{
    tims_print("con[%02d]: %s: purge\n", con->index,
               inet_ntoa(conList[con->index].addr.sin_addr));

    mailbox_delete_all(con->index);

//TODO: delete all mailboxes @ next level TCP Router

}

//
// cleanup and signal_handler
//

void cleanup(void)
{
    int i;

    tims_print("Terminate\n");

    terminate = 1;

    // close connection sockets
    for (i = 0; i < MAX_CONNECTIONS; i++)
    {
        if (conList[i].socket != -1)    // socket opened
            connection_delete(&conList[i]);
    }

    // join watchdog thread
    if (init_flags & TIMS_ROUTER_WATCHDOG)
    {
        pthread_join(watchdogThread, NULL);
        tims_dbgdetail("watchdogThread joined\n");
        init_flags &= ~TIMS_ROUTER_WATCHDOG;
    }

    // join connection threads
    for (i = 0; i < MAX_CONNECTIONS; i++)
    {
        if (conList[i].index != -1)
        {
            pthread_join(conList[i].conThread, NULL);
            tims_dbgdetail("connection thead[%d] joined\n", conList[i].index);
        }
    }

    // destroy send semaphore
    for (i = 0; i < MAX_CONNECTIONS; i++)
    {
        if (sem_flags & (1 << i) )
        {
            sem_destroy(&conList[i].sendSem);
            tims_dbgdetail("sendSem[%i] destroyed\n",i);
            sem_flags &= ~(1 << i);
        }
    }

    if (init_flags & TIMS_ROUTER_SEM_LIST)
    {
        sem_destroy(&mbxListSem);
        tims_dbgdetail("mbxListSem destroyed \n");
        init_flags &= ~TIMS_ROUTER_SEM_LIST;
    }

    if (tcpServerSocket != -1 )
    {
        close(tcpServerSocket);
        tcpServerSocket = -1;
        tims_dbgdetail("tcpServerSocket closed\n");
    }
}

void signal_handler(int arg)
{
    cleanup();
}

//
// tasks
//

// the tcpConnection_task_proc handles the connection to one client
void tcpConnection_task_proc(void *arg)
{
    tims_msg_head*        tcpMsg;
    connection_t*         con = (connection_t*)arg;
    tims_msg_head         replyMsg;
    int                   forwardConIndex;
    int                   ret, idx;

    signal(SIGHUP,  signal_handler);
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, signal_handler);

    idx = con->index;

    tims_dbg("con[%02d]: %s: starting TCP/IP connection task\n",
             idx, inet_ntoa(con->addr.sin_addr));

    tcpMsg = malloc(maxMsgSize);
    if (!tcpMsg)
    {
        tims_print("con[%02d] %s error: Can't allocate memory for tcpTimsMsg\n",
                   idx, inet_ntoa(con->addr.sin_addr));
        connection_close(con);
        return;
    }

    while (!terminate)
    {
        if (con->socket < 0)    // invalid socket
            break;

        ret = recvTcpTimsMsg(con, tcpMsg); // waiting for new command
        if (ret < 0)
            break;

        if ( !tcpMsg->dest &&
             !tcpMsg->src )  // handle TiMS command (internal)
        {
            switch (tcpMsg->type)
            {
                case TIMS_MSG_OK:        // watchdog lifesign reply
                    con->watchdog = 0;
                    break;

                case TIMS_MSG_ROUTER_LOGIN:
                    break;

                case TIMS_MSG_ROUTER_MBX_INIT:
                    mailbox_init(con, tcpMsg, NULL);
                    break;

                case TIMS_MSG_ROUTER_MBX_DELETE:
                    mailbox_cleanup(con, tcpMsg, NULL);
                    break;

                case TIMS_MSG_ROUTER_MBX_INIT_WITH_REPLY:
                    mailbox_init(con, tcpMsg, &replyMsg);
                    break;

                case TIMS_MSG_ROUTER_MBX_DELETE_WITH_REPLY:
                    mailbox_cleanup(con, tcpMsg, &replyMsg);
                    break;

                case TIMS_MSG_ROUTER_MBX_PURGE:
                    mailbox_purge(con);
                    break;

                default:
                    tims_print("con[%02d]: %s: received unexpected TiMS "
                               "Message %x -> %x type %i msglen %i\n", idx,
                               inet_ntoa(con->addr.sin_addr), tcpMsg->src,
                               tcpMsg->dest, tcpMsg->type, tcpMsg->msglen);
            }
        }
        else // ( tcpMsg->dest || tcpMsg->src )
        {

            // forward tims message
            forwardConIndex = mailbox_get(tcpMsg->dest);

            if (forwardConIndex >= 0) // mbx is available
            {
                sndTcpTimsMsg(&conList[forwardConIndex], tcpMsg);
            }
            else // mbx is not available
            {
                if (tcpMsg->type > 0)
                {
                    tims_fillhead(&replyMsg, TIMS_MSG_NOT_AVAILABLE, tcpMsg->src,
                                  tcpMsg->dest, tcpMsg->priority, tcpMsg->seq_nr,
                                  0, TIMS_HEADLEN);
                    sndTcpTimsMsg(con, &replyMsg);
                }
            }
        }
    }

    tims_print("con[%02d]: %s: logout\n", idx, inet_ntoa(con->addr.sin_addr));
    mailbox_delete_all(con->index);

    if (con->socket != -1)
        connection_close(con);

    if (tcpMsg)
    {
        free(tcpMsg);
        tcpMsg = NULL;
        tims_dbgdetail("con[%02d]: %s: free tcp message buffer\n", con->index,
                       inet_ntoa(con->addr.sin_addr));
    }

    con->index = -1;
    return;
}

// The tcpServerTask handles new incomming connections and creates new
// tcpConnectionTasks
void tcpServer_task_proc(void *arg)
{
    connection_t newCon;
    connection_t *freeCon;
    int i;
    int ret;

    ret = listen(tcpServerSocket, 1);
    if (ret)
    {
        tims_print("ERROR: Can't listen to tcpServerSocket\n");
        return;
    }

    tims_dbg("server task started, waiting for connections ... \n");

    while (!terminate)
    {
        newCon.addrLen = sizeof(newCon.addr);
        newCon.socket  = accept(tcpServerSocket,
                                (struct sockaddr *)&newCon.addr,
                                &newCon.addrLen);
        if (newCon.socket < 0)
        {
            tims_print("error: Can't accept new connection\n");
            return;
        }

        freeCon = connection_getFree(&newCon);
        if (!freeCon)
        {
            tims_print("ERROR: Can't accept more than %i connections "
                       "(ip %s connection refused)\n",
                       MAX_CONNECTIONS, inet_ntoa(newCon.addr.sin_addr));
            connection_close(&newCon);
        }
        else
        {
            i = freeCon->index;
            tims_print("con[%02d]: %s: login\n", i,
                       inet_ntoa(conList[i].addr.sin_addr));

            // create connection thread
            ret = pthread_create(&conList[i].conThread, NULL,
                                 (void *)tcpConnection_task_proc,
                                 &conList[i]);
            if (ret)
            {
                tims_print("error: Can't create thread for TCP/IP connection\n");
                connection_close(&conList[i]);
            }
        }
    }

    tims_dbg("server task: exit\n");
}

// The watchdogTask sends a lifesign to every connected client.
// Clients that don't respond will be disconnected
void watchdog_task_proc(void *arg)
{
    tims_msg_head lifesignMsg;
    int i;

    signal(SIGHUP,  signal_handler);
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, signal_handler);

    tims_dbg("watchdog task: start\n");

    tims_fillhead(&lifesignMsg, TIMS_MSG_ROUTER_GET_STATUS, 0, 0, 0, 0, 0,
                  TIMS_HEADLEN);

    while (!terminate)
    {
        for (i = 0; i < MAX_CONNECTIONS; i++)
        {
            if (conList[i].socket >= 0)
            {
                conList[i].watchdog = 1;
                sndTcpTimsMsg(&conList[i], &lifesignMsg);
            }
        }
        tims_dbg("watchdog wait ...\n");
        sleep(5);

        for (i = 0; i < MAX_CONNECTIONS; i++)
        {
            if ((conList[i].socket >= 0) &&
                (conList[i].watchdog == 1))
            {
                connection_delete(&conList[i]);
                tims_print("con[%02d]: Connection closed by watchdog\n", i);
            }
        }
    }
    tims_dbg("watchdog task: exit\n");
}


//
// init and main function
//

int init()
{
    int ret = 0;
    int i;

    init_flags = 0;
    sem_flags  = 0;

    // raise priority, will be inherited by sub-threads
    if (sched_param.sched_priority > 0)
    {
        ret = sched_setscheduler(0, SCHED_FIFO, &sched_param);
        if (ret)
        {
            tims_print("[INIT] ERROR: can't raise pipeSendSem\n");
            return ret;
        }
    }

    // init all connections
    for (i = 0; i < MAX_CONNECTIONS; i++)
    {
        if (sem_init(&conList[i].sendSem, 0, 1) < 0)
        {
            printf(NAME " con[%i] error: Can't create sendSem\n", i);
            goto init_error;
        }
        conList[i].socket = -1;
        conList[i].index  = -1;
        sem_flags |= ( 1 << i);
        tims_dbgdetail("sendSem[%i] created\n",i);
    }

    // init mailbox list semaphore
    if (sem_init(&mbxListSem, 0, 1) < 0)
    {
        tims_print("error: Can't create mbxListSem\n");
        goto init_error;
    }
    init_flags |= TIMS_ROUTER_SEM_LIST;

    // init watchdog task
    if (pthread_create(&watchdogThread, NULL, (void *)watchdog_task_proc, NULL))
    {
        tims_print("error: Can't create watchdog thread\n");
        goto init_error;
    }
    init_flags |= TIMS_ROUTER_WATCHDOG;

    // open server socket
    tcpServerSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (tcpServerSocket < 0)
    {
        tims_print("ERROR: Can't create tcpServerSocket, code = %d\n",
                   tcpServerSocket);
        ret = tcpServerSocket;
        goto init_error;
    }

    ret = bind(tcpServerSocket, (struct sockaddr *)&tcpServerAddr,
               sizeof(tcpServerAddr));
    if (ret)
    {
        tims_print("ERROR: Can't bind tcpServerSocket ip %s\n",
                   inet_ntoa(tcpServerAddr.sin_addr));
        goto init_error;
    }

    return 0;

init_error:
    cleanup();
    return ret;
}

int main(int argc, char* argv[])
{
    char ip[16];
    int ret, opt, port;

    // read parameter
    strncpy(ip, "\"any\"", 16);
    tcpServerAddr.sin_addr.s_addr = INADDR_ANY;
    port = DEFAULT_PORT;
    maxMsgSize = DEFAULT_MAX * 1024;

    while ((opt = getopt(argc, argv, "i:p:m:h:l:")) != -1)
    {
        switch (opt)
        {
            case 'l':
                sscanf(optarg, "%i", &loglevel);
                printf(NAME": opt -l loglevel: %i\n", loglevel);
                break;

            case 'i':
                strncpy(ip, optarg, 16);
                tcpServerAddr.sin_addr.s_addr = inet_addr(ip);
                tims_dbgdetail("opt -i ip: %s\n", ip);
                break;

            case 'p':
                sscanf(optarg, "%i", &port);
                tims_dbgdetail("opt -p port: %i\n", port);
                break;

            case 'm':
                sscanf(optarg, "%i", &maxMsgSize);
                tims_dbgdetail("opt -m maxMessageSize: %i kByte\n", maxMsgSize);
                maxMsgSize *= 1024;
                break;

            case 'P':
                sscanf(optarg, "%i", &sched_param.sched_priority);
                tims_dbgdetail("opt -P priority: %i\n", sched_param.sched_priority);
                break;

            case 'h':
            default:
                printf( "\n"
                "The tims router TCP is a Linux server that transferes "
                "tims messages between different RACK subsystems\n"
                "\n"
                "-i IP address of the tims router to listen on\n"
                "   (default: all interfaces)\n"
                "-p port of the tims router TCP\n"
                "   (default: 2000)\n"
                "-m maxMessageSize in kBytes\n"
                "   (default: 256 kByte)\n"
                "-l log level\n"
                "   debug log level, 0 = silent, 1 = some important messages, 2 = verbose\n"
                "-P RT-priority of the TimsClient\n"
                "   (default: 1)\n");
                return -1;
        }
    }

    // parse TCP Tims Message Router ip address
    if (tcpServerAddr.sin_addr.s_addr == INADDR_NONE)
    {
        tims_print("error: Ip %s not valid\n", ip);
        return -1;
    }

    tcpServerAddr.sin_port = htons((unsigned short)port);
    tcpServerAddr.sin_family = AF_INET;
    bzero(&(tcpServerAddr.sin_zero), 8);


    signal(SIGHUP,  signal_handler);
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, signal_handler);

    ret = init();
    if (ret)
        return ret;

    tims_print("Tims Router TCP (IP %s port %i)\n", ip, port);

    tcpServer_task_proc(&tcpServerSocket);

    cleanup();
    return 0;
}
