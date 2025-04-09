
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
- The MS5837_30 can go to depth of about 300 meter (30 Bar).
- The MS5837_02 is meant for altitude measurements as it can as low as 300 mBar.

The library does not use the partially 64 bit integer math as described in the datasheet.
Instead it uses float math.
This choice reduces footprint on e.g. AVR and increases the math performance.
This will however have an effect on the accuracy of the results, 
although these are expected relative small.

The library is not tested with hardware yet.

Feedback as always is welcome.


### Related

- https://github.com/RobTillaart/pressure Conversions
- https://github.com/RobTillaart/temperature Conversions
- https://github.com/RobTillaart/MS5611 temperature and pressure sensor
- https://www.usgs.gov/special-topics/water-science-school/science/water-density

See also .h file


### Compatibles

See mathType notes below.

|  Sensor         | mathType |  Celsius °C  |  pressure mBar  |  Notes  |
|:----------------|:--------:|:------------:|:---------------:|:-------:|
|  MS5837-30 bar  |    0     |  -20 to +85  |  0 to 30000     |  for depth
|  MS5837-02 bar  |    1     |  -20 to +85  |  300 to 1200    |  for altitude
|  MS5803-01 bar  |    2     |  -40 to +85  |  10 to 1300     |  for altitude, investigate


### Pressure mathType 

There are MS5837 compatibles for which the math for the pressure is different.
See **AN520__004: C-code example for MS56xx, MS57xx (except analog sensor), and MS58xx series pressure sensors**
The difference is in the constants (mostly powers of 2) used to calculate the pressure.

The library implements **reset(uint8_t mathMode = 0)** to select the mathMode.
- mathMode = 0 ==> datasheet MS5837_30  page 7
- mathMode = 1 ==> datasheet MS5837_02  page 7
- mathMode = 2 ==> datasheet MS5803_01  page 7

- other values will act as 0 (until new math pops up).


### Tests

The MS5837 library uses similar code as the tested MS5611 library.

TODO: test with hardware.

The library is tested with the following boards:

As always feedback is welcome.

Please let me know of other working platforms / processors (and failing ones!).


##  I2C

User has to call **Wire.begin()** (or equivalent) and optional set the I2C pins
before calling **MS5837.begin(mathMode)**.

### Performance

The maximum I2C speed is 400 kHz according to the datasheet.

The **read()** call has two blocking delays to read two ADC values.
THerefore the I2C speed is not a parameter to be tuned. 

### Address

The MS5837 has a fixed I2C address: 0x76.

### I2C multiplexing

Sometimes you need to control more devices than possible with the default
address range the device provides.
This is possible with an I2C multiplexer e.g. TCA9548 which creates up
to eight channels (think of it as I2C subnets) which can use the complete
address range of the device.

Drawback of using a multiplexer is that it takes more administration in
your code e.g. which device is on which channel.
This will slow down the access, which must be taken into account when
deciding which devices are on which channel.
Also note that switching between channels will slow down other devices
too if they are behind the multiplexer.

- https://github.com/RobTillaart/TCA9548


## Interface

```cpp
#include MS5837.h
```

### Constructor

- **MS5837(TwoWire \* wire = &Wire))** constructor, optional I2C bus.
- **bool begin(uint8_t mathMode)** initializes the library.
- **bool isConnected()** returns true if device visible on I2C bus.
- **bool reset(uint8_t mathMode)** resets the sensor and reads its configuration.
The mathMode must match the type of sensor, see table above.
- **uint8_t getType()** type indicates max pressure. 
Returns 30 or 2 or zero if unknown.
- **uint8_t getAddress()** returns 0x76 as this is a fixed address.


### Temperature and Pressure

- **bool read(uint8_t bits = 8)** the actual reading of the sensor. 
The bits determines the oversampling rate (OSR), see table below. 
Returns true upon success, false otherwise.
The call will block for 3 to 40 milliseconds, depending upon number of bits.
- **uint32_t lastRead()** returns the timestamp of the last call to read() in 
milliseconds since start. 
It does not take into account if the read call is successful or not.
- **float getPressure()** returns pressure in mBar.
Multiple calls will return the same value until read() is called again.
- **float getTemperature()** returns temperature in degrees Celsius.
Multiple calls will return the same value until read() is called again.
- **float getAltitude(float airPressure = 1013.25)** calculates the altitude,
based upon actual pressure measured and the current pressure at sea level.
Returns the altitude in meters.
Multiple calls will return the same value until read() is called again.
One can compensate for the actual air pressure at sea level.

|  type       |  bits read()  | 
|:-----------:|:-------------:|
|  MS5837_30  |  8..13        |
|  MS5837_02  |  8..13        |
|  MS5803_01  |  8..12        |


### Depth

For MS5837_30 only.

Depth calculation depends on the air pressure at sea level, and the density
of the liquid you are submerged in.
The density might not be a constant value and may vary over time.
It depends e.g. on the salinity (sea water) and temperature. 

The pressure is in fact the sum of the air pressure and the weight of the 
liquid above the sensor.
If the density of the is larger than water the maximum depth at which the
sensor will work will decrease. Denser liquids give more pressure.

- **void setDensity(float density = 0.99802)** set liquid density.
density water 20°C = 0.99802
- **float getDensity()** returns set liquid density.
- **float getDepth(float airPressure = 1013.25)** calculates the depth, 
based upon actual pressure and the pressure at sea level.
Returns depth in meters.
One can compensate for the actual air pressure at sea level.
Multiple calls will return the same value until read() is called again.


### Error handling

experimental / minimal

- **int lastError()** returns the last error code.
Resets to 0 when called.


