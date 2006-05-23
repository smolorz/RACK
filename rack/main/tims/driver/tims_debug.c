/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Authors
 *      Joerg Langenberg  <joerg.langenberg@gmx.net>
 *      Sebastian Smolorz <Sebastian.Smolorz@stud.uni-hannover.de>
 *
 */
#include <main/tims/driver/tims_debug.h>

//
// Module parameter
//

int dbglevel = 1;    // default TIMS_LEVEL_ERROR

module_param(dbglevel, int, 0400);
MODULE_PARM_DESC(dbglevel, "TiMS debug level, 0 = silent, 1 = error, 2 = info, "
                           "3 = warn, 4 = debug info, 5 = debug detail");
