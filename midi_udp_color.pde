//import controlP5.*;
import hypermedia.net.*;
import themidibus.*; //Import the library
import javax.sound.midi.MidiMessage; //Import the MidiMessage classes http://java.sun.com/j2se/1.5.0/docs/api/javax/sound/midi/MidiMessage.html

MidiBus myBus; // The MidiBus

int t = 0;
int s = 0;

UDP udp;  

String ip = "192.168.1.52"; // the remote IP address
int port = 8888; // the destination port

void setup() {
  size(400, 400);

  noStroke();
          
  udp = new UDP( this, 6000 );
  udp.listen( true );
  
  MidiBus.list(); // List all available Midi devices on STDOUT. This will show each device's index and name.
  myBus = new MidiBus(this, 0, 0); // Create a new MidiBus object


}

void draw() {}
  
  void rawMidi(byte[] data) {
 
  int x = (int)(data[0] & 0xFF);
 
  for (int i = 1;i < data.length;i++) {
    if ((i+1) == 2) { t = (int)(data[i] & 0xFF); }
    else if ((i+1) == 3) { s = (int)(data[i] & 0xFF); }
  } 
    String message = str(x)+ " " + str(t)+ " " + str(s);
    udp.send( message, ip, port);
  }

