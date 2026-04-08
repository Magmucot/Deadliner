import re

content = ""
with open("src/ui/i18n/language_manager.cpp", "r") as f:
    content = f.read()

# Generate the dict literal
new_items = []
new_strings = [
    (
        "Overview of upcoming reminders, active break profiles and today's events.",
        "Обзор предстоящих напоминаний, активных профилей и событий.",
    ),
    ("Manage one-time and recurring reminders.", "Управление разовыми и регулярными напоминаниями."),
    (
        "Tune reminder modes, break duration and snooze rules.",
        "Настройка режимов напоминаний, перерывов и правил откладывания.",
    ),
    ("Check local reminder outcomes and adherence trends.", "Локальная статистика выполнения напоминаний."),
    ("Adjust startup behavior, quiet hours, theme and language.", "Настройка автозагрузки, тихих часов, темы и языка."),
    ("Active profiles", "Активные профили"),
    ("Today's events", "События сегодня"),
    ("Manage profiles", "Управление профилями"),
    ("Manage events", "Управление событиями"),
    ("Profile", "Профиль"),
    ("Mode", "Режим"),
    ("Cadence", "Интервал"),
    ("Time", "Время"),
    ("Title", "Название"),
    ("Repeat", "Повтор"),
    ("Next break", "Следующий перерыв"),
    ("No break scheduled", "Нет перерывов"),
    ("%1 min work / %2 min break", "%1 мин работы / %2 мин отдыха"),
    ("Enabled", "Включено"),
    ("Paused", "На паузе"),
    ("Events", "События"),
    (
        "One-time and recurring reminders with profile-based behavior.",
        "Разовые и повторяющиеся напоминания с привязкой к профилям.",
    ),
    ("Next trigger", "Следующее"),
    ("State", "Статус"),
    ("Profiles", "Профили"),
    (
        "Profiles define reminder intensity, snooze limits and break behavior.",
        "Профили задают интенсивность, лимиты откладывания и перерывы.",
    ),
    (
        "Create at least one profile to control reminder behavior.",
        "Создайте хотя бы один профиль для управления напоминаниями.",
    ),
    ("Name", "Название"),
    ("Scope", "Назначение"),
    ("Origin", "Источник"),
    ("%1 / %2 min", "%1 / %2 мин"),
    ("%1 x %2 min", "%1 раз по %2 мин"),
    ("Built-in", "Встроенный"),
    ("Custom", "Кастомный"),
    ("Completed", "Выполнено"),
    ("Skipped", "Пропущено"),
    ("Snoozed", "Отложено"),
    ("Date", "Дата"),
    (
        "Lightweight local analytics for completed, skipped and snoozed reminders.",
        "Локальная статистика по выполненным и пропущенным напоминаниям.",
    ),
    ("Statistics will appear after reminders start running.", "Статистика появится после запуска напоминаний."),
    ("Break completed", "Сделано перерывов"),
    ("Break missed", "Пропущено перерывов"),
    (
        "Control startup behavior, quiet hours, theme and interface language.",
        "Управление запуском, режимом тишины, темой и языком интерфейса.",
    ),
    ("Default profile", "Профиль по умолчанию"),
    ("Pause reminders until", "Пауза до"),
    ("Quiet hours start", "Начало тихих часов"),
    ("Quiet hours end", "Конец тихих часов"),
    ("Soft reminders", "Мягкие напоминания"),
    ("Persistent reminders", "Настойчивые напоминания"),
    ("Strict breaks", "Строгие перерывы"),
    ("Tray integration is available on this desktop environment.", "Интеграция с треем доступна в вашей среде."),
    (
        "Tray integration is not available right now, so the main window stays as the primary control surface.",
        "Трей сейчас недоступен, поэтому главное окно остаётся открытым.",
    ),
    ("Welcome to Deadliner", "Добро пожаловать в Deadliner"),
    ("Set up your reminder workspace", "Настройте ваше рабочее пространство"),
    ("Choose interface language", "Выберите язык интерфейса"),
    (
        "The application supports Russian and English out of the box, and you can switch later in Settings without restarting.",
        "Приложение поддерживает русский и английский, язык всегда можно сменить в настройках.",
    ),
    ("Break reminders that respect your day", "Напоминания, которые уважают ваше время"),
    (
        "Deadliner starts gently, escalates only when needed, and keeps quiet hours configurable so reminders stay useful instead of annoying.",
        "Deadliner начинает мягко и использует тихие часы, чтобы напоминания были полезными, а не раздражающими.",
    ),
    ("Desktop-first workflow", "Удобство на десктопе"),
    (
        "Most daily actions are available from the system tray, while the main window keeps event, profile and statistics management organized.",
        "Частые действия доступны из трея, а главное окно управляет событиями и статистикой.",
    ),
    ("Recommended starter setup", "Рекомендуемый пресет"),
    (
        "You can start with a safe eye-rest preset now and fine-tune it later in Profiles.",
        "Вы можете начать с базового правила отдыха для глаз и настроить его позже.",
    ),
    (
        "Enable the recommended 20-20-20 inspired eye-rest rule (20 min work / 1 min break).",
        "Включить рекомендуемое правило 20-20-20 (20 мин работы / 1 мин отдыха глаз).",
    ),
    ("Back", "Назад"),
    ("Next", "Далее"),
    ("Finish", "Завершить"),
    (
        "Events inherit reminder mode and snooze policy from the selected profile.",
        "События наследуют режим напоминания и лимиты от профиля.",
    ),
    ("Profile behavior", "Поведение"),
    ("Start at / anchor", "Время начала / отсчёта"),
    ("Recurrence", "Повторение"),
    ("%1 mode, snooze %2 x %3 min, break %4 min", "Режим %1, отложить %2 раз по %3 мин, перерыв %4 мин"),
    ("Require post-break confirmation", "Требовать подтверждения возвращения"),
    ("Interval (minutes)", "Интервал (минуты)"),
    ("Break duration (minutes)", "Длительность перерыва (мин)"),
    ("Max snoozes", "Макс. откладываний"),
    ("Snooze minutes", "Минут при откладывании"),
    ("Quiet hours policy", "Правило тихих часов"),
    ("Reminder mode", "Режим напоминания"),
    ("Kind", "Тип профиля"),
    ("Database error", "Ошибка базы данных"),
    ("Manual break", "Ручной перерыв"),
    ("Started from the tray menu.", "Запущено из меню трея."),
    ("Reminders paused until %1", "Напоминания на паузе до %1"),
    ("No upcoming reminders to skip.", "Нет предстоящих напоминаний для пропуска."),
    ("The next reminder cannot be skipped automatically.", "Следующее напоминание нельзя пропустить автоматически."),
    ("Skipped next reminder: %1", "Пропущено следующее напоминание: %1"),
    ("Good morning", "Доброе утро"),
    ("Good afternoon", "Добрый день"),
    ("Good evening", "Добрый вечер"),
    ("%1. Here's your day at a glance.", "%1. Вот план на сегодня."),
    ("20-20-20 eye rest", "Отдых для глаз 20-20-20"),
    ("Short eye-rest reminder suggested during onboarding.", "Короткий перерыв для глаз, добавленный при настройке."),
]

insert_text = ""
for en, ru in new_strings:
    en_esc = en.replace('"', '\\"')
    ru_esc = ru.replace('"', '\\"')
    insert_text += f'    {{QStringLiteral("*\\n{en_esc}"), QStringLiteral("{ru_esc}")}},\n'

target_marker = '    {QStringLiteral("*\\nDark"), QStringLiteral("Тёмная")},\n'
if target_marker in content:
    content = content.replace(target_marker, target_marker + insert_text)
    with open("src/ui/i18n/language_manager.cpp", "w") as f:
        f.write(content)
    print("Patch applied successfully.")
else:
    print("Could not find insertion marker.")
