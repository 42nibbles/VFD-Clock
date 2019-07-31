#include "multiplexing.h"

#include <Arduino.h>
#include "hv5812.h"
#include <Ticker.h>
#include <cstdbool>
#include <cstring>

// Don't change this.  It's only for the internal build logic.
#define VFR_TUBE_IV3A 1
#define VFR_TUBE_IV12 2

//********************************************************************
// User configurable area - depends on clock hardware
//********************************************************************

// !!!You must select exactly ONE of these!!!
#define ACTIVE_VFR_TUBE VFR_TUBE_IV3A
//#define ACTIVE_VFR_TUBE VFR_TUBE_IV12

/// PINS of Gate muxing of shift reg
const uint8_t GATE[3] = {16, 17, 18};

// XX.XX.XX -> dot position
#define MONAT_DP 0x00008000L
#define TAG_DP 0x00000080L

#if ACTIVE_VFR_TUBE == VFR_TUBE_IV3A
/*Elements of the IV3A-tubes */
const uint8_t SEG_7[33] = {
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
#elif ACTIVE_VFR_TUBE == VFR_TUBE_IV12
/*Elements of the IV22b-tubes */
const uint8_t SEG_7[33] = {
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
    0b00000000, // dark
    0b00101110, // 4
    0b01011101, // 2
    0b00111000, // n
    0b00100000, // i
    0b01111010, // b
    0b01111010, // b
    0b00100100, // l
    0b01011011, // E
    0b01101011, // S
    0b00000000, // dark
    0b00000000, // dark
    0b00000000, // dark
    0b00000000, // dark
    0b00000000, // dark
    0b00000000  // dark
};
#endif

// Local constants
static const uint16_t US_PRO_MS = 1000;
static const uint8_t TICKS_PRO_US = 5; // 5 ticks/us if timer1_enable(TIM_DIV16,...)
static const uint32_t TIMER_TICKS = VFD_REFRESH_MS_PERIOD * US_PRO_MS * TICKS_PRO_US;

// Local variables
static bool _has_to_be_configured = true;
static uint8_t _vfd_output[VFD_TUBE_CNT];
static int _dot_blink_ms_half_period;
static bool _vfd_update_necessary;
static bool _vfd_log_off_necessary;

// Local function prototypes
static void ICACHE_RAM_ATTR vfd_refresh_callback();

void clearVfd()
{
  setVfd(VFD_OUTPUT_BLANK);
  setVfd(VFD_OUTPUT_BLANK);
  setVfd(VFD_OUTPUT_BLANK);
}

void logOffVfd()
{
  // This will be recognized by the callback function of the interrupt.
  _vfd_log_off_necessary = true;
  _has_to_be_configured = true;
}

void setVfd(const uint8_t vfd_output[VFD_TUBE_CNT])
{
  static uint8_t mux_gate;

  // Compute value for the output shift register
  long content_sreg = ((SEG_7[vfd_output[mux_gate]] << 8) | SEG_7[vfd_output[mux_gate + 3]] | (1 << GATE[mux_gate]));
  // Send this to shift register for output
  HV5812_vfdDriver(content_sreg);
  // Select gate for the next round
  if (++mux_gate > 2)
    mux_gate = 0;
}

void updateVfd(const uint8_t vfd_output[VFD_TUBE_CNT], int dot_blink_ms_period)
{
  // Parameter copy for interrupt handler function
  // TODO: Maybe we should lock timer interrupt while doing this?
  memcpy(_vfd_output, vfd_output, sizeof(vfd_output[0]) * VFD_TUBE_CNT);
  // The dot thing is special
  if (dot_blink_ms_period < 0)
    _dot_blink_ms_half_period = -1;
  else
    _dot_blink_ms_half_period = dot_blink_ms_period / 2;
  // Sync with interrupt
  _vfd_update_necessary = true;
  // This has to be done only once
  if (_has_to_be_configured)
  {
    timer1_isr_init();
    timer1_attachInterrupt(vfd_refresh_callback);
    timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
    timer1_write(TIMER_TICKS); // 5 ms refresh rate for VFD tubes
    _has_to_be_configured = false;
  }
}

//********************************************************************
// Local functions
//********************************************************************

// This callback function should be called by timer ISR.
static void ICACHE_RAM_ATTR vfd_refresh_callback()
{
  static bool dot_is_on = true;
  static int ms_counter_for_dot_logic;
  static uint8_t mux_gate;

  // If we should stop this we do it here and now.
  if (_vfd_log_off_necessary)
  {
    timer1_disable();
    timer1_detachInterrupt();
    _vfd_log_off_necessary = false;
    return;
    // Never reached...
  }

  // Logic for toggling tube dots
  if (_dot_blink_ms_half_period > 0)
  { // If there is a valid period defined this means toggling
    // Dot synchronization with output
    if (_vfd_update_necessary == true)
    {
      _vfd_update_necessary = false;
      ms_counter_for_dot_logic = _dot_blink_ms_half_period;
    }
    // Count the time until next toggle
    ms_counter_for_dot_logic += VFD_REFRESH_MS_PERIOD;
    if (ms_counter_for_dot_logic >= _dot_blink_ms_half_period)
    {
      dot_is_on = !dot_is_on;
      ms_counter_for_dot_logic = 0;
    }
  }
  else if (_dot_blink_ms_half_period < 0)
  { // It there is a negative period defined this means turn off
    dot_is_on = false;
  }
  else
  { // It there is no period defined this means turn on
    dot_is_on = true;
  }
  // Compute value for the output shift register
  long content_sreg = ((SEG_7[_vfd_output[mux_gate]] << 8) | SEG_7[_vfd_output[mux_gate + 3]] | (1 << GATE[mux_gate]));
  if (dot_is_on)
  {
    if (mux_gate == 1)
      content_sreg |= TAG_DP;
    if (mux_gate == 2)
      content_sreg |= MONAT_DP;
  }
  // Send this to shift register for output
  HV5812_vfdDriver(content_sreg);
  // Select gate for the next round
  if (++mux_gate > 2)
    mux_gate = 0;
  timer1_write(TIMER_TICKS); // 5 ms refresh rate for VFD tubes
}
