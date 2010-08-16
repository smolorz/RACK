/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2007 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Marko Reimer  <reimer@rts.uni-hannover.de>
 *
 */
package rack.gui.perception;

import java.awt.*;
import java.awt.event.*;
import java.util.StringTokenizer;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import javax.sound.midi.MidiChannel;
import javax.sound.midi.MidiSystem;
import javax.sound.midi.MidiUnavailableException;
import javax.sound.midi.Synthesizer;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.*;
import rack.main.*;
import rack.main.defines.Position3d;
import rack.main.defines.Point2d;
import rack.main.defines.ObjRecogObject;
import rack.perception.ObjRecogDataMsg;
import rack.perception.ObjRecogProxy;

public class ObjRecogGui extends RackModuleGui implements MapViewInterface
{
    protected ObjRecogDataMsg objRecogData;
    protected ObjRecogProxy   objRecog;

    protected JLabel          objectNumNameLabel = new JLabel("Object Num", SwingConstants.RIGHT);
    protected JLabel          objectIndexNameLabel = new JLabel("Object Index", SwingConstants.RIGHT);
    protected JLabel          xNameLabel         = new JLabel("X [mm]", SwingConstants.RIGHT);
    protected JLabel          yNameLabel         = new JLabel("Y [mm]", SwingConstants.RIGHT);
    protected JLabel          zNameLabel         = new JLabel("Z [mm]", SwingConstants.RIGHT);
    protected JLabel          phiNameLabel       = new JLabel("Phi [rad]", SwingConstants.RIGHT);
    protected JLabel          psiNameLabel       = new JLabel("Psi [rad]", SwingConstants.RIGHT);
    protected JLabel          rhoNameLabel       = new JLabel("Rho [rad]", SwingConstants.RIGHT);
    protected JLabel          xVarNameLabel      = new JLabel("Var X [mm]", SwingConstants.RIGHT);
    protected JLabel          yVarNameLabel      = new JLabel("Var Y [mm]", SwingConstants.RIGHT);
    protected JLabel          zVarNameLabel      = new JLabel("Var Z [mm]", SwingConstants.RIGHT);
    protected JLabel          phiVarNameLabel    = new JLabel("Var Phi [rad]", SwingConstants.RIGHT);
    protected JLabel          psiVarNameLabel    = new JLabel("Var Psi [rad]", SwingConstants.RIGHT);
    protected JLabel          rhoVarNameLabel    = new JLabel("VarRho [rad]", SwingConstants.RIGHT);
    protected JLabel          vxNameLabel        = new JLabel("VX [mm/s]", SwingConstants.RIGHT);
    protected JLabel          vyNameLabel        = new JLabel("VY [mm/s]", SwingConstants.RIGHT);
    protected JLabel          omegaNameLabel     = new JLabel("Omega [rad/s]", SwingConstants.RIGHT);
    protected JLabel          vxVarNameLabel     = new JLabel("Var VX [mm/s]", SwingConstants.RIGHT);
    protected JLabel          vyVarNameLabel     = new JLabel("Var VY [mm/s]", SwingConstants.RIGHT);
    protected JLabel          omegaVarNameLabel  = new JLabel("Var Omega [rad/s]", SwingConstants.RIGHT);
    protected JLabel          xDimNameLabel      = new JLabel("Dim X [mm]", SwingConstants.RIGHT);
    protected JLabel          yDimNameLabel      = new JLabel("Dim Y [mm]", SwingConstants.RIGHT);
    protected JLabel          zDimNameLabel      = new JLabel("Dim Z [mm]", SwingConstants.RIGHT);
    protected JLabel          probNameLabel      = new JLabel("Prob [%]", SwingConstants.RIGHT);
    protected JLabel          xINameLabel        = new JLabel("X [px]", SwingConstants.RIGHT);
    protected JLabel          yINameLabel        = new JLabel("Y [px]", SwingConstants.RIGHT);
    protected JLabel          widthINameLabel    = new JLabel("Width [px]", SwingConstants.RIGHT);
    protected JLabel          heightINameLabel   = new JLabel("Height [px]", SwingConstants.RIGHT);
    protected JLabel          objectIdNameLabel  = new JLabel("Object Id", SwingConstants.RIGHT);
    protected JLabel          objectTypeNameLabel = new JLabel("Object Type", SwingConstants.RIGHT);
    protected JLabel          xRefNameLabel      = new JLabel("Ref X [mm]", SwingConstants.RIGHT);
    protected JLabel          yRefNameLabel      = new JLabel("Ref Y [mm]", SwingConstants.RIGHT);
    protected JLabel          zRefNameLabel      = new JLabel("Ref Z [mm]", SwingConstants.RIGHT);
    protected JLabel          phiRefNameLabel    = new JLabel("Ref Phi [rad]", SwingConstants.RIGHT);
    protected JLabel          psiRefNameLabel    = new JLabel("Ref Psi [rad]", SwingConstants.RIGHT);
    protected JLabel          rhoRefNameLabel    = new JLabel("Ref Rho [rad]", SwingConstants.RIGHT);
    protected JLabel          xVarRefNameLabel   = new JLabel("Ref Var X [mm]", SwingConstants.RIGHT);
    protected JLabel          yVarRefNameLabel   = new JLabel("Ref Var Y [mm]", SwingConstants.RIGHT);
    protected JLabel          zVarRefNameLabel   = new JLabel("Ref Var Z [mm]", SwingConstants.RIGHT);
    protected JLabel          phiVarRefNameLabel = new JLabel("Ref Var Phi [rad]", SwingConstants.RIGHT);
    protected JLabel          psiVarRefNameLabel = new JLabel("Ref Var Psi [rad]", SwingConstants.RIGHT);
    protected JLabel          rhoVarRefNameLabel = new JLabel("Ref Var Rho [rad]", SwingConstants.RIGHT);

