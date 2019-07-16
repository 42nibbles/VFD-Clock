#include <Arduino.h>
#include "hv5812.h"

//### HV5812 SHIFT REGISTER ###################################################################

void shiftHV5812(long contentSreg)
{
  for (int i = 0; i < 20; i++)
  {
    if (contentSreg & (1 << (19 - i)) )
    {
      digitalWrite(SDATA, HIGH);
    }
    else
    {
      digitalWrite(SDATA, LOW);
    }
    delayMicroseconds(25);
    digitalWrite(CLK, HIGH);
    delayMicroseconds(25);
    digitalWrite(CLK, LOW);
  }
  digitalWrite(LE, LOW); // latch-pulse inverted
  delayMicroseconds(25);
  digitalWrite(LE, HIGH);
}
