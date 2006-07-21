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
package rack.main.gui;

import java.util.ArrayList;

public class MapViewActionList extends ArrayList
{
    public String title;

    private static final long serialVersionUID = 1L;

    public MapViewActionList(String n_title)
    {
        title = n_title;
    }

    public MapViewActionListItem addItem(String title, String actionCommand)
    {
        MapViewActionListItem newItem = new MapViewActionListItem(title,
                actionCommand);
        this.add(newItem);
        return newItem;
    }

    public class MapViewActionListItem
    {
        public String title;
        public String actionCommand;

        public MapViewActionListItem(String n_title, String n_actionCommand)
        {
            title = n_title;
            actionCommand = n_actionCommand;
        }
    }

}
