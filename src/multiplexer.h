/*---------------------------------------------------------------
  The seg_7 array contains the segments defined for IV22 VFD-7-Segment tubes
  the constant TAG_DP an MONAT_DP or's a dacimal point to the contentRegister
  the "gate" array defines the pins to the gates of the tube-array for muxing
  the constant value muxInt contains on-time for 1 muxgroup of tubes 
*/
#ifndef MULTIPLEXER_H
#define MULTIPLEXER_H

#include <stdint.h>

#define MONAT_DP 0x00008000L
#define TAG_DP 0x00000080L

const long muxInt = 5;

const uint8_t gate[3] = {16, 17, 18}; // PINS of Gate muxing of shift reg
const uint8_t seg_7[33] = {
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
/*Elements of the IV22b-tubes */
/*
const uint8_t seg_7 [33] = {
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
  0b00000000,   // dark
  0b00101110,   // 4
  0b01011101,   // 2
  0b00111000,   // n
  0b00100000,   // i
  0b01111010,   // b
  0b01111010,   // b
  0b00100100,   // l
  0b01011011,   // E
  0b01101011,   // S
  0b00000000,   // dark
  0b00000000,   // dark
  0b00000000,   // dark
  0b00000000,   // dark
  0b00000000,   // dark
  0b00000000    // dark
};
*/

#ifdef __cplusplus
extern "C"
{
#endif

  void multiplexer(uint8_t fieldMux[]);

#ifdef __cplusplus
}
#endif

#endif // MULTIPLEXER_H
