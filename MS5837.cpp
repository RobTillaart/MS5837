//
//    FILE: MS5837.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.0
//    DATE: 2023-11-12
// PURPOSE: Arduino library for MS5837 temperature and pressure sensor.
//     URL: https://github.com/RobTillaart/MS5837


#include "MS5837.h"

//  commands  (MS5611 alike)
#define MS5837_CMD_READ_ADC       0x00
#define MS5837_CMD_READ_PROM      0xA0
#define MS5837_CMD_RESET          0x1E
#define MS5837_CMD_CONVERT_D1     0x4A  //  differs from MS5611
#define MS5837_CMD_CONVERT_D2     0x5A  //  differs from MS5611


//  CONSTRUCTOR
MS5837::MS5837(TwoWire *wire)
{
  _wire = wire;
}

bool MS5837::begin()
{
  if (! isConnected()) return false;
  reset();

  return true;
}

bool MS5837::isConnected()
{
  _wire->beginTransmission(_address);
  return ( _wire->endTransmission() == 0);
}

bool MS5837::reset()
{
  command(MS5837_CMD_RESET);
  uint32_t start = millis();
  //  while loop prevents blocking RTOS
  while (micros() - start < 10)
  {
    yield();
    delay(1);
  }

  initConstants();

  //  SKIP CRC check

  //  Determine type
  uint16_t tmp = C[0] / 32;
  _type = MS5837_TYPE_UNKNOWN;
  if (tmp == 0)  _type = MS5837_TYPE_02;
  if (tmp == 21) _type = MS5837_TYPE_02;
  if (tmp == 26) _type = MS5837_TYPE_30;
  return true;
}

uint8_t MS5837::getType()
{
  return _type;
}

uint8_t MS5837::getAddress()
{
  return _address;
}


//////////////////////////////////////////////////////////////////////
//
//  SYNC INTERFACE  (is async possible)
//
bool MS5837::read()
{
  if (isConnected() == false) return false;

   //  D1 conversion
  _wire->beginTransmission(_address);
  _wire->write(MS5837_CMD_CONVERT_D1);
  _wire->endTransmission();  //  TODO check all of these

  uint32_t start = millis();
  //  while loop prevents blocking RTOS
  while (micros() - start < 20)
  {
    yield();
    delay(1);
  }
  //  NOTE: D1 and D2 are reserved in MBED (NANO BLE)
  uint32_t _D1 = readADC();

   //  D2 conversion
  _wire->beginTransmission(_address);
  _wire->write(MS5837_CMD_CONVERT_D2);
  _wire->endTransmission();

  start = millis();
  //  while loop prevents blocking RTOS
  while (micros() - start < 20)
  {
    yield();
    delay(1);
  }

  //  NOTE: D1 and D2 are reserved in MBED (NANO BLE)
  uint32_t _D2 = readADC();

  float dT = _D2 - C[5];
  _temperature = 2000 + dT * C[6];

  float offset = C[2] + dT * C[4];
  float sens   = C[1] + dT * C[3];
  _pressure = _D1 * sens + offset;


  //  Second order compensation
  if (_temperature < 20)
  {
    float ti = dT * dT * (11 * 2.91038304567E-11);  //  1 / 2^35
    float t = (_temperature - 2000) * (_temperature - 2000);
    float offset2 = t * (31 * 0.125);  //  1 / 2^3
    float sens2 = t * (63 * 0.03125);  //  1 / 2^5

    offset       -= offset2;
    sens         -= sens2;
    _temperature -= ti;
  }
  //                         1 / 2^21                     1 / 2^15 / 100
  _pressure = (_D1 * sens * 4.76837158203E-7 - offset) * 3.051757813E-5 * 0.01;
  _temperature *= 0.01;

  return true;
}


float MS5837::getPressure()
{
  return _pressure;
}

float MS5837::getTemperature()
{
  return _temperature;
}


//  https://www.mide.com/air-pressure-at-altitude-calculator  TODO: multimap the table.
//  used a simplified formula
// https://community.bosch-sensortec.com/t5/Question-and-answers/How-to-calculate-the-altitude-from-the-pressure-sensor-data/qaq-p/5702
//
float MS5837::getAltitude(float airPressure)
{
  float ratio = _pressure / airPressure;
	return 44330 * (1 - pow(ratio, 0.190294957));  
}



//////////////////////////////////////////////////////////////////////
//
//  DENSITY for depth
//
void MS5837::setDensity(float density)
{
  _density = density;
}

float MS5837::getDensity()
{
  return _density;
}

float MS5837::getDepth(float airPressure)
{
  //  9.80665 == Gravity constant.
  //  1 / (_density * 9.80665 * 10)  can be pre-calculated and cached in setDensity.
  //
  //  delta P = rho * g * h  => h = delta P / rho * g
  //  pressure = mbar, density grams/cm3 => correction factor 0.1 (=1/10)
  return (_pressure - airPressure)/(_density * 9.80665 * 10);  
}



//////////////////////////////////////////////////////////////////////
//
//  PRIVATE
//
int MS5837::command(uint8_t cmd)
{
  yield();
  _wire->beginTransmission(_address);
  _wire->write(cmd);
  _result = _wire->endTransmission();
  return _result;
}


void MS5837::initConstants()
{
  //  constants that were multiplied in read() - datasheet page 8
  //  do this once and you save CPU cycles
  //
  //                          datasheet MS5837  page 7
  C[0] = 1;
  C[1] = 65536L;          //  SENSt1   = C[1] * 2^16    v
  C[2] = 131072L;         //  OFFt1    = C[2] * 2^17    v
  C[3] = 7.8125E-3;       //  TCS      = C[3] / 2^7     v
  C[4] = 1.5625e-2;       //  TCO      = C[4] / 2^6     v
  C[5] = 256;             //  Tref     = C[5] * 2^8     v
  C[6] = 1.1920928955E-7; //  TEMPSENS = C[6] / 2^23    v

  for (uint8_t i = 0; i < 7 ; i++)
  {
    _wire->beginTransmission(_address);
    _wire->write(MS5837_CMD_READ_PROM + i + i);
    _wire->endTransmission();
    _wire->requestFrom(_address, 2);
    uint16_t tmp = _wire->read() << 8;
    tmp |= _wire->read();
    C[i] *= tmp;
  }
}


uint32_t MS5837::readADC()
{
  command(MS5837_CMD_READ_ADC);
  if (_result == 0)
  {
    uint8_t length = 3;
    int bytes = _wire->requestFrom(_address, length);
    if (bytes >= length)
    {
      uint32_t value = _wire->read() * 65536UL;
      value += _wire->read() * 256UL;
      value += _wire->read();
      return value;
    }
    return 0UL;
  }
  return 0UL;
}


//  -- END OF FILE --

