/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2007 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Authors
 *      Marko Reimer <reimer@rts.uni-hannover.de>
 *      Jan Kiszka <kiszka@rts.uni-hannover.de>
 *
 */

#include <linux/module.h>
#include <asm/byteorder.h>

#include <rtdm/rtdm_driver.h>
#include <rtdm/rtcan.h>
#include <rtdm/rtserial.h>
#include <main/rack_rtmac.h>
#include <main/tims/tims.h>
#include <main/tims/driver/tims_debug.h>

#define SYNC_NONE           0
#define SYNC_RTNET          1
#define SYNC_CAN_MASTER     2
#define SYNC_CAN_SLAVE      3
#define SYNC_SER_MASTER     4
#define SYNC_SER_SLAVE      5

#define CLOCK_SYNC_PRIORITY 0
#define CLOCK_SYNC_PERIOD   1000000000 /* 1 sec */

int clock_sync_mode = SYNC_NONE;
module_param(clock_sync_mode, int, 0400);
MODULE_PARM_DESC(clock_sync_mode, "Way of clock synchronization, "
                                  "default = 0 (no sync)");

char clock_sync_dev[RTDM_MAX_DEVNAME_LEN+1];
module_param_string(clock_sync_dev, clock_sync_dev,
                    sizeof(clock_sync_dev), 0400);
MODULE_PARM_DESC(clock_sync_dev, "Device for clock synchronization, "
                                 "default = none (no sync)");

static int clock_sync_can_id;
module_param(clock_sync_can_id, int, 0400);
MODULE_PARM_DESC(clock_sync_can_id, "Can ID of synchronisation message, "
                              "default = 0");

static rtdm_task_t      sync_task;
static rtdm_lock_t      sync_lock;
static int              sync_dev_fd;
static nanosecs_rel_t   sync_delay;
static nanosecs_rel_t   clock_offset;

struct rtdm_dev_context *sync_dev_ctx = NULL;


int tims_clock_ioctl(rtdm_user_info_t *user_info, unsigned int request,
                     void *arg)
{
    rtdm_lockctx_t lock_ctx;
    nanosecs_rel_t result = 0;

    switch(clock_sync_mode) {
        case SYNC_RTNET:
            sync_dev_ctx->ops->ioctl_rt(sync_dev_ctx, NULL,
                                        RTMAC_RTIOC_TIMEOFFSET, &result);
            break;

        case SYNC_CAN_SLAVE:
        case SYNC_SER_SLAVE:
            rtdm_lock_get_irqsave(&sync_lock, lock_ctx);
            result = clock_offset;
            rtdm_lock_put_irqrestore(&sync_lock, lock_ctx);
            break;
    }

    result += sync_delay;

    if (request == TIMS_RTIOC_GETTIME)
        result += rtdm_clock_read();

    return rtdm_safe_copy_to_user(user_info, arg, &result, sizeof(result));

}


