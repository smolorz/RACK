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
 *      Oliver Wulf <oliver.wulf@web.de>
 *
 */
#ifndef __RACK_DEBUG_H__
#define __RACK_DEBUG_H__

#include <stdarg.h> // va_start ...
#include <stdio.h>  // printf ...

#include <main/rack_mailbox.h>
#include <main/rack_name.h>

#define GDOS_MAX_MSG_SIZE   256     // size for message string and variables

// message types
#define GDOS_MSG_PRINT      -124    // printout (highest level)
#define GDOS_MSG_ERROR      -125    // error
#define GDOS_MSG_WARNING    -126    // warning
#define GDOS_MSG_DBG_INFO   -127    // debug information
#define GDOS_MSG_DBG_DETAIL -128    // detailed debug information

#define GDOS_MSG_DEBUG_BEGIN    GDOS_MSG_PRINT
#define GDOS_MSG_DEBUG_DEFAULT  GDOS_MSG_WARNING

//
// debug functions
//

#define printf_level(level, fmt, ... )                                  \
                    char *tfmt = (char *)fmt;                           \
                    do                                                  \
                    {                                                   \
                        char *pch;                                      \
                        do                                              \
                        {                                               \
                            pch = strstr(tfmt, "%n");                   \
                            if (pch)                                    \
                                strncpy(pch, "%x", 2);                  \
                        }                                               \
                        while (pch);                                    \
                        printf(tfmt, ##__VA_ARGS__);                    \
                    } while(0)

#if defined (__XENO__) || defined (__KERNEL__)

#include <main/rack_task.h>

// non realtime / realtime context
static inline int in_rt_context(void)
{
    int ret = RackTask::sleep(0);
    if (ret == -EPERM)
        return 0;
    return 1;
}

#define rack_print(level, fmt, ...)                                   \
            do                                                        \
            {                                                         \
                if (in_rt_context())                                  \
                {                                                     \
                    if (gdos)                                         \
                    {                                                 \
                        gdos->print(level, fmt, ##__VA_ARGS__);       \
                    }                                                 \
                    else                                              \
                    {                                                 \
                        /* do nothing */                              \
                    }                                                 \
                }                                                     \
                else                                                  \
                {                                                     \
                    if (gdos)                                         \
                    {                                                 \
                        gdos->print(level, fmt, ##__VA_ARGS__);       \
                    }                                                 \
                    else                                              \
                    {                                                 \
                        printf_level(level, fmt, ##__VA_ARGS__);      \
                    }                                                 \
                }                                                     \
            }                                                         \
            while(0)

#else // !__XENO__ && !__KERNEL__

#define rack_print(level, fmt, ...)                                   \
            do                                                        \
            {                                                         \
                if (gdos)                                             \
                {                                                     \
                    gdos->print(level, fmt, ##__VA_ARGS__);           \
                }                                                     \
                else                                                  \
                {                                                     \
                    printf_level(level, fmt, ##__VA_ARGS__);          \
                }                                                     \
            }                                                         \
            while(0)
#endif // __XENO__ || __KERNEL__

#define GDOS_PRINT(fmt, ...)       rack_print(GDOS_MSG_PRINT, fmt, ##__VA_ARGS__)
#define GDOS_ERROR(fmt, ...)       rack_print(GDOS_MSG_ERROR, fmt, ##__VA_ARGS__)
#define GDOS_WARNING(fmt, ...)     rack_print(GDOS_MSG_WARNING, fmt, ##__VA_ARGS__)
#define GDOS_DBG_INFO(fmt, ...)    rack_print(GDOS_MSG_DBG_INFO, fmt, ##__VA_ARGS__)
#define GDOS_DBG_DETAIL(fmt, ...)  rack_print(GDOS_MSG_DBG_DETAIL, fmt, ##__VA_ARGS__)


class GdosMailbox
{
    private:
        RackMailbox*    sendMbx;
        char            gdosLevel;

    public:

        GdosMailbox(RackMailbox *p_mbx, int gdos_level )
        {
            this->sendMbx   = p_mbx;
            this->gdosLevel = gdos_level;
        }

        ~GdosMailbox()
        {
        }

        void setMailbox(RackMailbox *p_newMbx)
        {
            if (p_newMbx)
                this->sendMbx = p_newMbx;
        }

        void setGdosLevel(int newLevel)
        {
            if (newLevel > GDOS_MSG_PRINT &&
                newLevel < GDOS_MSG_DBG_DETAIL)
            {
                return;
            }
            gdosLevel = newLevel;
        }

        void print(int level, char* format, ...)
        {
            tims_msg_head   head;
            char            buffer[GDOS_MAX_MSG_SIZE];
            int             percent = 0;
            char*           src;
            char*           dst;
            va_list         args;
            int             datasize = 0;
            int             valuesize = 0;

            if (!sendMbx)
            {
                return;
            }

            if (level < gdosLevel ||
                level > GDOS_MSG_PRINT)
            {
                return;
            }

            // copy format string
            src = format;
            dst = buffer;

            while (*src != 0 && datasize < (GDOS_MAX_MSG_SIZE-1) )
            {
                *dst++ = *src++;
                datasize++;
            }

            *dst = 0;
            datasize++;

            // copy values
            va_start(args, format);
            src = format;
            dst = &buffer[datasize];

            while (*src != 0)
            {
                if (percent)
                {
                    if (((*src < '0') || (*src > '9')) && (*src != '.'))
                    {
                        switch (*src)
                        {
                            case 'b':
                            case 'd':
                            case 'i':
                            case 'n':
                            case 'u':
                            case 'x':
                            case 'X':
                                valuesize = sizeof(int);
                                if ((datasize + valuesize) > GDOS_MAX_MSG_SIZE)
                                {
                                    return;
                                }
                                *((int*)dst) = va_arg(args, int);
                                dst              += valuesize;
                                datasize         += valuesize;
                                break;

                            case 'a':
                            case 'f':
                            case 'L':
                                valuesize = sizeof(long long);
                                if ((datasize + valuesize) > GDOS_MAX_MSG_SIZE)
                                {
                                    return;
                                }
                                *((long long*)dst) = va_arg(args, long long);
                                dst              += valuesize;
                                datasize         += valuesize;
                                break;

                            case 'p':
                                valuesize = sizeof(unsigned long);
                                if ((datasize + valuesize) > GDOS_MAX_MSG_SIZE)
                                {
                                    return;
                                }
                                *((unsigned long*)dst) = va_arg(args, unsigned long);
                                dst              += valuesize;
                                datasize         += valuesize;
                                break;
/*
//TODO
                            case 's':
                                valuesize = sizeof(char);
                                char *ptr = va_arg(args, char*);
                                if (!ptr)
                                    break;

                                while (*ptr)
                                {
                                    if ((datasize + valuesize - 1) > GDOS_MAX_MSG_SIZE)
                                    {
                                        *((char*)dst) = '\0'; // add char term
                                        return;
                                    }
                                    *((char*)dst) = *ptr++;
                                    dst      += valuesize;
                                    datasize += valuesize;
                                }
                                *((char*)dst) = '\0';  // add char term
                                break;
*/
                        }
                        percent = 0;
                    }
                }
                else if (*src == '%')
                    percent = 1;

                src++;
            }

            va_end(args);

            // init message head
            tims_fill_head(&head, level, RackName::create(GDOS, 0), sendMbx->getAdr(),
                          sendMbx->getPriority(), 0, 0, TIMS_HEADLEN + datasize);

            sendMbx->sendDataMsg(&head, 1, &buffer, datasize);
        }
};

#endif  // __RACK_DEBUG_H__
