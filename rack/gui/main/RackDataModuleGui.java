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
package rack.gui.main;

import java.util.Vector;

import javax.swing.JComponent;
import javax.swing.JLabel;

import rack.gui.GuiElementDescriptor;
import rack.main.*;
import rack.main.tims.*;

/**
 *
 * Entspricht dem ContinuousDataModule unter C Periodisches Verschicken der
 * Daten noch nicht richtig implementiert. Versenden an alle eingetragegenen
 * Zuhoerer bisher ueber writeWorkPackage()
 *
 */
public abstract class RackDataModuleGui extends RackModuleGui
{

    /** Modul hat ein ON-Kommandopaket empfangen und soll sich einschalten */
    public abstract boolean moduleOn();

    /** Modul hat ein OFF-Kommandopaket empfangen und soll sich ausschalten */
    public abstract void moduleOff();

    /** Modul soll die Daten zu kontinuierliche Daten liefern */
    public abstract boolean moduleLoop();

    /** Modul soll nach beenden aufraeumen */
    public abstract void moduleCleanup();

    /** Modul empfaengt internes Paket */
    public abstract void moduleCommand(TimsRawMsg raw);

    protected Tims tims;
    protected GDOS gdos;
    /** Kommandomailbox */
    protected TimsMbx commandMbx;
    /** Arbeitsmailbox */
    protected TimsMbx workMbx;
    /** Datenmailbox */
    protected TimsMbx dataMbx;
    
    /** Pause im disabled-Zustand in [ms] */
    protected int disabledTime = 100;
    /** Pause im error-Zustand in [ms] */
    protected int errorTime = 1000;

    /** Definiert in welchen Status sich das Modul befindet */
    protected byte moduleStatus = RackProxy.MSG_DISABLED;
    /** Wunschstatus */
    protected int moduleTargetStatus = RackProxy.MSG_DISABLED;
    /** mailbox, in der On-Kommandos beantwortet werden */
    protected int notifyIfOn = 0;
    /** Merker der Packet-Id zur Notify-Antwort */
    protected byte notifyIfOnId;

    /** Liste der Zuhoerer von kontinuierlichen Daten */
    protected Vector<Integer> dataListener = new Vector<Integer>();
    protected TimsMsg dataMsg = null;

    /** Data module period time */
    protected int periodTime = 100;

    /**
     * Initialisiert die Mailboxen des Moduls und startet die Task zum
     * Paketempfang
     */
    public RackDataModuleGui(int moduleName, int instance, GuiElementDescriptor guiElement)
    {
        this.tims = guiElement.getMainGui().getTims();

        try
        {
            commandMbx = tims.mbxInit(RackName.create(RackName.JOYSTICK, guiElement.getInstance(),0));
            dataMbx    = tims.mbxInit(RackName.create(RackName.JOYSTICK, guiElement.getInstance(),1));
            workMbx    = tims.mbxInit(RackName.create(RackName.JOYSTICK, guiElement.getInstance(),2));

            this.gdos = new GDOS(commandMbx, GDOS.WARNING);

            DataThread dt = new DataThread();
            dt.setDaemon(true); // Thread beendet sich wenn Programm sich
                                // beendet
            dt.start();
        }
        catch (TimsException e)
        {
            e.printStackTrace();
        }
    }

    /** beenden des Programmablaufs */
    public void terminate()
    {
        moduleCleanup();

        try
        {
            if(commandMbx != null)
                tims.mbxDelete(commandMbx);
            if(dataMbx != null)
                tims.mbxDelete(dataMbx);
            if(workMbx != null)
                tims.mbxDelete(workMbx);
        }
        catch (TimsException e) {}
        
        super.terminate();
    }

    public void setGdosLevel(byte gdosLevel)
    {
        gdos.setGdosLevel(gdosLevel);
    }

    /**
     * Gibt den container des GUIs zurueck Die Methode muss, wenn es ein Gui
     * gibt, ueberschrieben werden
     */
    public JComponent getComponent()
    {
        return (new JLabel("No Gui implemented yet."));
    }

    /** Sendet das uebergebene Paket an alle Listener */
    public void writeWorkMsg(TimsMsg msg)
    {
        dataMsg = msg;

        synchronized(dataListener)
        {
            for (int i = 0; i < dataListener.size(); i++)
            {
                try
                {
                    // System.out.println("Sende Karte to
                    // Listener:"+((Integer)listener.elementAt(i)).intValue());
                    msg.dest = dataListener.elementAt(i).intValue();
                    commandMbx.send(msg);
                }
                catch (TimsException e)
                {
                    e.printStackTrace();
                }
            }
        }
    }

