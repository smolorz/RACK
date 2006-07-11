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

 /*!
 * @ingroup rackos
 * @defgroup bitops Bit Operations
 * @{
 */

//######################################################################
//# class RackBits
//######################################################################

class RackBits
{
    private:
        uint32_t bits;   // 32 bits

    public:

/**
 * @brief Set a bit
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT, RT)
 *
 * Rescheduling: never.
 */
        void setBit(int bit)
        {
            bits |= (1 << bit);
        }

/**
 * @brief Clear a bit
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT, RT)
 *
 * Rescheduling: never.
 */
        void clearBit(int bit)
        {
            bits &= ~(1 << bit);
        }

/**
 * @brief Clear all bits
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT, RT)
 *
 * Rescheduling: never.
 */
        void clearAllBits(void)
        {
            bits = 0;
        }

/**
 * @brief Test a bit
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT, RT)
 *
 * Rescheduling: never.
 */
        int testBit(int bit)
        {
            return bits & (1 << bit);
        }

/**
 * @brief Clear a bit and return its old value
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT, RT)
 *
 * Rescheduling: never.
 */
        int testAndClearBit(int bit)
        {
            int oldBit = testBit(bit);
            if (oldBit)
                clearBit(bit);
            return oldBit;
        }

/**
 * @brief Set a bit and return its old value
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT, RT)
 *
 * Rescheduling: never.
 */
        int testAndSetBit(int bit)
        {
            int oldBit = testBit(bit);
            if (!oldBit)
                setBit(bit);
            return oldBit;
        }

/**
 * @brief Get all bits
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - User-space task (non-RT, RT)
 *
 * Rescheduling: never.
 */
        uint32_t getBits()
        {
            return bits;
        }
};

/*@}*/

#endif // __RACK_BITSOPS_H__
