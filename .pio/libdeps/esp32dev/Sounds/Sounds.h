
#ifndef Sounds_h
#define Sounds_h

#include "Arduino.h"

class Sounds
{
public:
  Sounds(int port);
  void temperature();
  void start();
private:
  int _port; 
};

#endif