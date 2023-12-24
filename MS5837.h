#pragma once
//
//    FILE: MS5837.h
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.0
//    DATE: 2023-11-12
// PURPOSE: Arduino library for MS5837 temperature and pressure sensor.
//     URL: https://github.com/RobTillaart/MS5837


#include "Arduino.h"
#include "Wire.h"


OSR factor in code...


#define MS5837_LIB_VERSION        (F("0.1.0"))


#define MS5837_TYPE_UNKNOWN        0
#define MS5837_TYPE_02             2
#define MS5837_TYPE_30            30


class MS5837
{
public:
  MS5837(TwoWire *wire = &Wire);

  bool    begin();
  bool    isConnected();
  bool    reset();
  uint8_t getType();
  uint8_t getAddress();


  //////////////////////////////////////////////////////////////////////
  //
  //  READ 
  //
  // will block for 40 milliseconds
  bool    read();
  //  see https://github.com/RobTillaart/pressure for conversions.
  //  returns mBar
  float   getPressure();
  //  see https://github.com/RobTillaart/temperature for conversions.
  //  returns Celsius
  float   getTemperature();
  //      compensate for actual air pressure if needed
  float   getAltitude(float airPressure = 1013.25);

  //////////////////////////////////////////////////////////////////////
  //
  //  DENSITY for depth and altitude (height)
  //
  //  density is temperature dependent, see
  //  https://www.usgs.gov/special-topics/water-science-school/science/water-density
  //
  //  density water 20°C = 0.99802
  //  density seawater is 
  //  density in grams / cm3  (so not in grams per liter
  void    setDensity(float density = 0.99802);
  float   getDensity();
  //      returns meters (SI unit)
  //      compensate for actual air pressure if needed
  float   getDepth(float airPressure = 1013.25);  



private:
  int        command(uint8_t cmd);
  void       initConstants();
  uint32_t   readADC();
  // uint16_t   readProm(uint8_t reg);

  uint8_t   _address = 0x76;   //  fixed address
  TwoWire * _wire = NULL;

  float     _pressure;
  float     _temperature;

  float     C[8];
  uint8_t   _type;
  int       _result;

  float     _density = 0.99802;
};


//////////////////////////////////////////////////////////////////////
//
//  DERIVED CLASSES
//

//  TODO


//  -- END OF FILE --

