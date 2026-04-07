---
16. Deviation Log
- 2026-04-05: Поддержка двух языков интерфейса RU/EN объявлена обязательной для текущего UI scope и больше не считается post-MVP улучшением. Причина: `docs/ui_spec.md` является главным источником UI/UX-требований и явно требует shipped bilingual UX с live language switch.
- 2026-04-05: Верхнеуровневая UI-навигация нормализована к `Today`, `Events`, `Profiles`, `Statistics`, `Settings`. Экран `Today` трактуется как product dashboard/summary, а сущность `Break Rules` в UI объединяется с профилями напоминаний на экране `Profiles`, потому что текущая доменная модель хранит break-политику внутри `ReminderProfile`, а не в отдельной таблице правил.
- 2026-04-05: Диалог события не получает отдельные per-event поля `mode`/`snooze policy`. Вместо этого UI явно показывает, что режим напоминания и snooze-политика наследуются от выбранного профиля. Причина: текущая доменная модель и persistence фиксируют эти параметры на уровне `ReminderProfile`, и добавление фиктивных UI-overrides создало бы ложное обещание неподдерживаемого поведения.
- 2026-04-05: Стандартное onboarding-правило `20-20-20` в MVP интерпретируется как closest supported preset `20 minutes work / 1 minute eye rest`, потому что текущая доменная модель break duration хранит минуты и не умеет секундную длительность без расширения scheduler/storage schema.
- 2026-04-05: Спецификация strict break flow скорректирована в пользу явного подтверждения возврата после завершения таймера. Причина: продуктовый план уже фиксирует, что break completion должен считаться по сценарию приложения только после минимальной длительности и подтверждения `Я вернулся`, а авто-закрытие окна по окончании таймера давало бы ложный completed state.
- 2026-04-05: Исправлено фактическое имя файла плана с `docs/reminder_app_plan.md,` на `docs/reminder_app_plan.md`, потому что исходное имя с запятой ломало прямое использование документа как основного источника требований и автоматизированное чтение файла.
- 2026-04-05: Для production-ready реализации packaging-пайплайн ограничен подготовкой `CPack`-конфигурации, `.desktop`-ресурсов и `PKGBUILD`-шаблона/README-инструкций. Полноценная сборка AppImage и Windows installer не автоматизируется внутри исходников без внешних packaging toolchains (`linuxdeploy`, `appimagetool`, Inno Setup/NSIS), что соответствует разделам плана, где polished packaging вынесен в отдельный milestone после стабилизации core.
- 2026-04-05: Схема `reminder_events` расширена runtime-полями `pending_snooze_count`, `pending_original_trigger_at` и `pending_mode`, потому что исходная схема не позволяла надежно восстановить активное snoozed-напоминание и лимиты snooze после перезапуска приложения.

