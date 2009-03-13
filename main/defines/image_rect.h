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
 *      Marco Reimer     <reimer@l3s.de>
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
#ifndef __IMAGE_RECT_H__
#define __IMAGE_RECT_H__

//######################################################################
//# Image Rect (static size - no message)
//######################################################################

typedef struct {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} __attribute__((packed)) image_rect;

class ImageRect
{
    public:
        static void le_to_cpu(image_rect *data)
        {
            data->x   = __le32_to_cpu(data->x);
            data->y   = __le32_to_cpu(data->y);
            data->width    = __le32_to_cpu(data->width);
            data->height   = __le32_to_cpu(data->height);
        }

        static void be_to_cpu(image_rect *data)
        {
            data->x   = __be32_to_cpu(data->x);
            data->y   = __be32_to_cpu(data->y);
            data->width    = __be32_to_cpu(data->width);
            data->height   = __be32_to_cpu(data->height);
        }

};
#endif /*__IMAGE_RECT_H__*/
