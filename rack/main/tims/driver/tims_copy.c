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
 *
 */
#include <main/tims/driver/tims_copy.h>
#include <main/tims/driver/tims_debug.h>

//
// external vakues
//
extern int dbglevel;

inline unsigned long get_remain_bytes_in_page(unsigned long ptr)
{
    return (unsigned long)((ptr & PAGE_MASK) + PAGE_SIZE - ptr);
}

// userpace task receives data from userspace mailbox
static unsigned long tims_copy_userslot_user(rtdm_user_info_t *user_info,
                                             timsMbxSlot *slot,
                                             const struct msghdr *msg)
{
    unsigned long akt_copy_size = 0;
    unsigned long copy_bytes    = 0;
    unsigned long bytes_copied  = 0;
    unsigned long bytes_in_page = 0;
    int i;

    unsigned long  ret;
    unsigned long  p_src_map    = slot->p_head_map;
    void          *p_src        = slot->p_head;
    void          *p_dest       = NULL;
    unsigned long  src_page     = slot->map_idx;
    timsMsgHead*   p_head       = (timsMsgHead *)slot->p_head_map;
    unsigned long  databytes    = p_head->msglen - TIMS_HEADLEN;

    for (i=0; i<2; i++)
    {
        if (!i)
            copy_bytes = TIMS_HEADLEN;
        else
            copy_bytes = databytes;

        p_dest = msg->msg_iov[i].iov_base;

        // check destination pointer
        ret = rtdm_rw_user_ok(user_info, p_dest, copy_bytes);
        if (!ret)
        {
            tims_error("ERROR: userspace destination 0x%p (%lu bytes) NOT OK \n",
                       p_dest, copy_bytes);
            return ret;
        }

        // copy data
        while (copy_bytes)
        {
            bytes_in_page = get_remain_bytes_in_page(p_src_map);
            akt_copy_size = min_t(unsigned long, bytes_in_page, copy_bytes);

            ret = rtdm_copy_to_user(user_info, p_dest, (void *)p_src_map,
                                    akt_copy_size);
            if (ret)
            {
                if (ret < 0)
                {
                    tims_error("ERROR while copy userbuffer -> user, "
                               "code = %lu \n", ret);
                }
                else
                {
                    tims_error("ERROR while copy userbuffer -> user, "
                               "only %lu/%lu bytes were copied \n",
                               akt_copy_size - ret, akt_copy_size);
                }
                return ret;
            }

            bytes_in_page -= akt_copy_size;
            copy_bytes    -= akt_copy_size;
            p_src_map     += akt_copy_size;
            p_src         += akt_copy_size;
            p_dest        += akt_copy_size;
            bytes_copied  += akt_copy_size;

            if (!bytes_in_page)
            {
                src_page++;
                if (slot->p_mbx->p_mapInfo[src_page].mapped)
                {
                    p_src_map = slot->p_mbx->p_mapInfo[src_page].virtual;
                }
                else
                {
                    return -EFAULT;
                }
            }
        }
    }
    return 0;
}

// userpace task receives data from kernelspace mailbox
static unsigned long tims_copy_kernelslot_user(rtdm_user_info_t *user_info,
                                               timsMbxSlot *slot,
                                               const struct msghdr *msg)
{
    void          *p_src      = slot->p_head;
    void          *p_dest     = NULL;
    int            veclen     = msg->msg_iovlen;
    unsigned long  copy_bytes = 0;
    int            i          = 0;
    unsigned long  ret        = 0;

    for (i=0; i<veclen; i++)
    {
        copy_bytes = msg->msg_iov[i].iov_len;
        p_dest     = msg->msg_iov[i].iov_base;

        // check destination pointer
        ret = rtdm_rw_user_ok(user_info, p_dest, copy_bytes);
        if (!ret) {
          tims_error("ERROR: userspace destination 0x%p (%lu bytes) NOT OK \n",
                      p_dest, copy_bytes);
          return ret;
        }

        // copy data
        ret = rtdm_copy_to_user(user_info, p_dest, p_src, copy_bytes);
        if (ret)
        {
            if (ret < 0)
            {
                tims_error("ERROR while copy kernelbuffer -> user, "
                           "code = %lu \n", ret);
            }
            else
            {
                tims_error("ERROR while copy kernelbuffer -> user, "
                           "only %lu/%lu bytes were copied \n",
                           copy_bytes - ret, copy_bytes);
            }
            return ret;
        }
        p_src += copy_bytes;
    }
    return 0;
}


#ifdef TIMS_ALLOW_KERNEL_TASKS

