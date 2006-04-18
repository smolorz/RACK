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

#define NAME "TcpTimsMsgRouter"

#define DEBUG
//#define DEBUG_DETAIL

#include <main/tims/tims.h>
#include <main/tims/msgtypes/tims_msg_types.h>
#include <main/tims/router/tims_msg_router.h>

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


#define                 DEFAULT_PORT   2000
#define                 DEFAULT_MAX    256

unsigned int            maxMsgSize;
unsigned int            init_flags = 0;
unsigned int            sem_flags = 0;
int                     terminate = 0;
int                     tcpServerSocket = -1;
struct sockaddr_in      tcpServerAddr;
pthread_t               watchdogThread;
int                     loglevel = 0;


typedef struct {
    int               	index;
    int                 socket;
    struct sockaddr_in  addr;
    socklen_t           addrLen;
    pthread_t           conThread;
    int                 watchdog;
    sem_t               sendSem;
} CONNECTION;

// if MAX_CONNECTIONS > 32 -> look @ sem_flags
#define MAX_CONNECTIONS 8
CONNECTION conList[MAX_CONNECTIONS];

typedef struct {
    int mbx;
    int conIndex;
} MBX_DATA;

#define MAX_MBX 256
MBX_DATA mbxList[MAX_MBX];
int      mbxNum = 0;
sem_t    mbxListSem;

//
// mailbox functions
//

// returns the connection index of the mbx, TIMS_MSG_ERROR if the mbx is unknown
int getMbx(int mbx)
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
    return TIMS_MSG_ERROR;
}

