/**
 * Ciao: Communicate with an Arduino over Bonjour
 *
 * (c) 2011 Mike Colagrosso
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 * 
 * @author		Mike Colagrosso http://colagrosso.net
 * @modified	02/20/2011
 * @version		1.0
 */

package com.ciaoapp;


import processing.core.*;

import processing.net.Server;
import processing.net.Client;

import processing.serial.Serial;

import java.io.IOException;
import java.util.Date;

import javax.jmdns.JmDNS;
import javax.jmdns.ServiceInfo;

public class Ciao {
	
	// myParent is a reference to the parent sketch
	PApplet myParent;
	
	Server myServer;

	public final static String VERSION = "1.0";

	public static final int DEFAULT_SERVER_PORT = 2426;  // C-I-A-O
    public final static String REMOTE_TYPE = "_ciao._tcp.local.";

    public static boolean DEBUG = true;
    
    private JmDNS jmdns;
    private ServiceInfo pairservice;
    private String name;
    private int port;
    private Descriptor descriptor;

    private static final String CIAO_HOSTNAME = "CIAO_HOSTNAME ";
    private static final String CIAO_NAME = "CIAO_NAME ";
    private static final String CIAO_PORT = "CIAO_PORT ";
    private static final String CIAO_DISPLAY = "CIAO_DISPLAY";   // No trailing space
    private static final String CIAO_BUTTON = "CIAO_BUTTON";     // No trailing space
    private static final String CIAO_ANNOUNCE = "CIAO_ANNOUNCE"; // No trailing space
    public static final String CIAO_PRINT = "CIAO_PRINT ";
    public static final String CIAO_PRINTLN = "CIAO_PRINTLN ";

	public Ciao(PApplet theParent, String name) {
		this(theParent, name, DEFAULT_SERVER_PORT);
	}

	public Ciao(PApplet theParent, String name, int port) {
		myParent = theParent;
		myParent.registerDispose(this);
        descriptor = new Descriptor();
		this.name = name;
		this.port = port;
	}

	public Ciao(PApplet theParent, Serial serial) {
	    myParent = theParent;
	    myParent.registerDispose(this);

	    while(serial.available() == 0) {
			theParent.delay(10);
		}

        String s;
        do {
            s = readline(serial);
            if (s.startsWith(CIAO_HOSTNAME)) {
                descriptor = new Descriptor();
            } else if (s.startsWith(CIAO_NAME)) {
                this.name = s.substring(CIAO_NAME.length());
            } else if (s.startsWith(CIAO_PORT)) {
                this.port = Integer.parseInt(s.substring(CIAO_PORT.length()));
            } else if (s.startsWith(CIAO_DISPLAY)) {
                String label = readline(serial);
                String prefix = readline(serial);
                addDisplay(label, prefix);
             } else if (s.startsWith(CIAO_BUTTON)) {
                String label = readline(serial);
                String send = readline(serial);
                addButton(label, send);
             }
             theParent.delay(10);
        } while (!s.startsWith(CIAO_ANNOUNCE));

		register();
		System.out.println("Constructed a Ciao object with this name, port, and TXT field:");
		System.out.println(name);
		System.out.println(port);
		System.out.println(descriptor.getValues());
	}
	
	public void register() {
        jmdns = null;
        Date d1 = new Date();
		try {
			if (DEBUG) System.out.print("Starting JmDNS to register with Bonjour. It's a beast. Think happy thoughts... ");
			jmdns = JmDNS.create();
		} catch (IOException e) {
			e.printStackTrace();
		}
        pairservice = ServiceInfo.create(REMOTE_TYPE, name, port, 0, 0, descriptor.getValues());
        try {
			jmdns.registerService(pairservice);
		} catch (IOException e) {
			e.printStackTrace();
		}
		myServer = new Server(myParent, port);
		
		Date d2 = new Date();
		if (DEBUG) System.out.println("Started in " + (d2.getTime() - d1.getTime())/1000 + " seconds.");
	}
	
	public void dispose() {
		if (DEBUG) System.out.print("Stopping JmDNS. It's just as much of a beast to stop... ");
		jmdns.unregisterService(pairservice);
        try {
			jmdns.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		if (DEBUG) System.out.println("Stopped.");
		
		if (DEBUG) {
			System.out.println("Stopping SocketServer. Ignore the exception:");
			System.out.println("  java.net.SocketException: Socket closed");
			System.out.println("It is a known Processing bug: http://www.processing.org/reference/libraries/net/Server_stop_.html http://processing.org/bugs/bugzilla/89.html");
		}
			
			
	}

	public Server getServer() {
		return myServer;
	}
	
	public Client available() {
		return myServer.available();
	}
	
	public void write(String data) {
		myServer.write(data);
	}

	public void addDisplay(String label) {
		descriptor.addDisplay(label);
	}
	
	public void addDisplay(String label, String prefix) {
		descriptor.addDisplay(label, prefix);
	}
	
	public void addButton(String label) {
		descriptor.addButton(label);
	}
	
	public void addButton(String label, String send) {
		descriptor.addButton(label, send);
	}

	
	/**
	 * return the version of the library.
	 * 
	 * @return String
	 */
	public static String version() {
		return VERSION;
	}

	private String readline(Serial serial) {
	    String s = null;
	    String line = "";
	    do {
	        s = serial.readStringUntil(10);
	        if (s != null) {
	            line = s.trim();
	        }
	    } while (s == null);
	    return line;
	}
}

