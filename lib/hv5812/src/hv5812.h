/**
  \file   hv5812.h
  \author 42nibbles DZ
  \date   2017
  \brief  Driver software for HV5812 shift register ic.
  \sa     https://www.google.com/search?q=datasheet+pdf+hv5812+microchip&oq=datasheet+pdf+hv5812+microchip
 */
#ifndef HV5812_H
#define HV5812_H

#define BLK 16   ///< Blanking input
#define LE 14    ///< Latch enable/Chip select
#define CLK 12   ///< Clock input
#define SDATA 13 ///< Serial data input
#define H_OFF 2  ///< Enable input of the switching regulator (heating)

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * \brief Transmit data to shift register ic.
   * \param contentSreg ???
   * \todo  Das hier zu Ende dokumentieren.
   */
  void shiftHV5812(long contentSreg);

#ifdef __cplusplus
}
#endif

#endif //HV5812_H