// kernelspace task receives data from kernelspace mailbox
static unsigned long tims_copy_kernelslot_kernel(rtdm_user_info_t *user_info,
                                                 timsMbxSlot *slot,
                                                 const struct msghdr *msg)
{
    unsigned long  copy_bytes = 0;
    int            i          = 0;
    void          *p_src      = slot->p_head;
    void          *p_dest     = NULL;
    int            veclen     = msg->msg_iovlen;

    for (i=0; i<veclen; i++)
    {
        copy_bytes = msg->msg_iov[i].iov_len;
        p_dest     = msg->msg_iov[i].iov_base;

        memcpy(p_dest, p_src, copy_bytes);

        p_src += copy_bytes;
    }
    return 0;
}

#endif // TIMS_ALLOW_KERNEL_TASKS

// userspace task sends data to userspace mailbox
static unsigned long tims_copy_user_userslot(rtdm_user_info_t *user_info,
                                             timsMbxSlot *slot,
                                             const struct msghdr *msg)
{
    unsigned long akt_copy_size = 0;
    unsigned long copy_bytes    = 0;
    unsigned long bytes_copied  = 0;
    unsigned long free_in_page  = 0;
    int           i             = 0;

    unsigned long ret;
    unsigned long p_dest_map    = slot->p_head_map;
    void*         p_dest        = slot->p_head;
    void*         p_src         = NULL;
    unsigned long dest_page     = slot->map_idx;

    int veclen = msg->msg_iovlen;
    for (i=0; i<veclen; i++)
    {
        copy_bytes = msg->msg_iov[i].iov_len;
        p_src      = msg->msg_iov[i].iov_base;

        // check source pointer
        ret = rtdm_read_user_ok(user_info, p_src, copy_bytes);
        if (!ret)
        {
            tims_error("Copy user -> user: user, src %p (%lu bytes) NOT OK \n",
                        p_src, copy_bytes);
            return -EINVAL;
        }

        // copy data
        while (copy_bytes)
        {
            free_in_page  = get_remain_bytes_in_page(p_dest_map);
            akt_copy_size = min_t(unsigned long, free_in_page, copy_bytes);

            ret = rtdm_copy_from_user(user_info, (void *)p_dest_map, p_src,
                                      akt_copy_size);
            if (ret)
            {
                if (ret < 0)
                {
                    tims_error("Can't copy user -> user, code = %lu \n", ret);
                }
                else
                {
                    tims_error("Can't copy user -> user, only %lu/%lu bytes "
                               " have been copied \n", akt_copy_size - ret,
                               akt_copy_size);
                }
                return ret;
            }

            free_in_page -= akt_copy_size;
            copy_bytes   -= akt_copy_size;
            p_dest_map   += akt_copy_size;
            p_dest       += akt_copy_size;
            p_src        += akt_copy_size;
            bytes_copied += akt_copy_size;

            if (!free_in_page)
            {
                dest_page++;
                if (slot->p_mbx->p_mapInfo[dest_page].mapped)
                {
                    p_dest_map = slot->p_mbx->p_mapInfo[dest_page].virtual;
                }
                else
                {
                    return -EFAULT;
                }
            }
        }
    }
    return 0;
}

// realtime or non realtime context (xenomai task or linux)
static unsigned long tims_copy_user_kernelslot(rtdm_user_info_t *user_info,
                                               timsMbxSlot *slot,
                                               const struct msghdr *msg)
{
    unsigned long   copy_bytes  = 0;
    int             i           = 0;
    unsigned long   ret         = 0;
    void*           p_dest      = slot->p_head;
    void*           p_src       = NULL;
    int             veclen      = msg->msg_iovlen;

    for (i=0; i<veclen; i++)
    {
        copy_bytes = msg->msg_iov[i].iov_len;
        p_src      = msg->msg_iov[i].iov_base;

        // check source pointer
        ret = rtdm_read_user_ok(user_info, p_src, copy_bytes);
        if (!ret)
        {
            tims_error("Copy user -> kernel: user src %p (%lu bytes) NOT OK \n",
                       p_src, copy_bytes);
            return -EINVAL;
        }

        ret = rtdm_copy_from_user(user_info, p_dest, p_src, copy_bytes);
        if (ret)
        {
            if (ret < 0)
            {
                tims_error("Can't copy user -> kernel, code = %lu \n", ret);
            }
            else
            {
                tims_error("Can't copy user -> kernel, only %lu/%lu bytes "
                           "have been copied \n", copy_bytes - ret, copy_bytes);
            }
            return ret;
        }
        p_dest += copy_bytes;
    }
    return 0;
}

#ifdef TIMS_ALLOW_KERNEL_TASKS

