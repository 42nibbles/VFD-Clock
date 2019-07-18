/*-----------------------------------------------------
* File: hv5812.h
* Auth: 42nibbles DZ
* YEAR: 2017
* defines  shift register ports and prototypes
//-----------------------------------------------------*/
#ifndef HV5812_H
#define HV5812_H

#define BLK 16   ///< Blanking input
#define LE 14    ///< Latch enable/Chip select
#define CLK 12   ///< Clock input
#define SDATA 13 ///< Serial data input
#define H_OFF 2  ///< Enable input of the switching regulator (heating)

#if defined(__DOXYGEN__)

/**
  * \brief Define output of the shift register.
  * \param contentSreg Serial data to be outputted.
  */
void shiftHV5812(long contentSreg);

#else

#ifdef __cplusplus
extern "C"
{
#endif

    void shiftHV5812(long contentSreg);

#ifdef __cplusplus
}
#endif

#endif //__DOXYGEN__

#endif //HV5812_H
