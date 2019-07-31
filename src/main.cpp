/**
  \file   main.cpp
  \author 42nibbles DZ, "Michael Hoffmann"<michael.hoffmann@fh-dortmund.de>
  \date   2017, 2019
  \brief  Main firmware compilation file for the six tube VFD clock.

  This clock is configured for use in the laboratory, which means the display
  is switched on on workdays from monday to friday from 8:00 to 19:00 exept on
  tuesday when "Open Lab" is running. The on-time on "Open Lab" is from 8.00
  to 23:00!
  On saturday and sunday the display is off!
  The ssid and the password is set on guest-network, in the FB4lab on
  "*****" and the passwd is set to "*****". In case of CONNECTION_FAILD
  the internal RTC is used as time-normal.  
  Be careful to use the "Time.h" and the "TimeLib.h" from Michael Hoffmann to prevent 
  errors in calculating the time.
*/
/**
  \mainpage 42nibbles six tube equipped VFD clock

  \section intro_sec Introduction

  This is the introduction.

  \section install_sec Installation

  \subsection step1 Step 1: Opening the box

  \todo Das hier zu Ende dokumentieren.
 */
#include <Arduino.h>

// ESP WiFi include stuff
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

// RTC include stuff
#include <DS1307RTC.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <Wire.h>

// WifiManager Stuff
#include <WiFiManager.h>

// VFD tube stuff
#include "hv5812.h"
#include "multiplexing.h"

#include <string>
#include <cstdint>
#include <cstring>
#include <ctime>

#ifdef DOXYGEN
/**
 * \def   SUPPORT_WIFI_NTP_SYNC
 * \brief Turn off WiFi by commenting this.
 *
 * If the clock will not be able to connect to a WiFi access point it will automatically become a WiFi access point
 * of its own and the user will be given chance to configure his WiFi settings by mobile phone or something else.
 * If you definitely do not plan to use WiFi this might bother you, because the clock will never start displaying
 * the time.
 *
 * By removing WiFi support you will not be able to synchronize against a NTP server.  Then the internal real time
 * clock will be used for synchronizing instead.
 *
 * The configuration of WiFi is described in the README.md file.
 *
 * Hinweis: Auf Messen bitte SUPPORT_WIFI_NTP_SYNC auskommentieren, falls die Uhr ohne WiFi laufen soll.
 */
#define SUPPORT_WIFI_NTP_SYNC

/**
 * \def   SUPPORT_POWER_SAVE_MODE
 * \brief Suport of power save mode.
 * \sa    is_idle_time()
 *
 * As a default the clock supports power saving.  The settings are defined in is_idle_time().  If you like to
 * run the clock around the clock comment this.
 *
 * Hinweis: Auf Messen bitte SUPPORT_POWER_SAVE_MODE auskommentieren, falls die Uhr 24/7 laufen soll.
 */
#define SUPPORT_POWER_SAVE_MODE

#else
#define SUPPORT_WIFI_NTP_SYNC   // Comment this if you are not going to use WiFi.
#define SUPPORT_POWER_SAVE_MODE // Comment this if you are not going to use power saving.
#endif                          // DOXYGEN

#define UART_BAUDRATE 115200UL ///< UART baudrate for info messages and the VFD Clock debug terminal.
#define UART_DEBUG 1           ///< Activate the VFD Clock debug terminal.

/**
 * \brief Power switch setting.
 * \sa    power_switch()
 */
typedef enum
{
  PWR_OFF = 0, ///< Power to be turned off
  PWR_ON       ///< Power to be turned on
} power_switch_e;

/// The URL of the NTP server to be used for clock synchronization.  You are advised to use the address of a NTP pool.
constexpr char NTP_SERVER_NAME_STR[] = "europe.pool.ntp.org";

// Central European Time (Frankfurt, Paris)
const TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120}; ///< Rule for the Central European Summer Time.
const TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};   ///< Rule for the Central European Standard Time.
Timezone CE(CEST, CET);                                       ///< Timezone object needed by the local time functions.