---
1. Цель и границы проекта
docs/reminder_app_plan.md
План кроссплатформенного desktop-приложения напоминаний и перерывов на Qt
1. Цель и границы проекта
Цель
Сделать кроссплатформенное desktop-приложение для Windows 10, CachyOS и Arch Linux, которое:
- напоминает о перерывах за ПК;
- напоминает о произвольных событиях;
- поддерживает несколько уровней настойчивости напоминаний;
- не только показывает уведомление, но и помогает реально провести перерыв;
- работает фоново через системный трей;
- сохраняет настройки и состояние между запусками.
Границы проекта
В проект входят:
- локальное desktop-приложение;
- локальное хранение данных;
- интеграция с системным треем, уведомлениями и автозапуском;
- экран режима перерыва;
- статистика базового уровня.
В проект не входят:
- облачная синхронизация;
- мобильные клиенты;
- корпоративное централизованное управление;
- глубокий мониторинг активности пользователя на уровне драйверов;
- блокировка системных функций ОС или опасные анти-пользовательские механизмы;
- OCR, webcam, eye-tracking, кейлоггинг, принудительная блокировка процессов.
Продуктовая позиция
Приложение должно быть полезным в двух сценариях:
- персональный wellness/ergonomics assistant для регулярных перерывов;
- универсальный локальный reminder manager для событий и рутин.
---
2. Допущения
Зафиксированные допущения
1. Основной UI делается на Qt Widgets, а не на QML.
Причина: быстрее собрать реалистичный desktop MVP для solo-разработки, проще системный трей, модальные окна, формы настроек и меньше поверхностной сложности.
2. Приложение работает локально и офлайн.
Причина: это снижает объем разработки, риски приватности и сложность сопровождения.
3. “Перерыв действительно выполнен” не определяется через слежение за пользователем.
Вместо этого используется безопасная и реалистичная модель подтверждения:
- пользователь должен выдержать минимальную длительность break-screen;
- досрочное закрытие считается пропуском;
- для части профилей можно требовать простое подтверждающее действие после таймера: например, Я вернулся;
- опционально можно считать перерыв выполненным, если пользователь не нажимал Skip и экран перерыва оставался активен заданное минимальное время.
4. Под Linux поддерживается современный desktop stack с StatusNotifierItem/AppIndicator там, где возможно, и fallback на QSystemTrayIcon.
Нужно явно учитывать, что поведение трея и уведомлений на Linux зависит от DE/WM.
5. Приложение не пытается быть “неубиваемым”.
Если пользователь завершает процесс или отключает автозапуск, приложение это не обходит.
6. Для MVP статистика агрегированная и локальная.
Не нужен event-sourcing и сложная аналитика.
7. Часы тишины трактуются как окно, в котором:
- мягкие уведомления подавляются или копятся;
- жесткие режимы могут переводиться в более мягкий или отложенный режим в зависимости от профиля;
- поведение должно настраиваться, но по умолчанию ночью не показывать агрессивные перерывы.
Неоднозначности, закрытые рабочим решением
- Нужно ли отслеживать user idle state.
  Решение: не включать в MVP. Это полезно, но кроссплатформенно усложняет решение. Можно добавить как phase 2.
- Нужно ли блокировать весь экран.
  Решение: только полноэкранное top-level окно приложения без опасных системных хуков и без отключения системных сочетаний клавиш.
- Нужно ли поддерживать несколько мониторов.
  Решение: да, но в MVP достаточно показывать break-screen на основном мониторе. Расширение до всех мониторов можно сделать отдельно.