// adds the mbx to the mbx list, returns TIMS_MSG_ERROR if the mbx is allready known
int initMbx(int mbx, int conIndex)
{
    int i, j;
    sem_wait(&mbxListSem);

    if (mbxNum >= MAX_MBX)
    {
        sem_post(&mbxListSem);
        return TIMS_MSG_ERROR;
    }

    // Find position i to insert the new mbx
    for (i = 0; i < mbxNum; i++)
    {
        if (mbxList[i].mbx > mbx)
        {
            break;
        }
        else if(mbxList[i].mbx == mbx) // mbx is allready contained in the list
        {
            sem_post(&mbxListSem);
            return TIMS_MSG_ERROR;
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
    return TIMS_MSG_OK;
}

// remove the mbx from the mbx list
void deleteMbx(int mbx)
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
void deleteAllMbx(int conIndex)
{
    int i, j = 0;
    sem_wait(&mbxListSem);
    for(i = 0; i < mbxNum; i++)
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


//
// TCP send / receive functions
//

int sndTcpTimsMsg(CONNECTION *con, timsMsgHead* sndMsg)
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
    	close(con->socket);
    	con->socket = -1;

    	sem_post(&con->sendSem);
    	tims_print("con[%02d]: %15s: %8x --(%4d)--> %8x, (%u bytes), "
    	           "send ERROR, code = %d\n", idx, inet_ntoa(con->addr.sin_addr),
    	           sndMsg->src, sndMsg->type, sndMsg->dest, sndMsg->msglen, errno);

        return -1;
    }
    sem_post(&con->sendSem);
    return 0;
}

int recvTcpTimsMsg(CONNECTION *con, timsMsgHead* recvMsg)
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
            close(con->socket);
            con->socket = -1;
            tims_print("con[%02d]: %15s: recv head ERROR, code = %d \n",
                       idx, inet_ntoa(con->addr.sin_addr), ret);
            return -1;
        }

        if (ret == 0)
        {
            close(con->socket);
            con->socket = -1;
            tims_print("con[%02d]: %15s: recv head ERROR, socket closed\n",
                       idx, inet_ntoa(con->addr.sin_addr));
            return -1;
        }

        len += ret;

        if (terminate)
        {
            return -EINTR;
        }
    }
    tims_parse_head_byteorder(recvMsg);

    if (recvMsg->msglen < TIMS_HEADLEN)
    {
        close(con->socket);
        con->socket = -1;
        tims_print("con[%02d]: %15s: recv ERROR, invalid message length: "
                   "%u is smaller than TIMS_HEADLEN\n", idx,
                   inet_ntoa(con->addr.sin_addr), recvMsg->msglen);
        return -1;
    }
    else if (recvMsg->msglen > maxMsgSize)
    {
        close(con->socket);
        con->socket = -1;
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
            if (ret < 0)
            {
                close(con->socket);
                con->socket = -1;
                tims_print("con[%02d]: %15s: %8x --(%4d)--> %8x, recv body "
                           "ERROR, code = %d \n", idx,
                           inet_ntoa(con->addr.sin_addr), recvMsg->src,
                           recvMsg->type, recvMsg->dest, ret);
                return -1;
            }

            if (ret == 0)
            {
                close(con->socket);
                con->socket = -1;
                tims_print("con[%02d]: %15s: %8x --(%4d)--> %8x, recv body "
                           "ERROR, socket closed\n", idx,
                           inet_ntoa(con->addr.sin_addr), recvMsg->src,
                           recvMsg->type, recvMsg->dest);
                return -1;
            }
            len += ret;

            if (terminate)
            {
                return -EINTR;
            }
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
// cleanup and signal_handler
//

void cleanup(void)
{
  int i;

  tims_print("Terminate\n");

  terminate = 1;

  // close connections
  for(i = 0; i < MAX_CONNECTIONS; i++) {
    if (conList[i].socket != -1) {
      deleteAllMbx(i);
      shutdown(conList[i].socket, SHUT_RDWR);
      close(conList[i].socket);
      conList[i].socket = -1;
      tims_dbgdetail("close connection[%i]\n",i);
    }
  }

  // join threads
  for(i = 0; i < MAX_CONNECTIONS; i++) {
    if (conList[i].index != -1) {
      pthread_join(conList[i].conThread, NULL);
      tims_dbgdetail("join watchdogThread\n");
    }
  }

  // destroy semaphore
  for(i = 0; i < MAX_CONNECTIONS; i++) {
    if (sem_flags & (1 << i) ) {
      sem_destroy(&conList[i].sendSem);
      tims_dbgdetail("destroy sendSem[%i]\n",i);
      sem_flags &= ~(1 << i);
    }
  }

  if (init_flags & TIMS_ROUTER_WATCHDOG) {
    pthread_join(watchdogThread, NULL);
    tims_dbgdetail("join watchdogThread\n");
    init_flags &= ~TIMS_ROUTER_WATCHDOG;
  }

  if (init_flags & TIMS_ROUTER_SEM_LIST) {
    sem_destroy(&mbxListSem);
    tims_dbgdetail("destroy mbxListSem\n");
    init_flags &= ~TIMS_ROUTER_SEM_LIST;
  }

  if (tcpServerSocket != -1 ){
    close(tcpServerSocket);
    tcpServerSocket = -1;
    tims_dbgdetail("close tcpServerSocket\n");
  }
}

void signal_handler(int arg)
{
    cleanup();
}

//
// tasks
//

// the tcpConnectionTask handles the connection to one client
void tcpConnectionTask(void *arg)
{
    timsMsgHead           replyMsg;
    timsMsgHead*          tcpMsg;
    CONNECTION*           con = (CONNECTION*)arg;
    int                   forwardConIndex;
    int                   ret, idx;
    timsMsgRouter_MbxMsg* mbxMsg;

    signal(SIGHUP,  signal_handler);
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, signal_handler);

    idx = con->index;

    tims_dbg("con[%02d]: %s: starting TCP/IP connection task\n",
             idx,inet_ntoa(con->addr.sin_addr));

    tcpMsg = malloc(maxMsgSize);
    if (!tcpMsg)
    {
        tims_print("con[%02d] %s error: Can't allocate memory for tcpTimsMsg\n",
                   idx, inet_ntoa(con->addr.sin_addr));
        close(con->socket);
        con->socket = -1;
        return;
    }

    while (!terminate)
    {
        if (con->socket < 0)
        {
            break;
        }

        ret = recvTcpTimsMsg(con, tcpMsg);
        if (ret < 0)
        {
            break;
        }

        if ( !tcpMsg->dest && !tcpMsg->src )
        {
            // handle internal tims message router communication
            switch (tcpMsg->type)
            {
                case TIMS_MSG_OK:
                    con->watchdog = 0;
                    break;

                case TIMS_MSG_ROUTER_LOGIN:
                    break;

                case TIMS_MSG_ROUTER_MBX_INIT:

                    if (tcpMsg->msglen < sizeof(timsMsgRouter_MbxMsg))
                    {
                        tims_print("con[%02d]: %s: init MBX %x -> message "
                                   "length invalid, is: %u bytes, "
                                   "must be %u bytes\n", idx,
                                   inet_ntoa(con->addr.sin_addr), mbxMsg->mbx,
                                   tcpMsg->msglen, sizeof(timsMsgRouter_MbxMsg));
                        break;
                    }

                    mbxMsg = timsMsgRouter_parse_mbxMsg(tcpMsg);

                    if (initMbx(mbxMsg->mbx, con->index) == TIMS_MSG_OK)
                    {
                        tims_print("con[%02d]: %s: init MBX %x\n", idx,
                                   inet_ntoa(con->addr.sin_addr), mbxMsg->mbx);

//TODO: Send Message to higher level TCP Router

                    }
                    else
                    {
                        tims_print("con[%02d] %s error: Can't init MBX %x\n", idx,
                                   inet_ntoa(con->addr.sin_addr), mbxMsg->mbx);
                    }
                    break;

                case TIMS_MSG_ROUTER_MBX_DELETE:

                    if (tcpMsg->msglen < sizeof(timsMsgRouter_MbxMsg))
                    {
                        tims_print("con[%02d]: %s: init MBX %x -> message "
                                   "length invalid, is: %u bytes, "
                                   "must be %u bytes\n", idx,
                                   inet_ntoa(con->addr.sin_addr), mbxMsg->mbx,
                                   tcpMsg->msglen, sizeof(timsMsgRouter_MbxMsg));
                        break;
                    }

                    mbxMsg = timsMsgRouter_parse_mbxMsg(tcpMsg);
                    tims_print("con[%02d]: %s: delete MBX %x\n", idx,
                               inet_ntoa(con->addr.sin_addr), mbxMsg->mbx);
                    deleteMbx(mbxMsg->mbx);

//TODO: Send Message to higher level TCP Router

                    break;

                case TIMS_MSG_ROUTER_MBX_INIT_WITH_REPLY:

                    if (tcpMsg->msglen < sizeof(timsMsgRouter_MbxMsg))
                    {
                        tims_print("con[%02d]: %s: init MBX %x -> "
                                   "message length invalid, "
                                   "is: %u bytes, must be %u bytes\n", idx,
                                   inet_ntoa(con->addr.sin_addr), mbxMsg->mbx,
                                   tcpMsg->msglen, sizeof(timsMsgRouter_MbxMsg));
                        break;
                    }

                    mbxMsg = timsMsgRouter_parse_mbxMsg(tcpMsg);

                    if (initMbx(mbxMsg->mbx, con->index) == TIMS_MSG_OK)
                    {
                        tims_print("con[%02d]: %s: init MBX %x\n", idx,
                                   inet_ntoa(con->addr.sin_addr), mbxMsg->mbx);

                        tims_fillhead(&replyMsg, TIMS_MSG_OK, tcpMsg->src,
                                      tcpMsg->dest, tcpMsg->priority,
                                      tcpMsg->seq_nr, 0, TIMS_HEADLEN);
                    }
                    else
                    {
                        tims_print("con[%02d] %s: error: can't init MBX %x\n",
                                   idx, inet_ntoa(con->addr.sin_addr),
                                   mbxMsg->mbx);

                        tims_fillhead(&replyMsg, TIMS_MSG_ERROR, tcpMsg->src,
                                      tcpMsg->dest, tcpMsg->priority,
                                      tcpMsg->seq_nr, 0, TIMS_HEADLEN);
                    }

//TODO: Send Message to higher level TCP Router

                    sndTcpTimsMsg(con, &replyMsg);
                    break;

                case TIMS_MSG_ROUTER_MBX_DELETE_WITH_REPLY:

                    if (tcpMsg->msglen < sizeof(timsMsgRouter_MbxMsg))
                    {
                        tims_print("con[%02d]: %s: init MBX %x -> "
                                   "message length invalid, "
                                   "is: %u bytes, must be %u bytes\n", idx,
                                   inet_ntoa(con->addr.sin_addr), mbxMsg->mbx,
                                   tcpMsg->msglen, sizeof(timsMsgRouter_MbxMsg));
                        break;
                    }

                    mbxMsg = timsMsgRouter_parse_mbxMsg(tcpMsg);

                    tims_print("con[%02d]: %s: delete MBX %x\n", idx,
                               inet_ntoa(con->addr.sin_addr), mbxMsg->mbx);

                    deleteMbx(mbxMsg->mbx);

                    tims_fillhead(&replyMsg, TIMS_MSG_OK, tcpMsg->src,
                                  tcpMsg->dest, tcpMsg->priority, tcpMsg->seq_nr,
                                  0, TIMS_HEADLEN);

                    sndTcpTimsMsg(con, &replyMsg);
                    break;

                case TIMS_MSG_ROUTER_MBX_PURGE:
                    tims_print("con[%02d]: %s: purge\n", idx,
                               inet_ntoa(conList[idx].addr.sin_addr));
                    deleteAllMbx(con->index);

//TODO: Send Message to higher level TCP Router

                    break;

                default:
                    tims_print("con[%02d]: %s: received unexpected Tims "
                               "Message %x -> %x type %i msglen %i\n", idx,
                               inet_ntoa(con->addr.sin_addr), tcpMsg->src,
                               tcpMsg->dest, tcpMsg->type, tcpMsg->msglen);
            }
        }
        else // ( tcpMsg->dest || tcpMsg->src )
        {
            // forward tims message
            forwardConIndex = getMbx(tcpMsg->dest);

            if (forwardConIndex != TIMS_MSG_ERROR) // mbx is available
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
    } // while (!terminate)

    tims_print("con[%02d]: %s: logout\n", idx, inet_ntoa(con->addr.sin_addr));
    deleteAllMbx(con->index);

    if (con->socket != -1)
    {
        close(con->socket);
        con->socket = -1;
    }

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

// The tcpServerTask handles new incomming connections and creates new tcpConnectionTasks
void tcpServerTask(void)
{
    CONNECTION newCon;
    int index;
    int i;
    int ret;

    tcpServerSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (tcpServerSocket < 0)
    {
        tims_print("ERROR: Can't create tcpServerSocket (%i)\n",
                   tcpServerSocket);
        return;
    }

    ret = bind(tcpServerSocket, (struct sockaddr *)&tcpServerAddr,
               sizeof(tcpServerAddr));
    if (ret)
    {
        tims_print("ERROR: Can't bind tcpServerSocket ip %s\n",
                   inet_ntoa(tcpServerAddr.sin_addr));
        return;
    }

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

        index = -1;
        for(i = 0; i < MAX_CONNECTIONS; i++)
        {
            if (conList[i].socket == -1)
            {
                memcpy(&conList[i].addr, &newCon.addr, sizeof(newCon.addr));
                conList[i].addrLen  = newCon.addrLen;
                conList[i].socket   = newCon.socket;
                conList[i].index    = i;
                conList[i].watchdog = 0;
                index = i;
                break;
            }
        }

        if (index != -1)
        {
            tims_print("con[%02d]: %s: login\n", index,
                       inet_ntoa(conList[i].addr.sin_addr));

            ret = pthread_create(&conList[index].conThread, NULL,
                                 (void *)tcpConnectionTask, &conList[index]);
            if(ret)
            {
                tims_print("error: Can't create thread for TCP/IP connection\n");
                close(conList[index].socket);
                conList[index].socket = -1;
            }
        }
        else
        {
            tims_print("ERROR: Can't accept more than %i connections "
                       "(ip %s connection refused)\n",
                       MAX_CONNECTIONS, inet_ntoa(newCon.addr.sin_addr));
            close(newCon.socket);
        }

    } // while(!terminate)

    tims_dbg("exit server task\n");
}

// the watchdogTask sends a lifesign to every connected client,
// clients that don't respond will be disconnected
void watchdogTask(void *arg)
{
  timsMsgHead lifesignMsg;
  int i;

  signal(SIGHUP,  signal_handler);
  signal(SIGINT,  signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGPIPE, signal_handler);

  tims_dbg("watchdog task started\n");

  tims_fillhead(&lifesignMsg, TIMS_MSG_ROUTER_GET_STATUS, 0, 0, 0, 0, 0, TIMS_HEADLEN);

  while (!terminate) {

    for (i = 0; i < MAX_CONNECTIONS; i++) {
      if (conList[i].socket >= 0) {
          conList[i].watchdog = 1;
          sndTcpTimsMsg(&conList[i], &lifesignMsg);
      }
    }

    sleep(5);

    for(i = 0; i < MAX_CONNECTIONS; i++) {
      if ((conList[i].socket >= 0) &&
          (conList[i].watchdog == 1)) {
        deleteAllMbx(i);
        shutdown(conList[i].socket, SHUT_RDWR);
        close(conList[i].socket);
        conList[i].socket = -1;
        conList[i].index  = -1;
        tims_print("con[%02d]: Connection closed by watchdog\n", i);
      }
    }
  }

  tims_dbg("exit watchdog task\n");
}


//
// init and main function
//

int init()
{
  int  ret;
  int i;

  init_flags = 0;
  sem_flags  = 0;

  for (i = 0; i < MAX_CONNECTIONS; i++) {
    if (sem_init(&conList[i].sendSem, 0, 1) < 0) {
      printf(NAME " con[%i] error: Can't create sendSem\n", i);
      goto init_error;
    }
    conList[i].socket = -1;
    conList[i].index  = -1;
    sem_flags |= ( 1 << i);
  }

  if (sem_init(&mbxListSem, 0, 1) < 0) {
    tims_print("error: Can't create mbxListSem\n");
    goto init_error;
  }
  init_flags |= TIMS_ROUTER_SEM_LIST;

  if (pthread_create(&watchdogThread, NULL, (void *)watchdogTask, NULL)) {
    tims_print("error: Can't create watchdog thread\n");
    goto init_error;
  }
  init_flags |= TIMS_ROUTER_WATCHDOG;

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
    port       = DEFAULT_PORT;
    maxMsgSize = DEFAULT_MAX * 1024;

    while ((opt = getopt(argc, argv, "i:p:m:h:l:")) != -1)
    {
        switch(opt)
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

            case 'h':
            default:
                printf( "\n"
                "The tcp_tims_msg_router is a Linux server that transferes "
                "tims messages between different RACK subsystems\n"
                "\n"
                "-i IP address of the TCP Tims message router\n"
                "   (default 127.0.0.1 localhost)\n"
                "-p port of the Tims message router\n"
                "   (default 2000)\n"
                "-m maxMessageSize in kBytes\n"
                "   (default 256 kByte)\n"
                "-l log level\n"
                "   debug log level, 0 = silent, 1 = some important messages, 2 = verbose\n");
                return -1;
        }
    }

    // parse TCP Tims Message Router ip address and port
    if (tcpServerAddr.sin_addr.s_addr == INADDR_NONE)
    {
        tims_print("error: Ip %s not valid\n", ip);
        return -1;
    }

    if ((port < 0x400) | ( port > 0xffff))
    {
        tims_print("error: Port %i not valid\n", port);
        return -1;
    }

    tcpServerAddr.sin_port   = htons((unsigned short)port);
    tcpServerAddr.sin_family = AF_INET;
    bzero(&(tcpServerAddr.sin_zero), 8);


    signal(SIGHUP,  signal_handler);
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, signal_handler);

    ret = init();
    if (ret)
    {
        return ret;
    }

    tims_print("TCP Tims Message Router IP %s port %i\n", ip, port);

    tcpServerTask();

    cleanup();
    return 0;
}
