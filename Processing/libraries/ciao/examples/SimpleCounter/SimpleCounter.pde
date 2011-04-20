/*
This example tests registering over Bonjour and communication with
the Ciao app.

Run this sketch and launch the Ciao app. Ciao will find the sketch
advertised through Bonjour and talk to it.

This sketch presents a display with an integer and buttons to
increment and decrement it. Load the sketch and then run Ciao to
modify the integer.

*/

import com.ciaoapp.Ciao;
import processing.net.*;
import processing.serial.Serial; // Ciao needs this

Ciao ciao;
int counterValue;
Client myClient;

void setup() {
  size(400,400);
  smooth();
  textAlign(CENTER);

  counterValue = 10;
  ciao = new Ciao(this, "Simple Counter");
  ciao.addDisplay("Value");  // Display Value in Ciao and update it with strings that start with Value
  ciao.addButton("Up");      // Display Up on the button and have Ciao send Up when pressed
  ciao.addButton("Down");
  ciao.register();
  
  PFont font = createFont("", 120);
  textFont(font);
}

void draw() {
  // Draw the current value
  if (counterValue >= 0) {
    background(0);
    fill(255);
  } else {
    background(255);
    fill(0);
  }
  text(counterValue, width/2, height/2 + 30);
  
  // Check for a command to change the current value
  if ((myClient = ciao.available()) != null) {
    String s = myClient.readStringUntil(10);  // Read until a newline
    if (s != null) {
      s = s.trim();
      if (s.toLowerCase().equals("up")) {
        counterValue++;
      } else if (s.toLowerCase().equals("down")) {
        counterValue--;
      }
      // Write the new value to all connected clients
      ciao.write(valueString());
    }
  }
}

// This is the format Ciao expects the value in:
//   prefix<space>what to display<CRLF>
// For example, in this case prefix is Value:
//   Value 10\r\n
String valueString() {
  return "Value " + counterValue + "\r\n";
}