## Density 

Some indicative figures about density of water and other liquids.
Different sources give slight variations, which are less than 0.1%.


### Temperature

Density table for (distilled) water H20, density in gram / cm3
From - https://www.usgs.gov/special-topics/water-science-school/science/water-density

|  Temp   |  Temp    |  Density  |
|:-------:|:--------:|:---------:|
|  32°F   |  0.0°C   |  0.99987  |
|  39.2°F |  4.0°C   |  1.00000  |
|  40°F   |  4.4°C   |  0.99999  |
|  50°F   |  10.0°C  |  0.99975  |
|  60°F   |  15.6°C  |  0.99907  |
|  70°F   |  21.1°C  |  0.99802  |
|  80°F   |  26.7°C  |  0.99669  |
|  90°F   |  32.2°C  |  0.99510  |
|  100°F  |  37.8°C  |  0.99318  |
|  120°F  |  48.9°C  |  0.98870  |
|  140°F  |  60.0°C  |  0.98338  |
|  160°F  |  71.1°C  |  0.97729  |
|  180°F  |  82.2°C  |  0.97056  |
|  200°F  |  93.3°C  |  0.96333  |
|  212°F  |  100°C   |  0.95865  |

One can linear interpolate between these points.


### Seawater

From - https://en.wikipedia.org/wiki/Seawater

The density of surface seawater ranges from about 1.020 to 1.029 g/cm3.
depending on the temperature and salinity. 
At a temperature of 25 °C, the salinity of 35 g/kg
and 1 atm pressure, the density of seawater is 1023.6 kg/m3.

The salinity of water can differ a lot e.g. near a river mouth sweet 
and salt water mix continuously.


### Depth (seawater)

The density varies also with depth, although for the range of this sensor
this difference is very small.

From - https://www.britannica.com/science/seawater/Density-of-seawater-and-pressure

|  Meter  |  Density  |
|:-------:|:---------:|
|  0      |  1.02813  |
|  1000   |  1.03285  |
|  2000   |  1.03747  |
|  4000   |  1.04640  |
|  6000   |  1.05495  |
|  8000   |  1.06315  |
|  10000  |  1.07104  |

This is almost a linear relation.
In formula (spreadsheet):

```
density = 1.02869 + 4.295e-6 * depth (meters)
```

For 30 meter the device can go under water the density is about ~1.02829.  
For 15 meter (average density from 0..30 meter) it is about ~1.02821.


### Other liquids

From - https://www.sfu.ca/phys/demos/demoindex/fluids/fl2b/density.html

Fluid Density (g/cm3) at 20C and 1 atm unless noted

|  Liquid         |  Density        |
|:---------------:|:---------------:|
|  water          |  0.99820        |
|  water (0 C)    |  0.99984        |
|  water (4 C)    |  0.99997        |
|  water (100 C)  |  0.95836        |
|  gasoline       |  0.66 - 0.69    |
|  ethyl alcohol  |  0.791          |
|  turpentine     |  0.85           |
|  olive oil      |  0.9            |
|  castor oil     |  0.969          |
|  sea water      |  1.03           |
|  milk           |  1.028 - 1.035  |
|  glycerin       |  1.260          |
|  mercury        |  13.55          |


### Solution - sodium chloride in water

From - https://www.sfu.ca/phys/demos/demoindex/fluids/fl2b/density.html

grams solute/100 grams solution, density (g/cm3)

|  Solution  |  Density  |  Notes  |
|:----------:|:---------:|:-------:|
|      3.5   |   1.0236  |  sea water, from Wikipedia, see above.
|     10.0   |   1.0726  |
|     20.0   |   1.1498  |
|     26.0   |   1.1993  |

in formula  (spreadsheet)

```
density = 0.995413 + 0.00779241 * solution;
```

### Solution - sucrose in water

From - https://www.sfu.ca/phys/demos/demoindex/fluids/fl2b/density.html

grams solute/100 grams solution, density (g/cm3)

|  Solution  |  Density  |
|:----------:|:---------:|
|     5      |  1.0197   |
|     10     |  1.0400   |
|     15     |  1.0610   |
|     19     |  1.0784   |


in formula (spreadsheet)

```
density = 0.998438 + 0.0041907 * solution;
```


## Future

#### Must

- improve documentation
- buy hardware - 30 or 2 bar version, both to test
- check TODO's in code / documentation


#### Should

- improve error handling
- create derived classes?
  - so one does not need to set mathMode.
  - for MS5803
- investigate the effects of float math on accuracy / precision.


#### Could

- add **uint16_t getPromZero()** read out manufacturer bits, CRC + extra bits.
  - useful for debugging?
- add **void setAirPressure(float airPressure)** idea is to set it only once when P changes.
- add **float getAirPressure()** return last set value.
- refactor type & mathMode
- add offset functions for all measurements?
  - 3 offsets float == 12 bytes + 6 functions get/set.
- async interface.
  - can we see conversion ready?
  - **void requestMeasurement()** starts conversion D1.
  - **bool ready()** checks D1 to be ready, starts D2, checks D2 to be ready
  - need multiple calls to initiate steps.
  - could be 0.2.0 if sync version works. 
  - ==> also MS5611 ?

```cpp
uint16_t getPromZero()
{
  uint16_t value = C[0];  //  first convert back to uint16_t.
  return value;
}
```

#### Won't (unless requested)

- performance test (as long as read is blocking)


## Support

If you appreciate my libraries, you can support the development and maintenance.
Improve the quality of the libraries by providing issues and Pull Requests, or
donate through PayPal or GitHub sponsors.

Thank you,

