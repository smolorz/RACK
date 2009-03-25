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
 *      Oliver Wulf <wulf@rts.uni-hannover.de>
 *
 */
package rack.main.tims;

public class TimsRouter
{
    public static final byte LOGIN                 = 10;
    public static final byte MBX_INIT              = 11;
    public static final byte MBX_DELETE            = 12;
    public static final byte MBX_INIT_WITH_REPLY   = 13;
    public static final byte MBX_DELETE_WITH_REPLY = 14;
    public static final byte MBX_PURGE             = 15;
    public static final byte GET_STATUS            = 17;
    public static final byte ENABLE_WATCHDOG       = 18;
    public static final byte DISABLE_WATCHDOG      = 19;
}
