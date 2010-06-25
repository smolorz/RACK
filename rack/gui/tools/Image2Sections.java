/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2009 University of Hannover
 *                    Institute for Systems Engineering - RTS
 *                    Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Author
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */
package rack.gui.tools;

import java.awt.image.BufferedImage;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Vector;

import javax.imageio.ImageIO;
import javax.imageio.stream.FileImageInputStream;
import javax.imageio.stream.FileImageOutputStream;

public class Image2Sections
{
	static String 	sourcefile = "C:/Elrob2009_Stuff/Karten/oulu_luft_cs_utm.jpg";
	static String 	imageDescription = "tfw";
	static String 	imgFileType = "jpg";
	static int 		pixelsPerTileX = 512;
	static int 		pixelsPerTileY = 512;
	static int		numberOfZoomsteps = 1;

    static Vector<BackgroundImageDescriptor>  	imgDesc  = new Vector<BackgroundImageDescriptor>();	
	static String 								destfolder = "C:/Elrob2009_Stuff/Karten/Luft_Cs_Tile/oulu_luft_cs";


	public static void main (String[] args)
	{
		readImageDescriptor(sourcefile, imageDescription);
		
		if (imgDesc.size() > 0)
		{
			divideImage();
		}
	}	
	
	//search folder for description filenames 
	static void readImageDescriptor(String fileName, String fileType)
	{
		String					  line;
		BackgroundImageDescriptor desc;
		BufferedReader			  br;
		int		  				  pointId;

		pointId 	= fileName.lastIndexOf('.');
			
		desc = null;
		desc = new BackgroundImageDescriptor();
		desc.descFileName = fileName.substring(0, pointId) + "." + fileType;
		desc.imgFileName  = fileName;
		System.out.println("descFileName="+desc.descFileName);
		
		try
		{
			br 	  = new BufferedReader(new FileReader(desc.descFileName));
			for (int j = 0; j < 6; j++)
			{
				line = br.readLine();
				if (line != null)
				{
					switch (j)
					{
						case 0:
							desc.scaleY = Double.parseDouble(line);
							break;
						case 3:
							desc.scaleX = Double.parseDouble(line);
							break;
						case 4:
							desc.upperLeftY  = Double.parseDouble(line);
							break;
						case 5:
							desc.upperLeftX  = Double.parseDouble(line);
							break;
					}
				}
			}
			br.close();						
		}
		catch(FileNotFoundException e)
		{
			desc = null;
			System.out.println("Image2Sections.readImageDescription: descriptor file not found");
		}
		catch (IOException e)
		{
			desc = null;
			System.out.println("Image2Sections.readImageDescription: cannot read descriptor file");
		}
	
		// add new descriptor
		if (desc != null)
		{
			imgDesc.add(desc);
			System.out.println("upperLeft="+desc.upperLeftX+", "+desc.upperLeftY+", scale="+desc.scaleX+", "+desc.scaleY);
		}
	}
	
	static void divideImage()
	{
		BufferedImage 	imageSrc;
		BufferedImage 	imageDst;		
		int 			numTilesX;
		int				numTilesY;
		int				currPxX;
		int				currPxY;
		int				sizeX;
		int				sizeY;
		int				numTile;
		double			currX;
		double			currY;
		String			imageDstName;
		String			dscDstName;
		
		try
		{
			imageSrc = ImageIO.read(new FileImageInputStream(new File(imgDesc.get(0).imgFileName)));
		}
		
		catch (IOException e)
		{
			imageSrc = null;
			System.out.println("Image2Sections.divideImage: cannot read image file");
		}
		
		if (imageSrc != null)
		{
		   numTilesX = (int)Math.ceil((float)imageSrc.getWidth() / (float)pixelsPerTileX);
		   numTilesY = (int)Math.ceil((float)imageSrc.getHeight() / (float)pixelsPerTileY);
		   numTile   = 0;

		   for (int i = 0; i < numTilesX; i++)
		   {
			   currPxX = i * pixelsPerTileX;
			   sizeX   = pixelsPerTileX;
			   System.out.println("currPxX="+currPxX+", i="+i);
			   System.out.println("(currPxX+sizeX)="+(currPxX+sizeX));			   
			   if ((currPxX + sizeX) > imageSrc.getWidth())
			   {
				   sizeX = imageSrc.getWidth() - currPxX;   
			   }
			   
			   for (int j = 0; j < numTilesY; j++)
			   {
				   currPxY = j * pixelsPerTileY;
				   sizeY   = pixelsPerTileY;
				   if ((currPxY + sizeY) > imageSrc.getHeight())
				   {
					   sizeY = imageSrc.getHeight() - currPxY;
				   }
				   System.out.println("width="+imageSrc.getWidth()+", height="+imageSrc.getHeight());
				   System.out.println("currPxX="+currPxX+", currPxY="+currPxY+", sizeX="+sizeX+", sizeY="+sizeY);
				   imageDst = null;
				   imageDst = imageSrc.getSubimage(currPxX, currPxY, sizeX, sizeY);

				   currX = imgDesc.get(0).upperLeftX + currPxY * imgDesc.get(0).scaleX;
				   currY = imgDesc.get(0).upperLeftY + currPxX * imgDesc.get(0).scaleY;
				   
				   imageDstName = destfolder + "_tile_" + Integer.toString(numTile) + "." + imgFileType;
				   dscDstName  = destfolder + "_tile_" + Integer.toString(numTile) + "." + imageDescription;
				   try
				   {
					   ImageIO.write (imageDst, imgFileType, new FileImageOutputStream(new File(imageDstName)));
					   System.out.println("write image "+imageDstName);
					   
						try
						{
							BufferedWriter bw = new BufferedWriter(new FileWriter(dscDstName));

							bw.write(Double.toString(imgDesc.get(0).scaleY), 0, Double.toString(imgDesc.get(0).scaleY).length());
							bw.newLine();
							bw.write("0.0");
							bw.newLine();							
							bw.write("0.0");
							bw.newLine();
							bw.write(Double.toString(imgDesc.get(0).scaleX), 0, Double.toString(imgDesc.get(0).scaleX).length());
							bw.newLine();							
							bw.write(Double.toString(currY), 0, Double.toString(currY).length());
							bw.newLine();
							bw.write(Double.toString(currX), 0, Double.toString(currX).length());
							bw.newLine();							
							bw.close();							
						}
						catch (IOException e)
						{
							System.out.println("Image2Sections writeDescriptionContent: cannot create FileWriter or cannot write");
						}
					   
					   numTile++;
				   }
				   catch (IOException e)
				   {
					   System.out.println("Image2Sections.divideImage: cannot write image file");
				   }
			   }
		   }
		}
	}
}