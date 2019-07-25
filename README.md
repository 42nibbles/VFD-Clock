# VFD Clock_1.1

## Übersetzen unter Windows

Ein Problem ergibt sich aus der Benutzung der Library *Time@~1.5, ID:44* von _Michael Morgolis/Paul Stoffregen._ Diese Library
wird durch die gegenwärtigen *platformio.ini* Konfiguration in den Ordner *%your-vfd-clock-dir%/.pio/libdeps/esp01_1m/Time_ID44*
geladen. In _Visual Studio Code_ wird das wahrscheinlich links in der _Folder-Ansicht_ *.pio, libdeps, esp01_1m, Time_ID44* zu
finden sein.

* Dort befindet sich eine Datei _Time.h._ Da Windows nicht zwischen Groß- und Kleinschreibung unterscheidet, gibt es Konflikte
mit der Systemdatei _time.h._

Dieses Thema wurde zum gegenwärtigen Zeitpunkt im PlatformIO-Forum diskutiert, aber noch nicht gelöst. Aus diesem Grund wird das
folgende Vorgehen vorgeschlagen:

1. Die Datei _Time.h_ in *_Time.h* umbenennen oder löschen. Der Sinn dieser Datei ist mir rätselhaft, weil sie nur für das
Includieren der Datei _TimeLib.h_ zuständig ist und ansonsten das Verhalten der Library platformabhängig macht.
1. Die Datei _DateStrings.cpp_ editieren, wobei im Include-Teil _#include "Time.h"_ durch _#include "TimeLib.h"_ ersetzt wird.
1. Die Datei _Time.cpp_ editieren, wobei im Include-Teil _#include "Time.h"_ durch _#include "TimeLib.h"_ ersetzt wird.

## Hochladen der Firmware

Nach dem Übersetzen kann die Firmware hochgeladen werden. TODO: Programmer-Hardware beschreiben

Möglicherweise müssen in der _platformio.ini_ die Einstellungen für _upload_port_ und _monitor_port_ angepaßt werden.

* Unter Linux wären _/dev/ttyUSB0,_ _/dev/ttyUSB1_ usw. plausible Werte.
* Unter Windows wären _COM3,_ _COM4_ usw. plausible Werte.

Falls noch keine anderen _USB Serial Port_-Konverter angeschlossen sind, funktioniert es wahrscheinlich von selbst.

**Vorsicht:** Falls mehrere Konverter mit dem PC verbunden sind, entscheidet das Autoselect zufällig, welcher davon
verwendet wird!

## Inbetriebnahme / Netzwerk-Konfiguration

Die Uhr verwendet den unter [https://github.com/tzapu/WiFiManager](https://github.com/tzapu/WiFiManager/ "WiFiManager Library")
beschriebenen *WiFiManager.* Diese Software versucht zuerst, die letzte funktionierende WiFi-Konfiguration zu benutzen, mit dem
Ziel einen *WiFi Client* zu starten. Falls das nicht funktioniert, wird ein *WiFi Server* gestartet.

Der Name des Servers ist *ESP_CLOCK#CHIP_ID#* - also z.B. *ESP_CLOCK3532264.*

Mit dem einem WiFi-Gerät kann man sich darauf einloggen, um eine neue WiFi Client-Konfiguration einzugeben.

**Hinweis:** Falls das Verwenden von WiFi nicht beabsichtigt ist, kann in der Datei _main.cpp_ die Zeile
_#define USE_WIFI_NTP_SYNC_ auskommentiert werden. Damit wird die Uhr lediglich die interne Echtzeituhr
zur Synchronisation verwenden und bleibt beim Start nicht im WiFi-Setup hängen.

**Hinweis:** Die Zeile _#define USE_WIFI_NTP_SYNC_ gibt es zweimal: Die erste Zeile ist für die Doxygen-Dokumentation und
soll nicht geändert werden; die zweite Zeile ist für den Compiler und verändert wirklich, ob die Software WiFi benutzen wird.