    /** Paketempfangstask */
    public void run()
    {
        TimsRawMsg cmdMsg;

        gdos.dbgInfo("Run command thread");

        while (!terminate)
        {
            // Paket der Kommandomailbox empfangen
            try
            {
                cmdMsg = commandMbx.receive(0);
            }
            catch (TimsException e1)
            {
                e1.printStackTrace();
                terminate();
                break;
            }

            // Empfangenes Paket analysieren
            switch (cmdMsg.type)
            {

                case RackProxy.MSG_ON:
                    switch (moduleTargetStatus)
                    {
                        case RackProxy.MSG_ENABLED:
                            if (moduleStatus == RackProxy.MSG_ENABLED)
                            {
                                try
                                {
                                    commandMbx.sendReply0(
                                            RackProxy.MSG_OK, cmdMsg);
                                }
                                catch (TimsException e)
                                {
                                    e.printStackTrace();
                                }
                            }
                            else
                            {
                                try
                                {
                                    commandMbx.sendReply0(
                                            RackProxy.MSG_ERROR, cmdMsg);
                                }
                                catch (TimsException e)
                                {
                                    e.printStackTrace();
                                }
                            }
                            break;

                        case RackProxy.MSG_DISABLED:
                            moduleTargetStatus = RackProxy.MSG_ENABLED;
                            notifyIfOn = cmdMsg.src;
                            notifyIfOnId = cmdMsg.seqNr;
                            break;

                        default:
                            try
                            {
                                commandMbx.sendReply0(RackProxy.MSG_ERROR,
                                        cmdMsg);
                            }
                            catch (TimsException e2)
                            {
                                e2.printStackTrace();
                            }
                    }
                    break;

                case RackProxy.MSG_OFF:
                    moduleTargetStatus = RackProxy.MSG_DISABLED;
                    try
                    {
                        commandMbx.sendReply0(RackProxy.MSG_OK, cmdMsg);
                    }
                    catch (TimsException e3)
                    {
                        e3.printStackTrace();
                    }
                    if (notifyIfOn > 0)
                    {
                        try
                        {
                            commandMbx.send0(RackProxy.MSG_ERROR, notifyIfOn,
                                             (byte) 0, (byte) notifyIfOnId);
                        }
                        catch (TimsException e)
                        {
                            e.printStackTrace();
                        }
                        notifyIfOn = 0;
                    }
                    break;

                case RackProxy.MSG_GET_STATUS:
                    try
                    {
                        commandMbx.sendReply0(moduleStatus, cmdMsg);
                    }
                    catch (TimsException e)
                    {
                        e.printStackTrace();
                    }
                    break;

                case RackProxy.MSG_GET_DATA:
                    try
                    {
                        if (dataMsg != null)
                        {
                            commandMbx.sendReply(RackProxy.MSG_DATA,
                                    cmdMsg, dataMsg);
                        }
                        else
                        {
                            System.out.println(RackName.nameString(commandMbx.getName()) +
                                    ": Im Java-Modul ist das Datenpacket nicht angelegt! (dataPackage == null)");
                            commandMbx.sendReply0(RackProxy.MSG_ERROR,
                                    cmdMsg);
                        }
                    }
                    catch (TimsException e)
                    {
                        e.printStackTrace();
                    }
                    break;

                case RackProxy.MSG_GET_CONT_DATA:
                    GetContDataMsg contPack;
                    try
                    {
                        contPack = new GetContDataMsg(cmdMsg);
                        
                        synchronized(dataListener)
                        {
                            int i;
                            boolean existent = false;
    
                            // check listener first
                            for (i = 0; i < dataListener.size(); i++)
                            {
                                int name = dataListener.elementAt(i).intValue();
                                if (name == contPack.dataMbx)
                                {
                                    existent = true;
                                    break;
                                }
                            }
                            if (!existent)
                            {
                                dataListener.addElement(new Integer(contPack.dataMbx));
                            }

                            ContDataMsg contData = new ContDataMsg();
                            contData.periodTime = periodTime;

                            commandMbx.sendReply(RackProxy.MSG_CONT_DATA,
                                                 cmdMsg, contData);
                        }
                    }
                    catch (TimsException e4)
                    {
                        e4.printStackTrace();
                    }
                    break;

                case RackProxy.MSG_STOP_CONT_DATA:
                    StopContDataMsg stopPack;
                    try
                    {
                        stopPack = new StopContDataMsg(cmdMsg);
                        synchronized(dataListener)
                        {
                            for (int i = 0; i < dataListener.size(); i++)
                            {
                                int name = dataListener.elementAt(i).intValue();
                                if (name == stopPack.dataMbx)
                                {
                                    dataListener.removeElementAt(i);
                                }
                            }
                            commandMbx.sendReply0(RackProxy.MSG_OK, cmdMsg);
                        }
                    }
                    catch (TimsException e5)
                    {
                        e5.printStackTrace();
                    }
                    break;

                default: // Paket konnte nicht interpretiert werden...
                    moduleCommand(cmdMsg); // ... dies wird nun dem
                                            // entsprechenden Modul �berlassen
            }
        }
    }

