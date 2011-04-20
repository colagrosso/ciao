/*
This example tests registering over Bonjour and communication with
the Ciao app.

Run this sketch and launch the Ciao app. Ciao will find the sketch
advertised through Bonjour and talk to it.

After the sketch launches, move the mouse in the black window and
see the mouse X- and Y-coordinates in Ciao.

*/

import com.ciaoapp.Ciao;
import processing.net.*;
import processing.serial.Serial; // Ciao needs this

Ciao ciao;
Client myClient;

void setup() {
  size(400,400);
  smooth();
  textAlign(CENTER);
  
  ciao = new Ciao(this, "Mouse Coordinates", 4365);  // Listen on a custom port with optional third argument
  ciao.addDisplay("X");
  ciao.addDisplay("Y");
  ciao.register();
  
  PFont font = createFont("", 60);
  textFont(font);
  background(0);
  fill(255);
}

void draw() {
  // Write the values on a new connection
  if (ciao.available() != null) {
    ciao.available().readString();
    ciao.write("X " + mouseX + "\r\n");
    ciao.write("Y " + mouseY + "\r\n");
  }    
  // Write the values when they change
  if (mouseX != pmouseX) {
    ciao.write("X " + mouseX + "\r\n");
  }
  if (mouseY != pmouseY) {
    ciao.write("Y " + mouseY + "\r\n");
  }
  delay(100);
}


