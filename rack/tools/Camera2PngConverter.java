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
package rack.tools;

/**


 */

import java.awt.image.BufferedImage;
import java.io.IOException;
import java.io.File;
import java.io.FileInputStream;

import javax.imageio.ImageIO;

import rack.drivers.CameraDataMsg;
import rack.main.tims.*;


public class Camera2PngConverter {


    public final static int BIG_ENDIAN   = 1;

    public static int bodyByteorder = BIG_ENDIAN;

    
    public CameraDataMsg data; 


    public  void main(String[] args) {

        String dirname = args[0];

        bodyByteorder = BIG_ENDIAN;
        
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
            if ( (endung.compareTo("rck") == 0))
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
	                data.readTimsMsgBodyEndianInput(dataIn);
	            }catch(Exception e){
	                System.out.println("File not useable."+e.toString());
	            }

	            try{
	                System.out.println("Store image data filename=" + saveFilename);
	                BufferedImage image = new BufferedImage(data.width, data.height, BufferedImage.TYPE_INT_RGB);// the image to be stored //";
	                image.setRGB(0, 0, data.width, data.height, data.imageRawData, 0, data.width);
	                File saveFile = new File(saveFilename);
	                ImageIO.write(image, "png", saveFile);
	            } catch(IOException e) {
	                System.out.println("Error storing image filename=" + saveFilename + e.toString());
	            }
            } else {
            		System.out.println("Ending(rcc) is not longer supported as the file stream now must contain width, height, depth and mode now");

            }
      }

    }
}


