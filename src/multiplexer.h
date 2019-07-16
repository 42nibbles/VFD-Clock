/*---------------------------------------------------------------
  The seg_7 array contains the segments defined for IV22 VFD-7-Segment tubes
  the constant TAG_DP an MONAT_DP or's a dacimal point to the contentRegister
  the "gate" array defines the pins to the gates of the tube-array for muxing
  the constant value muxInt contains on-time for 1 muxgroup of tubes 
*/
#ifndef MULTIPLEXER_H
#define MULTIPLEXER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  void multiplexer(uint8_t fieldMux[]);

#ifdef __cplusplus
}
#endif

#endif // MULTIPLEXER_H
