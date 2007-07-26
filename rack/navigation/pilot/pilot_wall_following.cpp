/*
 * RACK-RTS - Robotics Application Construction Kit (RTS internal)
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * All rights reserved.
 *
 * Authors
 *
 */

#include "pilot_wall_following.h"

// init_flags (for init and cleanup)
#define INIT_BIT_DATA_MODULE        0
#define INIT_BIT_MBX_SCAN2D         1
#define INIT_BIT_MBX_WORK           2
#define INIT_BIT_PROXY_SCAN2D       3
#define INIT_BIT_PROXY_POSITION     4
#define INIT_BIT_PROXY_CHASSIS      5

// define globalState
#define STATE_RUNNING               0
#define STATE_OBST                  1
#define STATE_OBST_UNAVOIDABLE      2


// data structures
//

PilotWallFollowing *p_inst;

argTable_t argTab[] = {

    { ARGOPT_REQ, "scan2dInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the Scan2d module", { -1 } },

    { ARGOPT_OPT, "chassisInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the chassis module", { 0 } },

    { ARGOPT_OPT, "positionInst", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "The instance number of the position module", { 0 } },

    { ARGOPT_OPT, "maxSpeed", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Maximum Speed, default 500", { 500 } },

    { ARGOPT_OPT, "omegaMax", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Maximum omega, default 20 deg/s", { 20 } },

    { ARGOPT_OPT, "mode", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "Wall following mode  (default 1)", { 1 } },

    { ARGOPT_OPT, "distance", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "distance  (default 1000)", { 1000 } },

    { ARGOPT_OPT, "testDis", ARGOPT_REQVAL, ARGOPT_VAL_INT,
      "test distance  (default 1000)", { 1000 } },

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

 int  PilotWallFollowing::moduleOn(void)
{
    rack_time_t   scan2dPeriodTime;
    int ret;

    ret = chassis->on();
    if (ret)
    {
        GDOS_ERROR("Can't turn on chassis, code = %d\n", ret);
        return ret;
    }

    // get chassis parameter data
    ret = chassis->getParam(&chasParData, sizeof(chassis_param_data));
    if (ret)
    {
        GDOS_ERROR("Can't get chassis parameter, code = %d\n", ret);
        return ret;
    }

    memcpy(&chasParDataTransForward, &chasParData, sizeof(chasParData));
    memcpy(&chasParDataTransBackward, &chasParData, sizeof(chasParData));

    chasParData.safetyMarginMove  = chasParData.safetyMarginMove;
    chasParDataTransForward.safetyMarginMove  = chasParDataTransForward.safetyMarginMove;
    chasParDataTransBackward.safetyMarginMove = chasParDataTransBackward.safetyMarginMove;
    chasParDataTransForward.boundaryBack = 0;
    chasParDataTransBackward.boundaryFront = 0;

    chasParData.axMax = 100;


    if (maxSpeed > chasParData.vxMax)
    {
        maxSpeed = chasParData.vxMax;
        GDOS_DBG_DETAIL("Max speed : %d\n", maxSpeed);
    }

    ret = position->on();
    if (ret)
    {
        GDOS_ERROR("Can't turn on Position(%i), code = %d\n", positionInst, ret);
        return ret;
    }

    // enable scan2d module
    ret = scan2d->on();
    if (ret)
    {
          GDOS_ERROR("Can't turn on Scan2d(%i), code = %d\n",
                     scan2dInst, ret);
      return ret;
    }

    scan2dDataMbx.clean();

    ret = scan2d->getContData(0, &scan2dDataMbx, &scan2dPeriodTime);
    if (ret)
    {
        GDOS_ERROR("Can't get continuous data from Scan2d(%i), "
                   "code = %d\n", scan2dInst, ret);
        return ret;
    }

    preMode                 = mode;
    globalState             = 0;
    preState                = 0;
    rightRot                = 0;
    leftRot                 = 0;
    subState                = 0;
    omega                   = 0.0f;
    angle                   = 0.0f;
    angleStart              = 0.0f;
    scan2dMsg.data.pointNum = 0;

    globalSpeed             = 0;
    radius                  = 0;

    // set databuffer period time to position
    setDataBufferPeriodTime(scan2dPeriodTime);

    GDOS_PRINT("maxSpeed %f m/s, Scan2d(%i)\n",
                (float)maxSpeed / 1000.0f, scan2dInst);

    return RackDataModule::moduleOn(); // has to be last command in moduleOn();
}

void PilotWallFollowing::moduleOff(void)
{
    RackDataModule::moduleOff();        // has to be first command in moduleOff();

    chassis->moveRadius(0, 0);
    scan2d->stopContData(&scan2dDataMbx);
}

// realtime context
int  PilotWallFollowing::moduleLoop(void)
{
    int          speed = 0;
    int          ret;
    message_info msgInfo;
    scan2d_data  *pS2dData = NULL;
    pilot_data*  pilotData = NULL;



    // get continuous data from scan2d module
    ret = scan2dDataMbx.recvDataMsgTimed(1000000000llu,
                                         &scan2dMsg.data,
                                         sizeof(scan2dMsg),
                                         &msgInfo);
    if (ret)
    {
        GDOS_ERROR("Can't read continuous data from Scan2d(%d), code = %d\n", scan2dInst, ret);
        return ret;
    }

    //scan2d data received
    if (msgInfo.type == MSG_DATA &&
        msgInfo.src  == scan2d->getDestAdr())
    {
        // message parsing
        pS2dData = Scan2dData::parse(&msgInfo);
        memcpy(&scan2dMsg.data, pS2dData, sizeof(scan2d_data) +
               pS2dData->pointNum * sizeof(scan_point));

        GDOS_DBG_DETAIL("mode:%d  globalState:%i preState:%d subState:%d\n", mode, globalState, preState, subState);

        // switch pilot modes
        switch (globalState)
        {
            case STATE_RUNNING:
                switch (mode)
                {
                    case 0:
                        ret = modeWallFollowing(&globalState);
                        break;
                    case 1:
                        ret = modeWallFollowingBraitenberger(&globalState);
                        break;
                    default:
                        mode = 0;
                }
                break;

            case STATE_OBST:
                switch (mode)
                {
                    case 0:
                        ret = modeWallFollowing(&globalState);
                        break;
                    case 1:
                        ret = modeWallFollowingBraitenberger(&globalState);
                        break;
                    default:
                        mode = 0;
                }
                break;

            case STATE_OBST_UNAVOIDABLE:
                switch (mode)
                {
                    case 0:
                        ret = modeWallFollowing(&globalState);
                        break;
                    case 1:
                        ret = modeWallFollowingBraitenberger(&globalState);
                        break;
                    default:
                        mode = 0;
                }
                break;

            default:
                    mode = 0;
                    preMode = 0;
                    globalState = 0;
                    preState = 0;
                    subState = 0;
        }

        if (ret)
        {
            return ret;
        }


        // get datapointer from rackdatabuffer
        pilotData = (pilot_data *)getDataBufferWorkSpace();

        pilotData->recordingTime = scan2dMsg.data.recordingTime;
        memset(&(pilotData->pos), 0, sizeof(pilotData->pos));
        memset(&(pilotData->dest), 0, sizeof(pilotData->dest));
        pilotData->speed     = speed;

        if (speed != 0)
        {
            pilotData->curve     = curve2Radius(0.0f);
        }
        else
        {
            pilotData->curve     = 0.0f;
        }

        pilotData->distanceToDest = -1;
        pilotData->splineNum      = 0;

        putDataBufferWorkSpace(sizeof(pilot_data));
    }

        RackTask::sleep(rackTime.toNano(getDataBufferPeriodTime(0)));
    return 0;
}

int  PilotWallFollowing::moduleCommand(message_info *msgInfo)
{
    // not for me -> ask RackDataModule
    return RackDataModule::moduleCommand(msgInfo);
}


int  PilotWallFollowing::testRec(int x, int y, int xSize, int ySize, scan2d_data *scan)
{
    int   test;
    int   i;
    int   xMiddle;
    int   yMiddle;
    int   xDistance;
    int   yDistance;

    xMiddle = (2 * x + xSize) / 2;
    yMiddle = (2 * y + ySize) / 2;
    xDistance = abs(xSize / 2);
    yDistance = abs(ySize / 2);

    test = 0;

    for (i = 0; i < scan->pointNum; i += 2)
    {
        if (((scan->point[i].type & TYPE_INVALID) == 0) &
           ((scan->point[i].type & TYPE_MASK) != TYPE_LANDMARK))
            if ((abs(scan->point[i].x - xMiddle) <= xDistance) && (abs(scan->point[i].y - yMiddle) <= yDistance))
            {
                test = 1;
                break;
            }
    }

    return test;
}


int  PilotWallFollowing::safeRot(float omega, scan2d_data *scan, chassis_param_data *param)
{
    int test;

    test = 1;

    if (omega > 0)
    {
        if (testRec(0, -param->boundaryLeft, param->boundaryFront + 2 * param->safetyMargin, param->boundaryLeft + param->boundaryRight + 2 * param->safetyMargin, scan) ||
            testRec(0, param->boundaryRight, -param->boundaryBack - 2 * param->safetyMargin, -param->boundaryLeft - param->boundaryRight - 2 * param->safetyMargin, scan))

            test = 0;
    }
    else
    {
        if (testRec(0, param->boundaryRight, param->boundaryFront + 2 * param->safetyMargin, -param->boundaryLeft - param->boundaryRight - 2 * param->safetyMargin, scan) ||
            testRec (0, -param->boundaryLeft, -param->boundaryBack - 2 * param->safetyMargin, param->boundaryLeft + param->boundaryRight +  2 * param->safetyMargin, scan))

            test = 0;
    }

    return test;
}

double  PilotWallFollowing::funDis(int x)
{
    double y;

    y = 1.0 / (float) 10000 * x;

    return y;
}

int PilotWallFollowing::controlSpeed(int oldSpeed)
{
    oldSpeed += chasParData.axMax * getDataBufferPeriodTime(0) / 1000;

    if (oldSpeed > maxSpeed)
        oldSpeed = maxSpeed;

    return oldSpeed;
}


float PilotWallFollowing::funRadius(int x)
{
    float y;

    if(x == 0)
    {
        y = 1;
    }
    else
    {
        x = abs(x);
        y = 1 -(1 - 0.2) * (float)chasParData.minTurningRadius / (float)x;
    }

    return y;
}

float PilotWallFollowing::radiusTest(int splineRadius, float length, scan2d_data *scan, chassis_param_data *param)
{
    float       test;
    int         i;
    int         t;
    float       l;
    float       lMin;
    int         disMin;
    point_2d    centerPos;

    test        = 0.0f;
    t           = 0;
    l           = 0.0f;
    disMin      = 10000;
    centerPos.x = 0;
    centerPos.y = splineRadius;
    lMin        = 0;

    if (splineRadius > 0)
    {
        if ( length > M_PI * splineRadius )
            length = M_PI * splineRadius;

        for (i = 0; i < scan->pointNum; i++)
        {
            if (((scan->point[i].type & TYPE_INVALID) == 0) &
            ((scan->point[i].type & TYPE_MASK) != TYPE_LANDMARK))
            {
                if (scan->point[i].x > 0)
                {
                    t = (int)sqrtf( (float)( (scan->point[i].x - centerPos.x)*(scan->point[i].x - centerPos.x) + (scan->point[i].y - centerPos.y)*(scan->point[i].y - centerPos.y))) - splineRadius;

                    if (scan->point[i].y < splineRadius)
                    {
                        l = atanf( (float)(scan->point[i].x) / (float)(splineRadius - scan->point[i].y) ) * splineRadius;
                    }
                    else if (scan->point[i].y == splineRadius)
                    {
                        l = splineRadius * M_PI / 4.0f;
                    }
                    else
                    {
                        l = splineRadius * (  M_PI - atanf( (float)(scan->point[i].x) / (float)(scan->point[i].y - splineRadius) )    );
                    }

                    if( (l <= length) && (  (t <= (param->boundaryLeft + param->safetyMargin)) && (t >= -(param->boundaryRight + param->safetyMargin))   )      )
                    {
                        test = -1.0f;
                        break;
                    }
                    else
                    {
                        if (l <= length)
                        {
                            if(abs(t) < disMin)
                            {
                                disMin = abs(t);
                                lMin = l;
                            }
                        }
                    }
                }

                if (scan->point[i].x == 0)
                {
                    if(scan->point[i].y <= splineRadius)
                    {
                        l = 0.0f;
                        t = (int)sqrtf( (float)( (scan->point[i].x - centerPos.x)*(scan->point[i].x - centerPos.x) + (scan->point[i].y - centerPos.y)*(scan->point[i].y - centerPos.y))) - splineRadius;
                    }
                    else
                    {
                        l = M_PI * splineRadius;
                        t = (int)sqrtf( (float)( (scan->point[i].x - centerPos.x)*(scan->point[i].x - centerPos.x) + (scan->point[i].y - centerPos.y)*(scan->point[i].y - centerPos.y))) - splineRadius;
                    }

                    if( (l <= length) && (  (t <= (param->boundaryLeft + param->safetyMargin)) && (t >= -(param->boundaryRight + param->safetyMargin))   )      )
                    {
                        test = -1.0f;
                        break;
                    }
                    else
                    {
                        if (l <= length)
                        {
                            if(abs(t) < disMin)
                            {
                                disMin = abs(t);
                                lMin = l;
                            }
                        }
                    }
                }
            }
        }
    }
    else if (splineRadius == 0)
    {
        for (i = 0; i < scan->pointNum; i++)
        {
            if (((scan->point[i].type & TYPE_INVALID) == 0) &
            ((scan->point[i].type & TYPE_MASK) != TYPE_LANDMARK))
            {
                if (scan->point[i].x >= 0)
                {
                    l = scan->point[i].x;
                    t = scan->point[i].y;

                    if( (l <= length) && ((t >= -(param->boundaryLeft + param->safetyMargin)) && (t <= (param->boundaryRight + param->safetyMargin))))
                    {
                        test = -1.0f;
                        break;
                    }
                    else
                    {
                        if (l <= length)
                        {
                            if(abs(t) < disMin)
                            {
                                disMin = abs(t);
                                lMin = l;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        if ( length > M_PI * abs(splineRadius) )
            length = M_PI * abs(splineRadius);

        for (i = 0; i < scan->pointNum; i++)
        {
            if (((scan->point[i].type & TYPE_INVALID) == 0) &
            ((scan->point[i].type & TYPE_MASK) != TYPE_LANDMARK))
            {
                if (scan->point[i].x > 0)
                {
                    t = (int)sqrtf( (float)( (scan->point[i].x - centerPos.x)*(scan->point[i].x - centerPos.x) + (scan->point[i].y - centerPos.y)*(scan->point[i].y - centerPos.y))) + splineRadius;

                    if (scan->point[i].y > splineRadius)
                    {
                        l = - splineRadius * atanf( (float)(scan->point[i].x) / (float)(scan->point[i].y - splineRadius) );
                    }
                    else if (scan->point[i].y == splineRadius)
                    {
                        l = - splineRadius * M_PI / 4.0f;
                    }
                    else
                    {
                        l = - splineRadius * (   M_PI - atanf((float)(scan->point[i].x) / (float)( splineRadius - scan->point[i].y))    );
                    }

                    if( (l <= length) && ((t <= (param->boundaryRight + param->safetyMargin)) && (t >= -(param->boundaryLeft + param->safetyMargin))))
                    {
                        test = -1.0f;
                        break;
                    }
                    else
                    {
                        if (l <= length)
                        {
                            if(abs(t) < disMin)
                            {
                                disMin = abs(t);
                                lMin = l;
                            }
                        }
                    }
                }

                if (scan->point[i].x == 0)
                {
                    if(scan->point[i].y >= splineRadius)
                    {
                        l = 0.0f;
                        t = (int)sqrtf( (float)( (scan->point[i].x - centerPos.x)*(scan->point[i].x - centerPos.x) + (scan->point[i].y - centerPos.y)*(scan->point[i].y - centerPos.y))) + splineRadius;
                    }
                    else
                    {
                        l = - M_PI * splineRadius;
                        t = (int)sqrtf( (float)( (scan->point[i].x - centerPos.x)*(scan->point[i].x - centerPos.x) + (scan->point[i].y - centerPos.y)*(scan->point[i].y - centerPos.y))) + splineRadius;
                    }

                    if( (l <= length) && ((t <= (param->boundaryRight + param->safetyMargin)) && (t >= -(param->boundaryLeft + param->safetyMargin))))
                    {
                        test = -1.0f;
                        break;
                    }
                    else
                    {
                        if (l <= length)
                        {
                            if(abs(t) < disMin)
                            {
                                disMin = abs(t);
                                lMin = l;
                            }
                        }
                    }
                }
            }
        }
    }


    if (test != -1.0f)
    {
        test = (float)funDis(disMin);
        test = test * funRadius(splineRadius);
    }

    return test;
}

int  PilotWallFollowing::modeWallFollowing(int* state)
{
    int             ret;
    int             i;
    position_data   positionData;
    int             tempRange = 0;
    int             pointX = 0;
    int             pointY = 0;
    float           a = 0;
    float           curve = 0;


    globalSpeed = controlSpeed(globalSpeed);

    switch (*state)
    {
        case 0:
            switch (subState)
            {
                case 0:
                    tempRange = scan2dMsg.data.maxRange;
                    for (i = 0;i < scan2dMsg.data.pointNum;i++)
                    {
                        if ((scan2dMsg.point[i].z < tempRange) && scan2dMsg.point[i].y >= 0)
                        {
                            subState = 1;
                            break;
                        }
                        else
                        {
                            subState = 2;
                        }
                    }
                    break;

                case 1:
                    // find nearest point
                    tempRange = scan2dMsg.data.maxRange;
                    for (i = 0;i < scan2dMsg.data.pointNum;i++)
                    {
                        if ((scan2dMsg.point[i].z < tempRange) && (scan2dMsg.point[i].y >= 0))
                        {
                            tempRange = scan2dMsg.point[i].z;
                            pointX = scan2dMsg.point[i].x;
                            pointY = scan2dMsg.point[i].y;

                        }
                    }

                    GDOS_DBG_DETAIL("distance %d tempRange %d pointX %d pointY %d\n", distance, tempRange, pointX, pointY);

                    a = chasParData.pilotParameterA * (tempRange - distance);

                    if (a > 30.0 * M_PI/180.0)
                    {
                        a = 30.0 * M_PI/180.0;
                    }
                    if (a < -30.0 * M_PI/180.0)
                    {
                        a = -30.0 * M_PI/180.0;
                    }

                    if (pointY == 0)
                    {
                        if (pointX >0)
                        {
                            omega = -omegaMaxRad;
                            angle = -M_PI / 2;
                        }
                        else
                        {
                            omega = omegaMaxRad;
                            angle = M_PI / 2;
                        }

                        // get current position from position
                        ret = position->getData(&positionData, sizeof(position_data), 0);
                        if (ret)
                        {
                            GDOS_ERROR("Can_t get data from Position, code = %d\n", ret);
                            return ret;
                        }

                        angleStart = positionData.pos.rho;
                        subState = 4;
                    }
                    else
                    {
                        if (pointX == 0)
                        {
                            angle = 0.0f;
                            omega = chasParData.pilotParameterB * a;
                            radius = 0;
                        }
                        else if (pointX < 0)
                        {
                            angle = (float)atan2(abs(pointX), abs(pointY));
                            omega = chasParData.pilotParameterB * (angle + a);

                            curve = omega / (float)abs(globalSpeed);

                            // limit curve value
                            if (curve > 1.0f / (float)chasParData.minTurningRadius)
                            {
                                curve = 1.0f / (float)chasParData.minTurningRadius;
                            }
                            if (curve < -1.0f / (float)chasParData.minTurningRadius)
                            {
                                curve = -1.0f / (float)chasParData.minTurningRadius;
                            }

                            radius = curve2Radius(curve);
                        }
                        else
                        {
                            angle = -(float)atan2(abs(pointX), abs(pointY));
                            omega = chasParData.pilotParameterB * (angle + a);

                            curve = omega / (float)abs(globalSpeed);

                            // limit curve value
                            if (curve > 1.0f / (float)chasParData.minTurningRadius)
                            {
                                curve = 1.0f / (float)chasParData.minTurningRadius;
                            }
                            if (curve < -1.0f / (float)chasParData.minTurningRadius)
                            {
                                curve = -1.0f / (float)chasParData.minTurningRadius;
                            }

                            radius = curve2Radius(curve);

                        }

                        GDOS_DBG_DETAIL("angle %a a %a radius %d\n", angle, a, radius);

                        if (omega > omegaMaxRad)
                            omega = omegaMaxRad;
                        if (omega < -omegaMaxRad)
                            omega = -omegaMaxRad;

                         subState = 3;
                    }

                    break;

                case 2:
                    // find nearest point
                    tempRange = scan2dMsg.data.maxRange;
                    for (i = 0;i < scan2dMsg.data.pointNum;i++)
                    {
                        if (scan2dMsg.point[i].z < tempRange)
                        {
                            tempRange = scan2dMsg.point[i].z;
                            pointX = scan2dMsg.point[i].x;
                            pointY = scan2dMsg.point[i].y;

                        }
                    }

                    if (tempRange == scan2dMsg.data.maxRange)
                    {
                        globalSpeed  = safeSpeed(globalSpeed, 0, NULL,
                        &scan2dMsg.data, &chasParDataTransForward);


                        ret = chassis->move(globalSpeed, 0, 0);
                        if (ret)
                        {
                            GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                            return ret;
                        }

                        subState = 0;
                    }
                    else
                    {
                        // stop move
                        ret = chassis->move(0, 0, 0);
                        if (ret)
                        {
                            GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                            return ret;
                        }

                        if (pointX == 0)
                        {
                            angle = M_PI;
                            omega = omegaMaxRad;
                        }
                        else if (pointX < 0)
                        {
                            angle = M_PI - (float)atan2(abs(pointX), abs(pointY));
                            omega = omegaMaxRad;
                        }
                        else
                        {
                            angle = (float)atan2(abs(pointX), abs(pointY)) - M_PI;
                            omega = -omegaMaxRad;
                        }

                        // get current position from position
                        ret = position->getData(&positionData, sizeof(position_data), 0);
                        if (ret)
                        {
                            GDOS_ERROR("Can_t get data from Position, code = %d\n", ret);
                            return ret;
                        }

                        angleStart = positionData.pos.rho;
                        subState = 4;

                    }

                    break;

                case 3:
                    globalSpeed  = safeSpeed(globalSpeed, radius, NULL,
                    &scan2dMsg.data, &chasParData);

                    if (globalSpeed < chasParData.vxMin)
                    {
                        // stop move
                        ret = chassis->move(0, 0, 0);
                        if (ret)
                        {
                            GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                            return ret;
                        }

                        *state = 1;
                    }
                    else
                    {
                        if (angle == 0)
                        {
                            if (safeRot(omega, &scan2dMsg.data, &chasParData) == 0)
                            omega = 0;

                            GDOS_DBG_DETAIL("move speed %d omega %a\n", globalSpeed, omega);

                            ret = chassis->move(globalSpeed, 0, omega);
                            if (ret)
                            {
                                GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                                return ret;
                            }


                        }
                        else
                        {

                            // move chassis
                            GDOS_DBG_DETAIL("move speed %d radius %d\n", globalSpeed, radius);

                            ret = chassis->moveRadius(globalSpeed, radius);
                            if (ret)
                            {
                                GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                                return ret;
                            }
                        }

                    }

                    subState =0;

                    break;

                case 4:
                    // get current position from position
                    ret = position->getData(&positionData, sizeof(position_data), 0);
                    if (ret)
                    {
                            GDOS_ERROR("Can_t get data from Position, code = %d\n", ret);
                            return ret;
                    }

                    if (((angle >= 0 ) && ( normaliseAngleSym0(positionData.pos.rho - angleStart ) >= angle )) ||
                    ((angle  < 0 ) && ( normaliseAngleSym0(positionData.pos.rho - angleStart ) <= angle )) ||
                    (safeRot(omega, &scan2dMsg.data, &chasParData) == 0))

                    {
                        globalSpeed = 0;

                        // stop chassis
                        ret = chassis->move(0, 0, 0.0f);
                        if (ret)
                        {
                            GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                            return ret;
                        }

                        if ((safeRot(omega, &scan2dMsg.data, &chasParData) == 0))
                        {
                            *state = 2;
                            subState = 0;
                        }
                        else
                            subState = 0;

                        omega = 0;
                    }


                    // turn chassis
                    ret = chassis->move(0, 0, omega);
                    if (ret)
                    {
                        GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                        return ret;
                    }

                    break;

                default:
                    subState = 0;
            }

            preState = 0;

            break;

        case 1:
            switch(subState)
            {
                case 0:
                     // find nearest point
                    tempRange = scan2dMsg.data.maxRange;
                    for (i = 0;i < scan2dMsg.data.pointNum;i++)
                    {
                        if (scan2dMsg.point[i].z < tempRange)
                        {
                            tempRange = scan2dMsg.point[i].z;
                            pointX = scan2dMsg.point[i].x;
                            pointY = scan2dMsg.point[i].y;

                        }
                    }

                    if (pointY == 0)
                    {
                        if (pointX > 0)
                        {
                            angle = -M_PI / 2.0f;
                            omega = -omegaMaxRad;
                        }
                        else
                        {
                            angle = M_PI / 2.0f;
                            omega = omegaMaxRad;
                        }
                    }
                    else if (pointY > 0)
                    {
                        if (pointX < 0)
                        {
                            angle = (float)atan2(abs(pointX), abs(pointY));
                            omega = omegaMaxRad;
                        }
                        else
                        {
                            angle = -(float)atan2(abs(pointX), abs(pointY));
                            omega = -omegaMaxRad;
                        }
                    }
                    else
                    {
                        if (pointX < 0)
                        {
                            angle = M_PI - (float)atan2(abs(pointX), abs(pointY));
                            omega = omegaMaxRad;
                        }
                        else
                        {
                            angle = (float)atan2(abs(pointX), abs(pointY)) - M_PI;
                            omega = -omegaMaxRad;
                        }
                    }

                    // get current position from position
                    ret = position->getData(&positionData, sizeof(position_data), 0);
                    if (ret)
                    {
                        GDOS_ERROR("Can_t get data from Position, code = %d\n", ret);
                        return ret;
                    }

                    angleStart = positionData.pos.rho;
                    subState = 1;

                    break;

                case 1:
                    // get current position from position
                    ret = position->getData(&positionData, sizeof(position_data), 0);
                    if (ret)
                    {
                            GDOS_ERROR("Can_t get data from Position, code = %d\n", ret);
                            return ret;
                    }

                    if (((angle >= 0 ) && ( normaliseAngleSym0(positionData.pos.rho - angleStart ) >= angle )) ||
                    ((angle  < 0 ) && ( normaliseAngleSym0(positionData.pos.rho - angleStart ) <= angle )) ||
                    (safeRot(omega, &scan2dMsg.data, &chasParData) == 0))

                    {
                        globalSpeed = 0;
                        // stop chassis
                        ret = chassis->move(0, 0, 0.0f);
                        if (ret)
                        {
                            GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                            return ret;
                        }

                        if ((safeRot(omega, &scan2dMsg.data, &chasParData) == 0))
                        {
                            *state = 2;
                            subState = 0;
                        }
                        else
                        {
                            *state = 0;
                            subState = 0;
                        }
                        omega = 0;
                    }


                    // turn chassis
                    ret = chassis->move(0, 0, omega);
                    if (ret)
                    {
                        GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                        return ret;
                    }

                    break;
                default:
                    subState = 0;
            }

            preState = 1;

            break;

        case 2:
            // stop chassis
            ret = chassis->move(0, 0, 0);
            if (ret)
            {
                GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                 return ret;
            }

            globalSpeed  = safeSpeed(globalSpeed, 0, NULL,
            &scan2dMsg.data, &chasParDataTransForward);

            if (globalSpeed > chasParData.vxMin)
            {
                *state = 0;
                preState = 2;
                subState = 0;
                globalSpeed = 0;
            }
            else
            {
                *state = 2;
                preState = 2;
                globalSpeed = 0;
            }
            break;

        default:
            *state =0;
            subState =0;
            preState = 0;


    }

    return 0;
}


int  PilotWallFollowing::modeWallFollowingBraitenberger(int* state)
{
    int             ret;
    int             i;
    position_data   positionData;
    int             rangeL = 0;
    int             rangeR = 0;
    int             rangeLF = 0;
    int             rangeRF = 0;
    int             tempRange = 0;
    int             pointRX = 0;
    int             pointRY = 0;
    int             pointLX = 0;
    int             pointLY = 0;
    int             pointRFX = 0;
    int             pointRFY = 0;
    int             pointLFX = 0;
    int             pointLFY = 0;
    int             pointX = 0;
    int             pointY = 0;
    float           angleL = 0;
    float           angleR = 0;
    float           aL = 0;
    float           aR = 0;
    float           a = 0;
    float           curve = 0;

    globalSpeed = controlSpeed(globalSpeed);

    switch(*state)
    {
        case 0:
            switch(subState)
            {
                case 0:
                    globalSpeed  = safeSpeed(globalSpeed, 0, NULL,
                    &scan2dMsg.data, &chasParDataTransForward);

                    if ((globalSpeed < chasParData.vxMin) || testRec(chasParData.boundaryFront, -chasParData.boundaryLeft - chasParData.safetyMargin, testDis, chasParData.boundaryLeft + chasParData.boundaryRight + 2 * chasParData.safetyMargin, &scan2dMsg.data))
                    {
                        // stop chassis
                        ret = chassis->move(0, 0, 0);
                        if (ret)
                        {
                            GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                            return ret;
                        }

                        globalSpeed = 0;
                        *state = 1;
                        subState = 0;
                    }
                    else
                    {

                        tempRange = scan2dMsg.data.maxRange;
                        rangeLF = scan2dMsg.data.maxRange;
                        rangeRF = scan2dMsg.data.maxRange;

                        for (i = 0;i < scan2dMsg.data.pointNum;i++)
                        {
                            if (scan2dMsg.point[i].z < tempRange)
                            {
                                tempRange = scan2dMsg.point[i].z;
                                pointX = scan2dMsg.point[i].x;
                                pointY = scan2dMsg.point[i].y;
                            }

                            if ((scan2dMsg.point[i].x >= -chasParData.boundaryBack) && (scan2dMsg.point[i].y > 0) && (scan2dMsg.point[i].z < rangeRF))
                            {
                                rangeRF = scan2dMsg.point[i].z;
                                pointRFX = scan2dMsg.point[i].x;
                                pointRFY = scan2dMsg.point[i].y;
                            }

                            if ((scan2dMsg.point[i].x >= -chasParData.boundaryBack) && (scan2dMsg.point[i].y < 0) && (scan2dMsg.point[i].z < rangeLF))
                            {
                                rangeLF = scan2dMsg.point[i].z;
                                pointLFX = scan2dMsg.point[i].x;
                                pointLFY = scan2dMsg.point[i].y;
                            }
                        }

                        if (tempRange == scan2dMsg.data.maxRange)
                        {
                            rightRot = 0;
                            leftRot = 0;
                            subState = 1;
                        }
                        else if ((rangeRF <= distance) && (rangeLF <= distance))
                        {
                            rightRot = 0;
                            leftRot = 0;
                            subState = 2;
                        }
                        else if ((rangeRF <= distance) && (rangeLF > distance))
                        {
                            rightRot = 0;
                            leftRot = 1;
                            subState = 3;
                        }
                        else if ((rangeRF > distance) && (rangeLF <= distance))
                        {
                            rightRot = 1;
                            leftRot = 0;
                            subState = 4;
                        }
                        else
                        {
                            if (pointY >= 0)
                            {
                                subState = 3;
                                rightRot = 0;
                                leftRot = 1;
                            }
                            else
                            {
                                subState = 4;
                                rightRot = 1;
                                leftRot = 0;
                            }

                        }
                    }

                    break;

                case 1:
                    tempRange = scan2dMsg.data.maxRange;
                    for (i = 0;i < scan2dMsg.data.pointNum;i++)
                    {
                        if (scan2dMsg.point[i].z < scan2dMsg.data.maxRange)
                        {
                           tempRange = scan2dMsg.point[i].z;
                           break;
                        }
                    }

                    if (tempRange < scan2dMsg.data.maxRange)
                    {
                        subState = 0;
                    }
                    else
                    {
                        globalSpeed  = safeSpeed(globalSpeed, 0, NULL,
                        &scan2dMsg.data, &chasParDataTransForward);

                        ret = chassis->move(globalSpeed, 0, 0);
                        if (ret)
                        {
                            GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                            return ret;
                        }
                    }

                    break;

                case 2:
                    rangeLF = scan2dMsg.data.maxRange;
                    rangeRF = scan2dMsg.data.maxRange;

                    for (i = 0;i < scan2dMsg.data.pointNum;i++)
                    {
                        if ((scan2dMsg.point[i].x >= -chasParData.boundaryBack) && (scan2dMsg.point[i].y > 0) && (scan2dMsg.point[i].z < rangeRF))
                        {
                            rangeRF = scan2dMsg.point[i].z;
                            pointRFX = scan2dMsg.point[i].x;
                            pointRFY = scan2dMsg.point[i].y;
                        }

                        if ((scan2dMsg.point[i].x >= -chasParData.boundaryBack) && (scan2dMsg.point[i].y < 0) && (scan2dMsg.point[i].z < rangeLF))
                        {
                            rangeLF = scan2dMsg.point[i].z;
                            pointLFX = scan2dMsg.point[i].x;
                            pointLFY = scan2dMsg.point[i].y;
                        }
                    }

                    if (testRec(chasParData.boundaryFront, -chasParData.boundaryLeft - chasParData.safetyMargin, testDis, chasParData.boundaryLeft + chasParData.boundaryRight + 2 * chasParData.safetyMargin, &scan2dMsg.data))
                    {
                        ret = chassis->move(0, 0, 0);
                        if (ret)
                        {
                            GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                            return ret;
                        }

                        globalSpeed = 0;
                        *state = 1;
                        subState = 0;
                    }
                    else if ((rangeLF <= distance) && (rangeRF <= distance))
                    {
                        aR =  chasParData.pilotParameterA * (rangeRF - distance);
                        aL = -chasParData.pilotParameterA * (rangeLF - distance);
                        a = (aR + aL) / 2;

                        if (a > 30.0 * M_PI/180.0)
                        {
                            a = 30.0 * M_PI/180.0;
                        }
                        if (a < -30.0 * M_PI/180.0)
                        {
                            a = -30.0 * M_PI/180.0;
                        }

                        if (pointRFX < 0)
                            angleR = atan2f(abs(pointRFX), abs(pointRFY));
                        else
                            angleR = - atan2f(abs(pointRFX), abs(pointRFY));


                        if (pointLFX < 0)
                            angleL = - atan2f(abs(pointLFX), abs(pointLFY));
                        else
                            angleL = atan2f(abs(pointLFX), abs(pointLFY));

                        angle = (angleR + angleL) / 2.0;


                        omega = chasParData.pilotParameterB * (angle + a);

                        if (omega > omegaMaxRad)
                            omega = omegaMaxRad;
                        if (omega < -omegaMaxRad)
                            omega = -omegaMaxRad;

                        curve = omega / (float)abs(globalSpeed);
                        // limit curve value
                        if (curve > 1.0f / (float)chasParData.minTurningRadius)
                        {
                            curve = 1.0f / (float)chasParData.minTurningRadius;
                        }
                        if (curve < -1.0f / (float)chasParData.minTurningRadius)
                        {
                            curve = -1.0f / (float)chasParData.minTurningRadius;
                        }

                        radius = curve2Radius(curve);


                        globalSpeed  = safeSpeed(globalSpeed, radius, NULL,
                        &scan2dMsg.data, &chasParData);

                        if (globalSpeed < chasParData.vxMin)
                        {
                            // stop chassis
                            ret = chassis->move(0, 0, 0);
                            if (ret)
                            {
                                GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                                return ret;
                            }

                            *state = 1;
                            subState = 0;
                        }
                        else
                        {
                            ret = chassis->moveRadius(globalSpeed, radius);
                            if (ret)
                            {
                                GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                                return ret;
                            }
                        }
                    }
                    else
                    {
                        subState = 0;
                    }

                    break;

                case 3:
                    rangeLF = scan2dMsg.data.maxRange;
                    rangeR = scan2dMsg.data.maxRange;

                    for (i = 0;i < scan2dMsg.data.pointNum;i++)
                    {
                        if ((scan2dMsg.point[i].y > 0) && (scan2dMsg.point[i].z < rangeR))
                        {
                            rangeR = scan2dMsg.point[i].z;
                            pointRX = scan2dMsg.point[i].x;
                            pointRY = scan2dMsg.point[i].y;
                        }

                        if ((scan2dMsg.point[i].x >= -chasParData.boundaryBack) && (scan2dMsg.point[i].y < 0) && (scan2dMsg.point[i].z < rangeLF))
                        {
                            rangeLF = scan2dMsg.point[i].z;
                            pointLFX = scan2dMsg.point[i].x;
                            pointLFY = scan2dMsg.point[i].y;
                        }
                    }

                    if (testRec(chasParData.boundaryFront, -chasParData.boundaryLeft - chasParData.safetyMargin, testDis, chasParData.boundaryLeft + chasParData.boundaryRight + 2 * chasParData.safetyMargin, &scan2dMsg.data))
                    {
                        ret = chassis->move(0, 0, 0);
                        if (ret)
                        {
                            GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                            return ret;
                        }

                        globalSpeed = 0;
                        *state = 1;
                        subState = 0;
                    }
                    else if ((rangeLF < rangeR) || (rangeLF <= distance) || (rangeR == scan2dMsg.data.maxRange))
                    {
                        subState = 0;
                    }
                    else
                    {
                        a = chasParData.pilotParameterA * (rangeR - distance);

                        if (a > 30.0 * M_PI/180.0)
                        {
                            a = 30.0 * M_PI/180.0;
                        }
                        if (a < -30.0 * M_PI/180.0)
                        {
                            a = -30.0 * M_PI/180.0;
                        }

                        if (pointRX < 0)
                            angleR = atan2f(abs(pointRX), abs(pointRY));
                        else
                            angleR = - atan2f(abs(pointRX), abs(pointRY));

                        omega = chasParData.pilotParameterB * (angleR + a);

                        if (omega > omegaMaxRad)
                            omega = omegaMaxRad;
                        if (omega < -omegaMaxRad)
                            omega = -omegaMaxRad;

                        curve = omega / (float)abs(globalSpeed);
                        // limit curve value
                        if (curve > 1.0f / (float)chasParData.minTurningRadius)
                        {
                            curve = 1.0f / (float)chasParData.minTurningRadius;
                        }
                        if (curve < -1.0f / (float)chasParData.minTurningRadius)
                        {
                            curve = -1.0f / (float)chasParData.minTurningRadius;
                        }

                        radius = curve2Radius(curve);


                        globalSpeed  = safeSpeed(globalSpeed, radius, NULL,
                        &scan2dMsg.data, &chasParData);

                        if (globalSpeed < chasParData.vxMin)
                        {
                            // stop chassis
                            ret = chassis->move(0, 0, 0);
                            if (ret)
                            {
                                GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                                return ret;
                            }

                            *state = 1;
                            subState = 0;
                        }
                        else
                        {
                            ret = chassis->moveRadius(globalSpeed, radius);
                            if (ret)
                            {
                                GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                                return ret;
                            }
                        }
                    }

                    break;

                case 4:
                    rangeL = scan2dMsg.data.maxRange;
                    rangeRF = scan2dMsg.data.maxRange;

                    for (i = 0;i < scan2dMsg.data.pointNum;i++)
                    {
                        if ((scan2dMsg.point[i].x >= -chasParData.boundaryBack) && (scan2dMsg.point[i].y > 0) && (scan2dMsg.point[i].z < rangeRF))
                        {
                            rangeRF = scan2dMsg.point[i].z;
                            pointRFX = scan2dMsg.point[i].x;
                            pointRFY = scan2dMsg.point[i].y;
                        }

                        if ((scan2dMsg.point[i].y < 0) && (scan2dMsg.point[i].z < rangeL))
                        {
                            rangeL = scan2dMsg.point[i].z;
                            pointLX = scan2dMsg.point[i].x;
                            pointLY = scan2dMsg.point[i].y;
                        }
                    }

                    if (testRec(chasParData.boundaryFront, -chasParData.boundaryLeft - chasParData.safetyMargin, testDis, chasParData.boundaryLeft + chasParData.boundaryRight + 2 * chasParData.safetyMargin, &scan2dMsg.data))
                    {
                        ret = chassis->move(0, 0, 0);
                        if (ret)
                        {
                            GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                            return ret;
                        }

                        globalSpeed = 0;
                        *state = 1;
                        subState = 0;
                    }
                    else if ((rangeRF < rangeL) || (rangeRF <= distance) || (rangeL == scan2dMsg.data.maxRange))
                    {
                        subState = 0;
                    }
                    else
                    {
                        a = - chasParData.pilotParameterA * (rangeL - distance);

                        if (a > 30.0 * M_PI/180.0)
                        {
                            a = 30.0 * M_PI/180.0;
                        }
                        if (a < -30.0 * M_PI/180.0)
                        {
                            a = -30.0 * M_PI/180.0;
                        }

                        if (pointLX <= 0)
                            angleL = - atan2f(abs(pointLX), abs(pointLY));
                        else
                            angleL = atan2f(abs(pointLX), abs(pointLY));

                        omega = chasParData.pilotParameterB * (angleL + a);

                        if (omega > omegaMaxRad)
                            omega = omegaMaxRad;
                        if (omega < -omegaMaxRad)
                            omega = -omegaMaxRad;

                        curve = omega / (float)abs(globalSpeed);
                        // limit curve value
                        if (curve > 1.0f / (float)chasParData.minTurningRadius)
                        {
                            curve = 1.0f / (float)chasParData.minTurningRadius;
                        }
                        if (curve < -1.0f / (float)chasParData.minTurningRadius)
                        {
                            curve = -1.0f / (float)chasParData.minTurningRadius;
                        }

                        radius = curve2Radius(curve);


                        globalSpeed  = safeSpeed(globalSpeed, radius, NULL,
                        &scan2dMsg.data, &chasParData);

                        if (globalSpeed < chasParData.vxMin)
                        {
                            // stop chassis
                            ret = chassis->move(0, 0, 0);
                            if (ret)
                            {
                                GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                                return ret;
                            }

                            *state = 1;
                            subState = 0;
                        }
                        else
                        {
                            ret = chassis->moveRadius(globalSpeed, radius);
                            if (ret)
                            {
                                GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                                return ret;
                            }
                        }
                    }

                    break;

                default:
                    subState = 0;

            }
            preState = 0;

            break;

        case 1:
            switch(subState)
            {
                case 0:
                    // find nearest point
                    tempRange = scan2dMsg.data.maxRange;
                    rangeL = scan2dMsg.data.maxRange;
                    rangeR = scan2dMsg.data.maxRange;

                    for (i = 0;i < scan2dMsg.data.pointNum;i++)
                    {
                        if ((scan2dMsg.point[i].z < tempRange) && ( scan2dMsg.point[i].x > chasParData.boundaryFront))
                        {
                            tempRange = scan2dMsg.point[i].z;
                            pointX = scan2dMsg.point[i].x;
                            pointY = scan2dMsg.point[i].y;

                            if( (scan2dMsg.point[i].x < chasParData.boundaryFront) && (scan2dMsg.point[i].x > - chasParData.boundaryBack))
                            {
                                if( (scan2dMsg.point[i].y > 0) && (scan2dMsg.point[i].z < rangeR)                        )
                                    rangeR = scan2dMsg.point[i].z;

                                if( (scan2dMsg.point[i].y < 0) && (scan2dMsg.point[i].z < rangeL)                        )
                                    rangeL = scan2dMsg.point[i].z;
                            }
                        }
                    }

                    if ((rightRot == 0) && (leftRot == 0))
                    {
                        if (pointY == 0)
                        {
                            if (rangeR > rangeL)
                            {
                                angle = M_PI / 2.0f;
                                omega = omegaMaxRad;
                                rightRot = 1;
                            }
                            else
                            {
                                angle = - M_PI / 2.0f;
                                omega = - omegaMaxRad;
                                leftRot = 1;
                            }
                        }
                        else if (pointY > 0)
                        {
                            angle = -(float)atan2(abs(pointX), abs(pointY));
                            omega = -omegaMaxRad;
                            leftRot = 1;
                        }
                        else
                        {
                            angle = (float)atan2(abs(pointX), abs(pointY));
                            omega = omegaMaxRad;
                            rightRot = 1;
                        }
                    }
                    else if(rightRot == 1)
                    {
                        angle = M_PI / 2.0f;
                        omega = omegaMaxRad;
                    }
                    else
                    {
                        angle = - M_PI / 2.0f;
                        omega = - omegaMaxRad;
                    }


                    // get current position from position
                    ret = position->getData(&positionData, sizeof(position_data), 0);
                    if (ret)
                    {
                        GDOS_ERROR("Can_t get data from Position, code = %d\n", ret);
                        return ret;
                    }

                    angleStart = positionData.pos.rho;
                    subState = 1;

                    break;

                case 1:
                    // get current position from position
                    ret = position->getData(&positionData, sizeof(position_data), 0);
                    if (ret)
                    {
                            GDOS_ERROR("Can_t get data from Position, code = %d\n", ret);
                            return ret;
                    }

                    if (((angle >= 0 ) && ( normaliseAngleSym0(positionData.pos.rho - angleStart ) >= angle )) ||
                    ((angle  < 0 ) && ( normaliseAngleSym0(positionData.pos.rho - angleStart ) <= angle )) ||
                    (safeRot(omega, &scan2dMsg.data, &chasParData) == 0) || !testRec(chasParData.boundaryFront, -chasParData.boundaryLeft - chasParData.safetyMargin, (int)(1.5 * testDis), chasParData.boundaryLeft + chasParData.boundaryRight + 2 * chasParData.safetyMargin, &scan2dMsg.data) )

                    {
                        // stop chassis
                        ret = chassis->move(0, 0, 0.0f);
                        if (ret)
                        {
                            GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                            return ret;
                        }

                        if ((safeRot(omega, &scan2dMsg.data, &chasParData) == 0))
                        {
                            *state = 2;
                            subState = 0;
                        }
                        else
                        {
                            *state = 0;
                            subState = 0;
                        }

                        omega = 0;
                    }

                    // turn chassis
                    ret = chassis->move(0, 0, omega);
                    if (ret)
                    {
                        GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                        return ret;
                    }

                    globalSpeed = 0;

                    break;

                default:
                    subState = 0;
            }

            preState = 1;

            break;

        case 2:
            // stop chassis
            ret = chassis->move(0, 0, 0);
            if (ret)
            {
                GDOS_ERROR("Can't send chassis_move, code = %d\n", ret);
                return ret;
            }

            globalSpeed  = safeSpeed(globalSpeed, 0, NULL,
            &scan2dMsg.data, &chasParDataTransForward);

            if (globalSpeed > chasParData.vxMin)
            {
                *state = 0;
                preState = 2;
                subState = 0;
                globalSpeed = 0;
            }
            else
            {
                globalSpeed = 0;
                *state = 2;
                preState = 2;
            }
            break;

        default:
            *state =0;
            subState =0;
            preState = 0;

    }

   return 0;
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

int  PilotWallFollowing::moduleInit(void)
{
    int ret;

    // call RackDataModule init function (first command in init)
    ret = RackDataModule::moduleInit();
    if (ret)
    {
        return ret;
    }
    initBits.setBit(INIT_BIT_DATA_MODULE);

    //
    // create mailboxes
    //

    // scan2d
    ret = createMbx(&scan2dDataMbx, 2, sizeof(scan2d_data_msg),
                    MBX_IN_KERNELSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_SCAN2D);

    // work mailbox
    // -> gets chassis parameter package
    // -> gets joystick data ...
    ret = createMbx(&workMbx, 10, sizeof(chassis_param_data),
                    MBX_IN_KERNELSPACE | MBX_SLOT);
    if (ret)
    {
        goto init_error;
    }
    initBits.setBit(INIT_BIT_MBX_WORK);

    //
    // create Proxys
    //

    // scan2d
    scan2d = new Scan2dProxy(&workMbx, 0, scan2dInst);
    if (!scan2d)
    {
        ret = -ENOMEM;
        goto init_error;
    }
    initBits.setBit(INIT_BIT_PROXY_SCAN2D);

    // position
    position = new PositionProxy(&workMbx, 0, positionInst);
    if (!position)
    {
        ret = -ENOMEM;
        goto init_error;
    }
    initBits.setBit(INIT_BIT_PROXY_POSITION);

    // chassis
    chassis = new ChassisProxy(&workMbx, 0, chassisInst);
    if (!chassis)
    {
        ret = -ENOMEM;
        goto init_error;
    }
    initBits.setBit(INIT_BIT_PROXY_CHASSIS);

    return 0;

init_error:
    moduleCleanup();
    return ret;
}

void PilotWallFollowing::moduleCleanup(void)
{
    // call RackDataModule cleanup function
    if (initBits.testAndClearBit(INIT_BIT_DATA_MODULE))
    {
        RackDataModule::moduleCleanup();
    }

    //
    // free proxies
    //

    if (initBits.testAndClearBit(INIT_BIT_PROXY_CHASSIS))
    {
        delete chassis;
    }

    if (initBits.testAndClearBit(INIT_BIT_PROXY_POSITION))
    {
        delete position;
    }

    if (initBits.testAndClearBit(INIT_BIT_PROXY_SCAN2D))
    {
        delete scan2d;
    }


    //
    // delete mailboxes
    //

    if (initBits.testAndClearBit(INIT_BIT_MBX_WORK))
    {
        destroyMbx(&workMbx);
    }

    if (initBits.testAndClearBit(INIT_BIT_MBX_SCAN2D))
    {
        destroyMbx(&scan2dDataMbx);
    }
}

PilotWallFollowing::PilotWallFollowing()
      : RackDataModule( MODULE_CLASS_ID,
                    5000000000llu,    // 5s datatask error sleep time
                    16,               // command mailbox slots
                    48,               // command mailbox data size per slot
                    MBX_IN_KERNELSPACE | MBX_SLOT,  // command mailbox flags
                    5,                // max buffer entries
                    10)               // data buffer listener
{
    // get value(s) out of your argument table
    chassisInst  = getIntArg("chassisInst", argTab);
    scan2dInst   = getIntArg("scan2dInst", argTab);
    positionInst = getIntArg("positionInst", argTab);
    maxSpeed     = getIntArg("maxSpeed", argTab);
    omegaMax     = getIntArg("omegaMax", argTab);
    mode         = getIntArg("mode", argTab);

    omegaMaxRad  = (float)(omegaMax * M_PI / 180.0f);
    distance     = getIntArg("distance", argTab);
    testDis      = getIntArg("testDis", argTab);

    // set dataBuffer size
    setDataBufferMaxDataSize(sizeof(pilot_data_msg));
}


int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "PilotWallFollowing");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new PilotWallFollowing
    p_inst = new PilotWallFollowing();
    if (!p_inst)
    {
        printf("Can't create new PilotWallFollowing -> EXIT\n");
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
