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
 *      Marko Reimer     <reimer@l3s.de>
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
package rack.drivers;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import javax.imageio.ImageIO;

import rack.main.tims.msg.*;
import rack.main.tims.msgtypes.*;
import rack.main.tims.exceptions.*;
import rack.main.tims.streams.*;

public class CameraDataMsg extends TimsMsg
{
    public final static int CAMERA_MODE_MONO8 = 1;
    public final static int CAMERA_MODE_MONO12 = 2;
    public final static int CAMERA_MODE_MONO16 = 3;
    public final static int CAMERA_MODE_RGB24 = 11;
    public final static int CAMERA_MODE_RGB565 = 12;
    public final static int CAMERA_MODE_YUV422 = 21;
    public final static int CAMERA_MODE_RAW8 = 31;
    public final static int CAMERA_MODE_RAW12 = 32;
    public final static int CAMERA_MODE_RAW16 = 33;
    public final static int CAMERA_MODE_JPEG = 41;

    public final static int COLORFILTER_RGGB = 512;
    public final static int COLORFILTER_GBRG = 513;
    public final static int COLORFILTER_GRBG = 514;
    public final static int COLORFILTER_BGGR = 515;

    public int recordingtime = 0;
    public int width = 0;
    public int height = 0;
    public int depth = 0;
    public int mode = 0;
    public int colorFilterId = 0;
    public int[] imageRawData;

    public int getDataLen()
    {
    	//width, height, depth and mode are short values! 
        return ( 16 + 2 * width * height);
    }

    public CameraDataMsg()
    {
        msglen = headLen + getDataLen();
//      super();
    }

    public CameraDataMsg(TimsDataMsg p) throws MsgException
    { 
        readTimsDataMsg(p);
    }

    public CameraDataMsg(int width, int height)
    {
        this.width = (short) width;
        this.height = (short) height;
        this.imageRawData = new int[width * height];

        this.recordingtime = 0;
        this.mode = 0;
        this.depth = 0;
        this.colorFilterId = 0;
    }

    protected boolean checkTimsMsgHead()
    {
        if (type == RackMsgType.MSG_DATA)
        {
            return (true);
        }
        else
            return (false);
    }

    protected int clip(int in)
    {
        if (in < 0)
        {
            return 0;
        }
        else if (in > 255)
        {
            return 255;
        }
        return in;
    }

