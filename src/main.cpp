
//Standard library includes
/*
  This clock is configured for use in the laboratory, which means the display is 
  switched on on workdays from monday to friday from 8:00 to 19:00 exept on 
  tuesday when "Open Lab" is running. The on-time on "Open Lab" is from 8.00
  to 23:00!
  On saturday and sunday the display is off!
  The ssid and the password is set on guest-network, in the FB4lab on
  "*****" and the passwd is set to "*****". In case of CONNECTION_FAILD
  the internal RTC is used as time-normal.  
  Be careful to use the "Time.h" and the "TimeLib.h" from Michael Hoffmann to prevent 
  errors in calculating the time.
*/
#include <Arduino.h>

//ESP WiFi include stuff
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

//RTC include stuff
#include <DS1307RTC.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <Wire.h>

//WifiManager Stuff
#include <WiFiManager.h>
#define UART_DEBUG 1

// Shift Register HV5812 and Ports #############################################
#include "hv5812.h"
#include "multiplexer.h"

#include <cstdint>

//NTP setting
constexpr char NTP_SERVER_NAME_S[] = "europe.pool.ntp.org";

//Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120}; //Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};   //Central European Standard Time
Timezone CE(CEST, CET);

//WiFiManager
const String AP_NAME = "ESP_CLOCK" + String(ESP.getChipId());
const char *AP_PASSWORD = "admin";

static WiFiUDP _udp;
static const unsigned int UDP_LOCAL_PORT = 2390; //local port to listen for UDP packets

// Shift Register fields #############################################
uint8_t fieldMux[6]; // field to display
unsigned long dDelay;
//Local function prototypes
static time_t getNtpTime();
static void uart_debug();
static time_t old_time_utc = 0;

static time_t getNtpTime()
{
  while (_udp.parsePacket())
    ; //discard any previously received packets

  // Request ntp server ip address
  IPAddress time_server_ip;
  WiFi.hostByName(NTP_SERVER_NAME_S, time_server_ip);
  Serial.print("Requesting network time from ");
  Serial.print(NTP_SERVER_NAME_S);
  Serial.print(" (");
  Serial.print(time_server_ip);
  Serial.println(")");

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
  Serial.print("NTP reply");
  delay(1000);
  int rply_size = _udp.parsePacket();
  if (rply_size)
  {
    Serial.println("\t\t\t[OK]");
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
    Serial.println("\t\t\t[FAILED]");
    return 0;
  }
}

void switchPower(uint8_t schalter)
{
  if (schalter)
  {
    digitalWrite(BLK, HIGH);
    digitalWrite(H_OFF, LOW);
  }
  else
  {
    digitalWrite(BLK, LOW);
    digitalWrite(H_OFF, HIGH);
  }
}

void setup()
{
  // put your setup code here, to run once:

  //Hello screen
  delay(2048);
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n");
  Serial.println("VFD Clock - Ver 1.0 on \nGeneric ESP8266 module");
  Serial.println("Testing VFD 7-Seg - hold on for 10 secs");
  Serial.println(F("\n\n42nibbles-NTP-Timezone"));

  pinMode(BLK, OUTPUT);   // Blanking Command input
  pinMode(LE, OUTPUT);    // Latch Enable Command input
  pinMode(CLK, OUTPUT);   // Shift Register clk input
  pinMode(SDATA, OUTPUT); // Shift Register data input
  pinMode(H_OFF, OUTPUT);

  digitalWrite(BLK, HIGH); // Blanking low active
  digitalWrite(LE, HIGH);  // Inverted in HW
  delay(500);
  Serial.println(F("Blanking Inactive!"));
  fieldMux[0] = 16;
  fieldMux[1] = 16; // indexing tenth of year
  fieldMux[2] = 2;  // switch on decimal point
  fieldMux[3] = 4;
  fieldMux[4] = 16; // switch on decimal point
  fieldMux[5] = 16;
  dDelay = millis();
  while ((millis() - dDelay) < 4000)
  {
    multiplexer(fieldMux);
    delay(1);
  }
  //Clock setting from RTC
  Serial.print("DS1307 real time clock on I2C");
  time_t rtc_time = RTC.get();
  if (rtc_time != 0)
  {
    setTime(rtc_time);
    Serial.println("\t[OK]");
    Serial.print("UTC date is: ");
    Serial.print(weekday());
    Serial.print("   ");
    Serial.print(day());
    Serial.print(".");
    Serial.print(month());
    Serial.print(".");
    Serial.println(year());
    Serial.print("UTC time is: ");
    Serial.print(hour());
    Serial.print(":");
    Serial.print(minute());
    Serial.print(":");
    Serial.println(second());
  }
  else
  {
    Serial.println("\t[FAILED]");
  }

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();

  //set custom ip for portal
  //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect(AP_NAME.c_str(), AP_PASSWORD);
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  if (true)
  {
    Serial.println("\t\t\t[OK]");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    //print_wifi_status();
    //Create UDP port
    Serial.print("Creating UDP port ");
    _udp.begin(UDP_LOCAL_PORT);
    Serial.println(_udp.localPort());
    //NTP sync
    time_t ntp_time = getNtpTime();
    if (ntp_time != 0)
    {
      setSyncProvider(getNtpTime);
      RTC.set(now());
    }
  }
  else
  {
    Serial.println("\t\t[FAILED]");
    Serial.println("Using DS1307 as time reference");
    setSyncProvider(RTC.get);
  }
  Serial.println("Using localtime zone 'Berlin', CEST, CET");
}

