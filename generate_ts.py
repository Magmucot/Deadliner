import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

translations = {
    "Allow": "Разрешить",
    "Break interval": "Интервал перерыва",
    "Breaks": "Перерывы",
    "Daily": "Ежедневно",
    "Date and time": "Дата и время",
    "Defer": "Отложить",
    "Does not repeat": "Не повторяется",
    "Downgrade": "Смягчить",
    "Due now": "Сейчас",
    "Events": "События",
    "Every %1 days": "Каждые %1 дней",
    "In %1 d %2 h": "Через %1 дн %2 ч",
    "In %1 h %2 min": "Через %1 ч %2 мин",
    "In %1 min": "Через %1 мин",
    "Mixed": "Смешанный",
    "Monthly": "Ежемесячно",
    "Not scheduled": "Не запланировано",
    "Paused until %1": "Приостановлено до %1",
    "Persistent": "Настойчивый",
    "Reminders are active": "Напоминания активны",
    "Soft": "Мягкий",
    "Strict break": "Строгий перерыв",
    "Suppress": "Скрывать",
    "Weekdays": "По будням",
    "Weekly custom": "По неделям",
    "Break complete": "Перерыв завершен",
    "Break time": "Время перерыва",
    "I am back": "Я вернулся",
    "Look into the distance, stretch your shoulders, or walk for a minute.": "Посмотрите вдаль, разомните плечи или пройдитесь минутку.",
    "Skip": "Пропустить",
    "Skip unlocks in %1 s": "Пропуск через %1 с",
    "Snooze": "Отложить",
    "Start now": "Начать сейчас",
    "Take a break away from the screen": "Сделайте перерыв от экрана",
    "%1 mode, snooze %2 x %3 min, break %4 min": "Режим %1, отложить %2 x %3 мин, перерыв %4 мин",
    "Cancel": "Отмена",
    "Create event": "Создать событие",
    "Description": "Описание",
    "Edit event": "Редактировать событие",
    "Enabled": "Включено",
    "Events inherit reminder mode and snooze policy from the selected profile.": "События наследуют режим и политику из выбранного профиля.",
    "One-time event": "Одноразовое",
    "Profile": "Профиль",
    "Profile behavior": "Поведение профиля",
    "Recurrence": "Повторение",
    "Save": "Сохранить",
    "Start at / anchor": "Начало",
    "Title": "Название",
    "Type": "Тип",
    "Delete": "Удалить",
    "Edit": "Изменить",
    "Next trigger": "Следующее срабатывание",
    "No upcoming events.": "Нет предстоящих событий.",
    "One-time and recurring reminders with profile-based behavior.": "Одноразовые и повторяющиеся напоминания.",
    "Paused": "Приостановлено",
    "Repeat": "Повторять",
    "State": "Состояние",
    "You have no upcoming events.": "У вас нет предстоящих событий.",
    "Adjust startup behavior, quiet hours, theme and language.": "Настройте автозапуск, тихие часы, тему и язык.",
    "Check local reminder outcomes and adherence trends.": "Смотрите статистику выполнения напоминаний.",
    "Deadliner": "Deadliner",
    "Manage one-time and recurring reminders.": "Управляйте напоминаниями и событиями.",
    "Overview of upcoming reminders, active break profiles and today's events.": "Сводка ближайших напоминаний и событий на сегодня.",
    "Profiles": "Профили",
    "Settings": "Настройки",
    "Statistics": "Статистика",
    "Today": "Сегодня",
    "Tune reminder modes, break duration and snooze rules.": "Настройте режимы напоминаний, длительность и откладывание.",
    "Back": "Назад",
    "Break reminders that respect your day": "Перерывы, которые уважают ваше время",
    "Choose interface language": "Выберите язык интерфейса",
    "Deadliner starts gently, escalates only when needed, and keeps quiet hours configurable so reminders stay useful instead of annoying.": "Напоминания начинаются мягко, становятся настойчивыми только по необходимости и соблюдают тихие часы.",
    "Desktop-first workflow": "Desktop-first подход",
    "Enable the recommended 20-20-20 inspired eye-rest rule (20 min work / 1 min break).": "Включить рекомендованное правило отдыха глаз 20-20-20 (20 мин работа / 1 мин перерыв).",
    "English": "Английский",
    "Finish": "Завершить",
    "Most daily actions are available from the system tray, while the main window keeps event, profile and statistics management organized.": "Большинство действий доступно из системного трея.",
    "Next": "Далее",
    "Recommended starter setup": "Рекомендуемая базовая настройка",
    "Russian": "Русский",
    "Set up your reminder workspace": "Настройте ваше пространство",
    "The application supports Russian and English out of the box, and you can switch later in Settings without restarting.": "Приложение поддерживает русский и английский.",
    "Welcome to Deadliner": "Добро пожаловать в Deadliner",
    "You can start with a safe eye-rest preset now and fine-tune it later in Profiles.": "Вы можете начать с безопасного пресета отдыха для глаз.",
    "Allow skip": "Разрешить пропуск",
    "Break duration (minutes)": "Длительность перерыва (мин)",
    "Create profile": "Создать профиль",
    "Edit profile": "Изменить профиль",
    "Interval (minutes)": "Интервал (мин)",
    "Kind": "Вид",
    "Max snoozes": "Максимум откладываний",
    "Name": "Имя",
    "Quiet hours policy": "Политика тихих часов",
    "Reminder mode": "Режим",
    "Require post-break confirmation": "Требовать подтверждения",
    "Snooze minutes": "Минут откладывания",
    "%1 / %2 min": "%1 / %2 мин",
    "%1 x %2 min": "%1 x %2 мин",
    "Built-in": "Встроенный",
    "Cadence": "Ритм",
    "Create at least one profile to control reminder behavior.": "Создайте хотя бы один профиль.",
    "Custom": "Пользовательский",
    "Mode": "Режим",
    "No reminder profiles yet.": "Пока нет профилей.",
    "Origin": "Происхождение",
    "Profiles define reminder intensity, snooze limits and break behavior.": "Профили определяют интенсивность напоминаний.",
    "Scope": "Область",
    "Done": "Сделано",
    "Mode: %1": "Режим: %1",
    "Reminder is due.": "Пора!",
    "Close to tray": "Закрывать в трей",
    "Control startup behavior, quiet hours, theme and interface language.": "Управление запуском, тихими часами, темой и языком.",
    "Dark": "Темная",
    "Default profile": "Профиль по умолчанию",
    "Language": "Язык",
    "Launch on startup": "Запускать при старте",
    "Light": "Светлая",
    "Minimize to tray": "Сворачивать в трей",
    "Pause reminders until": "Приостановить до",
    "Persistent reminders": "Настойчивые",
    "Quiet hours end": "Конец тихих часов",
    "Quiet hours start": "Начало тихих часов",
    "Soft reminders": "Мягкие",
    "Start minimized": "Запускать свернутым",
    "Strict breaks": "Строгие перерывы",
    "System": "Системная",
    "System default": "По умолчанию",
    "Theme": "Тема",
    "Tray integration is available on this desktop environment.": "Трей доступен в вашем окружении.",
    "Tray integration is not available right now, so the main window stays as the primary control surface.": "Трей недоступен, основное управление в главном окне.",
    "Break completed": "Перерывы завершены",
    "Break missed": "Пропущено",
    "Completed": "Завершено",
    "Date": "Дата",
    "Lightweight local analytics for completed, skipped and snoozed reminders.": "Статистика по напоминаниям.",
    "No statistics yet.": "Пока нет статистики.",
    "Skipped": "Пропущено",
    "Snoozed": "Отложено",
    "Statistics will appear after reminders start running.": "Статистика появится после запуска напоминаний.",
    "%1. Here's your day at a glance.": "%1. Ваш день вкратце.",
    "%1 min work / %2 min break": "%1 мин работа / %2 мин отдых",
    "Active profiles": "Активные профили",
    "Good afternoon": "Добрый день",
    "Good evening": "Добрый вечер",
    "Good morning": "Доброе утро",
    "Manage events": "Управление",
    "Manage profiles": "Управление",
    "Next break": "След. перерыв",
    "No break scheduled": "Нет перерывов",
    "Time": "Время",
    "Today's events": "События на сегодня",
    "Open dashboard": "Открыть главное окно",
    "Pause for 1 hour": "Пауза на 1 час",
    "Pause for 3 hours": "Пауза на 3 часа",
    "Pause until tomorrow": "Пауза до завтра",
    "Quit": "Выход",
    "Skip next reminder": "Пропустить следующее",
    "Start break now": "Начать перерыв",
}