---
3. Выбор технологического стека
Сравнение: C++/Qt 6 vs Python/PySide6
Критерий	C++ + Qt 6
Скорость разработки	Средняя
Простота поддержки	Высокая при дисциплине, но выше порог
Кроссплатформенность	Отличная
Сложность упаковки	Средняя, но предсказуемая
Доступ к системной интеграции	Лучший, прямой и предсказуемый
Размер и удобство поставки	Лучше для нативной поставки
Разбор по критериям
Скорость разработки
Python/PySide6 выигрывает на старте:
- быстрее собирать бизнес-логику;
- ниже стоимость прототипирования;
- меньше ceremony.
Но для этого продукта UI и логика несложные, а основная сложность лежит в системной интеграции, таймингах, стабильной фоновой работе и поставке. Там преимущество Python уменьшается.
Простота поддержки
Для desktop background app с долгим жизненным циклом C++/Qt 6 устойчивее:
- меньше скрытых проблем упаковки;
- меньше зависимости от Python runtime и frozen bundle tooling;
- проще предсказуемо диагностировать интеграцию с ОС;
- лучше контроль памяти, процессов, startup behavior.
PySide6 хорош для внутренних инструментов и быстрых итераций, но для продукта, который должен тихо жить в фоне, стартовать с системой и ставиться как обычное desktop-приложение, поддержка поставки со временем становится заметной болью.
Кроссплатформенность
Оба стека подходят. Qt abstraction закрывает:
- окна;
- таймеры;
- системный трей;
- настройки;
- SQL;
- уведомления частично.
Но при реальных edge-cases OS integration C++/Qt 6 дает больше свободы для изоляции platform adapters.
Сложность упаковки
C++/Qt 6:
- Windows: windeployqt, NSIS/Inno Setup;
- Linux: AppImage / tarball / distro-specific package;
- предсказуемый runtime footprint.
Python/PySide6:
- PyInstaller/Nuitka/briefcase-подобные варианты;
- больше проблем с plugins Qt, notifications backends, antivirus false positives на Windows, размером дистрибутива и стартовым временем;
- Linux-поставка для Arch/CachyOS технически решаема, но менее элегантна.
Доступ к системной интеграции
Для автозапуска, deep linking, tray quirks, native notifications adapters и возможного доступа к idle APIs C++/Qt 6 предпочтительнее.
Размер и удобство поставки
Для фоновго десктопного приложения это важный фактор.
C++/Qt 6 дает более “нативное” ощущение установки и поставки.
Выбранный основной стек
Основной стек: C++ + Qt 6 (Qt Widgets)
Почему выбран именно он
1. Лучше подходит для долгоживущего desktop background app.
2. Проще сделать аккуратную и предсказуемую поставку под Windows и Arch-based Linux.
3. Системная интеграция и кроссплатформенные адаптеры будут проще контролироваться.
4. Производительность здесь не критична, но надежность фоновой работы и старт с системой критичны.
5. Для solo-разработки стоимость старта выше, чем у Python, но стоимость сопровождения продукта ниже.
Когда имел бы смысл Python/PySide6
PySide6 стоило бы выбрать, если бы:
- цель была только MVP/прототип за минимальное время;
- продукт был внутренним инструментом;
- не было жестких требований к удобной поставке и polished desktop UX.
---
4. MVP
Что входит в MVP
Приоритет: P0
1. Фоновое приложение с system tray.
2. Интервальные напоминания о перерыве.
3. Напоминания по дате/времени.
4. Одноразовые и повторяющиеся события.
5. Профили напоминаний:
- минимум 2 встроенных профиля;
- 1 пользовательский профиль.
6. Режимы напоминаний:
- мягкий;
- настойчивый;
- режим перерыва.
7. Snooze.
8. Лимит snooze для настойчивого и break режима.
9. Quiet hours.
10. Сохранение настроек и событий между запусками.
11. Автозапуск с системой.
12. Базовая статистика:
- выполнено;
- пропущено;
- отложено.
13. Базовый break-screen с таймером и подтверждением возврата.
Что не входит в MVP
Приоритет: P1+
- idle-detection;
- sync/export/import;
- multi-monitor immersive mode;
- rich analytics;
- templates library;
- localization beyond one language;
- accessibility tuning beyond basic Qt support;
- advanced Linux packaging matrix.
Критерий MVP
Пользователь устанавливает приложение, включает автозапуск, видит трей, настраивает профиль и получает рабочие напоминания о перерывах и событиях без ручного запуска после каждого входа в систему.
---
5. Полный функционал
P0
- интервальные напоминания о перерыве;
- напоминания по времени/дате;
- одноразовые события;
- повторяющиеся события;
- профили напоминаний;
- системный трей;
- автозапуск;
- настройки и локальное хранение;
- quiet hours;
- snooze;
- лимиты snooze;
- статистика;
- break-screen;
- ручной старт перерыва из трея.
P1
- более гибкие recurrence rules;
- пауза напоминаний на 30/60/120 минут;
- “skip once”;
- шаблоны профилей: work focus, pomodoro-like, health;
- журнал последних событий;
- экспорт/импорт настроек JSON;
- выбор поведения в quiet hours для разных типов напоминаний;
- настройка звуков;
- multi-monitor behavior options.
P2
- optional idle-aware logic;
- suppression during fullscreen apps/games;
- smart defer if meeting detected через календарь или вручную;
- richer statistics with trends;
- optional “micro-break exercise cards”.
---
6. Архитектура
Архитектурный стиль
Практичный модульный monolith:
- одно desktop-приложение;
- четкое разделение domain / application / infrastructure / ui;
- platform-specific adapters изолированы за интерфейсами.
Это лучше, чем избыточная clean-architecture в чистом виде, потому что проект одиночный и desktop-centric.
Слои
UI Layer
Отвечает за:
- tray menu;
- окна настроек;
- окно редактирования события;
- статистику;
- break-screen;
- модальные persistent reminder dialogs.
Не содержит бизнес-логики планирования.
Application Layer
Use cases:
- создать/изменить/удалить событие;
- активировать профиль;
- вычислить следующее срабатывание;
- выполнить reminder escalation;
- обработать snooze/skip/complete;
- применить quiet hours;
- записать статистику.
Domain Layer
Содержит модели и правила:
- ReminderProfile
- ReminderEvent
- BreakPolicy
- QuietHoursPolicy
- SnoozePolicy
- ReminderOccurrence
- BreakSession
- StatsSnapshot
Infrastructure Layer
- storage (SQLite, QSettings);
- scheduler/timer services;
- notifications adapter;
- tray integration;
- autostart integration;
- logging;
- time provider;
- platform adapters.
Основной runtime-flow
1. Приложение стартует.
2. Загружает настройки, профили, события, статистику-агрегаты.
3. Инициализирует tray, scheduler, adapters.
4. Вычисляет ближайшие reminder occurrences.
5. Держит один central scheduling loop.
6. При наступлении события:
- проверяет quiet hours;
- применяет политику профиля;
- показывает reminder нужной жесткости;
- ждет действия пользователя;
- обновляет статистику;
- пересчитывает следующее срабатывание.
Почему один central scheduler
Нужен единый источник истины для таймингов:
- меньше гонок;
- проще отладка;
- проще тестировать;
- легче сохранять/восстанавливать состояние после рестарта.
Рекомендуемая схема планировщика
- in-memory priority queue ближайших occurrence;
- один QTimer, заведенный на ближайшее событие;
- при изменении данных очередь перестраивается;
- при старте приложения пересчет делается из persisted config.
Такой подход лучше, чем отдельный таймер на каждое событие.
Псевдокод
onAppStart():
  state = repository.loadAll()
  scheduler.rebuild(state.events, state.profiles, now)
  scheduler.armNext()
