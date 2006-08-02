/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
#include <signal.h>
#include <unistd.h>   // pause()
#include <stdio.h>    // printf()
#include <sys/mman.h> // mlockall()
#include <stdlib.h>   // malloc / free
#include <math.h>     // M_PI

#include <main/rack_mailbox.h>
#include <main/rack_task.h>
#include <main/rack_debug.h>

#define GDOS_MAILBOX_SLOTS           30
#define GDOS_TASK_PRIO               1

// init flags
#define RACKLOG_MBX_BUFFER           0x0001
#define RACKLOG_MBX_CREATED          0x0002
#define RACKLOG_TASK_CREATED         0x0004
#define RACKLOG_TASK_STARTED         0x0008

const ssize_t mbxBufferSize = (TIMS_HEADLEN + GDOS_MAX_MSG_SIZE) *
                              GDOS_MAILBOX_SLOTS;

char recv_buffer[GDOS_MAX_MSG_SIZE];

typedef struct rack_logger {
  RackMailbox     mbx;
  unsigned char*  p_mbxBuffer;
  RackTask        log_task;
  unsigned int    init_state;
  int             terminate;
  RTIME           disabledTime_ns;
  char            outputString[GDOS_MAX_MSG_SIZE * 2];
} rack_log_t;

rack_log_t *rlt = NULL;

int printMessage(message_info *msgInfo)
{
    char* inputString;
    char* outptr = rlt->outputString;
    char* param;
    char  formatString[16];
    int   formatStringLen;
    int   longLong;
    int   i;

    int           intParam      = 0;
    unsigned int  uintParam     = 0;
    double        doubleParam   = 0;
    long long     longLongParam = 0;

    inputString = (char *)msgInfo->p_data;

    param = inputString + strlen(inputString) + 1;
    formatString[0] = '%';

    while (*inputString != 0) {

      if (*inputString == '%') {
        i = 0;
        longLong = 0;

        do {

          inputString++;
          i++;
          formatString[i] = *(inputString);

          if (formatString[i] == 'L') {
            longLong = 1;
          }

        } while(((*inputString >= '0') & (*inputString <= '9')) |
                 (*inputString == '.') |
                 (*inputString == 'L'));

        formatString[++i] = 0;

        switch (*inputString) {
            case 'd':
            case 'i':
            case 'u':
            case 'x':
            case 'X':
                if (longLong == 0) {
                  intParam = *((int*)param);
                  param   += sizeof(int);
                  formatStringLen = sprintf(outptr, formatString, intParam);
                } else {
                  longLongParam = *((long long*)param);
                  param   += sizeof(long long);
                  formatStringLen = sprintf(outptr, formatString, longLongParam);
                }

                outptr          += formatStringLen;
                break;

            case 'f': // float value
                doubleParam = *((double*)param);
                param   += sizeof(double);

                formatStringLen = sprintf(outptr, formatString, doubleParam);

                outptr          += formatStringLen;
                break;

            case 'n':  // name (mailbox address)
                formatString[i - 1] = 'X';

                uintParam = *((unsigned int*)param);
                param   += sizeof(unsigned int);

                formatStringLen = sprintf(outptr, formatString, intParam);

                outptr          += formatStringLen;
                break;

            case 'a':
                formatString[i - 1] = 'f';

                doubleParam = *((double*)param) * 180.0 / M_PI;
                param   += sizeof(double);

                formatStringLen = sprintf(outptr, formatString, doubleParam);

                outptr          += formatStringLen;
                break;

            case 'b':  // vorlaeufig
                formatString[i - 1] = 'X';

                intParam = *((int*)param);
                param   += sizeof(int);

                formatStringLen = sprintf(outptr, formatString, intParam);

                outptr          += formatStringLen;
                break;

            case 'p':  // pointer (unsigned long)
                formatString[i - 1] = 'p';

                uintParam = *((unsigned int*)param);
                param   += sizeof(unsigned int);

                formatStringLen = sprintf(outptr, formatString, intParam);

                outptr          += formatStringLen;
                break;

            case '%':
                *(outptr++) = '%';
                *(outptr++) = '%';
                break;

          } // switch

            inputString++;

        } else {

          *(outptr++) = *inputString++;

        }
    }

    if (*(outptr - 1) != '\n') {
      *(outptr++) = '\n';
    }

    *outptr = 0;

    switch(msgInfo->type) {
        case (GDOS_MSG_PRINT):
            printf("        %6X: %s", msgInfo->src, rlt->outputString);
            break;

        case (GDOS_MSG_ERROR):
            printf("  error %6X: %s", msgInfo->src, rlt->outputString);
            break;

        case (GDOS_MSG_WARNING):
            printf("warning %6X: %s", msgInfo->src, rlt->outputString);
            break;

        case (GDOS_MSG_DBG_INFO):
            printf("   info %6X: %s", msgInfo->src, rlt->outputString);
            break;

        case (GDOS_MSG_DBG_DETAIL):
            printf(" detail %6X: %s", msgInfo->src, rlt->outputString);
            break;

        default:
            printf("%8X unknown message type %d: ", msgInfo->src, msgInfo->type);
    }
    RackTask::setMode(0, T_PRIMARY, NULL);

    return(0);
}

