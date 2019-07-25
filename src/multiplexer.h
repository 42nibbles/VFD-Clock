/**
  \file   multiplexer.h
  \author 42nibbles DZ
  \date   2017
  \brief  Software for a multiplexed access on the connected VFD tubes.

  The seg_7 array contains the segments defined for IV22 VFD-7-Segment tubes
  the constant TAG_DP an MONAT_DP or's a dacimal point to the contentRegister
  the "gate" array defines the pins to the gates of the tube-array for muxing
  the constant value muxInt contains on-time for 1 muxgroup of tubes 
*/
#ifndef MULTIPLEXER_H
#define MULTIPLEXER_H

#include <cstdint>

/// Count of accessible VFD tubes connected to the multiplexer.
const unsigned VFD_TUBE_CNT = 6U;
/// Special 'blank character' value for multiplexer() array for turning tube temporarily off.
const uint8_t VFD_BLANK = 16;
/// Refreshed tubes each 5 milliseconds.
const uint8_t VFD_REFRESH_MS_PERIOD = 5;


#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * \brief Output digits to VFD display.
   * \param vfd_output[] Array of digits to be displayed.
   * \param dot_blink_ms_period The period for dot blinking in milliseconds. Zero means "no blinking".
   * \sa    VFD_TUBE_CNT
   *
   * Each particular tube is dedicated to a element of the vfd_output array.  If
   * MAX_TUBE_CNT is six  the element [0] is the rightmost tube and the element [5] the
   * leftmost tube.
   *
   * The used tubes are VFD-7-Segement tubes so we are able to display 16 digits from 
   * '<kbd>0,1,2,...,F</kbd>'.  Valid values for the digits are from 0 to 15.  A special
   * value is 16 which means <kbd>' '</kbd> (blank).
   *
   * So this source code would be used for displaying <kbd>"  42  "</kbd> for four seconds:
    \code{.c}
    // VFD display greeting message "  42  " for 4 seconds.
    // Don't forget to do i/o setup before this.
    // VFD display greeting message "  42  "
    uint8_t vfd_output[VFD_TUBE_CNT];
    // Values are 0 to 15 for '0,1,2,...,F'. 16 is ' ' (blank)
    const uint8_t BLANK = 16; // choosing VFD_BLANK defined in multiplexer.h would be ok, too.
    vfd_output[0] = BLANK; // rightmost tube
    vfd_output[1] = BLANK;
    vfd_output[2] = 2;
    vfd_output[3] = 4;
    vfd_output[4] = BLANK;
    vfd_output[5] = BLANK; // leftmost tube
    const unsigned long DISPLAY_DELAY_MILLIS = 4000UL;
    const unsigned long START_MILLIS = millis();
    while ((millis() - START_MILLIS) < DISPLAY_DELAY_MILLIS) {
      updateVfd(vfd_output);
      delay(1UL);
    }
    \endcode
   *
   * At this moment the refresh rate of the Display is 5 milliseconds. So the resolution of
   * dot_blink_ms_period is 5 ms, too.
   */
  void updateVfd(const uint8_t vfd_output[VFD_TUBE_CNT], int dot_blink_ms_period=0);

#ifdef __cplusplus
}
#endif

#endif // MULTIPLEXER_H