onTimerFired():
  due = scheduler.popDue(now)
  for occurrence in due:
    if quietHoursPolicy.suppress(occurrence, now):
      rescheduled = occurrence.deferAccordingToPolicy()
      repository.save(rescheduled)
      continue
    action = reminderCoordinator.present(occurrence)
    switch action:
      case Completed:
        stats.recordCompleted(occurrence)
      case Snoozed:
        if snoozePolicy.canSnooze(occurrence):
          occurrence = occurrence.snooze()
          scheduler.requeue(occurrence)
          stats.recordSnoozed(occurrence)
        else:
          reminderCoordinator.escalateOrForceBreak(occurrence)
      case Skipped:
        stats.recordSkipped(occurrence)
  scheduler.armNext()
---
## 7. Модули и ответственность
### 1. `AppCore`
- запуск приложения;
- инициализация модулей;
- wiring dependencies;
- lifecycle.
### 2. `TrayController`
- иконка в трее;
- контекстное меню;
- быстрые действия:
  - начать перерыв сейчас;
  - отложить все на 30 минут;
  - открыть настройки;
  - открыть список событий;
  - выход.
### 3. `ReminderScheduler`
- вычисляет ближайшие occurrence;
- управляет `QTimer`;
- перестраивает расписание при изменениях;
- восстанавливает состояние после рестарта.
### 4. `ReminderCoordinator`
- решает, как показать reminder;
- применяет режим жесткости;
- эскалация между мягким/настойчивым/break mode;
- принимает результат действия пользователя.
### 5. `ProfileService`
- CRUD профилей;
- валидация политик напоминаний;
- выбор активного профиля по умолчанию.
### 6. `EventService`
- CRUD событий;
- recurrence;
- next occurrence calculation.
### 7. `QuietHoursService`
- определяет suppression/defer behavior;
- поддерживает интервалы через полночь.
### 8. `StatisticsService`
- пишет агрегаты;
- выдает summary для UI.
### 9. `SettingsService`
- общие настройки приложения;
- автозапуск;
- тема;
- поведение окна/трея.
### 10. `NotificationAdapter`
Интерфейс:
- `showPassiveNotification(...)`
- `showPersistentDialog(...)`
- `showBreakWindow(...)`
Реализации:
- Qt-only base;
- platform-specific optional augmentations.
### 11. `AutostartAdapter`
Интерфейс:
- `isEnabled()`
- `setEnabled(bool)`
Реализации:
- Windows Startup shortcut or registry `Run`;
- Linux `.desktop` in `~/.config/autostart`.
### 12. `Repository`
- SQLite-доступ;
- миграции;
- транзакции.
### 13. `SettingsStore`
- `QSettings` для небольших app-level опций.
### 14. `PlatformIntegration`
Изолированные platform-specific части:
- tray quirks;
- notifications fallback behavior;
- autostart implementation;
- optional future idle detection.
---
8. Модель данных
Почему комбинировать SQLite и QSettings
- QSettings подходит для мелких app settings:
  - geometry;
  - last selected tab;
  - theme;
  - flags UI.