    protected int[] buildColoredRawImage(int[] uninterpolatedImageData,
            int width, int height, int colorFilterId)
    {
        int[] interpolatedImage = new int[width * height];
        switch (colorFilterId)
        {
            case COLORFILTER_RGGB:
                for (int j = 0; j < height; j++)
                {
                    for (int i = 0; i < width; i++)
                    {
                        if (j % 2 == 1)
                        { // uneven row -> rg
                            if (i % 2 == 1)
                            {
                                interpolatedImage[width * j + i] = (255 << 24)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0xff) << 16)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00)) << 8
                                        | (int) uninterpolatedImageData[width
                                                * j + i] & 0x00;
                            }
                            else
                            {
                                interpolatedImage[width * j + i] = (255 << 24)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00) << 16)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0xff)) << 8
                                        | (int) uninterpolatedImageData[width
                                                * j + i] & 0x00;
                            }
                        }
                        else
                        {
                            if (i % 2 == 1)
                            {
                                interpolatedImage[width * j + i] = (255 << 24)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00) << 16)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0xff)) << 8
                                        | (int) uninterpolatedImageData[width
                                                * j + i] & 0x00;
                            }
                            else
                            {
                                interpolatedImage[width * j + i] = (255 << 24)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00) << 16)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00)) << 8
                                        | (int) uninterpolatedImageData[width
                                                * j + i] & 0xff;
                            }
                        }
                    }// for i
                }// for j
                break;
            case COLORFILTER_GRBG:
                for (int j = 0; j < height; j++)
                {
                    for (int i = 0; i < width; i++)
                    {
                        if (j % 2 == 1)
                        { // uneven row -> rg
                            if (i % 2 == 1)
                            {
                                interpolatedImage[width * j + i] = (255 << 24)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00) << 16)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0xff)) << 8
                                        | (int) uninterpolatedImageData[width
                                                * j + i] & 0x00;
                            }
                            else
                            {
                                interpolatedImage[width * j + i] = (255 << 24)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0xff) << 16)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00)) << 8
                                        | (int) uninterpolatedImageData[width
                                                * j + i] & 0x00;
                            }
                        }
                        else
                        {
                            if (i % 2 == 1)
                            {
                                interpolatedImage[width * j + i] = (255 << 24)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00) << 16)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00)) << 8
                                        | (int) uninterpolatedImageData[width
                                                * j + i] & 0xff;
                            }
                            else
                            {
                                interpolatedImage[width * j + i] = (255 << 24)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00) << 16)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0xff)) << 8
                                        | (int) uninterpolatedImageData[width
                                                * j + i] & 0x00;
                            }
                        }
                    }// for i
                }// for j
                break;
            case COLORFILTER_GBRG:
                for (int j = 0; j < height; j++)
                {
                    for (int i = 0; i < width; i++)
                    {
                        if (j % 2 == 1)
                        { // uneven row -> rg
                            if (i % 2 == 1)
                            {
                                interpolatedImage[width * j + i] = (255 << 24)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00) << 16)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0xff)) << 8
                                        | (int) uninterpolatedImageData[width
                                                * j + i] & 0x00;
                            }
                            else
                            {
                                interpolatedImage[width * j + i] = (255 << 24)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00) << 16)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00)) << 8
                                        | (int) uninterpolatedImageData[width
                                                * j + i] & 0xff;
                            }
                        }
                        else
                        {
                            if (i % 2 == 1)
                            {
                                interpolatedImage[width * j + i] = (255 << 24)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0xff) << 16)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00)) << 8
                                        | (int) uninterpolatedImageData[width
                                                * j + i] & 0x00;
                            }
                            else
                            {
                                interpolatedImage[width * j + i] = (255 << 24)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00) << 16)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0xff)) << 8
                                        | (int) uninterpolatedImageData[width
                                                * j + i] & 0x00;
                            }
                        }
                    }// for i
                }// for j
                break;
            default: // case COLORFILTER_BGGR:
                for (int j = 0; j < height; j++)
                {
                    for (int i = 0; i < width; i++)
                    {
                        if (j % 2 == 1)
                        { // uneven row -> rg
                            if (i % 2 == 1)
                            {
                                interpolatedImage[width * j + i] = (255 << 24)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00) << 16)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00)) << 8
                                        | (int) uninterpolatedImageData[width
                                                * j + i] & 0xff;
                            }
                            else
                            {
                                interpolatedImage[width * j + i] = (255 << 24)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00) << 16)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0xff)) << 8
                                        | (int) uninterpolatedImageData[width
                                                * j + i] & 0x00;
                            }
                        }
                        else
                        {
                            if (i % 2 == 1)
                            {
                                interpolatedImage[width * j + i] = (255 << 24)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00) << 16)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0xff)) << 8
                                        | (int) uninterpolatedImageData[width
                                                * j + i] & 0x00;
                            }
                            else
                            {
                                interpolatedImage[width * j + i] = (255 << 24)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0xff) << 16)
                                        | (((int) uninterpolatedImageData[width
                                                * j + i] & 0x00)) << 8
                                        | (int) uninterpolatedImageData[width
                                                * j + i] & 0x00;
                            }
                        }
                    }// for i
                }// for j
                break;
        }
        return interpolatedImage;
    }

    protected void readTimsMsgBody(InputStream in) throws IOException
    {
        EndianDataInputStream dataIn;
        if (bodyByteorder == BIG_ENDIAN)
        {
            dataIn = new BigEndianDataInputStream(in);
        }
        else
        {
            dataIn = new LittleEndianDataInputStream(in);
        }
        readTimsMsgBodyEndianInput(dataIn);

        bodyByteorder = BIG_ENDIAN;
    }

    public void readTimsMsgBodyEndianInput(EndianDataInputStream dataIn)
            throws IOException
    {
        recordingtime = dataIn.readInt();
        width  = dataIn.readShort();
        height = dataIn.readShort();
        depth  = dataIn.readShort();
        mode   = dataIn.readShort();
        colorFilterId = dataIn.readInt();
        System.out.println("got image data of width:"+width+" height:"+height+" depth:"+depth+" mode:"+mode);
        imageRawData = new int[width * height];
        byte actualData; // structur to multiply data.

        switch (mode)
        {
            case CameraDataMsg.CAMERA_MODE_JPEG:
            	System.out.println("got a jpeg image");
                // colorFilterId is missused as length of the jpeg stream here.
                byte[] rawImageBytes = new byte[colorFilterId];
                dataIn.readFully(rawImageBytes, 0, colorFilterId);
                BufferedImage image = ImageIO.read(new ByteArrayInputStream(
                        rawImageBytes));
                if (image != null)
                {
                    image.getRGB(0, 0, image.getWidth(), image.getHeight(),
                            imageRawData, 0, image.getWidth());
                }
                else
                {
                    System.out
                            .print("No buffered image created in cameraDataPackage->readPackageBodyEndianInput\n");
                }
                break;
            case CameraDataMsg.CAMERA_MODE_MONO8:
                // every byte from the input data is put to all colors in one
                // int
                for (int j = 0; j < height; j++)
                {
                    for (int i = 0; i < width; i++)
                    {
                        actualData = dataIn.readByte();
                        imageRawData[width * j + i] = (255 << 24)
                                | (((int) actualData & 0xff) << 16)
                                | (((int) actualData & 0xff)) << 8
                                | (int) actualData & 0xff;
                    }
                }
                break;
            case CameraDataMsg.CAMERA_MODE_RGB24: // need an array of
                                                        // int's!
                // every three bytes from the input data are combined to one int
            	System.out.println("got rgb24 image");
                for (int j = 0; j < height; j++)
                {
                    for (int i = 0; i < width; i++)
                    {
                        imageRawData[width * j + i] = // order is important!
                                                        // order defines color!!
                        (
                        	      (((int) dataIn.readByte() & 0xff) << 16)
                                | (((int) dataIn.readByte() & 0xff) << 8)
                                | (((int) dataIn.readByte() & 0xff)) 
								| (255 << 24));
                    }
                }
                break;
            case CameraDataMsg.CAMERA_MODE_YUV422:
                // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dnwmt/html/YUVFormats.asp
                // expecting: U Y0 V Y1 as defined in IIDC 1.30
                // two bytes are one pixel.
                // we need the information from the next pixel as well.
                // read four bytes and build two pixel.
                int R,
                G,
                B;
                int Y0,
                Y1,
                U,
                V;
                
                for (int j = 0; j < height; j++)
                {
                    for (int i = 0; i < width; i = i + 2)
                    {
                    	//System.out.println("width loop " + i);
                        U = (int) dataIn.readByte() & 0xff;
                        Y0 = (int) dataIn.readByte() & 0xff;
                        V = (int) dataIn.readByte() & 0xff;
                        Y1 = (int) dataIn.readByte() & 0xff;
                        B = clip((298 * (Y0 - 16) + 409 * (V - 128) + 128) >> 8);
                        G = clip((298 * (Y0 - 16) - 100 * (U - 128) - 208
                                * (V - 128) + 128) >> 8);
                        R = clip((298 * (Y0 - 16) + 516 * (U - 128) + 128) >> 8);
                        imageRawData[width * j + i] = // order is important!
                                                        // order defines color!!
                        ((R & 0xff) | ((G & 0xff) << 8) | ((B & 0xff) << 16) | (255 << 24));

                        B = clip((298 * (Y1 - 16) + 409 * (V - 128) + 128) >> 8);
                        G = clip((298 * (Y1 - 16) - 100 * (U - 128) - 208
                                * (V - 128) + 128) >> 8);
                        R = clip((298 * (Y1 - 16) + 516 * (U - 128) + 128) >> 8);
                        imageRawData[width * j + i + 1] = // order is
                                                            // important! order
                                                            // defines color!!
                        ((R & 0xff) | ((G & 0xff) << 8) | ((B & 0xff) << 16) | (255 << 24));
                    }
                }
                break;

            case CameraDataMsg.CAMERA_MODE_RAW8:
                int[] uninterpolatedImageData = new int[width * height];
                for (int j = 0; j < height; j++)
                {
                    for (int i = 0; i < width; i++)
                    {
                        uninterpolatedImageData[width * j + i] = ((int) dataIn
                                .readByte() & 0xff);
                    }
                }
                imageRawData = buildColoredRawImage(uninterpolatedImageData,
                        width, height, colorFilterId);
                break;

            case CameraDataMsg.CAMERA_MODE_RGB565:
                // every three bytes from the input data are combined to one int
                for (int j = 0; j < height; j++)
                {
                    for (int i = 0; i < width; i++)
                    {
                        int data0 = dataIn.readByte();
                        int data1 = dataIn.readByte();
                        int data = (data0 & 0xff) | ((data1 & 0xff) << 8);
                        int dataR = (data & 0x1f) << 3;
                        int dataG = (data & 0x7e0) << 5;
                        int dataB = (data & 0xf800) << 8;
                        imageRawData[width * j + i] = // order is important!
                                                        // order defines color!!
                        (dataR | dataG | dataB | (255 << 24));
                    }
                }
                break;
        }// switch
    }

    public void writeTimsMsgBody(OutputStream out) throws IOException
    {
        DataOutputStream dataOut = new DataOutputStream(out);

        dataOut.writeInt(recordingtime);
        dataOut.writeShort(width);
        dataOut.writeShort(height);
        dataOut.writeShort(depth);
        dataOut.writeShort(mode);
        dataOut.writeInt(colorFilterId);
        for (int i = 0; i < width * height * depth / 8; i++)
        {
            dataOut.writeByte(imageRawData[i]);
        }
    }

    public String toString()
    {
        return "width " + width + " height " + height + " mode " + mode
                + " depth " + depth + " colorFilterId " + colorFilterId;
    }
}
