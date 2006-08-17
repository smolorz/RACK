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
package rack.main.debug;

import rack.main.naming.*;
import rack.main.tims.Tims;
import rack.main.tims.exceptions.*;

public class GDOS
{
    /** GDOS message type PRINTOUT (HIGHEST LEVEL) */
    public static final byte PRINT      = -124;
    /** GDOS message type  ERROR */
    public static final byte ERROR      = -125;
    /** GDOS message type  WARNING */
    public static final byte WARNING    = -126;
    /** GDOS message type  DEBUG INFORMATION */
    public static final byte DBG_INFO   = -127;
    /** GDOS message type  DETAILED DEBUG INFORMATION */
    public static final byte DBG_DETAIL = -128;

    public static void print(String message, int module, int gdosLevel)
    {
        GDOSDataMsg msg = new GDOSDataMsg(message);

        try {
            Tims.send(GDOS.PRINT,
                               RackName.create(RackName.GDOS, 0),
                               module,
                               (byte)0,
                               (byte)0,
                               msg);
        } catch (MsgException e) {
        }
    }

    public static void error(String message, int module, int gdosLevel)
    {
        if(gdosLevel <= GDOS.ERROR)
        {
            GDOSDataMsg msg = new GDOSDataMsg(message);

            try {
                Tims.send(GDOS.ERROR,
                                   RackName.create(RackName.GDOS, 0),
                                   module,
                                   (byte)0,
                                   (byte)0,
                                   msg);
            } catch (MsgException e) {
            }
        }
    }

    public static void warning(String message, int module, int gdosLevel)
    {
        if(gdosLevel <= GDOS.WARNING)
        {
            GDOSDataMsg msg = new GDOSDataMsg(message);

            try {
                Tims.send(GDOS.WARNING,
                                    RackName.create(RackName.GDOS, 0),
                                    module,
                                    (byte)0,
                                    (byte)0,
                                    msg);
            } catch (MsgException e) {
            }
        }
    }

    public static void dbgInfo(String message, int module, int gdosLevel)
    {
        if(gdosLevel <= GDOS.DBG_INFO)
        {
            GDOSDataMsg msg = new GDOSDataMsg(message);

            try {
                Tims.send(GDOS.DBG_INFO,
                                   RackName.create(RackName.GDOS, 0),
                                   module,
                                   (byte)0,
                                   (byte)0,
                                   msg);
            } catch (MsgException e) {
            }
        }
    }

    public static void dbgDetail(String message, int module, int gdosLevel)
    {
        if(gdosLevel <= GDOS.DBG_DETAIL)
        {
            GDOSDataMsg msg = new GDOSDataMsg(message);

            try {
                Tims.send(GDOS.DBG_DETAIL,
                                   RackName.create(RackName.GDOS, 0),
                                   module,
                                   (byte)0,
                                   (byte)0,
                                   msg);
            } catch (MsgException e) {
            }
        }
    }
}
