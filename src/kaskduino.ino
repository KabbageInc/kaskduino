#include <Wiegand.h>
#include <FT330.h>

#define VERSION "0.0.3"
#define HEARTBEAT_INTERVAL 5000

unsigned long g_lastHeartbeat = 0;
int g_ledStatus = 0;

Wiegand g_wiegand;
int g_wiegand_pin0;
int g_wiegand_pin1;

FT330 g_ft330;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);

  sendStartupEvent();

  setupFt330();
  setupRfid();
}

void setupFt330()
{
  g_ft330.onPourStart(ft330OnPourStart);
  g_ft330.onPourEnd(ft330OnPourEnd);
}

void setupRfid()
{
  g_wiegand.onReceive(wiegandOnReceive);
  g_wiegand.onStateChange(wiegandOnStateChange);

  g_wiegand.begin(WIEGAND_LENGTH_AUTO);
}

void loop()
{
  checkFt330();
  checkRfid();
  checkCommand();
  sendHeartbeat();
}

void resetHeartbeat()
{
  g_lastHeartbeat = millis();
}

void sendHeartbeat()
{
  unsigned long now = millis();

  if (now < g_lastHeartbeat || (now - g_lastHeartbeat) > HEARTBEAT_INTERVAL)
  {
    g_lastHeartbeat = now;
    g_ledStatus = !g_ledStatus;

    digitalWrite(LED_BUILTIN, g_ledStatus ? HIGH : LOW);

    Serial.print("heartbeat");
  }
}

void sendStartupEvent()
{
  Serial.print("startup:");
  Serial.println(VERSION);
}

void checkCommand()
{
  if (Serial.available() == 0)
  {
    return;
  }

  resetHeartbeat();

  String entireCommand = Serial.readStringUntil('\n');
  entireCommand.trim();

  String code = getSplitStringValue(entireCommand, ':', 0);

  if (code == "version")
  {
    commandVersion();
  }
  else if (code == "wiegand_init")
  {
    commandWiegand(entireCommand);
  }
  else if (code == "ft330_init")
  {
    commandFt330Init(entireCommand);
  }
  else
  {
    Serial.print("unknown_cmd:");
    Serial.println(entireCommand);
  }
}

void commandVersion()
{
  Serial.print("v:");
  Serial.println(VERSION);
}

void commandWiegand(String entireCommand)
{
  if (g_wiegand_pin0 != -1)
  {
    detachInterrupt(digitalPinToInterrupt(g_wiegand_pin0));
    detachInterrupt(digitalPinToInterrupt(g_wiegand_pin1));
  }

  g_wiegand_pin0 = getSplitStringValue(entireCommand, ':', 1).toInt();
  g_wiegand_pin1 = getSplitStringValue(entireCommand, ':', 2).toInt();

  pinMode(g_wiegand_pin0, INPUT);
  pinMode(g_wiegand_pin1, INPUT);

  attachInterrupt(digitalPinToInterrupt(g_wiegand_pin0), wiegandOnPinStateChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(g_wiegand_pin1), wiegandOnPinStateChange, CHANGE);

  Serial.print("wiegand_ack");

  wiegandOnPinStateChange();
}

void commandFt330Init(String entireCommand)
{
  int pinCount = getSplitStringValue(entireCommand, ':', 1).toInt();
  String pins = getSplitStringValue(entireCommand, ':', 2);
  int delay = getSplitStringValue(entireCommand, ':', 3).toInt();
  int threshold = getSplitStringValue(entireCommand, ':', 4).toInt();

  int *pinArray = new int[pinCount];

  for (int i = 0; i < pinCount; i++)
  {
    int pin = getSplitStringValue(pins, ',', i).toInt();
    pinArray[i] = pin;
  }

  g_ft330.end();
  g_ft330.begin(pinCount, pinArray, delay, threshold);

  Serial.print("ft330_ack");
}

String getSplitStringValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void checkFt330()
{
  if (g_ft330.check())
  {
    resetHeartbeat();
  }
}

void checkRfid()
{
  noInterrupts();
  g_wiegand.flush();
  interrupts();
}

void wiegandOnPinStateChange()
{
  g_wiegand.setPin0State(digitalRead(g_wiegand_pin0));
  g_wiegand.setPin1State(digitalRead(g_wiegand_pin1));
}

void wiegandOnStateChange(bool connected, const char *message)
{
  resetHeartbeat();

  Serial.print("wiegand_state:");
  Serial.println(connected ? "1" : "0");
}

void wiegandOnReceive(uint8_t *data, uint8_t bits, const char *message)
{
  resetHeartbeat();

  Serial.print("wiegand_receive:");
  Serial.print(bits);
  Serial.print(":");

  uint8_t bytes = (bits + 7) / 8;
  for (int i = 0; i < bytes; i++)
  {
    Serial.print(data[i] >> 4, 16);
    Serial.print(data[i] & 0xF, 16);
  }

  Serial.println();
}

void ft330OnPourStart(int pinNum)
{
  Serial.print("ft330_start:");
  Serial.println(pinNum);
}

void ft330OnPourEnd(int pinNum, int pulseCount, long duration)
{
  Serial.print("ft330_end:");
  Serial.print(pinNum);
  Serial.print(":");
  Serial.print(pulseCount);
  Serial.print(":");
  Serial.println(duration);
}
