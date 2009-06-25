/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2009	   University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Frauke Wübbold	<wuebbold@rts.uni-hannover.de>
 *
 */
package rack.gui.tools;

import java.util.*;
import java.awt.*;
import java.awt.geom.AffineTransform;
import java.awt.image.AffineTransformOp;
import java.awt.image.BufferedImage;
import java.awt.image.BufferedImageOp;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import javax.imageio.ImageIO;
import javax.imageio.stream.FileImageInputStream;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.MapViewActionEvent;
import rack.gui.main.MapViewComponent;
import rack.gui.main.MapViewGraphics;
import rack.gui.main.MapViewGui;
import rack.gui.main.MapViewInterface;
import rack.gui.main.RackModuleGui;

//NOTE: this file does not contain MapViewGui-Code for: if "txt" - readGlobalInfo etc. 

public class BackgroundImageViewerGui extends RackModuleGui implements MapViewInterface
{
	protected String				    descFileType  = "tfw";
	protected String					imgFileType   = "jpg";
//	protected double          			utmOffsetX    = 5804519000.0;   // uni Hannover offsets    
//    protected double          			utmOffsetY    = 548406000.0;    // per default
	protected double          			utmOffsetX     = 5806400000.0;   // uni Hannover offsets    
    protected double          			utmOffsetY     = 3548500000.0;    // per default
	static int 							pixelsPerTileX = 512;
	static int 							pixelsPerTileY = 512;    
   
    protected String					imgSource;

	protected MapViewComponent  		mapComponent;
    protected boolean           		mapViewIsShowing;
    protected MapViewGui        		mapViewGui;
    
    Vector<BackgroundImageDescriptor>  	imgDesc  = new Vector<BackgroundImageDescriptor>();
    Vector<Integer>						lastImages = null;

    protected BufferedImage                bgImg;
    protected int                          bgX;
    protected int                          bgY;
    protected int                          bgW;
    protected int                          bgH;
    protected boolean                      bgBicubic;
    protected ExtendedImage[][]			   extendedImages;
    
  
    protected String 						param;
    
