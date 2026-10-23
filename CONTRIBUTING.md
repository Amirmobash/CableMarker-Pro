# Contributing

Thanks for improving CableMarker Pro.

## Before opening a pull request

1. Keep hardware-facing behavior explicit and documented.
2. Avoid long blocking delays in the main control loop.
3. Preserve the existing pin mapping unless the change is documented as a hardware revision.
4. Add or update host-side tests when protocol parsing changes.
5. Keep English documentation as the source of truth and update the matching German document when user-facing behavior changes.
6. Run:

```bash
g++ -std=c++17 -Ifirmware/CableMarkerPro tests/test_laser_protocol.cpp firmware/CableMarkerPro/LaserProtocol.cpp -o /tmp/cablemarker-test
/tmp/cablemarker-test
platformio run
```

## Style

Prefer small classes with one hardware responsibility. Comments should explain *why* a constraint exists rather than restating the code.