    class DataThread extends Thread
    {

        public void run()
        {
            gdos.dbgInfo("Run data thread");

            while (!terminate)
            {
                switch (moduleStatus)
                {
                    case RackProxy.MSG_ENABLED:

                        if (moduleTargetStatus == RackProxy.MSG_ENABLED)
                        {
                            if (!moduleLoop())
                            {
                                gdos.error("Error");
                                moduleStatus = RackProxy.MSG_ERROR;
                                moduleOff();
                            }
                            if (notifyIfOn > 0)
                            {
                                try
                                {
                                    commandMbx.send0(RackProxy.MSG_OK, notifyIfOn,
                                                     (byte) 0, notifyIfOnId);
                                }
                                catch (TimsException e)
                                {
                                    e.printStackTrace();
                                }
                                notifyIfOn = 0;
                            }
                        }
                        else
                        {
                            moduleStatus = RackProxy.MSG_DISABLED;
                            gdos.dbgInfo("Turn off");
                            moduleOff();
                            gdos.print("Module off");
                        }
                        break;

                    case RackProxy.MSG_DISABLED:
                        if (moduleTargetStatus == RackProxy.MSG_ENABLED)
                        {
                            synchronized(dataListener)
                            {
                                dataListener.removeAllElements();
                            }
                            gdos.dbgInfo("Turn on");
                            if (moduleOn() == true)
                            {
                                gdos.print("Module on");
                                moduleStatus = RackProxy.MSG_ENABLED;
                                if (notifyIfOn > 0)
                                {
                                    try
                                    {
                                        commandMbx.send0(RackProxy.MSG_OK, notifyIfOn, 
                                                         (byte) 0, notifyIfOnId);
                                    }
                                    catch (TimsException e)
                                    {
                                        e.printStackTrace();
                                    }
                                    notifyIfOn = 0;
                                }
                            }
                            else
                            {
                                gdos.error("Error");
                                moduleTargetStatus = RackProxy.MSG_DISABLED;
                                moduleStatus = RackProxy.MSG_DISABLED;
                                moduleOff();
                                if (notifyIfOn > 0)
                                {
                                    try
                                    {
                                        commandMbx.send0(RackProxy.MSG_ERROR, notifyIfOn,
                                                         (byte) 0, notifyIfOnId);
                                    }
                                    catch (TimsException e)
                                    {
                                        e.printStackTrace();
                                    }
                                    notifyIfOn = 0;
                                }
                            }

                        }
                        else
                        {
                            try
                            {
                                Thread.sleep(disabledTime);
                            }
                            catch (InterruptedException e)
                            {
                            }

                        }
                        break;

                    case RackProxy.MSG_ERROR:
                        try
                        {
                            Thread.sleep(errorTime);
                        }
                        catch (InterruptedException e)
                        {
                        }

                        if (moduleTargetStatus == RackProxy.MSG_ENABLED)
                        {
                            synchronized(dataListener)
                            {
                                dataListener.removeAllElements();
                            }
                            gdos.dbgInfo("Turn on");
                            if (moduleOn())
                            {
                                gdos.print("Module on");
                                moduleStatus = RackProxy.MSG_ENABLED;

                                if (notifyIfOn > 0)
                                {
                                    try
                                    {
                                        commandMbx.send0(RackProxy.MSG_OK, notifyIfOn,
                                                         (byte) 0, notifyIfOnId);
                                    }
                                    catch (TimsException e)
                                    {
                                        e.printStackTrace();
                                    }
                                    notifyIfOn = 0;
                                }
                            }
                            else
                            {
                                gdos.dbgInfo("Turn off");
                                moduleOff();

                                if (notifyIfOn > 0)
                                {
                                    try
                                    {
                                        commandMbx.send0(RackProxy.MSG_ERROR, notifyIfOn,
                                                         (byte) 0, notifyIfOnId);
                                    }
                                    catch (TimsException e)
                                    {
                                        e.printStackTrace();
                                    }
                                    notifyIfOn = 0;
                                }
                            }
                        }
                        else
                        {
                            gdos.print("Module off");
                            moduleStatus = RackProxy.MSG_DISABLED;
                        }
                        break;

                    default:

                }
            }
        }
    }

}
