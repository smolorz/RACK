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

/**

 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */

import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.File;
import java.io.FileInputStream;

import javax.imageio.ImageIO;

import rack.main.tims.streams.*;


public class Camera2PngConverter {
    public final static int CAMERA_MODE_MONO8  = 1;
    public final static int CAMERA_MODE_MONO12 = 2;
    public final static int CAMERA_MODE_MONO16 = 3;
    public final static int CAMERA_MODE_RGB24  = 11;
    public final static int CAMERA_MODE_RGB565 = 12;
    public final static int CAMERA_MODE_YUV422 = 21;
    public final static int CAMERA_MODE_RAW8   = 31;
    public final static int CAMERA_MODE_RAW12  = 32;
    public final static int CAMERA_MODE_RAW16  = 33;
    public final static int CAMERA_MODE_JPEG   = 41;

    public final static int BIG_ENDIAN   = 1;

    public final static int COLORFILTER_RGGB   = 512;
    public final static int COLORFILTER_GBRG   = 513;
    public final static int COLORFILTER_GRBG   = 514;
    public final static int COLORFILTER_BGGR   = 515;
    public static int recordingtime  = 0;
    public static int colorFilterId  = 0;
    public static int width  = 0;
    public static int height = 0;
    public static int depth  = 0;
    public static int mode   = 0;
    public static int[] imageRawData;
    public static int bodyByteorder = BIG_ENDIAN;
    public static int jpegByteStreamLength = 0;

    public static int clip(int in) {
        if (in < 0) {
            return 0;
        }else if (in > 255) {
            return 255;
        }
        return in;
        }

    public static void readPackageBodyEndianInput(EndianDataInputStream dataIn) throws IOException
    {
/*      recordingtime   = dataIn.readInt();
        width  = dataIn.readShort();
        height = dataIn.readShort();
        depth  = dataIn.readShort();
        mode   = dataIn.readShort();
        colorFilterId = dataIn.readInt();*/
        imageRawData  = new int[width*height];

        switch(mode) {
        case CameraDataMsg.CAMERA_MODE_JPEG:
	        System.out.println("got a jpeg image");
	        // colorFilterId is missused as length of the jpeg stream here.
	        byte[] rawImageBytes = new byte[jpegByteStreamLength];
	        dataIn.readFully(rawImageBytes, 0, jpegByteStreamLength);
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
        case CameraDataMsg.CAMERA_MODE_YUV422:
                //http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dnwmt/html/YUVFormats.asp
                //expecting: U Y0 V Y1 as defined in IIDC 1.30
                //two bytes are one pixel.
                //we need the information from the next pixel as well.
                //read four bytes and build two pixel.
                int R,G,B;
                int Y0, Y1 ,U ,V ;
                for(int j=0;j<height;j++) {
                    for (int i = 0; i <width; i=i+2)    {
                        U  = (int)dataIn.readByte() & 0xff;
                        Y0 = (int)dataIn.readByte() & 0xff;
                        V  = (int)dataIn.readByte() & 0xff;
                        Y1 = (int)dataIn.readByte() & 0xff;
                        B = clip(( 298 * (Y0-16)                 + 409 * (V-128) + 128) >> 8);
                        G = clip(( 298 * (Y0-16) - 100 * (U-128) - 208 * (V-128) + 128) >> 8);
                        R = clip(( 298 * (Y0-16) + 516 * (U-128)                 + 128) >> 8);
                        imageRawData[width*j+i] = //order is important! order defines color!!
                            ((R & 0xff)|((G & 0xff)<<8)|((B & 0xff)<<16)|(255<<24));

                        B = clip(( 298 * (Y1-16)                 + 409 * (V-128) + 128) >> 8);
                        G = clip(( 298 * (Y1-16) - 100 * (U-128) - 208 * (V-128) + 128) >> 8);
                        R = clip(( 298 * (Y1-16) + 516 * (U-128)                 + 128) >> 8);
                        imageRawData[width*j+i+1] = //order is important! order defines color!!
                            ((R & 0xff)|((G & 0xff)<<8)|((B & 0xff)<<16)|(255<<24));
                    }
                }
                break;
            case CameraDataMsg.CAMERA_MODE_RGB24:
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
        }
    }


    public static void main(String[] args) {

        String dirname = args[0];

        //      this information may also be gathered from the .sav file
        width  = Integer.parseInt(args[1]);
        height = Integer.parseInt(args[2]);
        depth  = Integer.parseInt(args[3]);
        mode   = Integer.parseInt(args[4]);
        colorFilterId = Integer.parseInt(args[5]);
        jpegByteStreamLength = Integer.parseInt(args[6]);
        
        File file;
        String filename;
        String saveFilename;
        String endung;

        File workDir = new File(dirname);
        String[] filenames = workDir.list();

        for (int i = 0; i < filenames.length; i++)
        {
            filename = dirname.concat(filenames[i]);

            endung = filename.substring(filename.indexOf('.') + 1, filename.length());
            if ( (endung.compareTo("rck") == 0) || (endung.compareTo("rcc") == 0))
            {
	            file  = new File(filename);
	            saveFilename = filename.substring(0,filename.indexOf('.'));
	            saveFilename = saveFilename.concat(".png");
	
	            EndianDataInputStream dataIn;
	
	            try{
	                FileInputStream fileInputStream = new FileInputStream(file);
	                if(bodyByteorder == BIG_ENDIAN)
	                {
	                    dataIn = new BigEndianDataInputStream(fileInputStream);
	                } else {
	                    dataIn = new LittleEndianDataInputStream(fileInputStream);
	                }
	                //only works for yuv422 yet
	                readPackageBodyEndianInput(dataIn);
	            }catch(Exception e){
	                System.out.println("File not useable."+e.toString());
	            }

	            try{
	                System.out.println("Store image data filename=" + saveFilename);
	                BufferedImage image = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);// the image to be stored //";
	                image.setRGB(0, 0, width, height, imageRawData, 0, width);
	                File saveFile = new File(saveFilename);
	                ImageIO.write(image, "png", saveFile);
	            } catch(IOException e) {
	                System.out.println("Error storing image filename=" + saveFilename + e.toString());
	            }
            }
      }

    }
}