- SQLite подходит для предметных данных:
  - события;
  - профили;
  - статистика;
  - журналы выполнений.
Такое разделение проще и чище, чем пытаться хранить все в QSettings.
Сущности
reminder_profiles
Поля:
- id
- name
- kind (break, generic, mixed)
- interval_minutes
- break_duration_minutes
- severity_mode (soft, persistent, break)
- max_snooze_count
- snooze_minutes
- quiet_hours_policy_id
- require_post_break_confirmation bool
- allow_skip bool
- enabled bool
- created_at
- updated_at
reminder_events
Поля:
- id
- title
- description
- type (break_interval, datetime)
- profile_id
- start_at
- timezone_mode (local)
- recurrence_rule
- is_one_time bool
- enabled bool
- last_triggered_at
- next_trigger_at
- created_at
- updated_at
Примечание по recurrence
Для MVP не нужен RFC 5545 полностью.
Достаточно собственного ограниченного формата:
- none
- daily
- weekdays
- weekly:<days>
- monthly:<day>
- every_n_days:<n>
Это сильно снижает сложность.
quiet_hours_policies
Поля:
- id
- name
- start_time
- end_time
- behavior_soft (suppress, defer)
- behavior_persistent (defer, downgrade, allow)
- behavior_break (defer, downgrade, allow)
- created_at
- updated_at
occurrence_log
Поля:
- id
- event_id
- triggered_at
- shown_mode
- result (completed, skipped, snoozed, dismissed, expired)
- snooze_count
- break_expected_seconds
- break_actual_seconds
- completed_at
Используется для статистики и отладки.
stats_daily
Поля:
- date
- completed_count
- skipped_count
- snoozed_count
- break_completed_count
- break_missed_count
Можно строить как materialized aggregate из occurrence_log, но для MVP допустимо обновлять напрямую.
App settings в QSettings
- ui/minimize_to_tray
- ui/close_to_tray
- ui/last_window_geometry
- ui/theme
- general/launch_on_startup
- general/start_minimized
- general/default_profile_id
- general/pause_until
- debug/log_level
---
9. UX и режимы напоминаний
Основные сценарии использования
Сценарий 1. Регулярные перерывы
Пользователь включает профиль:
- каждые 50 минут работы;
- 10 минут перерыв;
- не больше 2 snooze;
- после этого break-screen.
Сценарий 2. Напоминание о событии
Пользователь создает событие:
- "Созвон с врачом";
- завтра в 15:00;
- повторять не нужно;
- режим настойчивый.
Сценарий 3. Повторяющаяся рутина
Пользователь создает событие:
- "Выпить воду";
- каждый будний день в 11:30, 14:30, 17:00;
- мягкий режим.
Сценарий 4. Quiet Hours
Ночью приложение не показывает агрессивные окна, а переносит напоминания на утро.
Сценарий 5. Реальный перерыв
Когда приходит break reminder, пользователь не просто закрывает popup, а проходит через break-screen с таймером и ограниченным количеством отложений.
Ключевые окна
1. Tray menu
Должно быть основным способом повседневного взаимодействия.
Пункты:
- Начать перерыв сейчас
- Отложить все на 15 минут
- Отложить все на 30 минут
- Пауза до...
- События
- Профили
- Статистика
- Настройки
- Выход
2. Главное окно
Не должно быть перегружено.
Вкладки:
- Сегодня
- События
- Профили
- Статистика
- Настройки
3. Окно редактирования события
Поля:
- название;
- описание;
- тип;
- дата/время или интервал;
- повторяемость;
- профиль;
- режим;
- snooze policy;
- quiet hours behavior override опционально позже.
4. Reminder popup/dialog
Используется для мягкого и настойчивого режимов.
Кнопки:
- Сделано
- Отложить
- Пропустить если разрешено
- Начать перерыв для break-related reminders
5. Break screen
Полноэкранное окно приложения.
Содержимое:
- крупный таймер;
- короткий текст "Сделайте паузу, отойдите от экрана";
- одна-две безопасные подсказки:
  - посмотреть вдаль;
  - встать и пройтись;
  - размять плечи.
