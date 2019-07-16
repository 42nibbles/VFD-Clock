# VFD Clock_0.9

## Netzwerk konfigurieren

Die Uhr verwendet den unter [https://github.com/tzapu/WiFiManager](https://github.com/tzapu/WiFiManager/ "WiFiManager Library")
beschriebenen *WiFiManager.* Diese Software versucht zuerst, die letzte funktionierende Konfiguration zu benutzen, mit dem Ziel
einen *WiFi Client* zu starten. Falls das nicht funktioniert, wird ein *WiFi Server* gestartet.

Der Name des Servers ist *ESP_CLOCK#CHIP_ID#* - also z.B. *ESP_CLOCK3532264.*

Mit dem einem WiFi-Gerät kann man sich darauf einloggen, um eine neue WiFi Client-Konfiguration einzugeben.

Hier kommt ein Beispiel für die Ausgabe, die eine VFD Clock auf der Konsole erzeugt, die gerade konfiguriert wird:

```text
VFD Clock - Ver.0-pre005 on
Generic ESP8266 module
Testing VFD 7-Seg - hold on for 10 secs


42nibbles-NTP-Timezone
Blanking Inactive!
DS1307 real time clock on I2C   [OK]
UTC date is: 3   16.7.2019
UTC time is: 17:41:50
*WM:
*WM: AutoConnect
*WM: Connecting as wifi client...
*WM: Using last saved values, should be faster
*WM: Connection result:
*WM: 1
*WM:
*WM: Configuring access point...
*WM: ESP_CLOCK3532264
*WM: Invalid AccessPoint password. Ignoring
*WM:
*WM: AP IP address:
*WM: 192.168.4.1
*WM: HTTP server started
*WM: Request redirected to captive portal
*WM: Request redirected to captive portal
*WM: Handle root
*WM: Request redirected to captive portal
*WM: Request redirected to captive portal
*WM: Scan done
*WM: DUP AP: FB4STUD
*WM: DUP AP: FB4PMD
*WM: DUP AP: FB4PMD
*WM: DUP AP: FB4INFO
*WM: DUP AP: FB4INFO
*WM: DUP AP: FB4INFO
*WM: DUP AP: FB4GAST
*WM: DUP AP: FB4SW2
*WM: DUP AP: FB4SW2
*WM: DUP AP: IDiAL FHDO
*WM: DUP AP: eduroam
*WM: DUP AP: SST-MOBILE
*WM: DUP AP: SST-MOBILE
*WM: TP-Link_9A4E
*WM: -60
*WM: hw1_gast
*WM: -60
*WM: NixieClock
*WM: -65
*WM: FRITZ!Box 7590 HO
*WM: -75
*WM: AI-THINKER_D18A05
*WM: -79
*WM: FB4STUD
*WM: -80
*WM: FB4PMD
*WM: -81
*WM: FB4INFO
*WM: -81
*WM: FB4GAST
*WM: -81
*WM: FB4SW2
*WM: -81
*WM: IDiAL FHDO
*WM: -81
*WM: eduroam
*WM: -82
*WM: SST-MOBILE
*WM: -82
*WM: Sent config page
*WM: Request redirected to captive portal
*WM: Request redirected to captive portal
*WM: Request redirected to captive portal
*WM: WiFi save
*WM: Sent wifi save page
*WM: Connecting to new AP
*WM: Connecting as wifi client...
*WM: Connection result:
*WM: 3
connected...yeey :)
                        [OK]
IP address: 192.168.0.107
Creating UDP port 2390
Requesting network time from europe.pool.ntp.org (185.159.125.100)
NTP reply                       [OK]
Requesting network time from europe.pool.ntp.org (185.159.125.100)
NTP reply                       [OK]
Using localtime zone 'Berlin', CEST, CET
*WM: freeing allocated params!
Requesting network time from europe.pool.ntp.org (91.236.251.12)
NTP reply                       [OK]
Requesting network time from europe.pool.ntp.org (85.254.217.2)
NTP reply                       [FAILED]
Requesting network time from europe.pool.ntp.org (193.70.90.148)
NTP reply                       [OK]
Requesting network time from europe.pool.ntp.org (178.63.9.110)
NTP reply                       [OK]
```
