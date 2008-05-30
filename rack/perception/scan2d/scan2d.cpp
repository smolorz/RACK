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
#include "scan2d.h"
#include <main/argopts.h>

// init_flags
#define INIT_BIT_DATA_MODULE        0
#define INIT_BIT_MBX_WORK           1
#define INIT_BIT_MBX_LADAR          2
#define INIT_BIT_PROXY_LADAR        3
#define INIT_BIT_PROXY_CAMERA       4

// filter flags:
#define FILTER_REFLECTOR_90DEG   0x01


typedef struct {
    ladar_data    data;
    int32_t       buffer[LADAR_DATA_MAX_DISTANCE_NUM];
} __attribute__((packed)) ladar_data_msg;

typedef struct {
    scan2d_data     data;
    scan_point      point[SCAN2D_POINT_MAX];
} __attribute__((packed)) scan2d_msg;

static inline void swap(int32_t *a, int32_t *b)
{
    int32_t tmp;

    tmp = *a;
    *a  = *b;
    *b  = tmp;
}

//
// data structures
//

Scan2d *p_inst;

argTable_t argTab[] = {

    { ARGOPT_REQ, "ladarInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the ladar driver", { -1 } },

    { ARGOPT_OPT, "cameraInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the camera driver", { -1 } },

    { ARGOPT_OPT, "ladarOffsetX", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Ladar X offset (default 0)", { 0 } },

    { ARGOPT_OPT, "ladarOffsetY", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Ladar Y offset (default 0)", { 0 } },

    { ARGOPT_OPT, "ladarOffsetRho", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Ladar rho offset (default 0)", { 0 } },

    { ARGOPT_OPT, "ladarUpsideDown", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Ladar mounted upside down (0: normal (default), 1: upside down)", { 0 } },

    { ARGOPT_OPT, "maxRange", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Maximal ladar range", { 30000 } },

    { ARGOPT_OPT, "reduce", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "reduce (default 1)", { 1 } },

    { ARGOPT_OPT, "angleMin", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "minimum angle (default -180)", { -180 } },

    { ARGOPT_OPT, "angleMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "maximum angle (default 180)", { 180 } },

    { ARGOPT_OPT, "reflectorFilterMode", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "reflector filter bitset [0: off, 0x01: 90deg filter], (default 0)", { 0 } },

    { ARGOPT_OPT, "medianFilter", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "enable median filter (default 0)", { 0 } },

    { 0, "", 0, 0, "", { 0 } } // last entry
};

/*******************************************************************************
 *   !!! REALTIME CONTEXT !!!
 *
 *   moduleOn,
 *   moduleOff,
 *   moduleLoop,
 *   moduleCommand,
 *
 *   own realtime user functions
 ******************************************************************************/

 int  Scan2d::moduleOn(void)
{
    int ret;

    // get parameter
    ladarOffsetX        = getInt32Param("ladarOffsetX");
    ladarOffsetY        = getInt32Param("ladarOffsetY");
    ladarOffsetRho      = getInt32Param("ladarOffsetRho");
    ladarUpsideDown     = getInt32Param("ladarUpsideDown");
    maxRange            = getInt32Param("maxRange");
    reduce              = getInt32Param("reduce");
    angleMin            = getInt32Param("angleMin");
    angleMax            = getInt32Param("angleMax");
    medianFilter        = getInt32Param("medianFilter");
    reflectorFilterMode = getInt32Param("reflectorFilterMode");

    angleMinFloat       = (double)angleMin       * M_PI / 180.0;
    angleMaxFloat       = (double)angleMax       * M_PI / 180.0;
    ladarOffsetRhoFloat = (double)ladarOffsetRho * M_PI / 180.0;

    GDOS_DBG_DETAIL("scan2d filter:\n");
    GDOS_DBG_DETAIL("  medianFilter        = %i\n", medianFilter);
    GDOS_DBG_DETAIL("  reflectorFilterMode = %i\n", reflectorFilterMode);


    ret = ladar->on();
    if (ret)
    {
        GDOS_ERROR("Can't turn on Ladar(%d), code = %d \n", ladarInst, ret);
        return ret;
    }

    if (cameraInst >= 0)
    {
        ret = camera->on();
        if (ret)
        {
            GDOS_ERROR("Can't turn on Camera(%d), code = %d\n", cameraInst, ret);
            return ret;
        }
    }

    ret = ladar->getContData(0, &ladarMbx, &dataBufferPeriodTime);
    if (ret)
    {
        GDOS_ERROR("Can't get continuous data from Ladar(%d), "
                   "code = %d \n", ladarInst, ret);
        return ret;
    }

    return RackDataModule::moduleOn();  // has to be last command in moduleOn();
}

void Scan2d::moduleOff(void)
{
    RackDataModule::moduleOff();        // has to be first command in moduleOff();

    ladar->stopContData(&ladarMbx);
}

int  Scan2d::moduleLoop(void)
{
    scan2d_data*    data2D;
    ladar_data*     dataLadar;
    message_info    msgInfo;
    double          angle, x, y;
    int             i, j, ret, numRef = 0;

    // get datapointer from rackdatabuffer
    data2D = (scan2d_data *)getDataBufferWorkSpace();

    // get Ladar data
    ret = ladarMbx.peekTimed(1000000000llu, &msgInfo); // 1s
    if (ret)
    {
        GDOS_ERROR("Can't receive ladar data on DATA_MBX, "
                   "code = %d \n", ret);
        return ret;
    }

    if ((msgInfo.type != MSG_DATA) ||
        (msgInfo.src  != ladar->getDestAdr()))
    {
        GDOS_ERROR("Received unexpected message from %n to %n type %d on "
                   "data mailbox\n", msgInfo.src, msgInfo.dest, msgInfo.type);

        ladarMbx.peekEnd();
        return -EINVAL;
    }

    dataLadar = LadarData::parse(&msgInfo);

    if (dataLadar->distanceNum > LADAR_DATA_MAX_DISTANCE_NUM)
    {
        GDOS_ERROR("DistanceNum (%d) too great\n", dataLadar->distanceNum);
        ladarMbx.peekEnd();
        return -EINVAL;
    }

    data2D->recordingTime = dataLadar->recordingTime;
    data2D->duration      = dataLadar->duration;
    data2D->sectorNum     = 1;
    data2D->sectorIndex   = 0;

    if (dataLadar->maxRange < maxRange)
        data2D->maxRange      = dataLadar->maxRange;
    else
        data2D->maxRange      = maxRange;

    if (ladarUpsideDown)
    {
        turnUpsideDown(dataLadar);
    }

    if(medianFilter)
    {
        filterMedian(dataLadar);
    }

    data2D->pointNum = 0;
    angle = dataLadar->startAngle;

    for (i=0; i < dataLadar->distanceNum; i += reduce)
    {
        if ((angle >= angleMinFloat) &&
            (angle <= angleMaxFloat))
        {
            j = data2D->pointNum;
            data2D->point[j].type      = TYPE_UNKNOWN;
            data2D->point[j].segment   = 0;
            data2D->point[j].intensity = 0;

            if (dataLadar->distance[i] < 0)
            {
                dataLadar->distance[i]  = -dataLadar->distance[i];
                data2D->point[j].type  |= TYPE_REFLECTOR;

               numRef++;
            }

            if ((dataLadar->distance[i] >= data2D->maxRange) || (dataLadar->distance[i] == 0))
            {
                dataLadar->distance[i]  = data2D->maxRange;
                data2D->point[j].type  |= TYPE_MAX_RANGE;
                data2D->point[j].type  |= TYPE_INVALID;
            }

            x = (double)dataLadar->distance[i] *
                        cos(angle + ladarOffsetRhoFloat);
            y = (double)dataLadar->distance[i] *
                        sin(angle + ladarOffsetRhoFloat);

            data2D->point[j].x = (int)x + ladarOffsetX;
            data2D->point[j].y = (int)y + ladarOffsetY;
            data2D->point[j].z = dataLadar->distance[i];

            data2D->pointNum++;
        }
        angle += reduce * dataLadar->angleResolution;
    }

    // filter invalid reflector points:
    filterReflector(data2D, reflectorFilterMode);

    // add intensity to scanPoints
    if(cameraInst >= 0)
    {
        ret = addScanIntensity(data2D);
        if(ret)
        {
            GDOS_ERROR("Can't add scan intensity, code = %d\n", ret);
            return ret;
        }
    }

    ladarMbx.peekEnd();

    GDOS_DBG_DETAIL("RecordingTime %u pointNum %d\n",
                    data2D->recordingTime, data2D->pointNum);

    putDataBufferWorkSpace(Scan2dData::getDatalen(data2D));

    return 0;
}


void  Scan2d::turnUpsideDown(ladar_data* dataLadar)
{
    int  i, num, max;
    int32_t  *left, *right;

    num = dataLadar->distanceNum;

    max   = num / 2;
    left  = &dataLadar->distance[0];
    right = &dataLadar->distance[num-1];

    for (i = 0; i < max; i++)
    {
        swap(left, right);

        left++;
        right--;
    }
}

void  Scan2d::filterMedian(ladar_data* dataLadar)
{
    int32_t a, b, c, d;
    int i;

    d = dataLadar->distance[0];

    for(i = 1; i < dataLadar->distanceNum - 1; i++)
    {
        a = dataLadar->distance[i-1];
        b = dataLadar->distance[i];
        c = dataLadar->distance[i+1];

        dataLadar->distance[i-1] = d;

        if((a > 0) && (b > 0) && (c > 0))
        {
            // filter only none reflector points

            if(a > b)
            {
                swap(&a, &b);
            }
            if(b > c)
            {
                swap(&b, &c);
            }
            if(a > b)
            {
                swap(&a, &b);
            }
        }

        d = b;
    }

    dataLadar->distance[i-1] = d;
}

#define REFL_FILTER_ANGLE   15.0 // [deg]
#define MAX_NB_DIST        250   // [mm]
int Scan2d::filterReflector(scan2d_data* data, int filter)
{
    int     i, left, right, numInvalid = 0;
    double  m, n;
    double  alpha, beta, gamma;


    // filter reflector points, which are on a line of scanpoints about 90deg to the laser ray:
    // ----------------------------------------------------------------------------------------
    if (filter & FILTER_REFLECTOR_90DEG)
    {
        for (i=0; i<data->pointNum; i++)
        {
            if ((data->point[i].type & TYPE_REFLECTOR) != TYPE_REFLECTOR) // no reflector point
                continue;

            // if there are two reflector points close to the current, this should be a correct reflector point:
            if (i>0  &&  i<data->pointNum-1  &&
                ((data->point[i-1].type & TYPE_REFLECTOR) == TYPE_REFLECTOR) &&
                ((data->point[i+1].type & TYPE_REFLECTOR) == TYPE_REFLECTOR))
                continue;

            if (i>1  &&
                ((data->point[i-1].type & TYPE_REFLECTOR) == TYPE_REFLECTOR) &&
                ((data->point[i-2].type & TYPE_REFLECTOR) == TYPE_REFLECTOR))
                continue;

            if (i<data->pointNum-2  &&
                ((data->point[i+1].type & TYPE_REFLECTOR) == TYPE_REFLECTOR) &&
                ((data->point[i+2].type & TYPE_REFLECTOR) == TYPE_REFLECTOR))
                continue;

            // find index of left and right point in distance:
            getNeighborhood(data, i, &left, &right);

            if (left == right) // no neighbor points
            {
                // GDOS_DBG_DETAIL("can't filter scanpoint %i, no neighbor points (left=%i, right=%i)\n", i, left, right);
                continue;
            }

            // get regression line:
            getRegressLine(data, left, right, &m, &n);

            // get angles:
            // GDOS_DBG_DETAIL("m=%f, n=%f, dx=%i, dy=%i\n", m, n, data->point[i].x, data->point[i].y);

            alpha = atan(m) * 180.0/M_PI;
            beta  = atan2(data->point[i].y, data->point[i].x) * 180.0/M_PI;
            gamma = fabs(beta - alpha);

            if (gamma > 90)
                gamma = 180.0 - gamma;

            // get angle of scanner spot to line between current and next scanpoint:
            // GDOS_DBG_DETAIL("p:%i, alpha:%f, beta:%f, gamma:%f\n", i, alpha, beta, gamma);

            if (90.0-gamma <= REFL_FILTER_ANGLE)
            {
                data->point[i].type &= (~TYPE_REFLECTOR);
                numInvalid++;

                // GDOS_WARNING("scanpoint %i: INVALID reflector: angle = %f\n", i, 90.0-gamma);
            }
            else
            {
                // GDOS_DBG_DETAIL("scanpoint %i: VALID reflector: angle = %f\n", i, 90.0-gamma);
            }

        }

        GDOS_DBG_DETAIL("ReflectorFilter(90deg): %d invalid reflector points filtered\n", numInvalid);
    }

    return (0);
}


int Scan2d::getNeighborhood(scan2d_data* data, int refIdx, int *left, int *right)
{
    int    j;
    float  dx, dy;


    *left = *right = refIdx;

    // find left neighbor:
    for (j=refIdx+1; j < data->pointNum; j++)
    {
        dx = data->point[j].x - data->point[refIdx].x;
        dy = data->point[j].y - data->point[refIdx].y;

        if (sqrt(dx*dx + dy*dy) > MAX_NB_DIST)
            break;

        *left = j;
    }

    // find right neighbor:
    for (j=refIdx-1; j>=0; j--)
    {
        dx = data->point[j].x - data->point[refIdx].x;
        dy = data->point[j].y - data->point[refIdx].y;

        if (sqrt(dx*dx + dy*dy) > MAX_NB_DIST)
            break;

        *right = j;
    }

    return (0);
}


int Scan2d::getRegressLine(scan2d_data* data, int left, int right, double *m, double *n)
{
    int         i, num, start, end;
    double      sumX  = 0.0;
    double      sumY  = 0.0;
    double      sumXY = 0.0;
    double      sumXX = 0.0;
    double      sumYY = 0.0;
    scan_point *ptr   = NULL;


    if (right < left)
    {
        start = right;
        end   = left;
    }
    else
    {
        start = left;
        end   = right;
    }

    ptr = &data->point[start];

    // loop from right to left index:
    for (i=start; i<=end; i++, ptr++)
    {
        sumX  += (double)ptr->x;
        sumY  += (double)ptr->y;
        sumXY += (double)(ptr->x * ptr->y);
        sumXX += (double)(ptr->x * ptr->x);
        sumYY += (double)(ptr->y * ptr->y);
    }

    // get regression line:
    num = end - start + 1;

    // GDOS_DBG_DETAIL("right=%i, left=%i, num:%i, sumXX:%f, sumX:%f, Nenner:%f",right, left,num, sumXX, sumX, ((num * sumXX) - (sumX * sumX)));
    *m  = (double)((((double)num * sumXY) - (sumX * sumY)) / (((double)num * sumXX) - (sumX * sumX)));
    *n  = (double)((sumY - (*m * sumX)) / ((double)num));

    return (0);
}


int  Scan2d::moduleCommand(message_info *msgInfo)
{
    // not for me -> ask RackDataModule
    return RackDataModule::moduleCommand(msgInfo);
}

/*******************************************************************************
 *   !!! NON REALTIME CONTEXT !!!
 *
 *   moduleInit,
 *   moduleCleanup,
 *   Constructor,
 *   Destructor,
 *   main,
 *
 *   own non realtime user functions
 ******************************************************************************/

int Scan2d::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    // get static parameter
    ladarInst       = getInt32Param("ladarInst");
    cameraInst      = getInt32Param("cameraInst");

    // work mailbox
    ret = createMbx(&workMbx, 1, sizeof(camera_data_ladar_msg), MBX_IN_KERNELSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    // ladar-data mailbox
    ret = createMbx(&ladarMbx, 1, sizeof(ladar_data_msg),
                    MBX_IN_USERSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_LADAR);

    // create Ladar Proxy
    ladar = new LadarProxy(&workMbx, 0, ladarInst);
    if (!ladar)
    {
        ret = -ENOMEM;
        goto init_error;
    }
    initBits.setBit(INIT_BIT_PROXY_LADAR);

    // create Camera Proxy
    if (cameraInst >= 0)
    {
        camera = new CameraProxy(&workMbx, 0, cameraInst);
        if (!camera)
        {
            ret = -ENOMEM;
            goto init_error;
        }
        initBits.setBit(INIT_BIT_PROXY_CAMERA);
    }

    return 0;

init_error:
    // !!! call local cleanup function !!!
    Scan2d::moduleCleanup();
    return ret;
}

void Scan2d::moduleCleanup(void)
{
    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }


    // free proxies
    if (initBits.testAndClearBit(INIT_BIT_PROXY_CAMERA))
    {
        delete camera;
    }

    if (initBits.testAndClearBit(INIT_BIT_PROXY_LADAR))
    {
        delete ladar;
    }

    // delete mailboxes
    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }

    if (initBits.testAndClearBit(INIT_BIT_MBX_LADAR))
    {
        destroyMbx(&ladarMbx);
    }
}

Scan2d::Scan2d(void)
      : RackDataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s datatask error sleep time
                    16,               // command mailbox slots
                    240,              // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    10,               // max buffer entries
                    10)               // data buffer listener
{
    dataBufferMaxDataSize = sizeof(scan2d_msg);
}

int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "Scan2d");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new Scan2d
    p_inst = new Scan2d();
    if (!p_inst)
    {
        printf("Can't create new Scan2d -> EXIT\n");
        return -ENOMEM;
    }

    // init
    ret = p_inst->moduleInit();
    if (ret)
        goto exit_error;

    p_inst->run();

    return 0;

exit_error:
    delete (p_inst);
    return ret;
}

int Scan2d::addScanIntensity(scan2d_data* data2D)
{
    short   intensity;
    int     ret, i;

    ret = camera->getData(&(cameraMsg.data), sizeof(cameraMsg), data2D->recordingTime);
    if(ret)
    {
        GDOS_ERROR("Can't read camera data, code = %d\n", ret);
        return ret;
    }

    if(cameraMsg.data.recordingTime != data2D->recordingTime)
    {
        GDOS_ERROR("Camera recordingTime doesn't fit\n");
        return -EIO;
    }

    if( ((cameraMsg.data.width != 1) && (cameraMsg.data.height != 1)) ||
        ((cameraMsg.data.mode  != CAMERA_MODE_MONO12) && (cameraMsg.data.mode  != CAMERA_MODE_MONO16)) )
    {
        GDOS_ERROR("Camera data doesn't fit (width %i height %i depth %i mode %x)", cameraMsg.data.width, cameraMsg.data.height, cameraMsg.data.depth, cameraMsg.data.mode);
        return -EIO;
    }

    if (cameraMsg.data.height != data2D->pointNum)
    {
        GDOS_ERROR("ScanPointNum:%i is not matching cameraWidth:%i\n", data2D->pointNum, cameraMsg.data.width);
        return -EINVAL;
    }

    // loop over all points of this intensity image
    for (i = 0; i < data2D->pointNum; i++)
    {
        intensity  = (short) ( (((cameraMsg.byteStream[i * 2]) << 8) & 0xff00) | ((cameraMsg.byteStream[i * 2+1]) & 0x00ff) );
        data2D->point[i].intensity = intensity;
    }
    return 0;
}
