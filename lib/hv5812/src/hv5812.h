/**
  \file   hv5812.h
  \author 42nibbles DZ
  \date   2017
  \brief  Driver software for HV5812 shift register ic.
  \sa     https://www.google.com/search?q=datasheet+pdf+hv5812+microchip&oq=datasheet+pdf+hv5812+microchip
 */
#ifndef HV5812_H
#define HV5812_H

#include <stdint.h>

/**
 * \brief Blanking line setting.
 * \sa    [HV5812.pdf](../../lib/hv5812/docs/HV5812.pdf "Hardware specs")
 */
typedef enum {
  BLANKING_ON, ///< Value of blanking on
  BLANKING_OFF ///< Value of blanking off
} vfd_driver_blanking_e;

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * \brief Accesses BL to control the output source drivers.
   * \param blanking BLANKING_ON to turn off high voltages drivers, BLANKING_OFF to turn them on.
   * \sa    vfd_driver_blanking_e
   * \sa    [HV5812.pdf](../../lib/hv5812/docs/HV5812.pdf "Hardware specs")
   */
  void HV5812_blanking(vfd_driver_blanking_e blanking);

  /**
   * \brief Initializes the io lines for the HV5812
   * \param bl Iopin of the BLANKING line.
   * \param strobe Iopin of the STROBE line.
   * \param clk Iopin of the CLOCK line.
   * \param data_in Iopin of the SERIAL DATA IN line.
   * \sa    [HV5812.pdf](../../lib/hv5812/docs/HV5812.pdf "Hardware specs")
   */
  void HV5812_init(uint8_t bl, uint8_t strobe, uint8_t clk, uint8_t data_in);

  /**
   * \brief Transmit data to shift register ic.
   * \param content_data_in Serial data to be outputted to the high voltage outputs HVout1 to HVout20.
   * \sa    [HV5812.pdf](../../lib/hv5812/docs/HV5812.pdf "Hardware specs")
   */
  void HV5812_vfdDriver(long content_data_in);

#ifdef __cplusplus
}
#endif

#endif //HV5812_H
