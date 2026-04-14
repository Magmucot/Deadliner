# Deadliner

Cross-platform desktop utility for break reminders and scheduled events, built with C++ and Qt 6.

[README на русском](README.ru.md)

## Features

- **System tray integration** — lives in the tray with a context menu for quick actions
- **Break reminders by interval** — periodic break notifications with configurable intervals and durations
- **Date/time reminders** — one-time and recurring events with custom recurrence rules (daily, weekdays, weekly, monthly, every N days)
- **Three reminder modes:**
  - **Soft** — passive notification that auto-dismisses
  - **Persistent** — frameless overlay that stays visible until acted upon
  - **Break** — fullscreen overlay with countdown timer, skip-lock, and post-break confirmation
- **Quiet hours** — per-mode behavior (suppress, defer, downgrade, allow) including overnight ranges
- **Snooze with limits** — per-profile snooze count caps and escalation support
- **Profiles** — reusable reminder profiles (break, generic, mixed) with built-in defaults and custom user profiles
- **Bilingual UI (RU/EN)** — live language switching without restart, system locale detection, onboarding language selection
- **Theming** — light, dark, and system themes via Fusion style + QPalette
- **Onboarding wizard** — first-run setup with language choice and recommended break rule preset
- **Overdue events tracking** — dedicated page for missed/past-due reminders
- **Today dashboard** — greeting, next-break countdown, active profiles, and upcoming events at a glance
- **Daily statistics** — completed, skipped, snoozed, and break outcome counts
- **SQLite persistence** — profiles, events, logs, and stats survive app restarts
- **QSettings persistence** — UI geometry, theme, language, tray behavior, and other app-level settings
- **Autostart** — platform adapters for Windows (Startup folder) and Linux (`~/.config/autostart`)
- **CI/CD** — GitHub Actions workflow producing NSIS installer + portable ZIP (Windows) and Arch package + tarball (Linux)

## Quick Start

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/deadliner
```

## Build Requirements

### Windows 10

- Visual Studio 2022 with MSVC
- CMake 3.24+
- Qt 6.5+ with `Core`, `Gui`, `Widgets`, `Sql`, `Test`, `LinguistTools`

### Arch Linux / CachyOS

```bash
sudo pacman -S cmake ninja qt6-base qt6-tools sqlite
```

## Build

### Windows

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Deploy for portable use:

```powershell
windeployqt build\deadliner.exe
```

For an installer, use the deployed output with the NSIS script at `packaging/windows/installer.nsi`.

### Linux

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Run locally:

```bash
./build/deadliner
```

Install to a staging prefix:

```bash
cmake --install build --prefix "$PWD/stage"
```

## Project Layout

```
src/
  app/            Application wiring and runtime flow
  domain/         Models and scheduling/policy rules
  application/    Scheduler and statistics services
  infrastructure/ SQLite, QSettings, logging, platform adapters
  ui/
    common/       Navigation, display string helpers
    i18n/         Language manager and embedded dictionary translator
    settings/     Settings page and theme manager
    overdue/      Overdue events page
    today/        Today dashboard page
    events/       Events list page
    profiles/     Profiles list page
    statistics/   Statistics page
    onboarding/   First-run onboarding wizard
    widgets/      Reusable empty-state widget
    main_window   Sidebar + stacked page shell
    tray_controller
    reminder_dialog
    break_window
    event_dialog
    profile_dialog
tests/            Unit and repository smoke tests
packaging/
  linux/          .desktop entry, PKGBUILD
  windows/        NSIS installer script
resources/
  icons/          App icons (PNG, ICO)
  i18n/           .ts translation files (EN, RU)
docs/             Product plan, UI spec, UI review
```

## Architecture

The application follows a practical modular monolith with clear layer separation:

| Layer | Responsibility |
|---|---|
| **UI** | Qt Widgets: sidebar navigation, pages, dialogs, break screen, tray, onboarding |
| **Application** | `ReminderScheduler` (central QTimer + priority queue), `StatisticsService` |
| **Domain** | Models (`ReminderProfile`, `ReminderEvent`, `QuietHoursPolicy`, etc.), scheduling logic, quiet hours rules, snooze policy |
| **Infrastructure** | `Repository` (SQLite + migrations), `SettingsStore` (QSettings), `AutostartManager` (platform adapters), logging |

**Runtime flow:**

1. App starts → loads settings, profiles, events, stats from SQLite/QSettings
2. Initializes tray, scheduler, autostart, language, theme
3. Rebuilds in-memory occurrence queue from persisted state
4. Central `QTimer` fires at the next due occurrence
5. Quiet hours check → profile policy applied → reminder presented (soft / persistent / break)
6. User action (complete / snooze / skip) → stats updated → schedule rebuilt

## Tests

The test suite covers:

- Recurrence calculation
- Quiet hours including overnight ranges
- Snooze limits
- Stats aggregation
- Repository open/seed/save/load behavior
- Scheduler rebuild and resync
- App controller initialization

```bash
ctest --test-dir build --output-on-failure
```

## Packaging

### Windows

The CI workflow builds an NSIS installer and a portable ZIP. Locally:

1. Build release binary.
2. Run `windeployqt` on `deadliner.exe`.
3. Run `makensis` with `packaging/windows/installer.nsi`.

### Arch Linux / CachyOS

1. Build with CMake.
2. Use `packaging/linux/PKGBUILD` with `makepkg` for a native Arch package.
3. Or install into a staging directory: `cmake --install build --prefix "$PWD/stage"`.

## Storage

- **SQLite database:** `QStandardPaths::AppDataLocation` (profiles, events, logs, stats)
- **Log file:** `logs/deadliner.log` under the same app data directory
- **QSettings:** platform-native backend (UI settings, theme, language, tray behavior)

## System Limitations

- Linux tray behavior depends on the desktop environment and StatusNotifierItem/AppIndicator support.
- Wayland may limit how reliably the break window stays above other applications.
- The app does not use invasive hooks and cannot prove the user physically left the computer. Break completion is tracked by the app's scenario (minimum duration + explicit confirmation), not by physical presence.
- Native installer and AppImage generation require external packaging tools not embedded in this repository.

## License

[MIT](LICENSE) — Copyright (c) 2026 Magmucot
