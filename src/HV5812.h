/*-----------------------------------------------------
* File: HV5812.h
* Auth: 42nibbles DZ
* YEAR: 2017
* defines  shift register ports and prototypes
//-----------------------------------------------------*/
#ifndef HV5812_H
#define HV5812_H

#define BLK     16
#define LE      14
#define CLK     12
#define SDATA   13
#define H_OFF   2

void shiftHV5812( long contentSreg);


#endif
