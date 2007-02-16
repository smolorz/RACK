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
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */
#ifndef __GPS_NMEA_H__
#define __GPS_NMEA_H__

#include <main/rack_datamodule.h>
#include <main/serial_port.h>
#include <main/angle_tool.h>
#include <drivers/gps_proxy.h>
#include <time.h>

// define module class
#define MODULE_CLASS_ID                     GPS

#define RMC_MSG 0
#define GGA_MSG 1
#define GSA_MSG 2
#define VTG_MSG 3

#define KNOTS_TO_MS 0.5144456334


typedef struct
{
    double    x;
    double    y;
    double    z;
} gps_nmea_pos_3d;

typedef struct
{
    rack_time_t        recordingTime;
    char             data[1024];
} gps_nmea;

//######################################################################
//# class GpsNmea
//######################################################################

class GpsNmea : public RackDataModule {
    private:
        int             serialDev;
        int             periodTime;
        int             trigMsg;
        int             posGKOffsetX;
        int             posGKOffsetY;

        rack_time_t     lastRecordingTime;
        SerialPort      serialPort;
        gps_nmea        nmea;
        gps_nmea_pos_3d posGKOld;
        gps_data        gpsData;

        // -> realtime context
        int readNMEAMessage();
        int analyseRMC(gps_data *data);
        int analyseGGA(gps_data *data);
        int analyseGSA(gps_data *data);
        int analyseVTG(gps_data *data);

        double degHMStoRad(double degHMS);
        long    toCalendarTime(float time, int date);

        void    posWGS84ToGK(gps_nmea_pos_3d *posLla, gps_nmea_pos_3d *posGk);
        void    helmertTransformation(double x, double y, double z,
                                      double *xo, double *yo, double *zo);
        void    besselBLToGaussKrueger(double b, double ll,
                                       double *re, double *ho);
        void    bLRauenberg(double x, double y, double z,
                            double *b, double *l, double *h);
        double  newF(double f, double x, double y,
                     double p, double eq, double a);

    protected:
        // -> realtime context
        int      moduleOn(void);
        void     moduleOff(void);
        int      moduleLoop(void);
        int      moduleCommand(message_info *msgInfo);

        // -> non realtime context
        void     moduleCleanup(void);

    public:
        // constructor und destructor
        GpsNmea();
        ~GpsNmea() {};

        // -> realtime context
        int  moduleInit(void);
};

#endif // __GPS_NMEA_H__
