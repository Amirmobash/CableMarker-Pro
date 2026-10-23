# CableMarker Pro

**ESP32-Mess- und Markiersystem für Kabel und Rohre bei langen Arbeitsstrecken.**

CableMarker Pro verbindet einen ESP32 mit einem UART-Laser-Entfernungssensor, einem servoangetriebenen Markiermechanismus, einem externen Triggereingang, Statusanzeigen und einer lokalen Weboberfläche. Das System ist für das Messen und physische Markieren von Kabel- oder Rohrstrecken gedacht. Mit einem passend ausgelegten Entfernungssensor und geeigneten Zielbedingungen kann der Arbeitsablauf **bis etwa 100 m** eingesetzt werden.

> **Hinweis zur Reichweite:** 100 m ist ein Anwendungsziel des Gesamtsystems, keine Garantie der Firmware. Die reale Reichweite und Genauigkeit hängen vom eingesetzten Lasermodul, der Oberfläche, Ausrichtung, Umgebungshelligkeit, Mechanik und Kalibrierung ab.

**English:** [README.md](README.md)

## Warum diese Version neu aufgebaut wurde

Das ursprüngliche Repository bestand im Wesentlichen aus einer großen Firmware-Datei und einer unvollständigen README. Version 2.0.0 behält die vorhandene ESP32-Pinbelegung und die Laser-Kommandobytes bei, trennt aber Sensor, Markiermechanik, Trigger, Weboberfläche und Protokolldecoder sauber voneinander.

## Funktionen

- UART-Laserentfernung messen und Telegramme auf Header, BCD-Daten und Prüfsumme prüfen.
- Mechanischen Kalibrier-Offset berücksichtigen.
- Servo-Markierzyklus als nicht blockierende Zustandsmaschine ausführen.
- Externen Trigger entprellt verarbeiten.
- Lokales EN/DE-Webinterface über einen ESP32-Access-Point bereitstellen.
- Laserstatus, Markierstatus, Distanz, Kalibrierung, Laufzeit und Markieranzahl anzeigen.
- Versionierte JSON-API bereitstellen und alte Endpunkte für bestehende Clients erhalten.
- Parser-Tests und PlatformIO-CI mitliefern.
- Zweisprachige GitHub-Pages-Seite für Suchmaschinen bereitstellen.

## Hardware

| Funktion | ESP32-Pin |
|---|---:|
| Laser RX | GPIO 16 |
| Laser TX | GPIO 17 |
| Laser Enable | GPIO 5 |
| Markierservo | GPIO 18 |
| Summer | GPIO 27 |
| Status-LED | GPIO 25 |
| Onboard-LED / Laserstatus | GPIO 2 |
| Trigger-Ausgang | GPIO 23 |
| Trigger-Eingang | GPIO 19 |

Die Pinbelegung entspricht dem bisherigen Projekt. Vor dem Einschalten bitte [docs/HARDWARE.de.md](docs/HARDWARE.de.md) lesen.

## Schnellstart mit Arduino IDE

1. **ESP32 by Espressif Systems** installieren.
2. Die Bibliothek **ESP32Servo** installieren.
3. `firmware/CableMarkerPro/CableMarkerPro.ino` öffnen.
4. **ESP32 Dev Module** auswählen.
5. Verdrahtung prüfen.
6. Firmware hochladen.
7. Mit folgendem WLAN verbinden:

```text
WLAN:     CableMarker-Pro
Passwort: CableMarker100
```

8. Die im seriellen Monitor ausgegebene IP öffnen, normalerweise `192.168.4.1`.

Das Standardpasswort sollte in `AppConfig.h` geändert werden, bevor das Gerät in einer gemeinsam genutzten Werkstatt eingesetzt wird.

## API

| Methode | Endpunkt | Funktion |
|---|---|---|
| GET | `/api/status` | Distanz und Systemstatus |
| POST | `/api/mark` | Einen Markierzyklus anfordern |
| POST | `/api/laser/restart` | Laser-Schnittstelle neu starten |
| GET | `/healthz` | Einfacher Health-Check |

Die alten Routen `/d`, `/m` und `/r` bleiben für vorhandene Clients erhalten.

## Kalibrierung

Im ursprünglichen Projekt war ein mechanischer Offset von `-33 mm` hinterlegt. Dieser Wert wurde bewusst übernommen, damit sich das Verhalten bestehender Hardware nicht unbemerkt ändert. Für eine neue Mechanik muss der Offset neu gemessen werden. Siehe [docs/CALIBRATION.de.md](docs/CALIBRATION.de.md).

## Arbeiten bis etwa 100 m

Die optische Maximalreichweite wird nicht vom ESP32-Code bestimmt. Für lange Kabel- oder Rohrstrecken muss ein UART-Entfernungssensor verwendet werden, dessen spezifizierte Reichweite, Zielbedingungen, Laserklasse und Genauigkeit zur Anwendung passen. Intern arbeitet die Firmware in Millimetern; 100 m entsprechen 100.000 mm und liegen problemlos im verwendeten Wertebereich.

Weitere Hinweise: [docs/LONG_RANGE_WORKFLOW.de.md](docs/LONG_RANGE_WORKFLOW.de.md).

## Weitere Dokumentation

- [HTTP-API](docs/API.de.md)
- [Migration](docs/MIGRATION.de.md)
- [Empfohlene GitHub-Einstellungen](docs/GITHUB_SETUP.de.md)
- [Architektur](docs/ARCHITECTURE.de.md)
- [Fehlersuche](docs/TROUBLESHOOTING.de.md)

## Suchmaschinen / Google

Unter `site/` liegt eine zweisprachige statische Website für GitHub Pages. Enthalten sind englische und deutsche URLs, canonical Links, hreflang, `robots.txt`, `sitemap.xml`, Open-Graph-Metadaten und strukturierte Daten. Eine bestimmte Google-Position lässt sich nicht garantieren. Nach Aktivierung von GitHub Pages sollte die Pages-URL in Google Search Console bestätigt und die Sitemap eingereicht werden.

Details: [docs/SEARCH_VISIBILITY.de.md](docs/SEARCH_VISIBILITY.de.md).

## Sicherheit

- Laserklasse und Sicherheitshinweise des Sensorherstellers beachten.
- Hände vom bewegten Servo-Markiermechanismus fernhalten.
- Nach Änderungen an den Servowinkeln zuerst ohne Produktionsmaterial testen.
- Servo und Sensor mit ausreichend dimensionierter Versorgung betreiben.
- Vor wertvollem Material immer die Kalibrierung kontrollieren.

## Lizenz

Im ursprünglichen Repository war keine Open-Source-Lizenz festgelegt. Deshalb wird hier nicht automatisch eine Lizenz für den Eigentümer ausgewählt. Siehe [LICENSE-NOTICE.md](LICENSE-NOTICE.md).
