#include "FT330.h"

FT330::FT330()
{
  _sensorCount = -1;
}

void FT330::begin(int sensorCount, int *sensorPins, long delay, int threshold)
{
  _sensorCount = sensorCount;
  _sensorPins = new int[_sensorCount];
  memcpy(_sensorPins, sensorPins, sizeof(*sensorPins) * sensorCount);
  _delay = delay;
  _threshold = threshold;

  _lastPinStateChangeTime = new long[_sensorCount];
  _firstPinStateChangeTime = new long[_sensorCount];
  _lastPinState = new int[_sensorCount];
  _pulseCount = new int[_sensorCount];

  long now = millis();

  for (int i = 0; i < _sensorCount; i++)
  {
    pinMode(_sensorPins[i], INPUT);
    digitalWrite(_sensorPins[i], HIGH);

    _pulseCount[i] = 0;
    _lastPinStateChangeTime[i] = now;
    _firstPinStateChangeTime[i] = now;
    _lastPinState[i] = digitalRead(_sensorPins[i]);
  }
}

void FT330::end()
{
  if (_sensorCount != -1)
  {
    _sensorCount = -1;
    _delay = -1;
    _threshold = -1;

    delete _sensorPins;
    _sensorPins = NULL;

    delete _pulseCount;
    _pulseCount = NULL;

    delete _lastPinStateChangeTime;
    _lastPinStateChangeTime = NULL;

    delete _firstPinStateChangeTime;
    _firstPinStateChangeTime = NULL;

    delete _lastPinState;
    _lastPinState = NULL;
  }
}

int FT330::check()
{
  if (_sensorCount == -1)
  {
    return 0;
  }

  long now = millis();
  int haveData = pollPins(now);

  for (int i = 0; i < _sensorCount; i++)
  {
    if ((now - _lastPinStateChangeTime[i]) > _delay && _lastPinStateChangeTime[i] > 0)
    {
      checkPour(now, i);

      _lastPinStateChangeTime[i] = now;
    }
  }

  return haveData;
}

int FT330::pollPins(long now)
{
  int haveData = 0;

  for (int i = 0; i < _sensorCount; i++)
  {
    int pinState = digitalRead(_sensorPins[i]);

    if (pinState != _lastPinState[i])
    {
      if (pinState == HIGH)
      {
        // needed?
        if (now - _lastPinStateChangeTime[i] > 1)
        {
          if (_pulseCount[i] == _threshold)
          {
            _firstPinStateChangeTime[i] = now;

            if (_func_pour_start)
            {
              _func_pour_start(_sensorPins[i]);
            }
          }

          _pulseCount[i]++;
        }

        _lastPinStateChangeTime[i] = now;
      }

      _lastPinState[i] = pinState;
      haveData = 1;
    }
  }

  return haveData;
}

void FT330::checkPour(long now, int pin)
{
  if (_pulseCount[pin] > 0)
  {
    if (_pulseCount[pin] > _threshold)
    {
      long duration = _lastPinStateChangeTime[pin] - _firstPinStateChangeTime[pin];

      if (_func_pour_end)
      {
        _func_pour_end(_sensorPins[pin], _pulseCount[pin], duration);
      }
    }

    _pulseCount[pin] = 0;
  }
}