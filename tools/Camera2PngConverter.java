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
 * 
 * 
 */

import java.awt.image.BufferedImage;
import java.io.IOException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.DataOutputStream;

import javax.imageio.ImageIO;

import rack.drivers.CameraDataMsg;
import rack.main.tims.*;

public class Camera2PngConverter {

	public final static int BIG_ENDIAN = 1;

	public static int bodyByteorder = BIG_ENDIAN;

	public static void main(String[] args) {

		bodyByteorder = BIG_ENDIAN;

		int recordingTime = 0;
		int colorFilterId = 512;
		short width  = 1280;
		short height = 960;
		short depth  = 16;
		short mode   = 21;

		File tempfile;
		File rawimagefile;
		File saveFile = null; 
		FileOutputStream fileOutputStream  = null;
		FileInputStream  fileInputStream   = null;
		FileInputStream rawfileInputStream = null;
		
		String filename;
		String saveFilename = null;
		String endung;
		CameraDataMsg data = new CameraDataMsg();

		String dirname = args[0];
		File workDir = new File(dirname);
		String[] filenames = workDir.list();
		
		System.out.println("Using (static) parameter: width:"+width+" height:"+height+" depth:"+depth+" mode:"+mode+" colorfilterId:"+colorFilterId);
		System.out.println("Processing all files in directory: "+workDir);

		for (int i = 0; i < filenames.length; i++) {
			filename = dirname.concat(filenames[i]);
			endung = filename.substring(filename.indexOf('.') + 1, filename.length());
			
			if ((endung.compareTo("raw") == 0)) {
				saveFilename = filename.substring(0, filename.indexOf('.'));
				saveFilename = saveFilename.concat(".png");

				try {
					tempfile = File.createTempFile("tempfile", null);
					fileOutputStream = new FileOutputStream(tempfile);
					fileInputStream = new FileInputStream(tempfile);

					rawimagefile = new File(filename);				
					rawfileInputStream = new FileInputStream(rawimagefile);

					saveFile = new File(saveFilename);

					DataOutputStream dataOut = new DataOutputStream(fileOutputStream);
	
					dataOut.writeInt(recordingTime);
					dataOut.writeShort(width);
					dataOut.writeShort(height);
					dataOut.writeShort(depth);
					dataOut.writeShort(mode);
					dataOut.writeInt(colorFilterId);
	
					EndianDataInputStream dataIn;
	
					int imagelength = width*height*depth/8;
					byte[] imagebytes = new byte[imagelength];
					
					System.out.println("Reading image data filename="+ filename);
					rawfileInputStream.read(imagebytes, 0, imagelength);
					
					dataOut.write(imagebytes, 0, imagelength);
					dataOut.flush();
					
					if (bodyByteorder == BIG_ENDIAN) {
						dataIn = new BigEndianDataInputStream(fileInputStream);
					} else {
						dataIn = new LittleEndianDataInputStream(fileInputStream);
					}
					
					//System.out.println("parsing file");
					
					data.readTimsMsgBodyEndianInput(dataIn);
					System.out.println("Store image data filename="+ saveFilename);
					BufferedImage image = new BufferedImage(data.width,data.height, BufferedImage.TYPE_INT_RGB);
					image.setRGB(0, 0, data.width, data.height,	data.imageRawData, 0, data.width);
					ImageIO.write(image, "png", saveFile);
				}catch(IOException ioe)
				{
					System.out.println("IOError " + ioe.toString());	
				}
			} else {
				System.out.println("Unknown fileending. (Ending(rcc) is not longer supported)");

			}
			
		}
	System.out.println("Converion finshed");
	}
}
