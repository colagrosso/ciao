/*
This example connects your USB Arduino with the Ciao app.
You should not modify this sketch, nor should you have to.

To configure your Arduino and talk to it with Ciao, load
one of the CiaoUsb sketches onto your Arduino and then run
this sketch.

The Arduino defines the name of the service, the port to
listen on (make sure it is above 1024), and displays and
buttons. Do all the programming on the Arduino, and let
this skecth serve merely as a conduit.

*/

import com.ciaoapp.Ciao;
import processing.net.*;
import processing.serial.Serial;

Serial serial;
Ciao ciao;

void setup() {
  size(400,400);
  smooth();
  background(0);
  fill(255);
  textAlign(CENTER);
  text("Ciao, Arduino.", width/2, height/2);

  // Open the first serial device
  serial = new Serial(this, Serial.list()[0], 115200);
  ciao = new Ciao(this, serial);
}

void draw() {
  if (serial.available() > 0) {
    String s = serial.readStringUntil(10);  // Read until a newline
    if (s != null) {
      if (s.startsWith(Ciao.CIAO_PRINT)) {
        s = s.substring(Ciao.CIAO_PRINT.length());
        s = s.substring(0, s.length() - 2);  // Take off the CRLF
        ciao.write(s);
      } else if (s.startsWith(Ciao.CIAO_PRINTLN)) {
        s = s.trim();
        s = s.substring(Ciao.CIAO_PRINTLN.length());
        ciao.write(s + "\r\n");
      } else {
        // If the line doesn't start with CIAO_PRINT or CIAO_PRINTLN,
        // it's a debug message. Print it to the screen.
        s = s.trim();
        println(s);
      }
    }
  }
  Client client = ciao.available();
  if (client != null) {
    if (client.available() > 0) {
      serial.write(client.read());
    }
  }
}