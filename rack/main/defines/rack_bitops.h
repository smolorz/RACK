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
#ifndef __RACK_BITSOPS_H__
#define __RACK_BITSOPS_H__

//######################################################################
//# class RackBits
//######################################################################

class RackBits
{

    private:
        uint32_t bits;   // 32 bits

    public:

        void setBit(int bit)
        {
        bits |= (1 << bit);
        }

        void clearBit(int bit)
        {
            bits &= ~(1 << bit);
        }

        void clearAllBits(void)
        {
            bits = 0;
        }

        int testBit(int bit)
        {
            return bits & (1 << bit);
        }

        // testAndClearBit - Del a bit and return its old value
        int testAndClearBit(int bit)
        {
            int oldBit = testBit(bit);
            if (oldBit)
                clearBit(bit);
            return oldBit;
        }

        // testAndSetBit - Set a bit and return its old value
        int testAndSetBit(int bit)
        {
            int oldBit = testBit(bit);
            if (!oldBit)
                setBit(bit);
            return oldBit;
        }

        uint32_t getBits()
        {
            return bits;
        }
};

#endif /*RACK_BITSOPS_H_*/