/**
  \brief  Admin settings for the WiFiManager
  \sa     https://github.com/tzapu/WiFiManager/
  
  The WiFiManager will try to use its saved credential settings to do a login on a WiFi access station.  If
  this fails it will become a WiFi access station of its own.  The name is something like *VFD-CLOCK_3532264*
  with the trailing number being the ESP Chip Id.  You can do a login to the VFD-Clock without password.
  After this just open a new browser page to enter the WiFiManager configuration page.  You can do a network
  scan and type your new network configuration credentials there.
 */
const String AP_NAME = "VFD-CLOCK_" + String(ESP.getChipId());
const char *AP_PASSWORD = "admin"; ///< \todo TODO: Malfunctional at this moment.

#define IODEF_VFD_DRIVER_BLANKING 16 ///< Blanking input
#define IODEF_VFD_DRIVER_STROBE 14   ///< Latch enable/Chip select
#define IODEF_VFD_DRIVER_CLOCK 12    ///< Clock input
#define IODEF_VFD_DRIVER_SDATA_IN 13 ///< Serial data input
#define IODEF_VFD_HEATING 2          ///< Enable input of the switching regulator (heating)

// UDP settings for NTP socket
static WiFiUDP _udp;
static const unsigned int UDP_LOCAL_PORT = 2390; //local port to listen for UDP packets

// Local function prototypes
static time_t timeProvider(void);
static time_t initialRtcRead(void);
static time_t getNtpTime(void);
static bool is_idle_time(int weekday, int hour);
static void power_switch(power_switch_e switch_setting);
static void uart_debug(void);

/// Arduino framework standard function.
void setup()
{
  // Serial setup to 115200 baud.
  delay(2048UL);
  Serial.begin(UART_BAUDRATE);
  delay(256UL);

  // Send greetings message to serial.  Startup VFD tubes and display "  42  ".
  Serial.println(F("\n  ******   VFD Clock - Ver 1.2   ******"));
  Serial.println(F("Running on Espressif Generic ESP8266 ESP-01 1M SoC module"));
  Serial.printf("ChipId %u, %i MHz clock speed, %u bytes flash @%2.1f MHz\n", ESP.getChipId(),
                ESP.getCpuFreqMHz(), ESP.getFlashChipSize(), (ESP.getFlashChipSpeed() / 1000000.0));
  Serial.println(F("\n -- VFD 8 tubes 7-Seg display startup --"));
  Serial.println(F("Setting blanking inactive and turn on heating"));
  Serial.println(F("Your display should indicate \"  42  \" now"));
  // I/O mode configuration
  HV5812_init(IODEF_VFD_DRIVER_BLANKING, IODEF_VFD_DRIVER_STROBE, IODEF_VFD_DRIVER_CLOCK, IODEF_VFD_DRIVER_SDATA_IN);
  pinMode(IODEF_VFD_HEATING, OUTPUT); // Heating control
  // External hardware configuration
  power_switch(PWR_ON);
  delay(512UL);
  // VFD display greeting message "  42  "
  uint8_t vfd_output[VFD_TUBE_CNT];
  // Values are 0 to 15 for '0,1,2,...,F'. 16 is ' ' (blank)
  const uint8_t BLANK = 16; // choosing VFD_BLANK defined in multiplexer.h would be ok, too.
  vfd_output[0] = BLANK;    // rightmost tube
  vfd_output[1] = BLANK;
  vfd_output[2] = 2;
  vfd_output[3] = 4;
  vfd_output[4] = BLANK;
  vfd_output[5] = BLANK; // leftmost tube
  const unsigned long DISPLAY_DELAY_MILLIS = 4000UL;
  const unsigned long START_MILLIS = millis();
  while ((millis() - START_MILLIS) < DISPLAY_DELAY_MILLIS)
  {
    setVfd(vfd_output);
    delay(1UL);
  }
  clearVfd();

#ifdef SUPPORT_WIFI_NTP_SYNC
  // WiFiManager: Try last stored WiFi client settings otherwise become a configurable server ;-)
  // Local intialization. Once its business is done, there is no need to keep it around
  Serial.println(F("\n -- 42nibbles VFD network startup --"));
  WiFiManager wifiManager;
  // reset saved settings
  //wifiManager.resetSettings();
  // set custom ip for portal
  //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect((const char *)AP_NAME.c_str(), AP_PASSWORD);
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  // if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  // Datagram service will be done by UDP
  Serial.printf("Creating UDP client port %u...", UDP_LOCAL_PORT);
  if (_udp.begin(UDP_LOCAL_PORT) == 1)
  {
    Serial.println(F("\t\t\t\t\t[passed]"));
  }
  else
  {
    Serial.println(F("\t\t\t\t\t[failed]"));
    Serial.println(F("-- bus error: unable to access clock device --"));
    Serial.println(F("-- rebooting and going for the next round --"));
    ESP.restart();
    // never reach this
  }
#else
  Serial.println(F("\nNo WiFi functionality was intended in this firmware\n -- NTP will not sync... --"));
#endif

  // Startup clock systems
  Serial.println(F("\n -- 42nibbles VFD clock startup --"));
  Serial.println(F("Installed timezone is 'Europe/Berlin', CEST, CET"));
  Serial.println(F("Starting background clock syncing system"));
  setSyncProvider(&timeProvider);
  Serial.println(F("\nRunning clock in endless loop..."));
}

