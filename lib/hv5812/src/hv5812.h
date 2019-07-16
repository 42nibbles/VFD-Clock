/*-----------------------------------------------------
* File: hv5812.h
* Auth: 42nibbles DZ
* YEAR: 2017
* defines  shift register ports and prototypes
//-----------------------------------------------------*/
#ifndef HV5812_H
#define HV5812_H

#define BLK 16
#define LE 14
#define CLK 12
#define SDATA 13
#define H_OFF 2

#ifdef __cplusplus
extern "C"
{
#endif

    void shiftHV5812(long contentSreg);

#ifdef __cplusplus
}
#endif

#endif //HV5812_H