    protected double[]						bgImageBorders; //ulX, ulY, drX, drY
    protected float[]						zoomArray;
	
    		    
    public BackgroundImageViewerGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);
        
        mapViewGui   = MapViewGui.findMapViewGui(ge);
        mapComponent = new MapViewComponent();
        mapComponent.addMapView(this);
        mapComponent.setPreferredSize(new Dimension(200,200));
        mapComponent.setDefaultVisibleRange(5000.0);

        onButton.addKeyListener(mapComponent.keyListener);
        offButton.addKeyListener(mapComponent.keyListener);
        rootPanel.add(mapComponent, BorderLayout.CENTER);
        
        // get module parameter
        String param = ge.getParameter("utmOffsetX");
        if (param.length() > 0)
        {
            utmOffsetX = Double.parseDouble(param) * 1000.0;
        }
        
        param = ge.getParameter("utmOffsetY");
        if (param.length() > 0)
        {
            utmOffsetY = Double.parseDouble(param) * 1000.0;
        }
        System.out.println("BackgroundimageViewerGui: utmOffset X, Y: " + utmOffsetX + ", " + utmOffsetY);
        
        param = ge.getParameter("imgSource");
        if (param.length() > 0)
        {
            imgSource = param;
        	System.out.println("BackgroundImageViewerGui: background image folder " + imgSource);
        }
        else
        {
        	System.out.println("BackgroundImageViewerGui: no background image specified");
        }
        
        // get background image information
    	readImageDescriptor(imgSource, descFileType);        
        
        setEnabled(false);
        
        mapViewIsShowing = false;
    }

    protected void setEnabled(boolean enabled)
    {
        mapComponent.setEnabled(enabled);
    }
    
    protected void runStart()
    {
        if(mapViewGui != null)
        {
            mapViewGui.addMapView(this);
        }
    }

    protected void runStop() throws NullPointerException
    {
        if(mapViewGui != null)
        {
        	mapViewGui.removeBackgroundImage();
        	mapViewGui.removeMapView(this);
            System.out.println("BackgroundImageViewerGui: void run Stop()");
        }
        
        //mapComponent.removeListener();
        //mapComponent = null;
    }    
    
    protected boolean needsRunData()
    {
        return (super.needsRunData() || mapViewIsShowing);
    }
    
    protected void runData()
    {
    	if (mapViewIsShowing == false)
    	{
    		mapComponent.repaint();
    		setEnabled(true);
    	//	mapViewIsShowing = false;
    	}
    }

    public void mapViewActionPerformed(MapViewActionEvent event)
    {
    }

    public synchronized void paintMapView(MapViewGraphics mvg)
    {
    	Rectangle  			viewPort;
    	BufferedImage 		image;
		BackgroundImageDescriptor desc;
        Vector<Integer>  	currImages  = new Vector<Integer>();
        int					i, j;
        int					id;
        boolean				repaint = false;

//        Graphics2D g = mvg.getWorldGraphics();

    	mapViewIsShowing = true;
        viewPort = mapViewGui.getViewPort();
        
        // get images to be displayed in the current viewPort
        for (i = 0; i < imgDesc.size(); i++)
        {
        	if (((imgDesc.get(i).centerX - imgDesc.get(i).sizeX/2) < viewPort.x) &&
        	    ((imgDesc.get(i).centerX + imgDesc.get(i).sizeX/2) > viewPort.x - viewPort.height) &&
        	    ((imgDesc.get(i).centerY + imgDesc.get(i).sizeY/2) > viewPort.y) &&
        	    ((imgDesc.get(i).centerY - imgDesc.get(i).sizeY/2) < viewPort.y + viewPort.width))
        	{
        		currImages.add(i);
        	}
        }
        
        
        // compare current images to last images
        if (lastImages != null)
        {
        	if (currImages.size() == lastImages.size())
        	{
        		// check image numbers
        		for (i = 0; i < currImages.size(); i++)
        		{
        			id = -1;
        			for (j = 0; j < lastImages.size(); j++)
        			{
        				if (lastImages.get(j) == currImages.get(i))
        				{
        					id = j;
        				}
        			}
        			
        			if (id < 0)
        			{
        				repaint = true;
        				break;
        			}
        		}
        	}
        	else
        	{
        		repaint = true;
        	}
        }
        else
        {
        	repaint = true;
        }

        // draw images
        if (repaint == true)
        {
	        for (j = 0; j < currImages.size(); j++)
	        {
	        	i = currImages.get(j);
	        		
				// read image 
				try
				{
					image = null;
					image = ImageIO.read(new FileImageInputStream(new File(imgSource + imgDesc.get(i).imgFileName)));
					
					// recheck size
					if (image.getWidth() != imgDesc.get(i).width)
					{
						desc = imgDesc.get(i);
						System.out.println("sizeY="+desc.sizeY+", centerY="+desc.centerY);						
						desc.sizeY  = (int)Math.abs((desc.scaleY * (double)image.getWidth()));
						
						desc.centerY -= (int)(desc.scaleY * (double)desc.width / 2.0);
						desc.centerY += (int)(desc.scaleY * (double)image.getWidth() / 2.0);
						desc.width    = image.getWidth();
						
						System.out.println("sizeY="+desc.sizeY+", centerY="+desc.centerY);						
						
						imgDesc.remove(i);
						imgDesc.add(i,desc);
					}
					
					if (image.getHeight() != imgDesc.get(i).height)
					{
						desc = imgDesc.get(i);
						desc.sizeX  = (int)Math.abs((desc.scaleX * (double)image.getHeight()));
						
						desc.centerX -= (int)(desc.scaleX * (double)desc.height / 2.0);
						desc.centerX += (int)(desc.scaleX * (double)image.getHeight() / 2.0);
						desc.height   = image.getHeight();
						
						imgDesc.remove(i);
						imgDesc.add(i,desc);						
					}
					
					
/*					AffineTransform at = new AffineTransform();
	
	                at.scale(Math.abs(imgDesc.get(i).scaleY), Math.abs(imgDesc.get(i).scaleX));
	                at.rotate( Math.PI / 2);
	                at.translate(( imgDesc.get(i).centerY * (double)(image.getWidth()) / imgDesc.get(i).sizeY) - image.getWidth()  / 2.0, 
	                        (-imgDesc.get(i).centerX * (double)(image.getHeight()) / imgDesc.get(i).sizeX) - image.getHeight() / 2.0);
	            
	
		            BufferedImageOp biop = new AffineTransformOp(at, AffineTransformOp.TYPE_BICUBIC);
		            g.drawImage(image, biop, 0, 0);
		            
		            System.out.println("drawImage="+imgDesc.get(i).imgFileName+", center="+imgDesc.get(i).centerX+", "+imgDesc.get(i).centerY+", size="+imgDesc.get(i).sizeX+", "+imgDesc.get(i).sizeY);*/

		            mapViewGui.setBackgroundImage(j, image, imgDesc.get(i).centerX, imgDesc.get(i).centerY, imgDesc.get(i).sizeY, imgDesc.get(i).sizeX, true);
				}
				catch (IOException e)
				{
					image = null;
					System.out.println("BackgroundImageViewerGui.paintMapView: cannot read image file");
				}
	    	}
        }

        // create a copy of the current images
        if (lastImages == null)
        {
        	lastImages = new Vector<Integer>();
        }
        
        lastImages.clear();
    	
    	for (i = 0; i < currImages.size(); i++)
    	{
    		lastImages.add(currImages.get(i));
    	}        
    }
    
   // search folder for description filenames 
	protected void readImageDescriptor(String source, String fileType)
	{
		File[] 	  				  files     	= new File(source).listFiles();
		String    				  fileName;
		String				      fileExtName;
		String					  line;
		BackgroundImageDescriptor desc;
		BufferedReader			  br;
		int		  				  pointId;

		for (int i = 0; i < files.length; i++)
		{
			if (files[i].isFile())
			{
				fileName 	= files[i].getName();
				pointId 	= fileName.lastIndexOf('.');
				fileExtName = fileName.substring(pointId + 1, fileName.length());
			
				// new descriptor entry
				if (fileExtName.compareToIgnoreCase(fileType) == 0)
				{
					desc = null;
					desc = new BackgroundImageDescriptor();
					desc.descFileName = fileName;
					desc.imgFileName  = fileName.substring(0, pointId) + "." + imgFileType;
					
					try
					{
						br 	  = new BufferedReader(new FileReader(imgSource + fileName));
						for (int j = 0; j < 6; j++)
						{
							line = br.readLine();
							if (line != null)
							{
								switch (j)
								{
									case 0:
										desc.scaleY = Double.parseDouble(line) * 1000.0;
										desc.width  = pixelsPerTileY;
										desc.sizeY  = (int)Math.abs(desc.scaleY * (double)desc.width);
										break;
									case 3:
										desc.scaleX = Double.parseDouble(line) * 1000.0;
										desc.height = pixelsPerTileX;
										desc.sizeX  = (int)Math.abs(desc.scaleX * (double)desc.height);
										break;
									case 4:
										desc.centerY  = (int)(Double.parseDouble(line) * 1000.0 - utmOffsetY);
										desc.centerY += (int)(desc.scaleY * (double)desc.width / 2.0);											
										break;
									case 5:
										desc.centerX  = (int)(Double.parseDouble(line) * 1000.0 - utmOffsetX);
										desc.centerX += (int)(desc.scaleX * (double)desc.height / 2.0);											
										break;
								}
							}
						}
						br.close();						
					}
					catch(FileNotFoundException e)
					{
						desc = null;
						System.out.println("BackgroundImageViewerGui.readImageDescription: descriptor file not found");
					}
					catch (IOException e)
					{
						desc = null;
						System.out.println("BackgroundImageViewerGui.readImageDescription: cannot read descriptor file");
					}
				
					// add new descriptor
					if (desc != null)
					{
						imgDesc.add(desc);
					}
				}
			}
		}
	}
}