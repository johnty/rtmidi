//*****************************************//
//  loopback.cpp
//  by johnty 2018 info@johnty.ca 
//
//  MIDI roundtrip latency tester 
//  mode 1: sender - sends a midi message, and times until a message is received
//  mode 2: receiver - sends a message back when a message is received 
//
//*****************************************//

#include <iostream>
#include <cstdlib>
#include <sys/time.h> //unix only!
#include <unistd.h>   //unix only!
#include "RtMidi.h"


struct timeval t0, t1;

void usage( void ) {
  // Error function in case of incorrect command-line
  // argument specifications.
  std::cout << "\nuseage: loopback <mode>\n";
  std::cout << "    mode = 0 for sender and 1 for recv; default 0.\n\n";
  exit( 0 );
}

void mycallback( double deltatime, std::vector< unsigned char > *message, void */*userData*/ )
{ /*
  unsigned int nBytes = message->size();
  for ( unsigned int i=0; i<nBytes; i++ )
    std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
  if ( nBytes > 0 )
    std::cout << "stamp = " << deltatime << std::endl;
  */
  gettimeofday(&t1, NULL);
  long elapsed = 1000000*(t1.tv_sec-t0.tv_sec) + t1.tv_usec - t0.tv_usec;
  std::cout<<"since t0 = " << elapsed <<"\n";
}

// This function should be embedded in a try/catch block in case of
// an exception.  It offers the user a choice of MIDI ports to open.
// It returns false if there are no ports available.
bool chooseMidiInPort( RtMidiIn *rtmidi );
bool chooseMidiOutPort( RtMidiOut *rtmidi );


int main( int argc, char ** argv )
{
  RtMidiIn *midiin = 0;
  RtMidiOut *midiout = 0;


  int mode = 0;

  // Minimal command-line check.
  if ( argc > 2 ) usage();

  if ( argc == 1) {
     mode = 0;
     std::cout<<"default to sender\n";

  }

  if ( argc == 2) {
      mode = argv[1][0]- '0';
      if (mode == 0)
         std::cout<<"set to sender\n";
      if (mode == 1)
         std::cout<<"set to receiver\n";
      if (mode < 0 || mode >1) {
         mode = 0;
         std::cout<<"invalid mode, default to 0 (sender)\n";
      }
  
  }
  try {

    // RtMidiIn constructor
    midiin = new RtMidiIn();
    midiout = new RtMidiOut();
   

    // Call function to select port.
    if ( chooseMidiInPort( midiin ) == false ) goto cleanup;
    std::cout<<"\n\n";
    if ( chooseMidiOutPort( midiout ) == false ) goto cleanup;
   

    // Set our callback function.  This should be done immediately after
    // opening the port to avoid having incoming messages written to the
    // queue instead of sent to the callback function.
    midiin->setCallback( &mycallback );

    // Don't ignore sysex, timing, or active sensing messages.
    midiin->ignoreTypes( false, false, false );

    std::cout << "\nReading MIDI input ... press <enter> to quit.\n";
    char input;
    std::cin.get(input);
    while (1) {
       if (mode ==0) { //sender
          unsigned char message[3];
          message[0] = 144;
          message[1] = 65;
          message[2] = 90;
          gettimeofday(&t0, NULL);
          midiout->sendMessage(message, 3);

          usleep(500*1000);
       }
       else {
       }
    }

  } catch ( RtMidiError &error ) {
    error.printMessage();
  }

 cleanup:

  delete midiin;
  delete midiout;

  return 0;
}

bool chooseMidiOutPort( RtMidiOut *rtmidi )
{

  std::string keyHit;

  std::string portName;
  unsigned int i = 0, nPorts = rtmidi->getPortCount();
  if ( nPorts == 0 ) {
    std::cout << "No output ports available!" << std::endl;
    return false;
  }

  if ( nPorts == 1 ) {
    std::cout << "\nOpening " << rtmidi->getPortName() << std::endl;
  }
  else {
    for ( i=0; i<nPorts; i++ ) {
      portName = rtmidi->getPortName(i);
      std::cout << "  Output port #" << i << ": " << portName << '\n';
    }

    do {
      std::cout << "\nChoose a port number: ";
      std::cin >> i;
    } while ( i >= nPorts );
  }

  std::cout << "\n";
  std::cout << "opening output port "<<i<<"\n";
  rtmidi->openPort( i );

  return true;
}

bool chooseMidiInPort( RtMidiIn *rtmidi )
{
  std::string keyHit;
  std::cout <<"Choose MIDI IN port:\n";
  std::string portName;
  unsigned int i = 0, nPorts = rtmidi->getPortCount();
  if ( nPorts == 0 ) {
    std::cout << "No input ports available!" << std::endl;
    return false;
  }

  if ( nPorts == 1 ) {
    std::cout << "\nOpening " << rtmidi->getPortName() << std::endl;
  }
  else {
    for ( i=0; i<nPorts; i++ ) {
      portName = rtmidi->getPortName(i);
      std::cout << "  Input port #" << i << ": " << portName << '\n';
    }

    do {
      std::cout << "\nChoose a port number: ";
      std::cin >> i;
    } while ( i >= nPorts );
    std::getline( std::cin, keyHit );  // used to clear out stdin
  }

  rtmidi->openPort( i );

  return true;
}