/// Arduino framework standard function.
void loop()
{
  static time_t old_time_utc;
  static bool has_idle_time;
  static int old_hour_utc = hour() - 1; // should be processed upcoming

  if (UART_DEBUG == 1)
  {
    uart_debug();
  }

  // This has to be processed only when the next second has arrived
  if (old_time_utc != now())
  {
    old_time_utc = now();
    // Variables for time zone calculation
    TimeChangeRule *tcr;
    time_t local_time = CE.toLocal(old_time_utc, &tcr);
    // Find out if the clock has arrived it's idle time
    if (old_hour_utc != hour())
    {
      old_hour_utc = hour();
      has_idle_time = is_idle_time(weekday(local_time), hour(local_time));
    }
    // Display output if necessary
    if (has_idle_time)
    {
      power_switch(PWR_OFF);
    }
    else
    {
      power_switch(PWR_ON);
      // Display setting
      int sec = second(local_time);
      uint8_t vfd_output[VFD_TUBE_CNT]; // used for VFD output
      if (sec > 55)
      {
        vfd_output[0] = year(local_time) % 10;
        vfd_output[1] = (year(local_time) - 2000) / 10; // indexing tenth of year
        vfd_output[2] = month(local_time) % 10;
        vfd_output[3] = month(local_time) / 10;
        vfd_output[4] = day(local_time) % 10;
        vfd_output[5] = day(local_time) / 10;
        updateVfd(vfd_output, 0); // Dots are permanently turned on
      }
      else
      {
        vfd_output[0] = second(local_time) % 10;
        vfd_output[1] = second(local_time) / 10;
        vfd_output[2] = minute(local_time) % 10;
        vfd_output[3] = minute(local_time) / 10;
        vfd_output[4] = hour(local_time) % 10;
        vfd_output[5] = hour(local_time) / 10;
        updateVfd(vfd_output, 1000); // Blinking dots with a period of 1000 ms
      }
    }
  }
}

//********************************************************************
// Local functions
//********************************************************************

