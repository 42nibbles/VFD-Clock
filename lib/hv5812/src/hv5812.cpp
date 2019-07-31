#include "hv5812.h"
#include <Arduino.h>

const unsigned CLOCK_DELAY_US = 2U;

static uint8_t _bl;
static uint8_t _strobe;
static uint8_t _clk;
static uint8_t _data_in;

void HV5812_vfdDriver(long content_data_in)
{
  for (int i = 0; i < 20; i++)
  {
    if (content_data_in & (1 << (19 - i)))
    {
      digitalWrite(_data_in, HIGH);
    }
    else
    {
      digitalWrite(_data_in, LOW);
    }
    delayMicroseconds(CLOCK_DELAY_US);
    digitalWrite(_clk, HIGH);
    delayMicroseconds(CLOCK_DELAY_US);
    digitalWrite(_clk, LOW);
  }
  digitalWrite(_strobe, LOW); // latch-pulse inverted
  delayMicroseconds(CLOCK_DELAY_US);
  digitalWrite(_strobe, HIGH);
}

void HV5812_blanking(vfd_driver_blanking_e blanking)
{
  switch (blanking)
  {
  case BLANKING_OFF:
    digitalWrite(_bl, HIGH);
    break;
  case BLANKING_ON:
    digitalWrite(_bl, LOW);
    break;
  }
}

void HV5812_init(uint8_t bl, uint8_t strobe, uint8_t clk, uint8_t data_in)
{
  _bl = bl;
  _strobe = strobe;
  _clk = clk;
  _data_in = data_in;

  pinMode(bl, OUTPUT);      // Blanking Command input
  pinMode(strobe, OUTPUT);  // Latch Enable Command input
  pinMode(clk, OUTPUT);     // Shift Register clk input
  pinMode(data_in, OUTPUT); // Shift Register data input

  digitalWrite(bl, HIGH);     // Blanking low active
  digitalWrite(strobe, HIGH); // Inverted in HW
}
