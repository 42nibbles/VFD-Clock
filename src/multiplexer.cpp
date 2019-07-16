#include "multiplexer.h"
#include "hv5812.h"
#include <Arduino.h>

// Don't change this.  It's only for the internal build logic.
#define VFR_TUBE_IV3A 1
#define VFR_TUBE_IV22B 2

//********************************************************************
// User configurable area - depends on clock hardware
//********************************************************************

// !!!You must select exactly ONE of these!!!
#define ACTIVE_VFR_TUBE VFR_TUBE_IV3A
//#define ACTIVE_VFR_TUBE VFR_TUBE_IV22B

/// PINS of Gate muxing of shift reg
const uint8_t GATE[3] = {16, 17, 18};

// XX.XX.XX -> dot position
#define MONAT_DP 0x00008000L
#define TAG_DP 0x00000080L

#if ACTIVE_VFR_TUBE == VFR_TUBE_IV3A
/*Elements of the IV3A-tubes */
const uint8_t SEG_7[33] = {
    //  HDCBAGFE   H=decimal point not in every tube
    0b01111011, // 0
    0b00110000, // 1
    0b01011101, // 2
    0b01111100, // 3
    0b00110110, // 4
    0b01101110, // 5
    0b01101111, // 6
    0b00111000, // 7
    0b01111111, // 8
    0b01111110, // 9
    0b00111111, // A
    0b01100111, // b
    0b01001011, // C
    0b01110101, // d
    0b01001111, // E
    0b00001111, // F
    0b00000000, // dark
    0b00110110, // 4
    0b01011101, // 2
    0b00100101, // n
    0b00100000, // i
    0b01100111, // b
    0b01100111, // b
    0b00110000, // l
    0b01001111, // E
    0b01101110, // S
    0b00000000, // dark
    0b00000000, // dark
    0b00000000, // dark
    0b00000000, // dark
    0b00000000, // dark
    0b00000000  // dark
};
#else if ACTIVE_VFR_TUBE VFR_TUBE_IV22B
/*Elements of the IV22b-tubes */
const uint8_t SEG_7[33] = {
    //  HGFEDCBA   H=decimal point not in every tube
    0b01110111, // 0
    0b00100100, // 1
    0b01011101, // 2
    0b01101101, // 3
    0b00101110, // 4
    0b01101011, // 5
    0b01111011, // 6
    0b00100101, // 7
    0b01111111, // 8
    0b01101111, // 9
    0b00111111, // A
    0b01111010, // b
    0b01010011, // C
    0b01111100, // d
    0b01011011, // E
    0b00011011, // F
    0b00000000, // dark
    0b00101110, // 4
    0b01011101, // 2
    0b00111000, // n
    0b00100000, // i
    0b01111010, // b
    0b01111010, // b
    0b00100100, // l
    0b01011011, // E
    0b01101011, // S
    0b00000000, // dark
    0b00000000, // dark
    0b00000000, // dark
    0b00000000, // dark
    0b00000000, // dark
    0b00000000  // dark
};
#endif

void multiplexer(uint8_t fieldMux[])
{
  const long MUX_INT = 5;

  static unsigned long prevMuxTime;
  unsigned long currMuxTime;
  long contentSreg;

  static uint8_t muxGate;
  static uint8_t muxCnt;

  currMuxTime = millis();
  if (currMuxTime - prevMuxTime >= MUX_INT)
  {
    prevMuxTime = currMuxTime;
    contentSreg = ((SEG_7[fieldMux[muxGate]] << 8) | SEG_7[fieldMux[muxGate + 3]] | (1 << GATE[muxGate]));
    muxCnt++; // update-time -> MUX_INT
    if (muxCnt < 100)
    {
      if (muxGate == 1)
      {
        contentSreg |= TAG_DP;
      }
      if (muxGate == 2)
      {
        contentSreg |= MONAT_DP;
      }
    }
    else
    {
      if (muxCnt > 200)
      {
        muxCnt = 0;
      }
    }
    shiftHV5812(contentSreg);
    muxGate++;
    if (muxGate > 2)
    {
      muxGate = 0;
    }
    contentSreg = 0;
  }
}