// kernelspace task sends data to userspace mailbox
static unsigned long tims_copy_kernel_userslot(rtdm_user_info_t *user_info,
                                               timsMbxSlot *slot,
                                               const struct msghdr *msg)
{
    unsigned long  akt_copy_size = 0;
    unsigned long  copy_bytes    = 0;
    unsigned long  free_in_page  = 0;
    int            i             = 0;

    unsigned long  p_dest_map    = slot->p_head_map;
    void          *p_dest        = slot->p_head;
    void          *p_src         = NULL;
    unsigned long  dest_page     = slot->map_idx;

    int veclen   = msg->msg_iovlen;
    for (i=0; i<veclen; i++)
    {
        copy_bytes = msg->msg_iov[i].iov_len;
        p_src      = msg->msg_iov[i].iov_base;

        // copy data
        while (copy_bytes)
        {
            free_in_page  = get_remain_bytes_in_page(p_dest_map);
            akt_copy_size = min_t(unsigned long, free_in_page, copy_bytes);

            memcpy((void *)p_dest_map, p_src, akt_copy_size);

            free_in_page -= akt_copy_size;
            copy_bytes   -= akt_copy_size;
            p_dest_map   += akt_copy_size;
            p_dest       += akt_copy_size;
            p_src        += akt_copy_size;

            if (!free_in_page)
            {
                dest_page++;
                if (slot->p_mbx->p_mapInfo[dest_page].mapped)
                {
                    p_dest_map = slot->p_mbx->p_mapInfo[dest_page].virtual;
                }
                else
                {
                    return -EFAULT;
                }
            }
        }
    }
    return 0;
}

// kernelspace task sends data to kernelspace mailbox
static unsigned long tims_copy_kernel_kernelslot(rtdm_user_info_t *user_info,
                                                 timsMbxSlot *slot,
                                                 const struct msghdr *msg)
{
    unsigned long   copy_bytes  = 0;
    int             i           = 0;
    void*           p_dest      = slot->p_head;
    void*           p_src       = NULL;
    int             veclen      = msg->msg_iovlen;

    for (i=0; i<veclen; i++)
    {
        copy_bytes = msg->msg_iov[i].iov_len;
        p_src      = msg->msg_iov[i].iov_base;

        memcpy(p_dest, p_src, copy_bytes);

        p_dest += copy_bytes;
    }
    return 0;
}

#endif // TIMS_ALLOW_KERNEL_TASKS

// copy function used by sending tasks
 int copy_msg_into_slot(rtdm_user_info_t *user_info, timsMbxSlot *slot,
                        const struct msghdr *msg, unsigned long mbxFlags)
{
    int ret = 0;

    if (!user_info) // sender is in kernelspace (e.g. pipe receive task)
    {
        if (test_bit(TIMS_MBX_BIT_USRSPCBUFFER, &mbxFlags))
        { // kernelspace task -> userspace mailbox
            ret = tims_copy_kernel_userslot(user_info, slot, msg);
            if (ret)
                return ret;
        }
        else // kernelspace task -> kernelspace mailbox
        {
            ret = tims_copy_kernel_kernelslot(user_info, slot, msg);
            if (ret)
                return ret;
        }
    }
    else // sender is in userspace
    {
        if (test_bit(TIMS_MBX_BIT_USRSPCBUFFER, &mbxFlags))
        { // userspace task -> userspace mailbox
            ret = tims_copy_user_userslot(user_info, slot, msg);
            if (ret)
                return ret;
        }
        else // userspace task -> kernelspace mailbox
        {
            ret = tims_copy_user_kernelslot(user_info, slot, msg);
            if (ret)
                return ret;
        }
    }
    return 0;
}

// copy functions used by receiving tasks
int copy_msg_out_slot(rtdm_user_info_t *user_info, timsMbxSlot *slot,
                      const struct msghdr *msg, unsigned long mbxFlags)
{
    int ret = 0;

    if (TIMS_ALLOW_KERNEL_TASKS && !user_info) // receiver is in kernelspace
    { // kernelspace mailbox -> kernelspace task
        ret = tims_copy_kernelslot_kernel(user_info, slot, msg);
        if (ret)
            return ret;
    }
    else // receiver is in userspace
    {
        if (test_bit(TIMS_MBX_BIT_USRSPCBUFFER, &mbxFlags)) // user -> user
        { // userspace mailbox -> userspace task
            ret = tims_copy_userslot_user(user_info, slot, msg);
            if (ret)
                return ret;
        }
        else // kernelspace mailbox -> userspace task
        {
            ret = tims_copy_kernelslot_user(user_info, slot, msg);
            if (ret)
                return ret;
        }
    }
    return 0;
}
