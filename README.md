# ReactionGrid

ReactionGrid ist die Firmware für eine interaktive 10×10-Reaktionsmatrix mit beleuchteten Tastern und einem umlaufenden LED-Rahmen. Das System basiert auf einem ESP32-ETH01, kommuniziert per UDP mit einem externen Spielserver und bietet zusätzlich ein Webinterface für Konfiguration, Selbsttest und Firmware-Updates.

## Funktionen

- 10×10 LED- und Tastermatrix mit 100 Spielfeldern
- Messung und Übertragung der Reaktionszeit
- Umlaufender LED-Rahmen mit 292 LEDs und 20 Segmenten
- Matrix- und Rahmen-Effekte
- Textanzeige mit 5×7-Pixelfont
- Ethernet-Kommunikation über UDP
- Heartbeat und Verbindungsstatus
- WLAN-Access-Point für lokale Konfiguration
- Webinterface mit Basic Authentication
- OTA-Firmware-Update über den Browser
- Integrierter Hardware-Selbsttest
- Einstellungen im nichtflüchtigen ESP32-Speicher

## Hardware

- ESP32-ETH01
- 100 adressierbare RGB-LEDs für die Matrix
- 292 adressierbare RGB-LEDs für den Rahmen
- 100 Taster
- 10 × PCF8575 I/O-Expander
- Zwei I²C-Busse
- LAN8720 Ethernet-PHY des ESP32-ETH01
- Ausreichend dimensionierte 5-V-Stromversorgung

## Pinbelegung

### LED-Ausgänge

| Funktion | GPIO |
|---|---:|
| Matrix-LEDs | 17 |
| Rahmen-LEDs | 5 |

### I²C-Busse

| Bus | SDA | SCL | PCF8575-Adressen |
|---|---:|---:|---|
| I²C 1 | 4 | 14 | 0x20–0x24 |
| I²C 2 | 32 | 33 | 0x20–0x24 |

### Ethernet

| Funktion | GPIO / Wert |
|---|---|
| PHY | LAN8720 |
| PHY-Adresse | 1 |
| MDC | 23 |
| MDIO | 18 |
| PHY Power | 16 |
| Clock | GPIO0 Input |

## Software und Bibliotheken

Getestete Versionen laut Firmwarekopf:

- Arduino ESP32 Core 2.0.17
- ArduinoJson 7.2.1
- Adafruit NeoPixel 1.15.1

Die übrigen verwendeten Komponenten wie `ETH`, `WiFi`, `WiFiUDP`, `WebServer`, `Preferences`, `Wire` und `Update` sind Bestandteil des ESP32-Arduino-Cores.

## Installation

1. Arduino IDE installieren.
2. ESP32-Boardpaket in Version 2.0.17 einrichten.
3. ArduinoJson und Adafruit NeoPixel installieren.
4. Dieses Repository herunterladen oder klonen.
5. `ReaktionGrid.ino` in der Arduino IDE öffnen.
6. Das passende ESP32-ETH01-Board beziehungsweise eine kompatible ESP32-Konfiguration auswählen.
7. Firmware kompilieren und auf den Controller übertragen.

## Netzwerk und Bedienung

Beim Start öffnet das Gerät zusätzlich einen WLAN-Access-Point.

Standardwerte:

| Einstellung | Standardwert |
|---|---|
| Hostname / AP-Name | `ReactionWall` |
| Benutzername | `admin` |
| Passwort | `12345678` |
| UDP-Server | `192.168.1.100:5000` |
| Lokaler UDP-Port | `5001` |
| Ethernet | DHCP |

Nach Verbindung mit dem Access Point kann das Webinterface über die angezeigte AP-IP geöffnet werden. Dort lassen sich Netzwerkdaten, UDP-Ziel, Heartbeat-Timeout und Zugangsdaten ändern. Außerdem stehen Selbsttest, Neustart und Firmware-Update zur Verfügung.

> Das Standardpasswort sollte vor einem produktiven Einsatz geändert werden.

## Kommunikationsprinzip

Die Firmware sendet Tastendrücke als JSON-Pakete per UDP an den konfigurierten Server. Enthalten sind Zeile, Spalte und gemessene Reaktionszeit. Der Server kann wiederum Befehle zum Setzen einzelner Pixel, ganzer Zeilen oder Spalten, Rahmen-Segmente, Zähler sowie Matrix- und Rahmen-Effekte senden.

Der Verbindungsstatus wird direkt auf Matrix und Rahmen dargestellt:

- `STARTING` – Systemstart
- `NO LINK` – keine Ethernet-Verbindung
- `WAIT DHCP` – keine IP-Adresse
- `WAIT SERVER` – Server oder Heartbeat fehlt
- `READY` – betriebsbereit

## Effekte

### Matrix

- Zentrierter Text
- Lauftext
- Gewinner-Effekt
- Verlierer-Effekt

### Rahmen

- Regenbogen
- Fade
- Fade Up
- Fade Down
- Flash
- Loader
- Prozentanzeige über `setCounter`

## Selbsttest

Der Selbsttest kann im Webinterface gestartet werden. Jeder Tastendruck schaltet das betroffene Matrixfeld und den Rahmen nacheinander durch:

1. Aus
2. Rot
3. Grün
4. Blau

Damit können Taster, Matrix-LEDs und Rahmen gemeinsam geprüft werden.

## Projektstruktur

| Datei | Aufgabe |
|---|---|
| `ReaktionGrid.ino` | Hauptprogramm und Ereignisverarbeitung |
| `Connection.*` | Ethernet, WLAN-AP, UDP und JSON-Protokoll |
| `Interface.*` | Webinterface, Einstellungen und OTA-Update |
| `Matrix.*` | 10×10-Matrix, Tasterabfrage und Matrixeffekte |
| `Border.*` | LED-Rahmen, Segmente und Rahmeneffekte |
| `SelfTest.*` | Hardware-Selbsttest |
| `Font5x7.h` | Pixelfont für Textdarstellung |

## Hinweise zur Stromversorgung

392 adressierbare RGB-LEDs können bei hoher Helligkeit einen erheblichen Strom aufnehmen. Die LEDs sollten nicht direkt über den ESP32 versorgt werden. Verwende eine ausreichend dimensionierte externe 5-V-Stromversorgung, gemeinsame Masse, geeignete Leitungsquerschnitte und mehrere Einspeisepunkte. Eine Sicherung pro Einspeisezweig ist empfehlenswert.

## Build-Artefakte

Kompilierte Dateien wie `.bin`, `.elf` oder `.hex` werden durch die `.gitignore` ausgeschlossen. Die im ursprünglichen Projektarchiv enthaltene Binärdatei ist daher nicht Teil des Repositories.

## Lizenz

Für dieses Projekt ist derzeit keine Lizenzdatei hinterlegt. Ohne ausdrückliche Lizenz bleiben alle Rechte beim jeweiligen Urheber.