static time_t timeProvider(void)
{
  static bool runs_first_time = true;
  time_t utc_time = 0;

  // initial round
  if (runs_first_time)
  {
    // setup of internal real time clock device
    Serial.print(F("Accessing DS1307/DS1308 real time clock on I2C bus (SLAVE 0x68)..."));
    utc_time = initialRtcRead();
    if (utc_time == (time_t)-1)
    { // Severe failure: Display a message and quit execution.
      Serial.println(F("\t[failed]"));
      Serial.println(F("-- bus error: unable to access clock device --"));
      Serial.println(F("-- rebooting and going for the next round --"));
      ESP.restart();
      // never reach this
    }
    else
    {
      Serial.println(F("\t[passed]"));
      Serial.printf("UTC date in internal RTC is %02i.%02i.%04i UTC\n", day(utc_time), month(utc_time), year(utc_time));
      Serial.printf("UTC time in internal RTC is %02i:%02i:%02i UTC\n", hour(utc_time), minute(utc_time), second(utc_time));
    }
    // NTP server connection would be an usefull feature but is not mandatory.
    Serial.printf("Trying NTP server query from (%s)...", NTP_SERVER_NAME_STR);
    utc_time = getNtpTime();
    if (utc_time == (time_t)-1)
    { // NTP not available at the moment but we can have the time from RTC anyway.
      Serial.println(F("\t\t\t[failed]"));
      Serial.println(F("Using internal real time clock for this time."));
      Serial.println(F("(Retrying further NTP queries later.)"));
      utc_time = RTC.get();
    }
    else
    { // We got time from the NTP and can now sync the RTC.
      RTC.set(utc_time);
      Serial.println(F("\t\t\t[passed]"));
      Serial.println(F("Time from NTP server received.  Synchronized RTC with time server."));
      Serial.printf("UTC date from NTP server is %02i.%02i.%04i UTC\n", day(utc_time), month(utc_time), year(utc_time));
      Serial.printf("UTC time from NTP server is %02i:%02i:%02i UTC\n", hour(utc_time), minute(utc_time), second(utc_time));
    }
    runs_first_time = false;
    return utc_time;
  }

  // usual way to go
  utc_time = getNtpTime();
  if (utc_time != (time_t)-1)
  {
    // Now we can synchronize clocks with time server.
    RTC.set(utc_time);
    Serial.printf("Time from NTP server received.  Synchronized RTC with time server @%li UTC.\n", utc_time);
  }
  else
  {
    // Use RTC for synchronization, because there was no answer from NTP server.
    utc_time = RTC.get();
    Serial.printf("Syncing with internal RTC @%li UTC.\n", utc_time);
  }
  return utc_time;
}

static time_t initialRtcRead(void)
{
  // Caution: In case of error RTC.get() returns 0 instead of expected ((time_t) -1)
  time_t utc_time = RTC.get();

  if (utc_time == 0) // something went wrong
  {
    if (RTC.chipPresent()) // at least RTC was found
    {
      // A chip is present, but it was not initialized.
      // Setting it initially to 1 January 1970 UTC.
      RTC.set(0);
      utc_time = 0;
    }
    else
    {
      // Severe hardware failure.  Unable to run the clock properly.
      utc_time = (time_t)-1;
    }
  }
  return utc_time;
}

static time_t getNtpTime(void)
{
  if (_udp.localPort() == 0)
  {
    // Network was turned off by commenting '#define SUPPORT_WIFI_NTP_SYNC'.
    return -1;
  }

  while (_udp.parsePacket())
    ; //discard any previously received packets

  // Request ntp server ip address
  IPAddress time_server_ip;
  WiFi.hostByName(NTP_SERVER_NAME_STR, time_server_ip);

  // Doing the NTP request
  const int NTP_PACKET_SIZE = 48;
  byte packet_buffer[NTP_PACKET_SIZE];
  memset(packet_buffer, 0, NTP_PACKET_SIZE);
  packet_buffer[0] = 0b11100011; // LI, Version, Mode
  packet_buffer[1] = 0;          // Stratum, or type of clock
  packet_buffer[2] = 6;          // Polling Interval
  packet_buffer[3] = 0xec;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packet_buffer[12] = 49;
  packet_buffer[13] = 0x4e;
  packet_buffer[14] = 49;
  packet_buffer[15] = 52;
  // timestamp request
  _udp.beginPacket(time_server_ip, 123); // NTP server port is 123
  _udp.write(packet_buffer, NTP_PACKET_SIZE);
  _udp.endPacket();

  // Handling of NTP reply
  delay(1000);
  int rply_size = _udp.parsePacket();
  if (rply_size)
  {
    _udp.read(packet_buffer, NTP_PACKET_SIZE);
    //the timestamp starts at byte 40 and is two words long
    unsigned long hword = word(packet_buffer[40], packet_buffer[41]);
    unsigned long lword = word(packet_buffer[42], packet_buffer[43]);
    unsigned long secs_since_1900 = hword << 16 | lword;
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800. So
    // we have to subtract seventy years.
    time_t unix_time = secs_since_1900 - 2208988800UL;
    return unix_time;
  }
  else
  {
    return (time_t)-1;
  }
}

