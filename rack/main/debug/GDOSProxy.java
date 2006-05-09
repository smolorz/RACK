/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Oliver Wulf      <wulf@rts.uni-hannover.de>
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
package rack.main.debug;

import rack.main.proxy.*;
import rack.main.naming.*;
import rack.main.tims.msg.*;
import rack.main.tims.msgtypes.*;
import rack.main.tims.router.*;
import rack.main.tims.exceptions.*;

/**
 * Stellt alle Funktionen zur Bedienung des GDOS-Systems zur Verfuegung.
 *
 * Bisherige Idee und Umsetzung von GDOS unter Java: Unter Java sollen die Nachrichten aller
 * Module gesammelt, angezeigt und ggf. gespeichert werden.
 * Fuer Systeme die nicht unter Java laufen wird eine Mailbox GDOS_EXTERN angelegt. Die Nachrichen die dort
 * ankommen koennen auf dem jeweiligen System ausgegeben werden oder werden alternativ zur (Java) GDOS Mailbox
 * weitergeleitet und dort angezeigt.
 * Die Weiterleitung erfolgt, wenn das Java GDOS eingeschaltet wird. (Diese Loesung wurde
 * bisher unter RTOS-UH realisiert)
 * ---------
 * Eine andere Moeglichkeit waere beim Start von GDOS auf den Remote Systemen zu ueberpruefen, ob schon eine (Java)
 * GDOS-Mailbox schon angelegt wurde und diese fuer die Ausgaben zu nutzen.
 * Nachteil: Module duerfen erst nach dem Gui geladen werden. Ausgabe auf dem Remote System dann nicht mehr moeglich.
 * Beide Moeglichkeiten sind mit der vorhandenen Proxyfunktion anwendbar.
 * ---------
*
 * Es existiert kein eigenstaendiges GDOS-Modul unter Java. (Waere ueberfluessiger Verwaltungsoverhead und Datenverkehr)
 * Die "Proxy"-funktionen  legen die GDOS-Mailbox an und uebernehmen die Verwaltung. Funktionen wie
 * Ein-und Ausschalten werden fuer das GUI simuliert.
 *
 * @version
 * $Id: $
 */

public class GDOSProxy extends RackDataProxy {

    /** PRINTOUT (HIGHEST LEVEL) */
    public static final byte GDOS_PRINT      = -124;
    /** ERROR */
    public static final byte GDOS_ERROR      = -125;
    /** WARNING */
    public static final byte GDOS_WARNING    = -126;
    /** DEBUG INFORMATION */
    public static final byte GDOS_DBG_INFO   = -127;
    /** DETAILED DEBUG INFORMATION */
    public static final byte GDOS_DBG_DETAIL = -128;
    /** Name (Nummer) der GDOS-Mailbox */
    private int GDOSMbx = RackName.create(RackName.GDOS, 0);
    /** Status des "GDOS-Moduls"(An/Aus). */
    private int status = RackMsgType.MSG_DISABLED;


    public GDOSProxy(int id, int replyMbx) {
        super(RackName.create(RackName.GDOS, id), replyMbx, 2000, 1000, 1000);
        this.id =id;

    }

    /** Der uebergebene Parameter spezifiziert das GDOSsystem, dass
     * einen Befehl zum forwarden von Nachrichten erhalten soll.
     * Alle GDOS-Meldungen werden zum JavaGDOS umgeleitet*/
    public synchronized void setForwardMbx(int GDOSTarget) {
        try {
            // xxx Vorlaeufige Loesung. sollte spaeter noch ueber
            // echtes GetCont Paket laufen.
            TimsMsgRouter.send0(
                RackMsgType.MSG_GET_CONT_DATA,
                GDOSTarget,
                GDOSMbx,
                (byte)0,
                (byte)0);
        } catch (MsgException e) {
            System.out.println(e.toString());
        }
    }

    public int getStatus() {
        return (status);
    }

    public void on() {
        status = RackMsgType.MSG_ENABLED;
    }

    public void off() {
        status = RackMsgType.MSG_DISABLED;
    }

    /** stoppt das forwarden von Nachrichten */
    public synchronized void stopForward(int GDOSTarget) {
        try {
            TimsMsgRouter.send0(
                RackMsgType.MSG_STOP_CONT_DATA,
                GDOSTarget,
                GDOSMbx,
                (byte)0,
                (byte)0);
        } catch (MsgException e) {
            System.out.println(e.toString());
        }
    }
    /** xxx! Testfunktion! Funktioniert noch nicht */
    public void GDOS_PRINT(int sender, String message) {
        try {
            GDOSDataMsg msg = new GDOSDataMsg();
            msg.dest = GDOSMbx;
            msg.src  = sender;
            msg.type = (byte)GDOS_PRINT;
            msg.message = message;
            TimsMsgRouter.send((TimsMsg) msg);
        } catch (MsgException e) {
            System.out.println(e.toString());
        }
    }

    public int getCommandMbx()
    {
      return(RackName.create(RackName.GDOS, id));
    }
}