static void sync_task_func(void *arg)
{
    int             ret;
    rtdm_lockctx_t  lock_ctx;
    nanosecs_abs_t  timestamp;
    nanosecs_abs_t  timestamp_master;
    rtser_event_t   ser_rx_event;

    can_frame_t can_frame = {
        .can_id = clock_sync_can_id,
        .can_dlc = sizeof(timestamp),
    };
    struct iovec iov = {
        .iov_base = &can_frame,
        .iov_len = sizeof(can_frame_t),
    };
    struct msghdr msg = {
        .msg_name = NULL,
        .msg_namelen = 0,
        .msg_iov = &iov,
        .msg_iovlen = 1,
        .msg_control = NULL,
        .msg_controllen = 0,
    };

    if (clock_sync_mode == SYNC_CAN_SLAVE) {
        msg.msg_control = &timestamp;
        msg.msg_controllen = sizeof(timestamp);
    }

    while (1) {
        switch (clock_sync_mode) {
            case SYNC_SER_MASTER:
                timestamp = cpu_to_be64(rtdm_clock_read());

                ret = sync_dev_ctx->ops->write_rt(sync_dev_ctx, NULL,
                    &timestamp, sizeof(timestamp));
                if (ret != sizeof(timestamp)) {
                    tims_error("[CLOCK SYNC]: can't write serial time stamp, "
                               "code = %d\n", ret);
                    goto exit_task;
                }

                rtdm_task_wait_period();
                break;

            case SYNC_SER_SLAVE:
                ret = sync_dev_ctx->ops->ioctl_rt(sync_dev_ctx, NULL,
                    RTSER_RTIOC_WAIT_EVENT, &ser_rx_event);
                if (ret < 0) {
                    tims_error("[CLOCK SYNC]: can't read serial time stamp, "
                               "code = %d\n", ret);
                    goto exit_task;
                }

                ret = sync_dev_ctx->ops->read_rt(sync_dev_ctx, NULL,
                    &timestamp_master, sizeof(timestamp_master));
                if (ret != sizeof(timestamp_master)) {
                    tims_error("[CLOCK SYNC]: can't read serial time stamp, "
                               "code = %d\n", ret);
                    goto exit_task;
                }

                timestamp_master = be64_to_cpu(timestamp_master);

                rtdm_lock_get_irqsave(&sync_lock, lock_ctx);
                clock_offset =
                    timestamp_master - ser_rx_event.rxpend_timestamp;
                rtdm_lock_put_irqrestore(&sync_lock, lock_ctx);
                break;

            case SYNC_CAN_MASTER:
                // workaround for kernel working on user data
                iov.iov_len = sizeof(can_frame_t);
                iov.iov_base = &can_frame;
                // workaround end
                *(nanosecs_abs_t *)can_frame.data =
                    cpu_to_be64(rtdm_clock_read());

                ret = sync_dev_ctx->ops->sendmsg_rt(sync_dev_ctx, NULL,
                                                    &msg, 0);
                if (ret < 0) {
                    tims_error("[CLOCK SYNC]: can't send CAN time stamp, "
                               "code = %d\n", ret);
                    goto exit_task;
                }

                rtdm_task_wait_period();
                break;

            case SYNC_CAN_SLAVE:
                // workaround for kernel working on user data
                iov.iov_len = sizeof(can_frame_t);
                iov.iov_base = &can_frame;
                // workaround end

                ret = sync_dev_ctx->ops->recvmsg_rt(sync_dev_ctx, NULL,
                                                    &msg, 0);
                if (ret < 0) {
                    tims_error("[CLOCK SYNC]: can't receive CAN time stamp, "
                               "code = %d\n", ret);
                    return;
                }

                timestamp_master =
                    be64_to_cpu(*(nanosecs_abs_t *)can_frame.data);

                rtdm_lock_get_irqsave(&sync_lock, lock_ctx);
                clock_offset = timestamp_master - timestamp;
                rtdm_lock_put_irqrestore(&sync_lock, lock_ctx);
                break;
        }
    }

 exit_task:
    rtdm_context_unlock(sync_dev_ctx);
}


static __initdata char *mode_str[] = {
    "Local Clock", "RTnet", "CAN Master", "CAN Slave",
    "Serial Master", "Serial Slave"
};

static __initdata struct rtser_config sync_serial_config = {
    .config_mask        = RTSER_SET_BAUD | RTSER_SET_FIFO_DEPTH |
                          RTSER_SET_TIMESTAMP_HISTORY | RTSER_SET_EVENT_MASK,
    .baud_rate          = 115200,
    .fifo_depth         = RTSER_FIFO_DEPTH_8,
    .timestamp_history  = RTSER_RX_TIMESTAMP_HISTORY,
    .event_mask         = RTSER_EVENT_RXPEND,
};