ts = ET.Element("TS", {"version": "2.1", "language": "ru_RU"})
context = ET.SubElement(ts, "context")
name = ET.SubElement(context, "name")
name.text = "deadliner"

for en, ru in translations.items():
    msg = ET.SubElement(context, "message")
    source = ET.SubElement(msg, "source")
    source.text = en
    translation = ET.SubElement(msg, "translation")
    translation.text = ru

tree = ET.ElementTree(ts)
ET.indent(tree, space="    ", level=0)
tree.write("resources/i18n/deadliner_ru.ts", encoding="utf-8", xml_declaration=True)

# Also generate the English one as a copy of the keys
ts_en = ET.Element("TS", {"version": "2.1", "language": "en_US"})
context_en = ET.SubElement(ts_en, "context")
name_en = ET.SubElement(context_en, "name")
name_en.text = "deadliner"
for en in translations.keys():
    msg = ET.SubElement(context_en, "message")
    source = ET.SubElement(msg, "source")
    source.text = en
    translation = ET.SubElement(msg, "translation")
    translation.text = en
tree_en = ET.ElementTree(ts_en)
ET.indent(tree_en, space="    ", level=0)
tree_en.write("resources/i18n/deadliner_en.ts", encoding="utf-8", xml_declaration=True)

print("TS files generated.")