- кнопки:
  - Начать сейчас
  - Отложить если лимит еще не исчерпан
  - после истечения таймера Я вернулся
Режимы напоминаний
Мягкий
Поведение:
- стандартное системное уведомление или ненавязчивое окно;
- можно закрыть;
- одна кнопка Отложить;
- один показ без эскалации.
Для чего:
- пить воду;
- мелкие рутины;
- не критичные события.
Настойчивый
Поведение:
- повторяющееся уведомление или модальный dialog;
- если просто закрыт, через N минут показывается снова;
- есть лимит snooze;
- по достижении лимита возможна эскалация в break mode или marked missed.
Для чего:
- важные персональные события;
- break reminder средней жесткости.
Режим перерыва
Поведение:
- полноэкранное окно;
- таймер;
- ограниченный snooze;
- досрочное закрытие = не выполнено;
- выполнение считается только после минимальной длительности + явного подтверждения возврата.
Это безопаснее и реалистичнее, чем пытаться реально блокировать систему.
Логика “перерыв действительно выполнен”
Реалистичный и безопасный вариант:
1. Перерыв считается начатым, когда пользователь подтверждает старт или не отменяет break-screen.
2. Перерыв считается выполненным, если:
- окно break-screen оставалось активным заданное минимальное время;
- пользователь не нажал Skip;
- после завершения таймера нажал Я вернулся.
Дополнительно:
- если окно закрыто раньше минимального порога, статус missed;
- если был использован snooze, это учитывается отдельно;
- не делаем ложных заявлений, что человек точно отошел от ПК;
- формулировка в статистике: перерыв выполнен по сценарию приложения, а не пользователь реально отошел.
UX-принципы
- минимум трения в обычных сценариях;
- жесткость должна быть настраиваемой;
- no dark patterns;
- приложение не должно ломать работу пользователя ночью или на созвоне;
- быстрые действия доступны из трея;
- важные решения отражены в понятных текстах UI.
---
10. Кроссплатформенные особенности
Общая стратегия
Сначала опираться на Qt abstractions.
Платформенно-зависимое писать только там, где Qt не закрывает задачу полностью или делает это ненадежно.
Windows 10
Что обычно просто
- QSystemTrayIcon;
- QSettings;
- SQLite;
- автозапуск;
- полноэкранное окно поверх обычных окон.
Что нужно учесть
- toast notifications через чистый Qt могут быть ограниченными;
- поведение полноэкранного окна поверх некоторых приложений может зависеть от z-order;
- автозапуск лучше делать надежно и прозрачно.
Решение
- MVP: QSystemTrayIcon::showMessage + fallback dialog;
- автозапуск:
  - предпочтительно ярлык в Startup folder;
  - альтернатива: registry Run.
- break-screen делать как top-level fullscreen window с WindowStaysOnTopHint, но без агрессивной системной блокировки.
Linux: Arch / CachyOS
Основные риски
- tray support зависит от desktop environment;
- notifications зависят от DBus notification daemon;
- Wayland/X11 отличается по поведению always-on-top и фокусу окна.
Решение
- tray:
  - использовать QSystemTrayIcon как базу;
  - дополнительно тестировать на KDE Plasma и GNOME-like окружении;
  - документировать ограничение: в некоторых WM/DE трей может требовать shell extension или работать неполно.
- notifications:
  - базово через Qt;
  - при необходимости отдельный Linux adapter через freedesktop notifications over DBus во 2-й фазе.
- автозапуск:
  - .desktop файл в ~/.config/autostart.
- break-screen:
  - fullscreen window без попыток обойти ограничения Wayland;
  - если окружение не дает надежно поднять окно наверх, делать fallback на настойчивый modal dialog + repeated notifications.