int __init tims_clock_init(void)
{
    struct can_filter   filter;
    int                 nr_filters = 1;
    struct ifreq        can_ifr;
    struct sockaddr_can can_addr;
    int                 ret;

    if (clock_sync_mode < SYNC_NONE || clock_sync_mode > SYNC_SER_SLAVE) {
        tims_error("invalid clock_sync_mode %d", clock_sync_mode);
        return -EINVAL;
    }

    printk("TIMS: clock sync mode is %s\n", mode_str[clock_sync_mode]);
    printk("TIMS: clock sync dev is %s\n", clock_sync_dev);

    rtdm_lock_init(&sync_lock);

    switch(clock_sync_mode) {
        case SYNC_NONE:
            return 0;

        case SYNC_RTNET:
            sync_dev_fd = rt_dev_open(clock_sync_dev, O_RDONLY);
            if (sync_dev_fd < 0)
                goto sync_dev_error;
            set_bit(TIMS_INIT_BIT_SYNC_DEV, &init_flags);
            break;

        case SYNC_CAN_MASTER:
        case SYNC_CAN_SLAVE:
            sync_dev_fd = rt_dev_socket(PF_CAN, SOCK_RAW, 0);
            if (sync_dev_fd < 0) {
                tims_error("[CLOCK SYNC]: error opening CAN socket: %d\n",
                           sync_dev_fd);
                return sync_dev_fd;
            }
            set_bit(TIMS_INIT_BIT_SYNC_DEV, &init_flags);

            strcpy(can_ifr.ifr_name, clock_sync_dev);
            ret = rt_dev_ioctl(sync_dev_fd, SIOCGIFINDEX, &can_ifr);
            if (ret) {
                tims_info("[CLOCK SYNC]: error resolving CAN interface: %d\n",
                          ret);
                return ret;
            }

            if (clock_sync_mode == SYNC_CAN_MASTER)
                nr_filters = 0;
            else {
                filter.can_id   = clock_sync_can_id;
                filter.can_mask = 0xFFFFFFFF;
            }

            ret = rt_dev_setsockopt(sync_dev_fd, SOL_CAN_RAW, CAN_RAW_FILTER,
                                    &filter, nr_filters*sizeof(can_filter_t));
            if (ret < 0)
                goto config_error;

            /* Bind socket to default CAN ID */
            can_addr.can_family  = AF_CAN;
            can_addr.can_ifindex = can_ifr.ifr_ifindex;

            ret = rt_dev_bind(sync_dev_fd, (struct sockaddr *)&can_addr,
                              sizeof(can_addr));
            if (ret < 0)
                goto config_error;

            /* Enable timestamps for incoming packets */
            ret = rt_dev_ioctl(sync_dev_fd, RTCAN_RTIOC_TAKE_TIMESTAMP,
                               RTCAN_TAKE_TIMESTAMPS);
            if (ret < 0)
                goto config_error;

            /* Calculate transmission delay */
            ret = rt_dev_ioctl(sync_dev_fd, SIOCGCANBAUDRATE, &can_ifr);
            if (ret < 0)
                goto config_error;

            /* (47+64 bit) * 1.000.000.000 (ns/sec) / baudrate (bit/s) */
            sync_delay = 1000 * (111000000 / can_ifr.ifr_ifru.ifru_ivalue);
            break;

        case SYNC_SER_MASTER:
        case SYNC_SER_SLAVE:
            sync_dev_fd = rt_dev_open(clock_sync_dev, O_RDWR);
            if (sync_dev_fd < 0)
                goto sync_dev_error;
            set_bit(TIMS_INIT_BIT_SYNC_DEV, &init_flags);

            ret = rt_dev_ioctl(sync_dev_fd, RTSER_RTIOC_SET_CONFIG,
                               &sync_serial_config);
            if (ret < 0)
                goto config_error;

            /* (80 bit) * 1.000.000.000 (ns/sec) / baudrate (bit/s) */
            sync_delay = 1000 * (80000000 / sync_serial_config.baud_rate);
            break;
    }

    sync_dev_ctx = rtdm_context_get(sync_dev_fd);

    if (clock_sync_mode != SYNC_RTNET) {
        ret = rtdm_task_init(&sync_task, "TIMSClockSync", sync_task_func,
                             NULL, CLOCK_SYNC_PRIORITY, CLOCK_SYNC_PERIOD);
        if (ret < 0)
            return ret;
        set_bit(TIMS_INIT_BIT_SYNC_TASK, &init_flags);
    }

    return 0;

 sync_dev_error:
    tims_error("[CLOCK SYNC]: cannot open %s\n", clock_sync_dev);
    return sync_dev_fd;

 config_error:
    tims_info("[CLOCK SYNC]: error configuring sync device: %d\n", ret);
    return ret;
}


void tims_clock_cleanup(void)
{
    if (test_and_clear_bit(TIMS_INIT_BIT_SYNC_DEV, &init_flags))
        rt_dev_close(sync_dev_fd);

    if (test_and_clear_bit(TIMS_INIT_BIT_SYNC_TASK, &init_flags))
        rtdm_task_join_nrt(&sync_task, 100);
}

