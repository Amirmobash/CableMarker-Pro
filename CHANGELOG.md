# Changelog

## 2.0.0 — 2026-10-23

- Rebuilt the firmware into focused components instead of one monolithic source file.
- Added non-blocking servo marking and buzzer timing.
- Added debounced external trigger handling.
- Added stricter laser frame validation, including BCD and checksum checks.
- Added stale-data health detection and asynchronous laser restart handling.
- Added a password-protected standalone ESP32 access point.
- Added a bilingual EN/DE device interface and versioned JSON API.
- Preserved the legacy `/d`, `/m`, and `/r` endpoints for migration.
- Added PlatformIO configuration, host-side protocol tests, and GitHub Actions CI.
- Added complete English and German documentation.
- Added an SEO-ready bilingual GitHub Pages site, sitemap, robots.txt, canonical URLs, hreflang, and structured metadata.

## 1.x — legacy repository

The original repository contained a single firmware source file and a partial README. Version 2.0.0 is a ground-up maintainability and documentation rebuild while retaining the original pin assignment and laser command bytes.