void loop()
{
  time_t local_time;
  time_t switch_hour;

  if (UART_DEBUG == 1)
  {
    uart_debug();
  }

  if (old_time_utc != now())
  {
    old_time_utc = now();
    TimeChangeRule *tcr;
    local_time = CE.toLocal(old_time_utc, &tcr);
    int sec = second(local_time);
    switch_hour = hour(local_time);
#if 0
    Serial.print("Hour-> ");
    Serial.print(hour(local_time));
    Serial.print("\tLocal-> ");
    Serial.print((int)switch_hour);
    Serial.print("\t  Weekday-> ");
    Serial.print(weekday(local_time));
    Serial.print("\t Tue: ");
    Serial.println(Tue);
#endif
    if (sec > 55)
    {
      fieldMux[0] = year(local_time) % 10;
      fieldMux[1] = (year(local_time) - 2000) / 10; // indexing tenth of year
      fieldMux[2] = month(local_time) % 10;         // switch on decimal point
      fieldMux[3] = month(local_time) / 10;
      fieldMux[4] = day(local_time) % 10; // switch on decimal point
      fieldMux[5] = day(local_time) / 10;
    }
    else
    {
      fieldMux[0] = second(local_time) % 10;
      fieldMux[1] = second(local_time) / 10;
      fieldMux[2] = minute(local_time) % 10; // switch on decimal point
      fieldMux[3] = minute(local_time) / 10;
      fieldMux[4] = hour(local_time) % 10; // switch on decimal point
      fieldMux[5] = hour(local_time) / 10;
    }

    /*   Switch display on and off according to weekday  */

#if 1
    switch (weekday(local_time))
    {
    case Sun: // Sunday
      /*   Switch off */
      switchPower(0);
      //    digitalWrite(BLK, LOW);                         // Blanking low active
      break;

    case Mon: // Monday
              /*   Switch on at 8:00h and off at 19:00h   */
      if ((switch_hour < 8) || (switch_hour >= 19))
      {
        switchPower(0);
        //      digitalWrite(BLK, LOW);                      // Blanking low active
      }
      else
      {
        //      digitalWrite(BLK, HIGH);
        Serial.println("here");
        switchPower(1);
      }
      break;
    case Tue: // Tuesday
              /*   Switch on at 8:00h and off at 23:00h   */
      if ((switch_hour < 8) || (switch_hour >= 23))
      {
        switchPower(0);
        //      digitalWrite(BLK, LOW);                      // Blanking low active
        //      Serial.println("BLK active! ");
      }
      else
      {
        //      digitalWrite(BLK, HIGH);
        switchPower(1);
      }
      break;
    case Wed: // Wednesday
              /*   Switch on at 8:00h and off at 19:00h   */
      if ((switch_hour < 8) || (switch_hour >= 19))
      {
        //      digitalWrite(BLK, LOW);                      // Blanking low active
        switchPower(0);
      }
      else
      {
        digitalWrite(BLK, HIGH);
        switchPower(1);
      }
      break;
    case Thu: // Thursday
              /*   Switch on at 8:00h and off at 19:00h   */
      if ((switch_hour < 8) || (switch_hour >= 19))
      {
        digitalWrite(BLK, LOW); // Blanking low active
        switchPower(0);
      }
      else
      {
        digitalWrite(BLK, HIGH);
        switchPower(1);
      }
      break;

    case Fri: // Friday
              /*   Switch on at 8:00h and off at 19:00h   */
      if ((switch_hour < 8) || (switch_hour >= 17))
      {
        //      digitalWrite(BLK, LOW);                      // Blanking low active
        switchPower(0);
        //      Serial.print("Freitag ->");
        //      Serial.println(switch_hour);
      }
      else
      {
        //      digitalWrite(BLK, HIGH);
        switchPower(1);
      }
      break;
    case Sat: // Saturday
              /*   Switch  off  */
              //    digitalWrite(BLK, LOW);
      switchPower(0);
      break;
    default:
      switchPower(0);
      break;
    }
#endif
  }

  multiplexer(fieldMux);
}

void uart_debug()
{
  char res_char;

  //Easy UART Debug interface
  if ((res_char = Serial.read()))
  {
    switch (res_char)
    {
    //Restart ESP5
    case 'p':
      ESP.restart();
      break;
    }
  }
}
