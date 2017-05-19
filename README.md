# Kabbage Kaskduino Overview

Arduino-based component of [Kabbage Kask](https://kask.kabbage.com) which performs the following functions:

  * Interrupt-driven Wiegand decoding for RFID readers
  * Polling based reading of GEMS FT-330 TurboFlow sensors
    * Tested with 4 sensors, supports varying number of sensors and GPIO pin layouts
    * Detection of pour start and pour end (reporting pulses & duration)
  * Robust build system based on [PlatformIO](http://platformio.org/) 
    * Supports custom reset code for the Alamode via GPIO18 during flashing

# Hardware

Although you can wire any Arduino up to a Raspberry Pi, this document will assume you have attached an Alamode shield which is Uno compatible and based on the ATMEGA328P. The Alamode supports programming over serial with the Pi's BCM2837 GPIO 18 hooked up to the Arduino reset line for button-less uploads.

The Arduino is 5V tolerant allowing for direct interaction with both Wiegand inputs and the ability to power and detect pulses on the GEMS FT-330 flow sensors.

## Arduino Pinout

| Pin | Name   | Usage                 | Notes                                            | 
| --- |------- | --------------------- | ------------------------------------------------ |
| 2   | INT0   | RFID Wiegand DATA0 In | Interrupt capable, required for Wiegand decoding |
| 3   | INT1   | RFID Wiegand DATA1 In | Interrupt capable, required for Wiegand decoding |
| 4   | GPIO4  | FT-330 Sensor #1      |                                                  |
| 5   | GPIO5  | FT-330 Sensor #2      |                                                  |
| 6   | GPIO6  | FT-330 Sensor #3      |                                                  |
| 7   | GPIO7  | FT-330 Sensor #4      |                                                  |

* Note: False reads were observed on pin 11 because most likely because of SPI MOSI usage. This can probably be disabled but moving to pins 4-7 avoid the issue.

# Toolchain

[PlatformIO](http://platformio.org/) is used to provide a robust command-line building process on Linux, Windows and MacOS.

## Installing PlatformIO

Follow the instructions to install the [PlatformIO Core](http://docs.platformio.org/en/latest/installation.html) at minimum, although you may wish to install the [PlatformIO IDE for Atom](http://docs.platformio.org/en/latest/ide/atom.html#ide-atom) for a full featured IDE which includes PlatformIO Core.

### Python Package Manager

```
pip install -U platformio
```

### MacOS using Homebrew

```
brew install platformio
```

## IDE support

Be sure to install Clang support for code error checking and indentation for Atom, Visual Studio Code, etc.

## Building

To build, execute:

```
platformio run
```  

## Deploy

The `platformio.ini` assumes that the Arduino's serial port is available at `/dev/ttyAMA0`. This can be changed in the `platformio.ini` via the `upload_port` setting or the command line param ` --upload-port /dev/ttyAMA1` for example.

To build and deploy, execute:

```
platformio run --target upload
```

# Serial Port Events and Commands

## Events

General format of events are as follows:

```
<event>:<timestamp>:...
```

| Name      | Data Type | Description             |
| --------- | --------- | ----------------------- |
| event     | string    | Event ID                |
| timestamp | long      | Timestamp from millis() |
| ...       |           | Defined by the event    |


### Startup

Sent on initial startup and any subsequent reset via the reset button or GPIO18 to the Raspberry Pi.

```
startup:<timestamp>:<version>
```

| Value   | Data Type | Description                 |
| ------- | --------- | --------------------------- |
| version | string    | Version string, e.g. 1.0.5  |

### Heartbeat

Every 5 seconds during idle periods (no data via serial, RFID or the FT-330 sensors), a heartbeat will be sent. There is no payload data.

```
heartbeat:<timestamp>
```

### RFID State

Outputs the RFID connection state.

```
wiegand_state:<timestamp>:<state>
```

| Value  | Data Type | Description                     |
| ------ | --------- | ------------------------------- |
| status | bool      | 1 = connected, 0 = disconnected |

### RFID Receive

On a scan of an RFID card, the Wiegand card ID and bit length will be sent.

```
wiegand_receive:<timestamp>:<bits>:<code>
```

| Value | Data Type | Description                   |
| ----- | --------- | ----------------------------- |
| bits  | byte      | Bit length                    |
| code  | string    | Card ID in hex, dynamic width |

### GEMS FT-330 Sensor Events

The FT-330 Sensor events include:

  * Pour start
  * Pour end

#### Pour Start

On the first pulse over the threshold, an event will be sent.

```
ft330_start:<timestamp>:<pin>
```

| Value | Data Type | Description     |
| ----- | --------- | --------------- |
| pin   | int       | GPIO pin number |

#### Pour End

After a pour is complete, the count of total pulses and duration will be sent. Pours lower than the threshold will not be sent.

```
ft330_end:<timestamp>:<pin>:<pulses>:<duration>
```

| Value    | Data Type | Description              |
| -------- | --------- | ------------------------ |
| pin      | int       | GPIO pin number          |
| pulses   | int       | Pulse count              |
| duration | long      | Duration in milliseconds |

## Commands

Commands generally return `<command_code>_ack>` as acknowledgement.

### Version

Requests the version number of the Kaskduino.

#### Request

```
version
```

#### Response

```
version:<version>
```

### GEMS FT-330 Initialize

Initialize the GEMS FT-330 with the:

  * Pin count and pin mapping
  * Pour delay and threshold

#### Request

```
ft330_init:<pin_count>:<pin0,pin1,...,pinN>:delay:threshold
```

| Value     | Data Type | Description                                                |
| --------- | --------- | ---------------------------------------------------------- |
| pin_count | int       | GPIO pin count                                             |
| pin0..N   | int       | GPIO pin numbers comma-delimited, count matching pin_count |
| delay     | long      | Pour delay                                                 |
| threshold | int       | Pour threshold                                             |

#### Response

```
ft330_ack:<command_payload>
```

### Initialize Wiegand

Initialize Wiegand processing on interrupt capable pins.

#### Request

```
wiegand_init:<data0_pin>,<data1_pin>
```

| Value     | Data Type | Description       |
| --------- | --------- | ----------------- |
| data0_pin | int       | Wiegand DATA0 pin |
| data1_pin | int       | Wiegand DATA1 pin |

#### Response

```
wiegand_ack:<command_payload>
```