    protected JLabel          objectNumLabel     = new JLabel("0");
    protected JLabel          objectIndexLabel   = new JLabel("0");
    protected JLabel          objectTypeLabel    = new JLabel("0");
    protected JLabel          xLabel             = new JLabel("-000000");
    protected JLabel          yLabel             = new JLabel("-000000");
    protected JLabel          zLabel             = new JLabel("-000000");
    protected JLabel          phiLabel           = new JLabel("-000.00");
    protected JLabel          psiLabel           = new JLabel("-000.00");
    protected JLabel          rhoLabel           = new JLabel("-000.00");
    protected JLabel          xVarLabel          = new JLabel("-000000");
    protected JLabel          yVarLabel          = new JLabel("-000000");
    protected JLabel          zVarLabel          = new JLabel("-000000");
    protected JLabel          phiVarLabel        = new JLabel("-000.00");
    protected JLabel          psiVarLabel        = new JLabel("-000.00");
    protected JLabel          rhoVarLabel        = new JLabel("-000.00");
    protected JLabel          vxLabel            = new JLabel("-000000");
    protected JLabel          vyLabel            = new JLabel("-000000");
    protected JLabel          omegaLabel         = new JLabel("-000.00");
    protected JLabel          vxVarLabel         = new JLabel("-000000");
    protected JLabel          vyVarLabel         = new JLabel("-000000");
    protected JLabel          omegaVarLabel      = new JLabel("-000.00");
    protected JLabel          xDimLabel          = new JLabel("-000000");
    protected JLabel          yDimLabel          = new JLabel("-000000");
    protected JLabel          zDimLabel          = new JLabel("-000000");
    protected JLabel          probLabel          = new JLabel("-000000");
    protected JLabel          xILabel            = new JLabel("-000000");
    protected JLabel          yILabel            = new JLabel("-000000");
    protected JLabel          widthILabel        = new JLabel("-000000");
    protected JLabel          heightILabel       = new JLabel("-000000");
    protected JLabel          objectIdLabel      = new JLabel();
    protected JLabel          xRefLabel          = new JLabel("-000000");
    protected JLabel          yRefLabel          = new JLabel("-000000");
    protected JLabel          zRefLabel          = new JLabel("-000000");
    protected JLabel          phiRefLabel        = new JLabel("-000.00");
    protected JLabel          psiRefLabel        = new JLabel("-000.00");
    protected JLabel          rhoRefLabel        = new JLabel("-000.00");
    protected JLabel          xVarRefLabel       = new JLabel("-000000");
    protected JLabel          yVarRefLabel       = new JLabel("-000000");
    protected JLabel          zVarRefLabel       = new JLabel("-000000");
    protected JLabel          phiVarRefLabel     = new JLabel("-000.00");
    protected JLabel          psiVarRefLabel     = new JLabel("-000.00");
    protected JLabel          rhoVarRefLabel     = new JLabel("-000.00");