/**
 * \brief  Determines if it is idle time.
 * \param  weekday The weekday as given by weekday()
 * \param  hour The hour as given by hour()
 * \return true if it is idle time, else false.
 */
static bool is_idle_time(int weekday, int hour)
{
  bool is_idle = false;

  // If power saving is not supported we will never have idle time.
#ifndef SUPPORT_POWER_SAVE_MODE
  return is_idle;
#endif

  switch (weekday)
  {
  case Sat: // Weekend detection-week end means idle time
  case Sun:
    is_idle = true;
    break;
  case Tue: // "Day of the Open Lab" at tuesday
    if (hour < 8 || hour >= 23)
      is_idle = true;
    break;
  case Fri: // Friday setting
    if (hour < 8 || hour >= 17)
      is_idle = true;
    break;
  default: // All else
    if (hour < 8 || hour >= 19)
      is_idle = true;
  }
  return is_idle;
}

/**
 * \brief Turns the display and heating off.
 * \param switch_setting Turn off if PWR_OFF, else turn on.
 * \sa    power_switch_e
 * 
 * The display and heating can be turned off.  In each case the clock will run on.
 */
static void power_switch(power_switch_e switch_setting)
{
  switch (switch_setting)
  {
  case PWR_ON:
    // No blanking for shift register.
    HV5812_blanking(BLANKING_OFF);
    // Turn on VFD tube heating wire.
    digitalWrite(IODEF_VFD_HEATING, LOW);
    break;
  case PWR_OFF:
    // Blanking enable for shift register.
    HV5812_blanking(BLANKING_ON);
    // Turn off VFD tube heating.
    digitalWrite(IODEF_VFD_HEATING, HIGH);
    break;
  }
}

static void uart_debug(void)
{
  // XXX (hoffmann): Changing the menu system is a bit difficult because
  // of the security entry requirements for the 'w' option.  This seems
  // to be effective, but the maintenance is a drag.
  static int entry_requirements;
  char user_input;

  // Easy UART Debug interface
  if ((user_input = Serial.read()))
  {
    switch (user_input)
    {
      // help screen
    case 'h':
      entry_requirements = 1;
      Serial.println(F("\nVFD Clock debug terminal - available commands"));
      Serial.println(F(" h\thelp"));
      Serial.println(F(" v\tfull version"));
      Serial.println(F(" r\trestart ESP"));
      Serial.println(F(" w\treconfigure WiFi settings"));
      Serial.println(F("  \t(smartphone or something like that needed)"));
      break;
      // restart ESP
    case 'r':
      logOffVfd();
      clearVfd();
      ESP.restart();
      break;
      // restart ESP
    case 'v':
      entry_requirements = 0;
      Serial.println();
      Serial.println(ESP.getFullVersion());
      Serial.println();
      break;
      // reconfigure WiFi settings
    case 'w':
      if (entry_requirements & 1)
      {
        if (entry_requirements & 2)
        {
          if (entry_requirements & 4)
          {
            logOffVfd();
            clearVfd();
            entry_requirements = 0;
            Serial.println(F("\n\n**********\nErasing your credentials."));
            Serial.print(F("Do a WiFi scan and login on device "));
            Serial.print(AP_NAME.c_str());
            Serial.println(".\nAfter that open a new browser page.");
            Serial.println("**********");
            Serial.println();
            Serial.println();
            WiFiManager wifiManager;
            wifiManager.resetSettings();
            delay(1024UL);
            ESP.restart();
          }
          else
          {
            entry_requirements |= 4;
          }
          Serial.println(F("\nW A R N I N G : This will erase your WiFi access credentials."));
          Serial.println(F("Do you really want to erase them?  -  Then press 'w' for a third time."));
        }
        else
        {
          entry_requirements |= 2;
        }
      }
      break;
    }
  }
}
