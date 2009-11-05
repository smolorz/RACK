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
 *      Matthias Hentschel <hentschel@rts.uni-hannover.de>
 *
 */
package rack.gui.drivers;

import java.awt.*;
import javax.swing.*;

import rack.gui.GuiElementDescriptor;
import rack.gui.main.RackModuleGui;
import rack.drivers.ClockDataMsg;
import rack.drivers.ClockProxy;

public class ClockGui extends RackModuleGui
{
    protected ClockProxy   clock;

    protected JLabel     timeLabel          = new JLabel();
    protected JLabel     timeNameLabel      = new JLabel("Time", SwingConstants.RIGHT);
    protected JLabel     dateLabel          = new JLabel();
    protected JLabel     dateNameLabel      = new JLabel("Date", SwingConstants.RIGHT);
    protected JLabel     dayOfWeekLabel     = new JLabel();
    protected JLabel     dayOfWeekNameLabel = new JLabel("Day", SwingConstants.RIGHT);
    protected JLabel     utcTimeLabel       = new JLabel();
    protected JLabel     utcTimeNameLabel   = new JLabel("UTC Time", SwingConstants.RIGHT);
    protected JLabel     syncModeLabel      = new JLabel();
    protected JLabel     syncModeNameLabel  = new JLabel("Mode", SwingConstants.RIGHT);
    protected JLabel     varTLabel          = new JLabel();
    protected JLabel     varTNameLabel      = new JLabel("Variance T", SwingConstants.RIGHT);

    public ClockGui(GuiElementDescriptor guiElement)
    {
        super(guiElement);

        clock = (ClockProxy) proxy;

        JPanel buttonPanel = new JPanel(new GridLayout(0, 2, 4, 2));
        JPanel labelPanel = new JPanel(new GridLayout(0, 2, 8, 0));
        JPanel northPanel = new JPanel(new BorderLayout(2, 2));

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);
        northPanel.add(new JLabel("clock"), BorderLayout.NORTH);
        northPanel.add(buttonPanel, BorderLayout.CENTER);

        labelPanel.add(timeNameLabel);
        labelPanel.add(timeLabel);
        labelPanel.add(dateNameLabel);
        labelPanel.add(dateLabel);
        labelPanel.add(dayOfWeekNameLabel);
        labelPanel.add(dayOfWeekLabel);
        labelPanel.add(utcTimeNameLabel);
        labelPanel.add(utcTimeLabel);
        labelPanel.add(syncModeNameLabel);
        labelPanel.add(syncModeLabel);
        labelPanel.add(varTNameLabel);
        labelPanel.add(varTLabel);

        rootPanel.add(northPanel, BorderLayout.NORTH);
        rootPanel.add(labelPanel, BorderLayout.CENTER);
        
        setEnabled(false);
    }

    protected void setEnabled(boolean enabled)
    {
        timeNameLabel.setEnabled(enabled);
        timeLabel.setEnabled(enabled);
        dateNameLabel.setEnabled(enabled);
        dateLabel.setEnabled(enabled);
        dayOfWeekNameLabel.setEnabled(enabled);
        dayOfWeekLabel.setEnabled(enabled);
        utcTimeNameLabel.setEnabled(enabled);
        utcTimeLabel.setEnabled(enabled);
        syncModeNameLabel.setEnabled(enabled);
        syncModeLabel.setEnabled(enabled);
        varTNameLabel.setEnabled(enabled);
        varTLabel.setEnabled(enabled);
    }

    protected void runData()
    {
        ClockDataMsg data;

        data = clock.getData();

        if (data != null)
        {
            timeLabel.setText(data.hour + ":" + data.minute + ":" + data.second);
            dateLabel.setText(data.day + "." + data.month + "." + data.year);
            utcTimeLabel.setText(data.utcTime + " s");
            varTLabel.setText(data.varT + "");

            switch (data.dayOfWeek)
            {
               case 1:
                  dayOfWeekLabel.setText("Monday");
                  break;
               case 2:
                  dayOfWeekLabel.setText("Tuesday");
                  break;
               case 3:
                  dayOfWeekLabel.setText("Wednesday");
                  break;
               case 4:
                  dayOfWeekLabel.setText("Thursday");
                  break;
               case 5:
                  dayOfWeekLabel.setText("Friday");
                  break;
               case 6:
                  dayOfWeekLabel.setText("Saturday");
                  break;
               case 7:
                  dayOfWeekLabel.setText("Sunday");
                  break;
               default:
                  dayOfWeekLabel.setText(data.dayOfWeek +"");
                  break;
            }

            switch (data.syncMode)
            {
               case ClockDataMsg.CLOCK_SYNC_MODE_NONE:
                  syncModeLabel.setText("No Sync");
                  break;
               case ClockDataMsg.CLOCK_SYNC_MODE_REMOTE:
                  syncModeLabel.setText("Sync");
                  break;
               default:
                  syncModeLabel.setText(data.syncMode + "");
                  break;
            }

            setEnabled(true);
        }
        else
        {
            setEnabled(false);
        }
    }
}
