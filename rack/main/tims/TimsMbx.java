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
package rack.main.tims;

import java.util.*;

class TimsMbx extends Vector
{
  private static final long serialVersionUID = 1L;

  public int name = 0;

  /**
   * das equals muss ueberschrieben werden, da zwei mailboxen nicht gleich sind,
   * wenn ihre inhalte gleich, sondern wenn ihre namen gleich sind!
   * arne michaelsen, 6.11.2003
   *
   * @param o the Object to be compared for equality with this Vector.
   * @return true if the specified Object is equal to this Vector
   */
  public synchronized boolean equals(Object o) {
    try{
      return name == ((TimsMbx) o).name;
    } catch (java.lang.ClassCastException e) {
      return false;
    }
  }
}
