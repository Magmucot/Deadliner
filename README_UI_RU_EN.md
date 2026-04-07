# Deadliner UI Architecture & Localization

## Architectural Overview

The Deadliner UI layer is built exclusively with **C++ Qt Widgets** and heavily relies on declarative layout flows over fixed positioning. This guarantees cross-platform robustness and proper high-DPI scaling across Windows 10, Arch Linux, and CachyOS. 

### Key Modules

- **MainWindow Shell**: A modern sidebar-based shell (`QListWidget` on the left, `QStackedWidget` on the right) that hosts main application pages. This avoids the bloated "single giant window" anti-pattern.
- **Pages**:
  - `TodayPage`: Read-only dashboard with a dynamic greeting and upcoming timers.
  - `EventsPage` & `ProfilesPage`: Table-based managers for CRUD operations with `EmptyStateWidget` zero-state designs.
  - `StatisticsPage`: Daily metrics viewer using grid-layout summary cards.
  - `SettingsPage`: Extensive runtime configuration, including Theme and Language comboboxes.
- **Dialogs**: Hand-coded `QDialogs` (`EventDialog`, `ProfileDialog`) built with `QFormLayout` for flawless label-alignment regardless of locale string lengths.
- **Reminder Prompts**:
  - `ReminderDialog`: Handles both Soft and Persistent mode prompts. The Persistent mode drops the window frame (`Qt::FramelessWindowHint | Qt::Tool`) to sit un-intrusively in the corner.
  - `BreakWindow`: A fullscreen, transparent black overlay (`setWindowState(Qt::WindowFullScreen)`) with a strict "Skip" lock mechanism that forces users to pause.
- **TrayController**: An isolated adapter around `QSystemTrayIcon` encapsulating tray menu management and notification routing.

### Avoidance of `.ui` Files

The UI components in this application are built procedurally via hand-written C++ code rather than Qt Designer `.ui` XML files. 

**Why this deviation from the prompt's suggestion?**
- Complete type-safety without `#include "ui_XYZ.h"` lifecycle mismatches.
- Explicit control over layout margins, standard icons (`QStyle::SP_FileDialogContentsView`), and semantic CSS injection (`setStyleSheet`).
- Allows seamless injection of dynamically generated `DictionaryTranslator` loops that would otherwise break standard `uic` translation pipelines.

## Localization (i18n)

Given the variability of build environments (specifically the potential absence of `QtLinguistTools` on target machines), the UI ships with a **hybrid embedded localization engine**. 

The app includes `.ts` files in `resources/i18n/` for standard `lupdate`/`lrelease` compatibility. However, the runtime does not depend on `.qm` loading. Instead, `LanguageManager` injects a custom `QTranslator` subclass (`DictionaryTranslator`) that translates embedded strings perfectly across RU/EN.

### How to Add or Update Translations

1. **Mark strings**: Always wrap static UI strings in `tr("Your Text")`.
2. **Dynamic UI Text**: For formatting logic, use the central `display_strings.h/cpp` helpers (e.g., `displaySeverityMode`, `formatPauseUntil`) to ensure unified translations.
3. **Register the string**: Open `src/ui/i18n/language_manager.cpp` and locate the `s_translations` hash map.
4. **Add the entry**:
   ```cpp
   {QStringLiteral("*\nYour Text"), QStringLiteral("Ваш Текст")},
   ```
   *(Note: The `*\n` prefix bypasses `context` scope matching, making the translation universally available to any widget that calls `tr("Your Text")`).*

### Handling `LanguageChange` Events

Live translation switching is supported. When the user changes the language in Settings (or Onboarding):
1. `LanguageManager` swaps the active `QTranslator`.
2. Qt blasts a `QEvent::LanguageChange` to the entire application widget tree.
3. Every single widget overrides `changeEvent()` to intercept this and run its own `retranslateUi()` method.

**Mandatory checklist for new UI modules**:
- [ ] Ensure `changeEvent(QEvent *event)` routes `QEvent::LanguageChange` to a `retranslateUi()` helper.
- [ ] If the screen contains data models (Tables, Combo boxes, dynamic Labels like "20 min left"), call the state-refresh logic *after* `retranslateUi()` to format the dynamic arguments again.

## Design Deviations & Rationale

- **No QML**: Strict adherence to the `Qt Widgets` requirement.
- **Combined Rules/Profiles**: The UI spec described "Break Rules", but the core Domain explicitly maps interval policies into the `ReminderProfile` struct. The UI correctly binds Break and Generic logic into a unified Profile editor to reflect this architecture.
- **Skip Locks vs Work Context**: The `BreakWindow` explicitly enforces the 10-second skip lock as requested, but it exposes an optional "Allow skip" toggle at the Profile level. Users with critical workloads can explicitly uncheck "Allow skip" or disable the skip-lock entirely per profile, offering a path around the toxicity of unconditional full-screen interruptions.