    protected JButton         estimateButton     = new JButton("Manual update");
    protected JSpinner	      objIndexSpinner    = new JSpinner();

    protected String          setEstimateCommand;

    protected boolean         mapViewIsShowing;
    protected MapViewGui      mapViewGui;

    // for sound output
	private Synthesizer	    synth;
	private MidiChannel     midiChannels[];
	private int             currentNote  = 97;

    public ObjRecogGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        objRecog = (ObjRecogProxy) proxy;

        JPanel northPanel = new JPanel(new BorderLayout(2,2));
        JPanel buttonPanel = new JPanel(new GridLayout(0,2,4,2));
        JPanel labelPanel = new JPanel(new GridLayout(0,2,8,0));
        JPanel southPanel = new JPanel(new GridLayout(0,2,4,0));

        objIndexSpinner.setModel(new SpinnerNumberModel(0, 0, 100, 1));
        objIndexSpinner.addChangeListener(new ChangeListener() {
			public void stateChanged(ChangeEvent e) {
//				runData();
			}
		});

        estimateButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e)
            {
                String s = (String) JOptionPane.showInputDialog(null, "Estimated object position is:\n" + "x, y, rho, objectId",
                        "Estimated object position", JOptionPane.PLAIN_MESSAGE, null, null, "0,0,0.0,1");
                if ((s != null) && (s.length() > 0))
                {
                    StringTokenizer st = new StringTokenizer(s, ",");
                    if (st.countTokens() == 4)
                    {
                        ObjRecogDataMsg objEstimateData = new ObjRecogDataMsg();

                        objEstimateData.object = new ObjRecogObject[1];
                        objEstimateData.objectNum = 1;
                        objEstimateData.object[0] = new ObjRecogObject();
                        objEstimateData.object[0].pos.x = Integer.parseInt(st.nextToken());
                        objEstimateData.object[0].pos.y = Integer.parseInt(st.nextToken());
                        objEstimateData.object[0].pos.rho = (float) Math.toRadians(Float.parseFloat(st.nextToken()));
                        objEstimateData.object[0].objectId = Integer.parseInt(st.nextToken());

                        objRecog.setEstimate(objEstimateData, 0);
                    }
                }
            }
        });

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);

        northPanel.add(new JLabel(RackName.nameString(objRecog.getCommandMbx())),BorderLayout.CENTER);
        northPanel.add(buttonPanel,BorderLayout.SOUTH);

        labelPanel.add(objectNumNameLabel);
        labelPanel.add(objectNumLabel);
        labelPanel.add(objectIndexNameLabel);
        labelPanel.add(objectIndexLabel);
        labelPanel.add(xNameLabel);
        labelPanel.add(xLabel);
        labelPanel.add(yNameLabel);
        labelPanel.add(yLabel);
        labelPanel.add(zNameLabel);
        labelPanel.add(zLabel);
        labelPanel.add(phiNameLabel);
        labelPanel.add(phiLabel);
        labelPanel.add(psiNameLabel);
        labelPanel.add(psiLabel);
        labelPanel.add(rhoNameLabel);
        labelPanel.add(rhoLabel);
        labelPanel.add(xVarNameLabel);
        labelPanel.add(xVarLabel);
        labelPanel.add(yVarNameLabel);
        labelPanel.add(yVarLabel);
        labelPanel.add(zVarNameLabel);
        labelPanel.add(zVarLabel);
        labelPanel.add(phiVarNameLabel);
        labelPanel.add(phiVarLabel);
        labelPanel.add(psiVarNameLabel);
        labelPanel.add(psiVarLabel);
        labelPanel.add(rhoVarNameLabel);
        labelPanel.add(rhoVarLabel);
        labelPanel.add(vxNameLabel);
        labelPanel.add(vxLabel);
        labelPanel.add(vyNameLabel);
        labelPanel.add(vyLabel);
        labelPanel.add(omegaNameLabel);
        labelPanel.add(omegaLabel);
        labelPanel.add(vxVarNameLabel);
        labelPanel.add(vxVarLabel);
        labelPanel.add(vyVarNameLabel);
        labelPanel.add(vyVarLabel);
        labelPanel.add(omegaVarNameLabel);
        labelPanel.add(omegaVarLabel);
        labelPanel.add(xDimNameLabel);
        labelPanel.add(xDimLabel);
        labelPanel.add(yDimNameLabel);
        labelPanel.add(yDimLabel);
        labelPanel.add(zDimNameLabel);
        labelPanel.add(zDimLabel);
        labelPanel.add(probNameLabel);
        labelPanel.add(probLabel);
        labelPanel.add(xINameLabel);
        labelPanel.add(xILabel);
        labelPanel.add(yINameLabel);
        labelPanel.add(yILabel);
        labelPanel.add(widthINameLabel);
        labelPanel.add(widthILabel);
        labelPanel.add(heightINameLabel);
        labelPanel.add(heightILabel);
        labelPanel.add(objectIdNameLabel);
        labelPanel.add(objectIdLabel);
        labelPanel.add(objectTypeNameLabel);
        labelPanel.add(objectTypeLabel);
        labelPanel.add(xRefNameLabel);
        labelPanel.add(xRefLabel);
        labelPanel.add(yRefNameLabel);
        labelPanel.add(yRefLabel);
        labelPanel.add(zRefNameLabel);
        labelPanel.add(zRefLabel);
        labelPanel.add(phiRefNameLabel);
        labelPanel.add(phiRefLabel);
        labelPanel.add(psiRefNameLabel);
        labelPanel.add(psiRefLabel);
        labelPanel.add(rhoRefNameLabel);
        labelPanel.add(rhoRefLabel);
        labelPanel.add(xVarRefNameLabel);
        labelPanel.add(xVarRefLabel);
        labelPanel.add(yVarRefNameLabel);
        labelPanel.add(yVarRefLabel);
        labelPanel.add(zVarRefNameLabel);
        labelPanel.add(zVarRefLabel);
        labelPanel.add(phiVarRefNameLabel);
        labelPanel.add(phiVarRefLabel);
        labelPanel.add(psiVarRefNameLabel);
        labelPanel.add(psiVarRefLabel);
        labelPanel.add(rhoVarRefNameLabel);
        labelPanel.add(rhoVarRefLabel);

        southPanel.add(estimateButton,BorderLayout.WEST);
        southPanel.add(objIndexSpinner,BorderLayout.EAST);

        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(labelPanel, BorderLayout.CENTER);
        rootPanel.add(southPanel, BorderLayout.SOUTH);

        setEnabled(false);

        if (guiElement.hasParameter("sound"))
        {
		    try
		    {
			    synth = MidiSystem.getSynthesizer();
			    synth.open();
			    midiChannels = synth.getChannels();
			    midiChannels[0].programChange(43);
		    }
		    catch (MidiUnavailableException e)
		    {
			    e.printStackTrace();
		    }
        }
        else
        {
        	synth = null;
        }
    }

    protected void setEnabled(boolean enabled)
    {
        objectNumNameLabel.setEnabled(enabled);
        objectIndexNameLabel.setEnabled(enabled);
        xNameLabel.setEnabled(enabled);
        yNameLabel.setEnabled(enabled);
        zNameLabel.setEnabled(enabled);
        phiNameLabel.setEnabled(enabled);
        psiNameLabel.setEnabled(enabled);
        rhoNameLabel.setEnabled(enabled);
        xVarNameLabel.setEnabled(enabled);
        yVarNameLabel.setEnabled(enabled);
        zVarNameLabel.setEnabled(enabled);
        phiVarNameLabel.setEnabled(enabled);
        psiVarNameLabel.setEnabled(enabled);
        rhoVarNameLabel.setEnabled(enabled);
        vxNameLabel.setEnabled(enabled);
        vyNameLabel.setEnabled(enabled);
        omegaNameLabel.setEnabled(enabled);
        vxVarNameLabel.setEnabled(enabled);
        vyVarNameLabel.setEnabled(enabled);
        omegaVarNameLabel.setEnabled(enabled);
        xDimNameLabel.setEnabled(enabled);
        yDimNameLabel.setEnabled(enabled);
        zDimNameLabel.setEnabled(enabled);
        probNameLabel.setEnabled(enabled);
        xINameLabel.setEnabled(enabled);
        yINameLabel.setEnabled(enabled);
        widthINameLabel.setEnabled(enabled);
        heightINameLabel.setEnabled(enabled);
        objectIdNameLabel.setEnabled(enabled);
        objectTypeNameLabel.setEnabled(enabled);
        xRefNameLabel.setEnabled(enabled);
        yRefNameLabel.setEnabled(enabled);
        zRefNameLabel.setEnabled(enabled);
        phiRefNameLabel.setEnabled(enabled);
        psiRefNameLabel.setEnabled(enabled);
        rhoRefNameLabel.setEnabled(enabled);
        xVarRefNameLabel.setEnabled(enabled);
        yVarRefNameLabel.setEnabled(enabled);
        zVarRefNameLabel.setEnabled(enabled);
        phiVarRefNameLabel.setEnabled(enabled);
        psiVarRefNameLabel.setEnabled(enabled);
        rhoVarRefNameLabel.setEnabled(enabled);

        objectNumLabel.setEnabled(enabled);
        objectIndexLabel.setEnabled(enabled);
        xLabel.setEnabled(enabled);
        yLabel.setEnabled(enabled);
        zLabel.setEnabled(enabled);
        phiLabel.setEnabled(enabled);
        psiLabel.setEnabled(enabled);
        rhoLabel.setEnabled(enabled);
        xVarLabel.setEnabled(enabled);
        yVarLabel.setEnabled(enabled);
        zVarLabel.setEnabled(enabled);
        phiVarLabel.setEnabled(enabled);
        psiVarLabel.setEnabled(enabled);
        rhoVarLabel.setEnabled(enabled);
        vxLabel.setEnabled(enabled);
        vyLabel.setEnabled(enabled);
        omegaLabel.setEnabled(enabled);
        vxVarLabel.setEnabled(enabled);
        vyVarLabel.setEnabled(enabled);
        omegaVarLabel.setEnabled(enabled);
        xDimLabel.setEnabled(enabled);
        yDimLabel.setEnabled(enabled);
        zDimLabel.setEnabled(enabled);
        probLabel.setEnabled(enabled);
        xILabel.setEnabled(enabled);
        yILabel.setEnabled(enabled);
        widthILabel.setEnabled(enabled);
        heightILabel.setEnabled(enabled);
        objectIdLabel.setEnabled(enabled);
        objectTypeLabel.setEnabled(enabled);
        estimateButton.setEnabled(enabled);
        xRefLabel.setEnabled(enabled);
        yRefLabel.setEnabled(enabled);
        zRefLabel.setEnabled(enabled);
        phiRefLabel.setEnabled(enabled);
        psiRefLabel.setEnabled(enabled);
        rhoRefLabel.setEnabled(enabled);
        xVarRefLabel.setEnabled(enabled);
        yVarRefLabel.setEnabled(enabled);
        zVarRefLabel.setEnabled(enabled);
        phiVarRefLabel.setEnabled(enabled);
        psiVarRefLabel.setEnabled(enabled);
        rhoVarRefLabel.setEnabled(enabled);
    }

    protected void runStart()
    {
        mapViewGui = MapViewGui.findMapViewGui(ge);
        if(mapViewGui != null)
        {
            mapViewGui.addMapView(this);
            setEstimateCommand = this.getName() + " - set estimate";
            mapViewGui.addMapViewAction(setEstimateCommand, this);
        }
    }

    protected void runStop()
    {
        if(mapViewGui != null)
        {
            mapViewGui.removeMapView(this);
            mapViewGui.removeMapViewActions(this);
        }
    }

    protected boolean needsRunData()
    {
        return (super.needsRunData() || mapViewIsShowing);
    }

    protected void runData()
    {
        ObjRecogDataMsg data;
        int objIndex;

        data = objRecog.getData();

        if(data != null)
        {
            synchronized(this)
            {
                objRecogData = data;
            }
            objIndex = ((SpinnerNumberModel) objIndexSpinner.getModel()).getNumber().intValue();
            if(data.objectNum > objIndex)
            {
                objectNumLabel.setText(data.objectNum + "");
                objectIndexLabel.setText(objIndex + "");
                xLabel.setText(data.object[objIndex].pos.x + "");
                yLabel.setText(data.object[objIndex].pos.y + "");
                zLabel.setText(data.object[objIndex].pos.z + "");
                phiLabel.setText(Math.rint(Math.toDegrees(data.object[objIndex].pos.phi)*100.0)/100.0 + "");
                psiLabel.setText(Math.rint(Math.toDegrees(data.object[objIndex].pos.psi)*100.0)/100.0 + "");
                rhoLabel.setText(Math.rint(Math.toDegrees(data.object[objIndex].pos.rho)*100.0)/100.0 + "");
                xVarLabel.setText(data.object[objIndex].varPos.x + "");
                yVarLabel.setText(data.object[objIndex].varPos.y + "");
                zVarLabel.setText(data.object[objIndex].varPos.z + "");
                phiVarLabel.setText(Math.rint(Math.toDegrees(data.object[objIndex].varPos.phi)*100.0)/100.0 + "");
                psiVarLabel.setText(Math.rint(Math.toDegrees(data.object[objIndex].varPos.psi)*100.0)/100.0 + "");
                rhoVarLabel.setText(Math.rint(Math.toDegrees(data.object[objIndex].varPos.rho)*100.0)/100.0 + "");
                vxLabel.setText(data.object[objIndex].vel.x + "");
                vyLabel.setText(data.object[objIndex].vel.y + "");
                omegaLabel.setText(Math.rint(Math.toDegrees(data.object[objIndex].vel.rho)*100.0)/100.0 + "");
                vxVarLabel.setText(data.object[objIndex].varVel.x + "");
                vyVarLabel.setText(data.object[objIndex].varVel.y + "");
                omegaVarLabel.setText(Math.rint(Math.toDegrees(data.object[objIndex].varVel.rho)*100.0)/100.0 + "");
                xDimLabel.setText(data.object[objIndex].dim.x + "");
                yDimLabel.setText(data.object[objIndex].dim.y + "");
                zDimLabel.setText(data.object[objIndex].dim.z + "");
                probLabel.setText(data.object[objIndex].prob + "");
                xILabel.setText(data.object[objIndex].imageArea.x + "");
                yILabel.setText(data.object[objIndex].imageArea.y + "");
                widthILabel.setText(data.object[objIndex].imageArea.width + "");
                heightILabel.setText(data.object[objIndex].imageArea.height + "");
                objectIdLabel.setText(data.object[objIndex].objectId + "");
                objectTypeLabel.setText(data.object[objIndex].type + "");
                xRefLabel.setText(data.refPos.x + "");
                yRefLabel.setText(data.refPos.y + "");
                zRefLabel.setText(data.refPos.z + "");
                phiRefLabel.setText(Math.rint(Math.toDegrees(data.refPos.phi)*100.0)/100.0 + "");
                psiRefLabel.setText(Math.rint(Math.toDegrees(data.refPos.psi)*100.0)/100.0 + "");
                rhoRefLabel.setText(Math.rint(Math.toDegrees(data.refPos.rho)*100.0)/100.0 + "");
                xVarRefLabel.setText(data.varRefPos.x + "");
                yVarRefLabel.setText(data.varRefPos.y + "");
                zVarRefLabel.setText(data.varRefPos.z + "");
                phiVarRefLabel.setText(Math.rint(Math.toDegrees(data.varRefPos.phi)*100.0)/100.0 + "");
                psiVarRefLabel.setText(Math.rint(Math.toDegrees(data.varRefPos.psi)*100.0)/100.0 + "");
                rhoVarRefLabel.setText(Math.rint(Math.toDegrees(data.varRefPos.rho)*100.0)/100.0 + "");
            }
            else
            {
                objectNumLabel.setText("0");
                objectIndexLabel.setText(objIndex+"");
                xLabel.setText("0");
                yLabel.setText("0");
                zLabel.setText("0");
                phiLabel.setText("0");
                psiLabel.setText("0");
                rhoLabel.setText("0");
                xVarLabel.setText("0");
                yVarLabel.setText("0");
                zVarLabel.setText("0");
                phiVarLabel.setText("0");
                psiVarLabel.setText("0");
                rhoVarLabel.setText("0");
                vxLabel.setText("0");
                vyLabel.setText("0");
                omegaLabel.setText("0");
                vxVarLabel.setText("0");
                vyVarLabel.setText("0");
                omegaVarLabel.setText("0");
                xDimLabel.setText("0");
                yDimLabel.setText("0");
                zDimLabel.setText("0");
                probLabel.setText("0");
                xILabel.setText("0");
                yILabel.setText("0");
                widthILabel.setText("0");
                heightILabel.setText("0");
                objectIdLabel.setText("0");
                objectTypeLabel.setText("0");
                xRefLabel.setText("0");
                yRefLabel.setText("0");
                zRefLabel.setText("0");
                phiRefLabel.setText("0");
                psiRefLabel.setText("0");
                rhoRefLabel.setText("0");
                xVarRefLabel.setText("0");
                yVarRefLabel.setText("0");
                zVarRefLabel.setText("0");
                phiVarRefLabel.setText("0");
                psiVarRefLabel.setText("0");
                rhoVarRefLabel.setText("0");
            }

            setEnabled(true);

            if ((synth != null) && (data.objectNum > 0))
            {
        	    midiChannels[0].noteOn(currentNote, 115);
//        	    midiChannels[0].noteOn(newNote, 127);
        	    midiChannels[0].noteOff(currentNote);
            }
        }
        else
        {
            setEnabled(false);
            if (synth != null)
            {
                midiChannels[0].noteOff(currentNote, 115);
            }
        }
        mapViewIsShowing = false;
    }

    public void mapViewActionPerformed(MapViewActionEvent event)
    {
        String command = event.getActionCommand();

        if(command.equals(setEstimateCommand))
        {
        	if (event.getEventId() == MapViewActionEvent.MOUSE_CLICKED_EVENT)
	        {
	            System.out.println("ObjRecog set estimate " + event.getRobotCursorPos() + " (ref " + event.getRobotPosition() + ")");

	            ObjRecogDataMsg objEstimateData = new ObjRecogDataMsg();
	            objEstimateData.refPos = new Position3d(event.getRobotPosition());
	            Position3d estimate = new Position3d(event.getRobotCursorPos());

	            objEstimateData.object = new ObjRecogObject[1];
	            objEstimateData.objectNum = 1;
	            objEstimateData.object[0] = new ObjRecogObject();
	            objEstimateData.object[0].pos = estimate;

	            objRecog.setEstimate(objEstimateData, 0);
	        }
        }
    }

    public synchronized void paintMapView(MapViewGraphics mvg)
    {
        int xEnd, yEnd;
    	Point2d cornerPoints[] = new Point2d[4];//lo, ro, lu, ru
    	double  rot_matrix[][] = {{0,0},{0,0}};
    	Point2d tempCornerPoint = new Point2d();

    	cornerPoints[0] = new Point2d();
    	cornerPoints[1] = new Point2d();
    	cornerPoints[2] = new Point2d();
    	cornerPoints[3] = new Point2d();

        mapViewIsShowing = true;

        if (objRecogData == null)
            return;

        Graphics2D g = mvg.getRobotGraphics(objRecogData.recordingTime, ge.getSystem());
        g.setStroke(new BasicStroke(100));

        // object painting
        for (int i = 0; i < objRecogData.objectNum; i++)
        {
            // select color
            switch (objRecogData.object[i].objectId % 6)
            {
                case 0:
                    g.setColor(Color.BLACK);
                    break;
                case 1:
                    g.setColor(Color.GREEN);
                    break;
                case 2:
                    g.setColor(Color.ORANGE);
                    break;
                case 3:
                    g.setColor(Color.MAGENTA);
                    break;
                case 4:
                    g.setColor(Color.CYAN);
                    break;
                case 5:
                    g.setColor(Color.PINK);
                    break;
            }

            if ((objRecogData.object[i].dim.x != 0) || (objRecogData.object[i].dim.y != 0))
            {
                xEnd = (int)(objRecogData.object[i].dim.x / 2 * Math.cos(objRecogData.object[i].pos.rho)) + objRecogData.object[i].pos.x;
                yEnd = (int)(objRecogData.object[i].dim.x / 2 * Math.sin(objRecogData.object[i].pos.rho)) + objRecogData.object[i].pos.y;
                g.drawLine(objRecogData.object[i].pos.x, objRecogData.object[i].pos.y, xEnd, yEnd);

            	cornerPoints[0].x = -objRecogData.object[i].dim.x / 2;
            	cornerPoints[0].y =  objRecogData.object[i].dim.y / 2;
            	cornerPoints[1].x =  objRecogData.object[i].dim.x / 2;
            	cornerPoints[1].y =  objRecogData.object[i].dim.y / 2;
            	cornerPoints[2].x = -objRecogData.object[i].dim.x / 2;
            	cornerPoints[2].y = -objRecogData.object[i].dim.y / 2;
            	cornerPoints[3].x =  objRecogData.object[i].dim.x / 2;
            	cornerPoints[3].y = -objRecogData.object[i].dim.y / 2;

            	//rotate unit vectors to orentation of box
            	rot_matrix[0][0] =  Math.cos(objRecogData.object[i].pos.rho);
            	rot_matrix[0][1] = -Math.sin(objRecogData.object[i].pos.rho);
            	rot_matrix[1][0] =  Math.sin(objRecogData.object[i].pos.rho);
            	rot_matrix[1][1] =  Math.cos(objRecogData.object[i].pos.rho);

            	tempCornerPoint.x = cornerPoints[0].x;
            	tempCornerPoint.y = cornerPoints[0].y;
            	cornerPoints[0].x = (int)( rot_matrix[0][0] * tempCornerPoint.x + rot_matrix[0][1] * tempCornerPoint.y) ;
            	cornerPoints[0].y = (int)( rot_matrix[1][0] * tempCornerPoint.x + rot_matrix[1][1] * tempCornerPoint.y) ;
            	tempCornerPoint.x = cornerPoints[1].x;
            	tempCornerPoint.y = cornerPoints[1].y;
            	cornerPoints[1].x = (int)( rot_matrix[0][0] * tempCornerPoint.x + rot_matrix[0][1] * tempCornerPoint.y) ;
            	cornerPoints[1].y = (int)( rot_matrix[1][0] * tempCornerPoint.x + rot_matrix[1][1] * tempCornerPoint.y) ;
            	tempCornerPoint.x = cornerPoints[2].x;
            	tempCornerPoint.y = cornerPoints[2].y;
            	cornerPoints[2].x = (int)( rot_matrix[0][0] * tempCornerPoint.x + rot_matrix[0][1] * tempCornerPoint.y) ;
            	cornerPoints[2].y = (int)( rot_matrix[1][0] * tempCornerPoint.x + rot_matrix[1][1] * tempCornerPoint.y) ;
            	tempCornerPoint.x = cornerPoints[3].x;
            	tempCornerPoint.y = cornerPoints[3].y;
            	cornerPoints[3].x = (int)( rot_matrix[0][0] * tempCornerPoint.x + rot_matrix[0][1] * tempCornerPoint.y) ;
            	cornerPoints[3].y = (int)( rot_matrix[1][0] * tempCornerPoint.x + rot_matrix[1][1] * tempCornerPoint.y) ;

                //print corner points of box
                g.drawLine(objRecogData.object[i].pos.x + cornerPoints[0].x,
                		   objRecogData.object[i].pos.y + cornerPoints[0].y,
                		   objRecogData.object[i].pos.x + cornerPoints[1].x,
                		   objRecogData.object[i].pos.y + cornerPoints[1].y);
                g.drawLine(objRecogData.object[i].pos.x + cornerPoints[0].x,
                    		objRecogData.object[i].pos.y + cornerPoints[0].y,
                    		objRecogData.object[i].pos.x + cornerPoints[2].x,
                	    	objRecogData.object[i].pos.y + cornerPoints[2].y);
                g.drawLine(objRecogData.object[i].pos.x + cornerPoints[3].x,
                			objRecogData.object[i].pos.y + cornerPoints[3].y,
                			objRecogData.object[i].pos.x + cornerPoints[1].x,
                			objRecogData.object[i].pos.y + cornerPoints[1].y);
                g.drawLine(objRecogData.object[i].pos.x + cornerPoints[3].x,
                			objRecogData.object[i].pos.y + cornerPoints[3].y,
                			objRecogData.object[i].pos.x + cornerPoints[2].x,
                			objRecogData.object[i].pos.y + cornerPoints[2].y);
            } else
            {

                g.drawArc(objRecogData.object[i].pos.x - 100,
                        objRecogData.object[i].pos.y - 100,
                        200, 200, 0, 360);

                xEnd = (int)(1000 * Math.cos(objRecogData.object[i].pos.rho)) +  objRecogData.object[i].pos.x;
                yEnd = (int)(1000 * Math.sin(objRecogData.object[i].pos.rho)) + objRecogData.object[i].pos.y;
                g.drawLine(objRecogData.object[i].pos.x, objRecogData.object[i].pos.y, xEnd, yEnd);
            }
        }
    }
}
