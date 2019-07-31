/**
  \file   multiplexing.h
  \brief  Software for a multiplexed access to the connected VFD tubes.

  The VFD clock display consists of six 7-segment VFD tubes.  The tube types
  may be IV-3A (ИВ-3А) or IV-12 (ИВ-12).

  \image html IV-3A_front_s.jpg "ИВ-3А, Translit IV-3A"
  \image html IV-12_front_s.jpg "ИВ-12, Translit IV-12"

  Tube descriptions can be found in the file
  [russian-book](../specs/russian-book-0001.djvu "russian-book-0001.djvu").
  For the description of ИВ-3А take a look at page 190, for ИВ-12 at page 193.

  A short help to get along with the russion language.

  <ul>
  <li><em>Точка, Translit Tochka</em> means dot.</li>
  <li>The physical quantity can often be derived from their units.</li>
  </ul>

  The tubes are driven in a multiplexed mode as shown here:

  <kbd>. . 1 . . 1</kbd><br />
  <kbd>. 2 . . 2 .</kbd><br />
  <kbd>3 . . 3 . .</kbd>

  It seems to be practical to switch on each group for ca. 5 milliseconds (200 Hz)
  or shorter to avoid the impression of flickering.  You can find example code
  under setVfd().

  Because this is rather annoying–especially if you like to strictly comply the
  timing–there is a second way doing this by using a background interrupt mechanism
  by using updateVfd().

  Beware: The background interrupt seems to clash with the WiFi manager in its
  server mode.  So do not use them together which would result in a crashing WiFi
  library.

  When using the function updateVfd() you also have control over the decimal dots.
*/
#ifndef MULTIPLEXING_H
#define MULTIPLEXING_H

#include <cstdint>

/// Count of accessible VFD tubes connected to the multiplexer.
const unsigned VFD_TUBE_CNT = 6U;
/// Special 'blank character' value for multiplexer() array for turning tube temporarily off.
const uint8_t VFD_BLANK = 16;
/// Refreshed tubes each 5 milliseconds.
const uint8_t VFD_REFRESH_MS_PERIOD = 5;
/// VFD output for a all blank display.
const uint8_t VFD_OUTPUT_BLANK[VFD_TUBE_CNT] = {VFD_BLANK, VFD_BLANK, VFD_BLANK, VFD_BLANK, VFD_BLANK, VFD_BLANK};

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * \brief Sets all VFD tubes to VFD_BLANK.
   * \sa    VFD_BLANK
   */
  void clearVfd();

  /**
   * \brief Turns off the background interrupt mechanism used by updateVfd().
   * \sa    updateVfd()
   */
  void logOffVfd();

  /**
   * \brief Output digits to VFD display.
   * \param vfd_output[] Array of digits to be displayed.
   * \sa    VFD_TUBE_CNT
   * \sa    VFD_BLANK
   *
   * Each particular tube is dedicated to a element of the vfd_output array.  If
   * MAX_TUBE_CNT is six the element [0] is the rightmost tube and the element [5] the
   * leftmost tube.
   *
   * The used tubes are VFD-7-Segement tubes so we are able to display 16 digits from 
   * '<kbd>0,1,2,...,F</kbd>'.  Valid values for the digits are from 0 to 15.  A special
   * value is 16 which means <kbd>' '</kbd> (blank).  This value is also defined in VFD_BLANK.
   *
   * So this source code could be used for displaying <kbd>"  42  "</kbd> for four seconds:
    \code{.c}
    // VFD display greeting message "  42  " for 4 seconds.
    // Don't forget to do i/o setup before this.
    // VFD display greeting message "  42  "
    uint8_t vfd_output[VFD_TUBE_CNT];
    // Values are 0 to 15 for '0,1,2,...,F'. 16 is ' ' (blank)
    vfd_output[0] = VFD_BLANK; // rightmost tube
    vfd_output[1] = VFD_BLANK;
    vfd_output[2] = 2;
    vfd_output[3] = 4;
    vfd_output[4] = VFD_BLANK;
    vfd_output[5] = VFD_BLANK; // leftmost tube
    const unsigned long DISPLAY_DELAY_MILLIS = 4000UL;
    const unsigned long START_MILLIS = millis();
    while ((millis() - START_MILLIS) < DISPLAY_DELAY_MILLIS) {
      setVfd(vfd_output);
      delay(1UL);
    }
    clearVfd();
    \endcode
   *
   * The recommended refresh rate of the Display is 5 milliseconds or shorter.  If it is too
   * slow the display has a visible flicker effect.  When you get even slower you can see the
   * multiplexing groups.
   */
  void setVfd(const uint8_t vfd_output[VFD_TUBE_CNT]);

  /**
   * \brief Output digits to VFD display.
   * \param vfd_output[] Array of digits to be displayed.
   * \param dot_blink_ms_period Control of dot blinking behaviour.
   * \sa    VFD_TUBE_CNT
   * \sa    VFD_BLANK
   * \sa    VFD_REFRESH_MS_PERIOD
   * \sa    logOffVfd()
   * \sa    clearVfd()
   *
   * Each particular tube is dedicated to a element of the vfd_output array.  If
   * MAX_TUBE_CNT is six the element [0] is the rightmost tube and the element [5] the
   * leftmost tube.
   *
   * The used tubes are VFD-7-Segement tubes so we are able to display 16 digits from 
   * '<kbd>0,1,2,...,F</kbd>'.  Valid values for the digits are from 0 to 15.  A special
   * value is 16 which means <kbd>' '</kbd> (blank).  This value is also defined in VFD_BLANK.
   *
   * So this source code could be used for displaying <kbd>"  42  "</kbd> for four seconds:
    \code{.c}
    // VFD display greeting message "  42  " for 4 seconds.
    // Don't forget to do i/o setup before this.
    // VFD display greeting message "  42  "
    uint8_t vfd_output[VFD_TUBE_CNT];
    // Values are 0 to 15 for '0,1,2,...,F'. 16 is ' ' (blank)
    vfd_output[0] = VFD_BLANK; // rightmost tube
    vfd_output[1] = VFD_BLANK;
    vfd_output[2] = 2;
    vfd_output[3] = 4;
    vfd_output[4] = VFD_BLANK;
    vfd_output[5] = VFD_BLANK; // leftmost tube
    const unsigned long DISPLAY_DELAY_MILLIS = 4000UL;
    updateVfd(vfd_output, -1);
    delay(DISPLAY_DELAY_MILLIS);
    logOffVfd(); // If you like to turn off background interrupt stuff for VFD stuff.
    clearVfd();
    \endcode
   *
   * The recommended refresh rate of the Display is 5 milliseconds or shorter.  If it is too
   * slow the display has a visible flicker effect.  When you get even slower you can see the
   * multiplexing groups.  The flash rate is determined by VFD_REFRESH_MS_PERIOD.
   *
   * If you set dot_blink_ms_period to 0 the dots are continuously turned on.  If you put
   * this to a negative number the dots are continuously turned off.  Otherwise you can set
   * a period time of e.g. 1000 ms, that would be synchronous with your tubes displaying the
   * value of the seconds.
   */
  void updateVfd(const uint8_t vfd_output[VFD_TUBE_CNT], int dot_blink_ms_period=0);

#ifdef __cplusplus
}
#endif

#endif // MULTIPLEXING_H