void log_function(void *arg)
{
    int ret;
    message_info msgInfo;

    while (!rlt->terminate)
    {
        RackTask::setMode(0, T_PRIMARY, NULL);
        ret = rlt->mbx.peek(&msgInfo);
        //ret = rlt->mbx.recv_data_msg_timed(TIMS_INFINITE, recv_buffer, GDOS_MAX_MSG_SIZE, &info);
        if (!ret)
        {
            switch(msgInfo.type)
            {
                case GDOS_MSG_PRINT:
                case GDOS_MSG_ERROR:
                case GDOS_MSG_WARNING:
                case GDOS_MSG_DBG_INFO:
                case GDOS_MSG_DBG_DETAIL:
                    printMessage(&msgInfo);
                    break;

                default :
                {
                    printf("RACKLOG: received undefined message from %x, type %d \n",
                            msgInfo.src, msgInfo.type);
                    RackTask::setMode(0, T_PRIMARY, NULL);
                    break;
                }
            } // switch()

            RackTask::setMode(0, T_PRIMARY, NULL);
            rlt->mbx.peekEnd();

        }
        else
        {
            if (!rlt->terminate)
            {
                printf("RACKLOG: can't receive message on mbx, code = %d\n", ret);
                RackTask::setMode(0, T_PRIMARY, NULL);
                RackTask::sleep(rlt->disabledTime_ns);
            }
            else
            {
                break;
            }
        }
    }
    printf("RACKLOG: exit task\n");
    return;
}

void cleanup(void)
{
  rlt->terminate = 1;

  if (rlt->init_state & RACKLOG_MBX_CREATED) {
    printf("RACKLOG: delete mailbox \n");
    rlt->mbx.remove();
    rlt->init_state &= ~RACKLOG_MBX_CREATED;
  }

  if (rlt->init_state & RACKLOG_TASK_CREATED) {
    printf("RACKLOG: delete task \n");
    rlt->log_task.destroy();
    rlt->init_state &= ~RACKLOG_TASK_CREATED;
  }

  if (rlt->init_state & RACKLOG_MBX_BUFFER) {
    printf("RACKLOG: free mailbox buffer\n");
    free(rlt->p_mbxBuffer);
    rlt->init_state &= ~RACKLOG_MBX_BUFFER;
  }

}

int init(void)
{
  int ret = 0;
  rlt->terminate = 0;
  rlt->init_state = 0;
  rlt->disabledTime_ns = 500000000llu; // 500ms

  printf("RACKLOG: allocate mailbox buffer\n");
  rlt->p_mbxBuffer = (unsigned char *)malloc(mbxBufferSize);
  if (!rlt->p_mbxBuffer) {
    printf("RACKLOG: ERROR can't allocate buffer for mailbox\n");
    goto exit_error;
  }
  rlt->init_state |= RACKLOG_MBX_BUFFER;
  memset(rlt->p_mbxBuffer, 0, mbxBufferSize);


  printf("RACKLOG: create mailbox \n");
  ret = rlt->mbx.create(GDOS_NO, GDOS_MAILBOX_SLOTS, GDOS_MAX_MSG_SIZE,
                        rlt->p_mbxBuffer, mbxBufferSize, GDOS_TASK_PRIO);
  if (ret) {
    printf("RACKLOG: ERROR can't create mailbox, code = %d\n", ret);
    goto exit_error;
  }
  rlt->init_state |= RACKLOG_MBX_CREATED;

  printf("RACKLOG: create task \n");
  ret = rlt->log_task.create(0, GDOS_TASK_PRIO, 0);
  if (ret) {
    printf("RACKLOG: can't create task, code = %d\n", ret);
    goto exit_error;
  }
  rlt->init_state |= RACKLOG_TASK_CREATED;

  printf("RACKLOG: start task \n");
  ret = rlt->log_task.start(&log_function, 0);
  if (ret) {
    printf("RACKLOG: can't start log task, code = %d\n", ret);
    goto exit_error;
  }
  rlt->init_state |= RACKLOG_TASK_STARTED;

  return 0;

exit_error:
  cleanup();
  return ret;
}

void signal_handler(int sig)
{
  printf("RACKLOG: signal_handler \n");
  cleanup();
  return;
}

int  main(int argc, char *argv[])
{
  /* Verhindert Speicher-Swapping für dieses Programm */
  mlockall(MCL_CURRENT | MCL_FUTURE);

  rlt = (rack_log_t *)malloc(sizeof(rack_log_t));
  if (!rlt) {
    return -ENOMEM;
  }

  int ret = 0;

  printf("RACKLOG: start \n");

  signal(SIGTERM, signal_handler);
  signal(SIGINT,  signal_handler);

  ret = init();
  if (ret) {
    return ret;
  }

  pause();

  free(rlt);

  printf("RACKLOG: exit \n");
  return 0;
}