Изоляция platform-specific кода
Нужны интерфейсы:
- INotificationPresenter
- IAutostartManager
- IWindowAttentionHelper
- IPlatformCapabilities
Capability detection
На старте приложение определяет:
- доступен ли tray;
- доступны ли системные уведомления;
- есть ли ограничения среды;
- какой fallback включить.
Это позволит не обещать одинаковое поведение там, где ОС не гарантирует его.
---
11. Сборка и упаковка
Build system
- CMake
- Qt 6
- сборка в CI для Windows и Linux
Зависимости
- Qt6::Core
- Qt6::Gui
- Qt6::Widgets
- Qt6::Sql
- при необходимости Qt6::DBus для Linux-улучшений во 2-й фазе
Windows
Цель:
- обычный installer.
Подход:
1. Сборка через MSVC.
2. windeployqt.
3. Упаковка через Inno Setup или NSIS.
Что поставить в MVP:
- zip portable build для внутренней проверки;
- installer как milestone до public beta.
Arch / CachyOS
Цель:
- realistic delivery without huge support burden.
Подход:
1. Базовый артефакт: AppImage для максимально простой поставки.
2. Дополнительно: PKGBUILD для Arch-based систем.
3. Для локальной интеграции:
- .desktop
- icon
- autostart desktop entry
Обновления
Для MVP не делать встроенный auto-update.
Причина:
- это отдельный пласт рисков и OS-specific поведения.
- можно выпускать новые версии через installer/AppImage/PKGBUILD.
Доставка
MVP:
- Windows installer или portable zip;
- Linux AppImage + PKGBUILD.
Логирование и диагностика
Нужно с MVP:
- локальный лог-файл;
- уровень логирования;
- логировать:
  - старт приложения;
  - загрузку конфигурации;
  - расчет расписания;
  - показ reminders;
  - результаты действий пользователя;
  - ошибки tray/notification/autostart.
