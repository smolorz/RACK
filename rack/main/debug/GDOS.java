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
import rack.main.tims.router.*;
import rack.main.tims.exceptions.*;

public class GDOS {

	public static void print(String message, int module, int gdosLevel)
	{
		GDOSDataMsg msg = new GDOSDataMsg(message);

		try {
			TimsMsgRouter.send(GDOSProxy.GDOS_PRINT,
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
		if(gdosLevel <= GDOSProxy.GDOS_ERROR)
		{
			GDOSDataMsg msg = new GDOSDataMsg(message);

			try {
				TimsMsgRouter.send(GDOSProxy.GDOS_ERROR,
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
		if(gdosLevel <= GDOSProxy.GDOS_WARNING)
		{
			GDOSDataMsg msg = new GDOSDataMsg(message);

			try {
				TimsMsgRouter.send(GDOSProxy.GDOS_WARNING,
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
		if(gdosLevel <= GDOSProxy.GDOS_DBG_INFO)
		{
			GDOSDataMsg msg = new GDOSDataMsg(message);

			try {
				TimsMsgRouter.send(GDOSProxy.GDOS_DBG_INFO,
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
		if(gdosLevel <= GDOSProxy.GDOS_DBG_DETAIL)
		{
			GDOSDataMsg msg = new GDOSDataMsg(message);

			try {
				TimsMsgRouter.send(GDOSProxy.GDOS_DBG_DETAIL,
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
