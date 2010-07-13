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
 *      Oliver Wulf        <wulf@rts.uni-hannover.de>
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 */
 #include "datalog_rec_class.h"

#include <main/argopts.h>


//
// data structures
//

arg_table_t argTab[] = {

    { 0, "", 0, 0, "", { 0 } } // last entry
};


int  main(int argc, char *argv[])
{
    int ret;

    // get args
    ret = RackModule::getArgs(argc, argv, argTab, "DatalogRec");
    if (ret)
    {
        printf("Invalid arguments -> EXIT \n");
        return ret;
    }

    // create new DatalogRec

    DatalogRec *pInst;

    pInst = new DatalogRec();
    if (!pInst)
    {
        printf("Can't create new DatalogRec -> EXIT\n");
        return -ENOMEM;
    }

    // init
    ret = pInst->moduleInit();
    if (ret)
        goto exit_error;

    pInst->run();

    return 0;

exit_error:
    delete (pInst);
    return ret;
}
