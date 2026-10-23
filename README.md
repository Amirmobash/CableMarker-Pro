# CableMarker Pro

**ESP32 cable and pipe measuring + marking controller for long-run workshop workflows.**

CableMarker Pro combines an ESP32, a UART laser distance sensor, a servo-driven marking mechanism, an external trigger input, status indicators, and a local web interface. The project is intended for measuring and physically marking cable or pipe runs; with a suitably rated rangefinder and appropriate target conditions, the workflow can be used on runs **up to about 100 m**.

> **Range note:** 100 m is a system use case, not a guarantee from the firmware. Real measuring range and accuracy depend on the installed laser module, target reflectivity, alignment, ambient light, mechanics, and calibration.

**Deutsch:** [README.de.md](README.de.md)

## Why this rebuild exists

The original repository had one large firmware file and a partial README. Version 2.0.0 keeps the original ESP32 pin assignment and laser command bytes, but separates responsibilities so the project is easier to test, maintain, calibrate, and extend.

## What it does

- Reads a UART laser rangefinder and validates measurement frames.
- Applies a configurable mechanical calibration offset.
- Drives a servo marker without blocking the web interface for the whole motion cycle.
- Accepts a debounced external trigger for marking.
- Exposes a local EN/DE web dashboard over an ESP32 access point.
- Reports laser health, mark state, distance, calibration, uptime, and completed mark cycles.
- Provides a versioned JSON API and keeps legacy endpoints for migration.
- Includes host-side parser tests and PlatformIO CI.
- Includes a bilingual GitHub Pages site prepared for search-engine crawling.

## Hardware

| Function | ESP32 pin |
|---|---:|
| Laser RX | GPIO 16 |
| Laser TX | GPIO 17 |
| Laser enable | GPIO 5 |
| Marker servo | GPIO 18 |
| Buzzer | GPIO 27 |
| Status LED | GPIO 25 |
| Built-in health LED | GPIO 2 |
| Trigger output / ready line | GPIO 23 |
| Trigger input | GPIO 19 |

The firmware preserves the original wiring to make migration straightforward. See [docs/HARDWARE.md](docs/HARDWARE.md) before powering the system.

## Software architecture

```text
CableMarkerPro.ino
├── LaserRangefinder     UART scheduling, restart, health tracking
│   └── LaserProtocol    pure frame validation / BCD decoding
├── MarkerActuator       non-blocking servo + buzzer state machine
├── DebouncedInput       external trigger filtering
├── WebUi                embedded EN/DE local control panel
└── AppConfig            pins, timing, calibration, AP credentials
```

The pure `LaserProtocol` component can be compiled and tested on a normal desktop compiler without Arduino hardware.

## Quick start — Arduino IDE

1. Install **ESP32 by Espressif Systems** in Arduino IDE.
2. Install the **ESP32Servo** library.
3. Open `firmware/CableMarkerPro/CableMarkerPro.ino`.
4. Select **ESP32 Dev Module**.
5. Confirm the wiring in [docs/HARDWARE.md](docs/HARDWARE.md).
6. Upload the sketch.
7. Connect to:

```text
Wi-Fi:   CableMarker-Pro
Password: CableMarker100
```

8. Open the IP printed in Serial Monitor (normally `192.168.4.1`).

Change the default Wi-Fi password in `AppConfig.h` before using the unit in a shared workshop.

## Quick start — PlatformIO

```bash
git clone https://github.com/Amirmobash/CableMarker-Pro.git
cd CableMarker-Pro
platformio run
platformio run --target upload
```

## API

| Method | Endpoint | Purpose |
|---|---|---|
| GET | `/api/status` | Current distance and system state |
| POST | `/api/mark` | Request one mark cycle |
| POST | `/api/laser/restart` | Restart the laser interface |
| GET | `/healthz` | Simple device health endpoint |

Legacy `/d`, `/m`, and `/r` routes remain available for older clients, but new integrations should use `/api/*`.

Example status response:

```json
{
  "version": "2.0.0",
  "distance_mm": 12467,
  "raw_distance_mm": 12500,
  "calibration_offset_mm": -33,
  "laser_healthy": true,
  "laser_restarting": false,
  "marker_busy": false,
  "completed_marks": 42,
  "uptime_ms": 187234
}
```

## Calibration

The legacy project used a `-33 mm` mechanical offset. It is preserved in `AppConfig.h` so existing hardware does not silently change behavior.

Do not copy that value to a different mechanical build without measuring it. Follow [docs/CALIBRATION.md](docs/CALIBRATION.md).

## 100 m workflows

CableMarker Pro itself does not define the optical limit. For long cable or pipe runs, choose a UART rangefinder whose **rated range, target requirements, laser class, and accuracy** match the job. The firmware keeps measurements in millimetres and uses 32-bit values, so 100,000 mm is comfortably within the software range.

See [docs/LONG_RANGE_WORKFLOW.md](docs/LONG_RANGE_WORKFLOW.md).

## Documentation

### English

- [Hardware and wiring](docs/HARDWARE.md)
- [Calibration](docs/CALIBRATION.md)
- [Operating guide](docs/OPERATING_GUIDE.md)
- [Long-range workflow](docs/LONG_RANGE_WORKFLOW.md)
- [Architecture](docs/ARCHITECTURE.md)
- [HTTP API](docs/API.md)
- [Migration from legacy firmware](docs/MIGRATION.md)
- [Recommended GitHub setup](docs/GITHUB_SETUP.md)
- [Troubleshooting](docs/TROUBLESHOOTING.md)
- [GitHub Pages & search visibility](docs/SEARCH_VISIBILITY.md)

### Deutsch

- [Hardware und Verdrahtung](docs/HARDWARE.de.md)
- [Kalibrierung](docs/CALIBRATION.de.md)
- [Bedienungsanleitung](docs/OPERATING_GUIDE.de.md)
- [Langstreckenbetrieb](docs/LONG_RANGE_WORKFLOW.de.md)
- [Architektur](docs/ARCHITECTURE.de.md)
- [Fehlersuche](docs/TROUBLESHOOTING.de.md)
- [GitHub Pages & Auffindbarkeit](docs/SEARCH_VISIBILITY.de.md)

## Search visibility / Google

The repository contains a static bilingual site under `site/` plus a GitHub Actions Pages deployment workflow. It includes:

- descriptive English and German URLs,
- canonical URLs,
- `hreflang` alternates,
- `robots.txt`,
- `sitemap.xml`,
- Open Graph metadata,
- schema.org `SoftwareSourceCode` / `TechArticle` structured data,
- original technical content rather than keyword-stuffed landing pages.

No repository can guarantee a Google ranking. After enabling GitHub Pages, add the final Pages URL to Google Search Console and submit the sitemap. See [docs/SEARCH_VISIBILITY.md](docs/SEARCH_VISIBILITY.md).

## Safety

- Follow the rangefinder manufacturer's laser safety instructions and laser class requirements.
- Keep hands clear of the servo-driven marking mechanism.
- Test the marker at low speed and without production material after changing servo angles.
- Use a power supply sized for ESP32, rangefinder, and servo transient current; do not power a servo from an undersized regulator.
- Verify the calibration before marking valuable material.

## Project status

This rebuild is tagged conceptually as **v2.0.0**. The laser command bytes and 13-byte frame layout are inherited from the existing project; the exact rangefinder model was not documented in the source repository. If you identify the model, add its datasheet and electrical-level requirements to the hardware documentation.

## License

The original repository did not specify an open-source license, so this rebuild intentionally does not select one for the owner. See [LICENSE-NOTICE.md](LICENSE-NOTICE.md).
