/*
This example tests registering over Bonjour and communication with
the Ciao app.

Run this sketch and launch the Ciao app. Ciao will find the sketch
advertised through Bonjour and talk to it.
*/

import com.ciaoapp.Ciao;
import processing.net.*;
import processing.serial.Serial; // Ciao needs this

Ciao ciao;
Client myClient;
color redColor = color(64, 0, 0);
color greenColor = color(0, 64, 0);
color blueColor = color(0, 0, 64);

void setup() {
  size(400,400);
  smooth();
  textAlign(CENTER);
  
  ciao = new Ciao(this, "Color Changer", 3456);  // Listen on a custom port with optional third argument
  ciao.addButton("Red");
  ciao.addButton("Green");
  ciao.addButton("Blue");
  ciao.register();
  
  PFont font = createFont("", 120);
  textFont(font);
  fill(255);
  background(greenColor);
  text("Green", width/2, height/2 + 30);
}

void draw() {
  // Check for a command to change the current value
  if ((myClient = ciao.available()) != null) {
    String s = myClient.readStringUntil(10);  // Read until a newline
    if (s != null) {
      s = s.trim();
      if (s.toLowerCase().equals("red")) {
        background(redColor);
        text("Red", width/2, height/2 + 30);
      } else if (s.toLowerCase().equals("green")) {
        background(greenColor);
        text("Green", width/2, height/2 + 30);
      } else if (s.toLowerCase().equals("blue")) {
        background(blueColor);
        text("Blue", width/2, height/2 + 30);
      }
    }
  }
}