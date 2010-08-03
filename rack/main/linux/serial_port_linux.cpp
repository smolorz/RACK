/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2010 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Oliver Wulf <oliver.wulf@web.de>
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#include <main/serial_port.h>

//
// c function wrappers
//

int open_serial_dev(const int serialDev)
{
    int fd;
    char filename[16];

    if(serialDev < 100)
    {
        snprintf(filename, 16, "/dev/ttyS%i", serialDev);
    }
    else
    {
        snprintf(filename, 16, "/dev/ttyUSB%i", (serialDev - 100));
    }

    // open port
    fd = open(filename, O_RDWR | O_NOCTTY | O_NDELAY);

    if(fd < 0)
    {
        // can't open port
        return fd;
    }

    // set blocking mode
    fcntl(fd, F_SETFL, 0);

    return fd;
}

int close_serial_dev(int filedes)
{
    return close(filedes);
}

//
// Constructor and destructor
//

SerialPort::SerialPort()
{
    module = NULL;
    fd = -1;
}

SerialPort::~SerialPort()
{
    module = NULL;
    if (fd != -1)
    {
        close_serial_dev(fd);
    }
}

//
// PortFunctions
//

int SerialPort::open(int dev, const rtser_config *config, RackModule *module)
{
    int ret;

    if (fd != -1)
    {
        // file is open
        return -EBUSY;
    }

    ret = open_serial_dev(dev);
    if (ret < 0)
    {
        return ret;
    }

    fd = ret;
    this->module = module;

    ret = setConfig(config);

    return ret;
}

int SerialPort::close(void)
{
    if(fd != -1)
        close_serial_dev(fd);

    module = NULL;
    fd = -1;

    return 0;
}

int SerialPort::setConfig(const rtser_config *config)
{
	struct termios options;

	// Get the current options for the port...
	tcgetattr(fd, &options);

	// Enable the receiver and set local mode...
	options.c_cflag |= (CLOCAL | CREAD);

    // options 8N1
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

    // disable hardware flow control
    //options.c_cflag &= ~CNEW_RTSCTS;
	options.c_cflag &= ~CRTSCTS;

    // disable software flow control
	options.c_iflag &= ~(IXON | IXOFF | IXANY);
    
    // raw input and output
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_oflag &= ~OPOST;

	// Set the new options for the port...
	tcsetattr(fd, TCSAFLUSH, &options);

    setBaudrate(config->baud_rate);

    setRecvTimeout(config->rx_timeout);

    return 0;
}

int SerialPort::setBaudrate(int baudrate)
{
	struct termios options;

	// Get the current options for the port...
	tcgetattr(fd, &options);

	// Set the baud rates...
    switch(baudrate)
    {
        case 1200:
        	cfsetispeed(&options, B1200);
	        cfsetospeed(&options, B1200);
            break;

        case 2400:
        	cfsetispeed(&options, B2400);
	        cfsetospeed(&options, B2400);
            break;

        case 4800:
        	cfsetispeed(&options, B4800);
	        cfsetospeed(&options, B4800);
            break;

        case 9600:
        	cfsetispeed(&options, B9600);
	        cfsetospeed(&options, B9600);
            break;

        case 19200:
        	cfsetispeed(&options, B19200);
	        cfsetospeed(&options, B19200);
            break;

        case 38400:
        	cfsetispeed(&options, B38400);
	        cfsetospeed(&options, B38400);
            break;

        case 57600:
        	cfsetispeed(&options, B57600);
	        cfsetospeed(&options, B57600);
            break;

        case 115200:
        	cfsetispeed(&options, B115200);
	        cfsetospeed(&options, B115200);
            break;

        default:
            return -1;
    }

	// Set the new options for the port...
	tcsetattr(fd, TCSAFLUSH, &options);

    return 0;
}

int SerialPort::setRecvTimeout(int64_t timeout)
{
	struct termios options;

	// Get the current options for the port...
	tcgetattr(fd, &options);

    if(timeout == RTSER_TIMEOUT_INFINITE)
    {
    	options.c_cc[VTIME] = 0;
	    options.c_cc[VMIN] = 1;
    }
    else if(timeout == RTSER_TIMEOUT_NONE)
    {
    	options.c_cc[VTIME] = 0;
	    options.c_cc[VMIN] = 0;
    }
    else
    {
        if(timeout > 25000000000ll)
        {
        	timeout = 25000000000ll;
        }

        timeout = timeout / 100000000ll;
    	options.c_cc[VTIME] = (int)timeout + 1;  // x 0.1s
	    options.c_cc[VMIN] = 0;
    }

	// Set the new options for the port...
	tcsetattr(fd, TCSANOW, &options);

    return 0;
}

int SerialPort::getControl(int32_t *bitmask)
{
    return -1;
}

int SerialPort::setControl(int32_t bitmask)
{
    return -1;
}

int SerialPort::getStatus(struct rtser_status *status)
{
    return -1;
}

int SerialPort::send(const void* data, int dataLen)
{
    int ret;

    ret = write(fd, data, dataLen);

    if (ret != dataLen)
    {
        return -EFAULT;
    }

    return 0;
}

// receive data with no timestamp and the default timeout
int SerialPort::recv(void *data, int dataLen)
{
    int ret;
    int dataRead = 0;

    do
    {
        ret = read(fd, (char*)data + dataRead, (dataLen - dataRead));

        if (ret == 0)
        {
            return -ETIMEDOUT;      // timeout
        }
        else if (ret < 0)
        {
            return ret;             // IO error
        }

        dataRead += ret;
    }
    while(dataRead < dataLen);

    return 0;
}

// receive data with timestamp and the default timeout
int SerialPort::recv(void *data, int dataLen, rack_time_t *timestamp)
{
    int ret;

    ret = recv(data, dataLen);

    if (timestamp)
    {
        *timestamp = module->rackTime.get();
    }

    return ret;
}

// receive data with timestamp and a specific timeout
int SerialPort::recv(void *data, int dataLen, rack_time_t *timestamp, int64_t timeout_ns)
{
    int ret;

    ret = setRecvTimeout(timeout_ns);
    if (ret)
    {
        return ret;
    }

    return recv(data, dataLen, timestamp);
}

int SerialPort::waitEvent(struct rtser_event *event)
{
    int ret;
    int count = 0;

    while((ret = read(fd, NULL, 0)) == 0)
    {
        if(count > 100)
        {
            event->events = 0;
            event->rx_pending = 0;

            return -ETIMEDOUT;
        }

        usleep(10000);  // 10ms
        count++;
    }

    if(ret < 0)
    {
        event->events = 0;
        event->rx_pending = 0;

        return -1;
    }
    else
    {
        event->events = RTSER_EVENT_RXPEND;
        event->rx_pending = ret;

        return 0;
    }
}

int SerialPort::clean(void)
{
	struct termios options;

	// flush port
    tcgetattr(fd, &options);
    tcsetattr(fd, TCSAFLUSH, &options);

    // clear input buffer
    char buffer;
    tcgetattr(fd, &options);
    this->setRecvTimeout(RTSER_TIMEOUT_NONE);
    while(this->recv(&buffer, 1) == 0);
    tcsetattr(fd, TCSANOW, &options);

    return 0;
}
