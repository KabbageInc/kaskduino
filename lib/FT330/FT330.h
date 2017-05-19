#ifndef _FT330_H
#define _FT330_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class FT330
{
public:
  FT330();

  void begin(int sensorCount, int *sensorPins, long delay, int threshold);
  void end();
  int check();

  void onPourStart(void (*func)(int pin))
  {
    _func_pour_start = (pour_start_callback)func;
  }

  void onPourEnd(void (*func)(int pin, int pulses, long duration))
  {
    _func_pour_end = (pour_end_callback)func;
  }

private:
  int pollPins(long now);
  void checkPour(long now, int pin);

  int _sensorCount;
  int *_sensorPins;

  long _delay;    // number of milliseconds to wait after pour before sending message
  int _threshold; // minimum pulse count to filter out noise, short bursts, etc

  int *_pulseCount;
  long *_lastPinStateChangeTime;
  long *_firstPinStateChangeTime;
  int *_lastPinState;

  typedef void (*pour_start_callback)(int pin);
  typedef void (*pour_end_callback)(int pin, int pulses, long duration);

  pour_start_callback _func_pour_start;
  pour_end_callback _func_pour_end;
};

#endif