Это критично для поддержки фонового приложения.
---
12. План тестирования
1. Unit tests
Покрыть:
- расчет следующего occurrence;
- recurrence rules;
- quiet hours, включая интервалы через полночь;
- snooze limits;
- escalation logic;
- break completion rules;
- stats aggregation.
Критичные случаи:
- смена даты;
- DST/переходы времени;
- приложение было закрыто во время пропущенного события;
- несколько событий на одно и то же время.
2. Integration tests
Покрыть:
- репозиторий SQLite;
- миграции;
- загрузку/сохранение профилей и событий;
- scheduler + repository;
- autostart adapters на smoke-level.
3. UI tests
Не делать тяжелый UI automation-first.
Для MVP:
- smoke tests на создание события;
- изменение профиля;
- показ break-screen;
- tray interaction частично вручную.
4. Manual cross-platform checklist
Windows
- старт приложения;
- трей появляется;
- уведомление приходит;
- автозапуск работает после reboot/logon;
- break-screen открывается;
- настройки сохраняются;
- installer ставит и удаляет чисто.
Arch/CachyOS
- запуск в Plasma;
- запуск в GNOME-like;
- tray доступен или fallback понятен;
- notifications приходят;
- autostart через .desktop работает;
- AppImage запускается;
- PKGBUILD собирается.
5. Soak testing
Для фонового reminder app это важно.
Нужно прогонять:
- 24-72 часа фоновой работы;
- многократные snooze;
- suspend/resume;
- смена системного времени;
- сон/пробуждение ноутбука.
6. Regression matrix
После каждого существенного этапа:
- soft reminder;
- persistent reminder;
- break-screen;
- quiet hours;
- restart app;
- restore schedule;
- stats update.
---
13. Риски и спорные места
1. Linux tray behavior
Риск: высокий.
Проблема:
- tray на Linux не одинаково работает в разных DE/WM.
Решение:
- capability detection;
- fallback UX;
- первичная поддержка ограниченного набора окружений;
- явная документация ограничений.
2. Полноэкранный break-screen под Wayland
Риск: высокий.
Проблема:
- Wayland ограничивает некоторые сценарии управления окнами.
Решение:
- не обещать абсолютную поверхностность окна;
- fallback на persistent dialog + repeated notification;
- platform adapter и capability flag.
3. Корректность календарной логики
Риск: средний.
Проблема:
- recurrence, quiet hours, DST, timezone edge cases.
Решение:
- ограниченный формат recurrence в MVP;
- сильные unit tests;
- хранение локального времени последовательно.
4. Ложное ощущение “реально выполненного перерыва”
Риск: продуктовый.
Проблема:
- приложение не может надежно доказать, что пользователь ушел от ПК.
Решение:
- честная формулировка;
- считать выполнение по сценарию приложения, а не по физическому факту.
5. Перегрузка MVP
Риск: высокий для solo-разработки.
Проблема:
- очень легко начать делать analytics, idle detection, advanced recurrence слишком рано.
Решение:
- жестко держать MVP;
- сначала один надежный scheduling core и три reminder modes.
6. Упаковка и поставка
Риск: средний.
Проблема:
- polished desktop delivery часто занимает больше времени, чем сама предметная логика.
Решение:
- в раннем MVP собирать portable artifacts;
- installer и polished AppImage доводить после стабилизации core.
7. Неправильная настойчивость UX
Риск: средний.
Проблема:
- слишком мягко: приложение игнорируют;
- слишком жестко: приложение раздражает и удаляется.
Решение:
- профили с понятной жесткостью;
- лимиты snooze;
- quiet hours;
- быстрый pause from tray.
---
14. Пошаговый roadmap по этапам
Этап 0. Технический foundation
Приоритет: P0
- создать skeleton приложения на C++/Qt 6 Widgets;
- настроить CMake;
- базовый app lifecycle;
- логирование;
- каркас main window + tray;
- capability detection stub.
Результат:
- приложение стартует, сворачивается в трей, корректно завершается.
Этап 1. Хранение данных и доменная модель
Приоритет: P0
- SQLite schema;
- repository;
- QSettings integration;
- модели профилей, событий, quiet hours, stats;
- миграции.
Результат:
- данные сохраняются и восстанавливаются.
Этап 2. Scheduling core
Приоритет: P0
- next occurrence calculation;
- in-memory queue;
- central QTimer;
- rebuild/refresh on change;
- basic recurrence.
Результат:
- приложение надежно знает, когда показать следующее reminder.
Этап 3. Reminder presentation
Приоритет: P0
- soft notification;
- persistent dialog;
- break-screen;
- actions: complete, snooze, skip;
- snooze limit.
Результат:
- все три режима работают end-to-end.
Этап 4. Profiles and event editor
Приоритет: P0
- UI событий;
- UI профилей;
- валидация;
- встроенные профили по умолчанию.
Результат:
- пользователь может настроить приложение без ручного редактирования конфигов.
Этап 5. Quiet hours and statistics
Приоритет: P0
- quiet hours policy;
- suppression/defer behavior;
- occurrence log;
- basic statistics screen.
Результат:
- приложение не мешает ночью и показывает базовую полезную статистику.
Этап 6. Autostart and platform polishing
Приоритет: P0
- Windows autostart adapter;
- Linux autostart adapter;
- fallback behavior for tray/notifications;
- cross-platform manual validation.
Результат:
- приложение реально живет как desktop utility.
Этап 7. Packaging
Приоритет: P1
- Windows deploy pipeline;
- installer;
- Linux AppImage;
- PKGBUILD.
Результат:
- приложение можно ставить и проверять вне dev environment.
Этап 8. Hardening
Приоритет: P1
- soak testing;
- suspend/resume fixes;
- corrupted state recovery;
- better diagnostics.
Результат:
- стабильный beta-quality продукт.
Этап 9. Post-MVP enhancements
Приоритет: P2
- idle-aware optional logic;
- richer recurrence;
- profile templates;
- better Linux integration through DBus;
- import/export.
---
15. Критерии готовности
Критерии готовности MVP
1. Приложение запускается на Windows 10 и на Arch-based Linux.
2. Трей работает или включается документированный fallback.
3. Можно создать:
- интервальный break reminder;
- одноразовое событие;
- повторяющееся событие.
4. Есть три режима напоминаний:
- soft;
- persistent;
- break-screen.
5. Есть snooze и лимиты snooze.
6. Есть quiet hours.
7. Есть автозапуск.
8. Настройки и события переживают перезапуск приложения.
9. Есть базовая статистика completed/skipped/snoozed.
10. Break reminder считается выполненным только после прохождения сценария перерыва.
11. Поведение проверено на ручных cross-platform сценариях.
12. Есть поставляемый артефакт хотя бы для тестовой установки.
Критерии готовности полной версии v1
1. Упаковка:
- Windows installer;
- Linux AppImage;
- PKGBUILD.
2. Стабильность scheduler подтверждена soak testing.
3. Основные edge cases покрыты unit/integration tests.
4. Linux-specific ограничения обработаны fallback-механизмами.
5. Журналирование достаточно для диагностики пользовательских проблем.
6. UX не требует main window для ежедневного использования, все основные действия доступны из tray.
---
