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
package rack.main;

import rack.main.tims.*;

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

    protected TimsMbx moduleMbx;
    protected byte    gdosLevel;
    protected int     gdosName = RackName.create(RackName.GDOS, 0);
    
    public GDOS(TimsMbx moduleMbx, byte gdosLevel)
    {
        this.moduleMbx = moduleMbx;
        this.gdosLevel = gdosLevel;
    }
    
    public void print(String message)
    {
        GDOSDataMsg msg = new GDOSDataMsg(message);

        try {
            moduleMbx.send(GDOS.PRINT,
                               gdosName,
                               (byte)0,
                               (byte)0,
                               msg);
        } catch (TimsException e) {
        }
    }

    public void error(String message)
    {
        if(gdosLevel <= GDOS.ERROR)
        {
            GDOSDataMsg msg = new GDOSDataMsg(message);

            try {
                moduleMbx.send(GDOS.ERROR,
                                   gdosName,
                                   (byte)0,
                                   (byte)0,
                                   msg);
            } catch (TimsException e) {
            }
        }
    }

    public void warning(String message)
    {
        if(gdosLevel <= GDOS.WARNING)
        {
            GDOSDataMsg msg = new GDOSDataMsg(message);

            try {
                moduleMbx.send(GDOS.WARNING,
                                    gdosName,
                                    (byte)0,
                                    (byte)0,
                                    msg);
            } catch (TimsException e) {
            }
        }
    }

    public void dbgInfo(String message)
    {
        if(gdosLevel <= GDOS.DBG_INFO)
        {
            GDOSDataMsg msg = new GDOSDataMsg(message);

            try {
                moduleMbx.send(GDOS.DBG_INFO,
                                   gdosName,
                                   (byte)0,
                                   (byte)0,
                                   msg);
            } catch (TimsException e) {
            }
        }
    }

    public void dbgDetail(String message)
    {
        if(gdosLevel <= GDOS.DBG_DETAIL)
        {
            GDOSDataMsg msg = new GDOSDataMsg(message);

            try {
                moduleMbx.send(GDOS.DBG_DETAIL,
                                   gdosName,
                                   (byte)0,
                                   (byte)0,
                                   msg);
            } catch (TimsException e) {
            }
        }
    }
    
    public void setGdosLevel(byte gdosLevel)
    {
        this.gdosLevel = gdosLevel;
    }
}
