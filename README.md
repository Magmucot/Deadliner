# Deadliner

Deadliner is a Qt 6 desktop utility for break reminders and scheduled events on Windows 10, Arch Linux, and CachyOS. It follows the plan in `docs/reminder_app_plan.md` and ships a production-oriented MVP with tray integration, quiet hours, snooze limits, persistence, autostart, break mode, and basic statistics.

## Quick Start

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/deadliner
```

## Features

- Qt Widgets desktop app with system tray workflow
- Break reminders by interval
- Date/time reminders with one-time and recurring rules
- Reminder modes: soft, persistent, break screen
- Quiet hours with per-mode behavior
- Snooze with per-profile limits and escalation support
- SQLite persistence for profiles, events, logs, and stats
- QSettings persistence for UI and app-level settings
- Windows and Linux autostart adapters
- Basic daily statistics for completed, skipped, snoozed, and break outcomes

## Project Layout

- `src/app`: application wiring and runtime flow
- `src/domain`: models and scheduling/policy rules
- `src/application`: scheduler and statistics services
- `src/infrastructure`: SQLite, QSettings, logging, platform adapters
- `src/ui`: main window, dialogs, break screen, tray controller
- `tests`: unit and repository smoke tests
- `packaging`: Linux desktop entry and PKGBUILD scaffold

## Build Requirements

### Windows 10

- Visual Studio 2022 with MSVC
- CMake 3.24+
- Qt 6.5+ with `Core`, `Gui`, `Widgets`, `Sql`, `Test`

### Arch Linux / CachyOS

- `cmake`
- `ninja`
- `qt6-base`
- `sqlite`

Example:

```bash
sudo pacman -S cmake ninja qt6-base sqlite
```

## Build

### Windows 10

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

To prepare a portable runtime:

```powershell
windeployqt build\deadliner.exe
```

For an installer, use the deployed output with Inno Setup or NSIS. This repository prepares the application for that step but does not bundle the external installer toolchain.

### Arch Linux / CachyOS

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

## Packaging

### Windows

1. Build release binary.
2. Run `windeployqt` on `deadliner.exe`.
3. Package the deployed folder with Inno Setup or NSIS.

### Arch Linux / CachyOS

1. Build with CMake.
2. Install into a staging directory or package root.
3. Use `packaging/linux/PKGBUILD` as a starting point for a native Arch package.
4. For AppImage, build the project, stage the install tree, then run external tools like `linuxdeploy` and `appimagetool`.

## Running

- Start the app from the built binary: `deadliner`
- The app stays resident in the tray when supported by the environment
- On Linux desktops without working tray support, the main window remains the primary control surface

## Tests

The test suite covers:

- recurrence calculation
- quiet hours including overnight ranges
- snooze limits
- stats aggregation
- repository open/seed/save/load behavior

Run:

```bash
ctest --test-dir build --output-on-failure
```

## System Limitations

- Linux tray behavior depends on the desktop environment and status notifier support.
- Wayland may limit how reliably the break window stays above other applications.
- The app does not use invasive hooks and cannot prove the user physically left the computer.
- Native installer and AppImage generation require external packaging tools not embedded in this repository.

## Storage

- SQLite database: app data directory under `QStandardPaths::AppDataLocation`
- Log file: `logs/deadliner.log` under the same app data directory
- QSettings: platform-native settings backend

## Quick Start Again

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/deadliner
```
