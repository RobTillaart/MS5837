
[![Arduino CI](https://github.com/RobTillaart/MS5837/workflows/Arduino%20CI/badge.svg)](https://github.com/marketplace/actions/arduino_ci)
[![Arduino-lint](https://github.com/RobTillaart/MS5837/actions/workflows/arduino-lint.yml/badge.svg)](https://github.com/RobTillaart/MS5837/actions/workflows/arduino-lint.yml)
[![JSON check](https://github.com/RobTillaart/MS5837/actions/workflows/jsoncheck.yml/badge.svg)](https://github.com/RobTillaart/MS5837/actions/workflows/jsoncheck.yml)
[![GitHub issues](https://img.shields.io/github/issues/RobTillaart/MS5837.svg)](https://github.com/RobTillaart/MS5837/issues)

[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)](https://github.com/RobTillaart/MS5837/blob/master/LICENSE)
[![GitHub release](https://img.shields.io/github/release/RobTillaart/MS5837.svg?maxAge=3600)](https://github.com/RobTillaart/MS5837/releases)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/robtillaart/library/MS5837.svg)](https://registry.platformio.org/libraries/robtillaart/MS5837)


# MS5837

Library for the MS5837 temperature and pressure sensor and compatibles.


## Description

**Experimental**

The MS5837 is a waterproof device to measure temperature and pressure to a high level
of accuracy.
From the pressure one can calculate the depth or the altitude of the sensor to some extend.


#### Related

- https://github.com/RobTillaart/pressure Conversions
- https://github.com/RobTillaart/temperature Conversions
- https://github.com/RobTillaart/MS5611 temperature and pressure sensor

See also .h file


#### Compatibles

TODO

|  Sensor        |  temperature  |  pressure   |  Notes  |
|:---------------|:-------------:|:-----------:|:-------:|
|  MS5837-30bar  |               |             |
|  MS5837-02bar  |               |             |
|  MS5803-01bar  |               |             |  assumed compatible


#### Tests

The library is tested with the following boards: TODO

As always feedback is welcome.
Please let me know of other working platforms / processors (and failing ones!).



##  I2C

#### Performance

The maximum I2C speed is 400 KHz. (TODO verify).


#### Multiplexing

The MS5837 has one fixed address 0x76. So you can have only one per I2C bus.
If one needs to control more MS5837's one need an I2C multiplexer.

- https://github.com/RobTillaart/TCA9548 I2C multiplexer.


## Interface

```cpp
#include MS5837.h
```

#### Constructor

- **MS5837(TwoWire \* wire = &Wire))** constructor, optional 
- **bool begin()** initializes the library.
- **bool isConnected()** returns true if device visible on I2C bus.
- **bool reset()** resets the sensor and rereads its configuration.
- **uint8_t getType()** type indicates max pressure. 
Returns 30 or 2 or zero if unknown.
- **uint8_t getAddress()** returns 0x76 as this is a fixed address.


#### Temperature and Pressure

- **bool read()** reads the sensor and all its data.
Will block for 40 milliseconds.
- **float getPressure()** returns mBar.
- **float getTemperature()** returns degrees Celsius.
- **float getAltitude(float airPressure = 1013.25)** calculates the altitude, based upon actual pressure.
Returns meters.
One can compensate for the actual air pressure at sea level.

#### Density

- **void setDensity(float density)** set liquid density.
- **float getDensity()** returns set liquid density.
- **float get Depth(float airPressure = 1013.25)** calculates the altitude, based upon actual pressure.
returns meters.
One can compensate for the actual air pressure at sea level.


## Future

#### Must

- documentation
- buy hardware - 30 or 2 bar version ?
- test 
- add **lastRead()**

#### Should

- derived class for 5803 if compatible
- performance test
- can we tune the delays?
- can we see conversion ready?


#### Could

- add offset functions


#### Won't (unless requested)


## Support

If you appreciate my libraries, you can support the development and maintenance.
Improve the quality of the libraries by providing issues and Pull Requests, or
donate through PayPal or GitHub sponsors.

Thank you,

