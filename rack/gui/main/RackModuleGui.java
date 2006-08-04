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


import javax.swing.*;

import rack.main.proxy.*;

/**
 * Diese Klasse ist die Superklasse aller GUI-Module.
 * Dadurch das die abtrakten Methoden implementiert werden
 * muessen, enthaelt jedes GUI Standardeigenschaften die von dem
 * MainGUI genutzt werden.
 */

abstract public class RackModuleGui extends Thread//JComponent implements Runnable
{
    // use constructor RackModuleGui(RackProxy moduleProxy) or
    // RackModuleGui(Integer moduleIndex, RackProxy[] proxyList, RackModuleGui[] guiList)

    abstract public JComponent getComponent();

    /** Liefert den Namen des GUIs zurck */
    abstract public String getModuleName();

    /** Liefert die verwendete Proxyfunktion des GUIs zurck */
    abstract public RackProxy getProxy();

    /** Wird bei Beendigung des "MainGUIs" aufgerufen und soll
     * ein sofortiges Ende der GUI-Komponente bewirken */
    abstract public void terminate();



    /** Legt fest of dieses Gui eine Kartendarstellung hat,
     * Diese Funktion muss gegebenenfals ueberschrieben werden.*/
    public boolean hasMapView() {
        return false;
    }

    /** Legt fest of dieses Gui einen Cursor fuer die Kartendarstellung hat,
     * Diese Funktion muss gegebenenfals ueberschrieben werden.*/
    public boolean hasMapViewCursor()
    {
        return false;
    }

    /** ModulGui wird aufgeforder seine Representation in den
     * drawContext zu Zeichnen */
    public void paintMapView(MapViewDrawContext drawContext)
    {
    }

    /** ModulGui wird aufgeforder seine Cursor in den
     * cursorDrawContext zu Zeichnen */
    public void paintMapViewCursor(MapViewCursorDrawContext cursorDrawContext)
    {
    }

    /**
     * Gibt an welche Aktionen dieses Gui dem MapViewer anbietet
     * siehe auch Benutzung in rcc.navigation.localisation.LocalisationGui
     *
     * dort:
     *    private MapViewActionList mapViewActionList;
     *    mapViewActionList = new MapViewActionList(getModuleName());
     *    mapViewActionList.addItem("set position", "setPosition");
     */
    public MapViewActionList getMapViewActionList()
    {
        return null;
    }

    /** Wird vom mapViewer aufgerufen,
     * wenn eine Aktion auf der Karte ausgefuehrt wird */
    public void mapViewActionPerformed(MapViewActionEvent event)
    {
    }
}
