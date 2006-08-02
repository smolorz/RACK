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

/*!
 * @ingroup driverapi
 * @defgroup rtserial Serial Port API
 *
 * This is the Serial Port interface of RACK provided to application programs
 * in userspace.
 * @{
 */

#include <stdio.h>
#include <unistd.h>

#include <main/serial_port.h>
#include <errno.h>


//
// Constructor and destructor
//

SerialPort::SerialPort()
{
    fd = -1;
}

SerialPort::~SerialPort()
{
    if (fd != -1)
        close();
}

//
// PortFunctions
//

int SerialPort::open(int dev, const rtser_config *config, RackModule *module)
{
    int ret;
    char filename[10];

    if (fd != -1) // file is open
        return -EBUSY;

    snprintf(filename, 10, "rtser%i", dev);

    ret = rt_dev_open(filename, 0);
    if (ret < 0)
        return ret;

    fd = ret;
    this->module = module;

    return rt_dev_ioctl(fd, RTSER_RTIOC_SET_CONFIG, config);
}

// non realtime context !!!
int SerialPort::close(void)
{
    int i = 5;
    int ret;

    do
    {
        ret = rt_dev_close(fd);
        if (!ret) // exit on success
        {
            fd = -1;
            return 0;
        }
        else if (ret == -EAGAIN) // try it again (max 5 times)
        {
            sleep(1); // wait 1s
        }
    }
    while (ret != -EAGAIN && i--);

    module = NULL;
    return ret;
}

int SerialPort::setConfig(const rtser_config *config)
{
    return rt_dev_ioctl(fd, RTSER_RTIOC_SET_CONFIG, config);
}

int SerialPort::setBaudrate(int baudrate)
{
    struct rtser_config setbaud_cfg;

    setbaud_cfg.config_mask = RTSER_SET_BAUD;
    setbaud_cfg.baud_rate   = baudrate;

    return rt_dev_ioctl(fd, RTSER_RTIOC_SET_CONFIG, &setbaud_cfg);
}

int SerialPort::setRxTimeout(int64_t timeout)
{
    struct rtser_config settime_cfg;

    // setting timeout
    settime_cfg.config_mask = RTSER_SET_TIMEOUT_RX;
    settime_cfg.rx_timeout  = timeout;

    return rt_dev_ioctl(fd, RTSER_RTIOC_SET_CONFIG, &settime_cfg);
}

int SerialPort::setEventTimeout(int64_t timeout)
{
    struct rtser_config settime_cfg;

    // setting timeout
    settime_cfg.config_mask   = RTSER_SET_TIMEOUT_EVENT;
    settime_cfg.event_timeout = timeout;

    return rt_dev_ioctl(fd, RTSER_RTIOC_SET_CONFIG, &settime_cfg);
}

int SerialPort::getControl(int32_t *bitmask)
{
    return rt_dev_ioctl(fd, RTSER_RTIOC_GET_CONTROL, bitmask);
}

int SerialPort::setControl(int32_t bitmask)
{
    return rt_dev_ioctl(fd, RTSER_RTIOC_SET_CONTROL, bitmask);
}

int SerialPort::getStatus(struct rtser_status *status)
{
    return rt_dev_ioctl(fd, RTSER_RTIOC_GET_STATUS, status);
}

int SerialPort::send(const void* data, int dataLen)
{
    int ret;

    ret = rt_dev_write(fd, data, dataLen);
    if (ret != dataLen)
        return -EFAULT;

    return 0;
}

// receive data with no timestamp and the default timeout
int SerialPort::recv(void *data, int dataLen)
{
    int ret;

    ret = rt_dev_read(fd, data, dataLen);
    if (ret != dataLen)
        return ret;

    return 0;
}

// receive data with timestamp and the default timeout
int SerialPort::recv(void *data, int dataLen, rack_time_t *timestamp)
{
    int ret;
    rtser_event_t rx_event;

    if (timestamp)
    {
        // get timestamp
        ret = rt_dev_ioctl(fd, RTSER_RTIOC_WAIT_EVENT, &rx_event);
        if (ret)
            return ret;

        *timestamp = module->rackTime.fromNano(rx_event.rxpend_timestamp);
    }

    return recv(data, dataLen);
}

// receive data with timestamp and a specific timeout
int SerialPort::recv(void *data, int dataLen, rack_time_t *timestamp,
                     int64_t timeout_ns)
{
    int ret;

    ret = setRxTimeout(timeout_ns);
    if (ret)
        return ret;

    return recv(data, dataLen, timestamp);
}


int SerialPort::recv_pending(void *data, int maxdataLen)
{
    int ret;

    ret = setRxTimeout(-1);    // NONBLOCK
    if (ret)
        return ret;

    return rt_dev_read(fd, data, maxdataLen);
}

int SerialPort::clean(void)
{
    return rt_dev_ioctl(fd, RTIOC_PURGE, RTDM_PURGE_RX_BUFFER |
                                         RTDM_PURGE_TX_BUFFER);
}

/*@}*/
