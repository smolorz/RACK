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
 *      Jan Kiszka <jan.kiszka@web.d>
 *
 */

#include <rack_config.h>

const char __attribute__((used)) rack_version[] = "*** " RACK_PACKAGE_STRING
#ifdef RACK_PACKAGE_SVN_REVISION
    " (SVN revision #" RACK_PACKAGE_SVN_REVISION ") ***";
#else /* !RACK_PACKAGE_SVN_REVISION */
    " ***";
#endif /* RACK_PACKAGE_SVN_REVISION */
