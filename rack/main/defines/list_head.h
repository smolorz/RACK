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
#ifndef __LIST_HEAD_H__
#define __LIST_HEAD_H__

//######################################################################
//# class ListHead
//######################################################################
#define LIST_POISON1  ((void *) 0x00100100)
#define LIST_POISON2  ((void *) 0x00200200)

class ListHead
{
    private:

        void __add(ListHead *prev, ListHead *next)
        {
            next->prev = this;
            this->next = next;
            this->prev = prev;
            prev->next = this;
        };

        void __del(ListHead *prev, ListHead *next)
        {
            next->prev = prev;
            prev->next = next;
        };

    public:
        ListHead *next;
        ListHead *prev;

        ListHead()
        {
            next = this;
            prev = this;
        }

        void add(ListHead *head)
        {
            __add(head, head->next);
        }

        void add_tail(ListHead *head)
        {
            __add(head->prev, head);
        }

        void del(void)
        {
            __del(this->prev, this->next);
            this->next = (ListHead *)LIST_POISON1;
            this->prev = (ListHead *)LIST_POISON2;
        }

        void del_init(void)
        {
            __del(this->prev, this->next);
            this->next = this;
            this->prev = this;
        }

        void move(ListHead *newhead)
        {
            __del(this->prev, this->next);
            add(newhead);
        }

        void move_tail(ListHead *newhead)
        {
            __del(this->prev, this->next);
            add_tail(newhead);
        }

        int empty(void)
        {
            return this->next == this;
        }

        int empty_careful(void)
        {
            ListHead *next = this->next;
            return (next == this) && (next == this->prev);
        }

};

#endif /*__LIST_HEAD_H__*/
