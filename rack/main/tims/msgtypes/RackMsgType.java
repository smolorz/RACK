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
package rack.main.tims.msgtypes;

public class RackMsgType
{

    // global commands (positive)
    public static final byte MSG_ON = 1;
    public static final byte MSG_OFF = 2;
    public static final byte MSG_GET_STATUS = 3;
    public static final byte MSG_GET_DATA = 4;
    public static final byte MSG_GET_CONT_DATA = 5;
    public static final byte MSG_STOP_CONT_DATA = 6;

    // TODO
    public static final byte MSG_SET_LOG_LEVEL = 7;

    // TODO
    public static final byte MSG_GET_PERIOD_TIME = 8;

    // global returns (negative)
    public static final byte MSG_OK = 0;
    public static final byte MSG_ERROR = -1;
    public static final byte MSG_TIMEOUT = -2;
    public static final byte MSG_NOT_AVAILABLE = -3;
    public static final byte MSG_ENABLED = -4;
    public static final byte MSG_DISABLED = -5;
    public static final byte MSG_DATA = -6;
    public static final byte MSG_CONT_DATA = -7;

    public static final byte RACK_PROXY_MSG_POS_OFFSET = 20;
    public static final byte RACK_PROXY_MSG_NEG_OFFSET = -20;

    /** gibt einen String des Message-Typs zurueck */
    public static String toString(byte type)
    {
        switch (type)
        {
            case MSG_ON:
                return ("on   ");
            case MSG_OFF:
                return ("off  ");
            case MSG_GET_STATUS:
                return ("gstat");
            case MSG_GET_DATA:
                return ("gdata");
            case MSG_GET_CONT_DATA:
                return ("gcdat");
            case MSG_STOP_CONT_DATA:
                return ("scdat");
            case MSG_OK:
                return ("ok   ");
            case MSG_ERROR:
                return ("error");
            case MSG_TIMEOUT:
                return ("tmout");
            case MSG_NOT_AVAILABLE:
                return ("notav");
            case MSG_ENABLED:
                return ("enbld");
            case MSG_DISABLED:
                return ("dsbld");
            case MSG_DATA:
                return ("data ");
            default:
                return (Integer.toString(type));
        }
    }
}
