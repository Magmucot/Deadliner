#include "ui/i18n/language_manager.h"

#include <QApplication>
#include <QHash>
#include <QLocale>
#include <QTranslator>

#include <utility>

namespace deadliner::ui
{

    namespace
    {

        class DictionaryTranslator final : public QTranslator
        {
        public:
            explicit DictionaryTranslator(QString languageCode, QObject *parent = nullptr)
                : QTranslator(parent), m_languageCode(std::move(languageCode))
            {
            }

            QString translate(const char *context,
                              const char *sourceText,
                              const char *disambiguation,
                              int n) const override
            {
                Q_UNUSED(disambiguation)
                Q_UNUSED(n)

                if (m_languageCode != QStringLiteral("ru") || sourceText == nullptr)
                {
                    return {};
                }

                const QString ctx = QString::fromUtf8(context ? context : "");
                const QString key = ctx + QLatin1Char('\n') + QString::fromUtf8(sourceText);
                const auto it = s_translations.constFind(key);
                if (it != s_translations.constEnd())
                {
                    return it.value();
                }

                const auto fallback = s_translations.constFind(QStringLiteral("*\n") + QString::fromUtf8(sourceText));
                return fallback != s_translations.constEnd() ? fallback.value() : QString{};
            }

        private:
            QString m_languageCode;
            static const QHash<QString, QString> s_translations;
        };

        // ─── Russian translations ────────────────────────────────────────────────────
        // Key format:  "*\n<source>"       — context-independent fallback
        // Rules:
        //   • Keys must exactly match the source strings passed to tr().
        //   • %1, %2 … placeholders must be preserved verbatim.
        //   • Use ё where grammatically required.
        const QHash<QString, QString> DictionaryTranslator::s_translations = {
            // App identity
            {QStringLiteral("*\nDeadliner"), QStringLiteral("Deadliner")},

            // Navigation
            {QStringLiteral("*\nToday"), QStringLiteral("Сегодня")},
            {QStringLiteral("*\nEvents"), QStringLiteral("События")},
            {QStringLiteral("*\nProfiles"), QStringLiteral("Профили")},
            {QStringLiteral("*\nStatistics"), QStringLiteral("Статистика")},
            {QStringLiteral("*\nSettings"), QStringLiteral("Настройки")},
            {QStringLiteral("*\nBreaks"), QStringLiteral("Перерывы")},
            {QStringLiteral("*\nDashboard"), QStringLiteral("Сводка")},

            // Section subtitles
            {QStringLiteral("*\nOverview of upcoming reminders, active break profiles and today's events."),
             QStringLiteral("Обзор предстоящих напоминаний, активных профилей и событий сегодня.")},
            {QStringLiteral("*\nManage one-time and recurring reminders."),
             QStringLiteral("Управление разовыми и регулярными напоминаниями.")},
            {QStringLiteral("*\nTune reminder modes, break duration and snooze rules."),
             QStringLiteral("Настройка режимов напоминаний, длительности перерывов и правил откладывания.")},
            {QStringLiteral("*\nCheck local reminder outcomes and adherence trends."),
             QStringLiteral("Локальная статистика выполнения напоминаний.")},
            {QStringLiteral("*\nAdjust startup behavior, quiet hours, theme and language."),
             QStringLiteral("Настройка автозагрузки, тихих часов, темы и языка.")},

            // Reminder / event types
            {QStringLiteral("*\nBreak interval"), QStringLiteral("Перерыв по интервалу")},
            {QStringLiteral("*\nDate and time"), QStringLiteral("Дата и время")},

            // Severity modes
            {QStringLiteral("*\nSoft"), QStringLiteral("Мягкий")},
            {QStringLiteral("*\nPersistent"), QStringLiteral("Настойчивый")},
            {QStringLiteral("*\nStrict break"), QStringLiteral("Строгий перерыв")},

            // Quiet behavior
            {QStringLiteral("*\nSuppress"), QStringLiteral("Подавлять")},
            {QStringLiteral("*\nDefer"), QStringLiteral("Переносить")},
            {QStringLiteral("*\nDowngrade"), QStringLiteral("Смягчать")},
            {QStringLiteral("*\nAllow"), QStringLiteral("Разрешать")},

            // Recurrence
            {QStringLiteral("*\nDoes not repeat"), QStringLiteral("Не повторяется")},
            {QStringLiteral("*\nDaily"), QStringLiteral("Ежедневно")},
            {QStringLiteral("*\nWeekdays"), QStringLiteral("По будням")},
            {QStringLiteral("*\nWeekly custom"), QStringLiteral("Еженедельно")},
            {QStringLiteral("*\nMonthly"), QStringLiteral("Ежемесячно")},
            {QStringLiteral("*\nEvery %1 days"), QStringLiteral("Каждые %1 дн.")},

            // Time / due labels
            {QStringLiteral("*\nNot scheduled"), QStringLiteral("Не запланировано")},
            {QStringLiteral("*\nDue now"), QStringLiteral("Сейчас")},
            {QStringLiteral("*\nIn %1 min"), QStringLiteral("Через %1 мин")},
            {QStringLiteral("*\nIn %1 h %2 min"), QStringLiteral("Через %1 ч %2 мин")},
            {QStringLiteral("*\nIn %1 d %2 h"), QStringLiteral("Через %1 д %2 ч")},
            {QStringLiteral("*\nPaused until %1"), QStringLiteral("Пауза до %1")},
            {QStringLiteral("*\nReminders are active"), QStringLiteral("Напоминания активны")},

            // Tray menu
            {QStringLiteral("*\nOpen dashboard"), QStringLiteral("Открыть окно")},
            {QStringLiteral("*\nStart break now"), QStringLiteral("Начать перерыв сейчас")},
            {QStringLiteral("*\nPause for 1 hour"), QStringLiteral("Пауза на 1 час")},
            {QStringLiteral("*\nPause for 3 hours"), QStringLiteral("Пауза на 3 часа")},
            {QStringLiteral("*\nPause until tomorrow"), QStringLiteral("Пауза до завтра")},
            {QStringLiteral("*\nQuit"), QStringLiteral("Выход")},
            {QStringLiteral("*\nOpen"), QStringLiteral("Открыть")},

            // Common actions
            {QStringLiteral("*\nSave"), QStringLiteral("Сохранить")},
            {QStringLiteral("*\nCancel"), QStringLiteral("Отмена")},
            {QStringLiteral("*\nCreate"), QStringLiteral("Создать")},
            {QStringLiteral("*\nEdit"), QStringLiteral("Изменить")},
            {QStringLiteral("*\nDelete"), QStringLiteral("Удалить")},
            {QStringLiteral("*\nDone"), QStringLiteral("Сделано")},
            {QStringLiteral("*\nSnooze"), QStringLiteral("Отложить")},
            {QStringLiteral("*\nSkip"), QStringLiteral("Пропустить")},
            {QStringLiteral("*\nBack"), QStringLiteral("Назад")},
            {QStringLiteral("*\nNext"), QStringLiteral("Далее")},
            {QStringLiteral("*\nFinish"), QStringLiteral("Завершить")},

            // CRUD titles
            {QStringLiteral("*\nCreate event"), QStringLiteral("Создать событие")},
            {QStringLiteral("*\nCreate profile"), QStringLiteral("Создать профиль")},
            {QStringLiteral("*\nEdit event"), QStringLiteral("Редактировать событие")},
            {QStringLiteral("*\nEdit profile"), QStringLiteral("Редактировать профиль")},

            // Empty states
            {QStringLiteral("*\nNo upcoming events."), QStringLiteral("Нет предстоящих событий.")},
            {QStringLiteral("*\nYou have no upcoming events."), QStringLiteral("У вас нет предстоящих событий.")},
            {QStringLiteral("*\nNo reminder profiles yet."), QStringLiteral("Пока нет профилей напоминаний.")},
            {QStringLiteral("*\nNo statistics yet."), QStringLiteral("Пока нет статистики.")},
            {QStringLiteral("*\nSelect an item first."), QStringLiteral("Сначала выберите элемент.")},

            // Dialogs / confirmations
            {QStringLiteral("*\nSelection required"), QStringLiteral("Нужно выбрать элемент")},
            {QStringLiteral("*\nBuilt-in profiles cannot be deleted."), QStringLiteral("Встроенные профили нельзя удалить.")},
            {QStringLiteral("*\nBuilt-in profile"), QStringLiteral("Встроенный профиль")},
            {QStringLiteral("*\nDatabase error"), QStringLiteral("Ошибка базы данных")},

            // Reminder dialog
            {QStringLiteral("*\nReminder is due"), QStringLiteral("Напоминание наступило")},
            {QStringLiteral("*\nReminder is due."), QStringLiteral("Напоминание наступило.")},
            {QStringLiteral("*\nMode: %1"), QStringLiteral("Режим: %1")},

            // Break window
            {QStringLiteral("*\nBreak time"),
             QStringLiteral("Время перерыва")},
            {QStringLiteral("*\nTake a break away from the screen"),
             QStringLiteral("Сделайте паузу и отойдите от экрана")},
            {QStringLiteral("*\nLook into the distance, stretch your shoulders, or walk for a minute."),
             QStringLiteral("Посмотрите вдаль, разомните плечи или немного пройдитесь.")},
            {QStringLiteral("*\nStart now"), QStringLiteral("Начать сейчас")},
            {QStringLiteral("*\nI am back"), QStringLiteral("Я вернулся")},
            {QStringLiteral("*\nBreak complete"), QStringLiteral("Перерыв завершён")},
            {QStringLiteral("*\nSkip unlocks in %1 s"), QStringLiteral("Пропуск станет доступен через %1 с")},

            // Settings page
            {QStringLiteral("*\nClose to tray"), QStringLiteral("Закрывать в трей")},
            {QStringLiteral("*\nLaunch on startup"), QStringLiteral("Запускать при старте системы")},
            {QStringLiteral("*\nStart minimized"), QStringLiteral("Запускать свёрнутым")},
            {QStringLiteral("*\nMinimize to tray"), QStringLiteral("Сворачивать в трей")},
            {QStringLiteral("*\nLanguage"), QStringLiteral("Язык")},
            {QStringLiteral("*\nTheme"), QStringLiteral("Тема")},
            {QStringLiteral("*\nSystem default"), QStringLiteral("Системный")},
            {QStringLiteral("*\nEnglish"), QStringLiteral("English")},
            {QStringLiteral("*\nRussian"), QStringLiteral("Русский")},
            {QStringLiteral("*\nSystem"), QStringLiteral("Системная")},
            {QStringLiteral("*\nLight"), QStringLiteral("Светлая")},
            {QStringLiteral("*\nDark"), QStringLiteral("Тёмная")},
    {QStringLiteral("*\nOverview of upcoming reminders, active break profiles and today's events."), QStringLiteral("Обзор предстоящих напоминаний, активных профилей и событий.")},
    {QStringLiteral("*\nManage one-time and recurring reminders."), QStringLiteral("Управление разовыми и регулярными напоминаниями.")},
    {QStringLiteral("*\nTune reminder modes, break duration and snooze rules."), QStringLiteral("Настройка режимов напоминаний, перерывов и правил откладывания.")},
    {QStringLiteral("*\nCheck local reminder outcomes and adherence trends."), QStringLiteral("Локальная статистика выполнения напоминаний.")},
    {QStringLiteral("*\nAdjust startup behavior, quiet hours, theme and language."), QStringLiteral("Настройка автозагрузки, тихих часов, темы и языка.")},
    {QStringLiteral("*\nActive profiles"), QStringLiteral("Активные профили")},
    {QStringLiteral("*\nToday's events"), QStringLiteral("События сегодня")},
    {QStringLiteral("*\nManage profiles"), QStringLiteral("Управление профилями")},
    {QStringLiteral("*\nManage events"), QStringLiteral("Управление событиями")},
    {QStringLiteral("*\nProfile"), QStringLiteral("Профиль")},
    {QStringLiteral("*\nMode"), QStringLiteral("Режим")},
    {QStringLiteral("*\nCadence"), QStringLiteral("Интервал")},
    {QStringLiteral("*\nTime"), QStringLiteral("Время")},
    {QStringLiteral("*\nTitle"), QStringLiteral("Название")},
    {QStringLiteral("*\nRepeat"), QStringLiteral("Повтор")},
    {QStringLiteral("*\nNext break"), QStringLiteral("Следующий перерыв")},
    {QStringLiteral("*\nNo break scheduled"), QStringLiteral("Нет перерывов")},
    {QStringLiteral("*\n%1 min work / %2 min break"), QStringLiteral("%1 мин работы / %2 мин отдыха")},
    {QStringLiteral("*\nEnabled"), QStringLiteral("Включено")},
    {QStringLiteral("*\nPaused"), QStringLiteral("На паузе")},
    {QStringLiteral("*\nEvents"), QStringLiteral("События")},
    {QStringLiteral("*\nOne-time and recurring reminders with profile-based behavior."), QStringLiteral("Разовые и повторяющиеся напоминания с привязкой к профилям.")},
    {QStringLiteral("*\nNext trigger"), QStringLiteral("Следующее")},
    {QStringLiteral("*\nState"), QStringLiteral("Статус")},
    {QStringLiteral("*\nProfiles"), QStringLiteral("Профили")},
    {QStringLiteral("*\nProfiles define reminder intensity, snooze limits and break behavior."), QStringLiteral("Профили задают интенсивность, лимиты откладывания и перерывы.")},
    {QStringLiteral("*\nCreate at least one profile to control reminder behavior."), QStringLiteral("Создайте хотя бы один профиль для управления напоминаниями.")},
    {QStringLiteral("*\nName"), QStringLiteral("Название")},
    {QStringLiteral("*\nScope"), QStringLiteral("Назначение")},
    {QStringLiteral("*\nOrigin"), QStringLiteral("Источник")},
    {QStringLiteral("*\n%1 / %2 min"), QStringLiteral("%1 / %2 мин")},
    {QStringLiteral("*\n%1 x %2 min"), QStringLiteral("%1 раз по %2 мин")},
    {QStringLiteral("*\nBuilt-in"), QStringLiteral("Встроенный")},
    {QStringLiteral("*\nCustom"), QStringLiteral("Кастомный")},
    {QStringLiteral("*\nCompleted"), QStringLiteral("Выполнено")},
    {QStringLiteral("*\nSkipped"), QStringLiteral("Пропущено")},
    {QStringLiteral("*\nSnoozed"), QStringLiteral("Отложено")},
    {QStringLiteral("*\nDate"), QStringLiteral("Дата")},
    {QStringLiteral("*\nLightweight local analytics for completed, skipped and snoozed reminders."), QStringLiteral("Локальная статистика по выполненным и пропущенным напоминаниям.")},
    {QStringLiteral("*\nStatistics will appear after reminders start running."), QStringLiteral("Статистика появится после запуска напоминаний.")},
    {QStringLiteral("*\nBreak completed"), QStringLiteral("Сделано перерывов")},
    {QStringLiteral("*\nBreak missed"), QStringLiteral("Пропущено перерывов")},
    {QStringLiteral("*\nControl startup behavior, quiet hours, theme and interface language."), QStringLiteral("Управление запуском, режимом тишины, темой и языком интерфейса.")},
    {QStringLiteral("*\nDefault profile"), QStringLiteral("Профиль по умолчанию")},
    {QStringLiteral("*\nPause reminders until"), QStringLiteral("Пауза до")},
    {QStringLiteral("*\nQuiet hours start"), QStringLiteral("Начало тихих часов")},
    {QStringLiteral("*\nQuiet hours end"), QStringLiteral("Конец тихих часов")},
    {QStringLiteral("*\nSoft reminders"), QStringLiteral("Мягкие напоминания")},
    {QStringLiteral("*\nPersistent reminders"), QStringLiteral("Настойчивые напоминания")},
    {QStringLiteral("*\nStrict breaks"), QStringLiteral("Строгие перерывы")},
    {QStringLiteral("*\nTray integration is available on this desktop environment."), QStringLiteral("Интеграция с треем доступна в вашей среде.")},
    {QStringLiteral("*\nTray integration is not available right now, so the main window stays as the primary control surface."), QStringLiteral("Трей сейчас недоступен, поэтому главное окно остаётся открытым.")},
    {QStringLiteral("*\nWelcome to Deadliner"), QStringLiteral("Добро пожаловать в Deadliner")},
    {QStringLiteral("*\nSet up your reminder workspace"), QStringLiteral("Настройте ваше рабочее пространство")},
    {QStringLiteral("*\nChoose interface language"), QStringLiteral("Выберите язык интерфейса")},
    {QStringLiteral("*\nThe application supports Russian and English out of the box, and you can switch later in Settings without restarting."), QStringLiteral("Приложение поддерживает русский и английский, язык всегда можно сменить в настройках.")},
    {QStringLiteral("*\nBreak reminders that respect your day"), QStringLiteral("Напоминания, которые уважают ваше время")},
    {QStringLiteral("*\nDeadliner starts gently, escalates only when needed, and keeps quiet hours configurable so reminders stay useful instead of annoying."), QStringLiteral("Deadliner начинает мягко и использует тихие часы, чтобы напоминания были полезными, а не раздражающими.")},
    {QStringLiteral("*\nDesktop-first workflow"), QStringLiteral("Удобство на десктопе")},
    {QStringLiteral("*\nMost daily actions are available from the system tray, while the main window keeps event, profile and statistics management organized."), QStringLiteral("Частые действия доступны из трея, а главное окно управляет событиями и статистикой.")},
    {QStringLiteral("*\nRecommended starter setup"), QStringLiteral("Рекомендуемый пресет")},
    {QStringLiteral("*\nYou can start with a safe eye-rest preset now and fine-tune it later in Profiles."), QStringLiteral("Вы можете начать с базового правила отдыха для глаз и настроить его позже.")},
    {QStringLiteral("*\nEnable the recommended 20-20-20 inspired eye-rest rule (20 min work / 1 min break)."), QStringLiteral("Включить рекомендуемое правило 20-20-20 (20 мин работы / 1 мин отдыха глаз).")},
    {QStringLiteral("*\nBack"), QStringLiteral("Назад")},
    {QStringLiteral("*\nNext"), QStringLiteral("Далее")},
    {QStringLiteral("*\nFinish"), QStringLiteral("Завершить")},
    {QStringLiteral("*\nEvents inherit reminder mode and snooze policy from the selected profile."), QStringLiteral("События наследуют режим напоминания и лимиты от профиля.")},
    {QStringLiteral("*\nProfile behavior"), QStringLiteral("Поведение")},
    {QStringLiteral("*\nStart at / anchor"), QStringLiteral("Время начала / отсчёта")},
    {QStringLiteral("*\nRecurrence"), QStringLiteral("Повторение")},
    {QStringLiteral("*\n%1 mode, snooze %2 x %3 min, break %4 min"), QStringLiteral("Режим %1, отложить %2 раз по %3 мин, перерыв %4 мин")},
    {QStringLiteral("*\nRequire post-break confirmation"), QStringLiteral("Требовать подтверждения возвращения")},
    {QStringLiteral("*\nInterval (minutes)"), QStringLiteral("Интервал (минуты)")},
    {QStringLiteral("*\nBreak duration (minutes)"), QStringLiteral("Длительность перерыва (мин)")},
    {QStringLiteral("*\nMax snoozes"), QStringLiteral("Макс. откладываний")},
    {QStringLiteral("*\nSnooze minutes"), QStringLiteral("Минут при откладывании")},
    {QStringLiteral("*\nQuiet hours policy"), QStringLiteral("Правило тихих часов")},
    {QStringLiteral("*\nReminder mode"), QStringLiteral("Режим напоминания")},
    {QStringLiteral("*\nKind"), QStringLiteral("Тип профиля")},
    {QStringLiteral("*\nDatabase error"), QStringLiteral("Ошибка базы данных")},
    {QStringLiteral("*\nManual break"), QStringLiteral("Ручной перерыв")},
    {QStringLiteral("*\nStarted from the tray menu."), QStringLiteral("Запущено из меню трея.")},
    {QStringLiteral("*\nReminders paused until %1"), QStringLiteral("Напоминания на паузе до %1")},
    {QStringLiteral("*\nNo upcoming reminders to skip."), QStringLiteral("Нет предстоящих напоминаний для пропуска.")},
    {QStringLiteral("*\nThe next reminder cannot be skipped automatically."), QStringLiteral("Следующее напоминание нельзя пропустить автоматически.")},
    {QStringLiteral("*\nSkipped next reminder: %1"), QStringLiteral("Пропущено следующее напоминание: %1")},
    {QStringLiteral("*\nGood morning"), QStringLiteral("Доброе утро")},
    {QStringLiteral("*\nGood afternoon"), QStringLiteral("Добрый день")},
    {QStringLiteral("*\nGood evening"), QStringLiteral("Добрый вечер")},
    {QStringLiteral("*\n%1. Here's your day at a glance."), QStringLiteral("%1. Вот план на сегодня.")},
    {QStringLiteral("*\n20-20-20 eye rest"), QStringLiteral("Отдых для глаз 20-20-20")},
    {QStringLiteral("*\nShort eye-rest reminder suggested during onboarding."), QStringLiteral("Короткий перерыв для глаз, добавленный при настройке.")},
    {QStringLiteral("*\nOverview of upcoming reminders, active break profiles and today's events."), QStringLiteral("Обзор предстоящих напоминаний, активных профилей и событий.")},
    {QStringLiteral("*\nManage one-time and recurring reminders."), QStringLiteral("Управление разовыми и регулярными напоминаниями.")},
    {QStringLiteral("*\nTune reminder modes, break duration and snooze rules."), QStringLiteral("Настройка режимов напоминаний, перерывов и правил откладывания.")},
    {QStringLiteral("*\nCheck local reminder outcomes and adherence trends."), QStringLiteral("Локальная статистика выполнения напоминаний.")},
    {QStringLiteral("*\nAdjust startup behavior, quiet hours, theme and language."), QStringLiteral("Настройка автозагрузки, тихих часов, темы и языка.")},
    {QStringLiteral("*\nActive profiles"), QStringLiteral("Активные профили")},
    {QStringLiteral("*\nToday's events"), QStringLiteral("События сегодня")},
    {QStringLiteral("*\nManage profiles"), QStringLiteral("Управление профилями")},
    {QStringLiteral("*\nManage events"), QStringLiteral("Управление событиями")},
    {QStringLiteral("*\nProfile"), QStringLiteral("Профиль")},
    {QStringLiteral("*\nMode"), QStringLiteral("Режим")},
    {QStringLiteral("*\nCadence"), QStringLiteral("Интервал")},
    {QStringLiteral("*\nTime"), QStringLiteral("Время")},
    {QStringLiteral("*\nTitle"), QStringLiteral("Название")},
    {QStringLiteral("*\nRepeat"), QStringLiteral("Повтор")},
    {QStringLiteral("*\nNext break"), QStringLiteral("Следующий перерыв")},
    {QStringLiteral("*\nNo break scheduled"), QStringLiteral("Нет перерывов")},
    {QStringLiteral("*\n%1 min work / %2 min break"), QStringLiteral("%1 мин работы / %2 мин отдыха")},
    {QStringLiteral("*\nEnabled"), QStringLiteral("Включено")},
    {QStringLiteral("*\nPaused"), QStringLiteral("На паузе")},
    {QStringLiteral("*\nEvents"), QStringLiteral("События")},
    {QStringLiteral("*\nOne-time and recurring reminders with profile-based behavior."), QStringLiteral("Разовые и повторяющиеся напоминания с привязкой к профилям.")},
    {QStringLiteral("*\nNext trigger"), QStringLiteral("Следующее")},
    {QStringLiteral("*\nState"), QStringLiteral("Статус")},
    {QStringLiteral("*\nProfiles"), QStringLiteral("Профили")},
    {QStringLiteral("*\nProfiles define reminder intensity, snooze limits and break behavior."), QStringLiteral("Профили задают интенсивность, лимиты откладывания и перерывы.")},
    {QStringLiteral("*\nCreate at least one profile to control reminder behavior."), QStringLiteral("Создайте хотя бы один профиль для управления напоминаниями.")},
    {QStringLiteral("*\nName"), QStringLiteral("Название")},
    {QStringLiteral("*\nScope"), QStringLiteral("Назначение")},
    {QStringLiteral("*\nOrigin"), QStringLiteral("Источник")},
    {QStringLiteral("*\n%1 / %2 min"), QStringLiteral("%1 / %2 мин")},
    {QStringLiteral("*\n%1 x %2 min"), QStringLiteral("%1 раз по %2 мин")},
    {QStringLiteral("*\nBuilt-in"), QStringLiteral("Встроенный")},
    {QStringLiteral("*\nCustom"), QStringLiteral("Кастомный")},
    {QStringLiteral("*\nCompleted"), QStringLiteral("Выполнено")},
    {QStringLiteral("*\nSkipped"), QStringLiteral("Пропущено")},
    {QStringLiteral("*\nSnoozed"), QStringLiteral("Отложено")},
    {QStringLiteral("*\nDate"), QStringLiteral("Дата")},
    {QStringLiteral("*\nLightweight local analytics for completed, skipped and snoozed reminders."), QStringLiteral("Локальная статистика по выполненным и пропущенным напоминаниям.")},
    {QStringLiteral("*\nStatistics will appear after reminders start running."), QStringLiteral("Статистика появится после запуска напоминаний.")},
    {QStringLiteral("*\nBreak completed"), QStringLiteral("Сделано перерывов")},
    {QStringLiteral("*\nBreak missed"), QStringLiteral("Пропущено перерывов")},
    {QStringLiteral("*\nControl startup behavior, quiet hours, theme and interface language."), QStringLiteral("Управление запуском, режимом тишины, темой и языком интерфейса.")},
    {QStringLiteral("*\nDefault profile"), QStringLiteral("Профиль по умолчанию")},
    {QStringLiteral("*\nPause reminders until"), QStringLiteral("Пауза до")},
    {QStringLiteral("*\nQuiet hours start"), QStringLiteral("Начало тихих часов")},
    {QStringLiteral("*\nQuiet hours end"), QStringLiteral("Конец тихих часов")},
    {QStringLiteral("*\nSoft reminders"), QStringLiteral("Мягкие напоминания")},
    {QStringLiteral("*\nPersistent reminders"), QStringLiteral("Настойчивые напоминания")},
    {QStringLiteral("*\nStrict breaks"), QStringLiteral("Строгие перерывы")},
    {QStringLiteral("*\nTray integration is available on this desktop environment."), QStringLiteral("Интеграция с треем доступна в вашей среде.")},
    {QStringLiteral("*\nTray integration is not available right now, so the main window stays as the primary control surface."), QStringLiteral("Трей сейчас недоступен, поэтому главное окно остаётся открытым.")},
    {QStringLiteral("*\nWelcome to Deadliner"), QStringLiteral("Добро пожаловать в Deadliner")},
    {QStringLiteral("*\nSet up your reminder workspace"), QStringLiteral("Настройте ваше рабочее пространство")},
    {QStringLiteral("*\nChoose interface language"), QStringLiteral("Выберите язык интерфейса")},
    {QStringLiteral("*\nThe application supports Russian and English out of the box, and you can switch later in Settings without restarting."), QStringLiteral("Приложение поддерживает русский и английский, язык всегда можно сменить в настройках.")},
    {QStringLiteral("*\nBreak reminders that respect your day"), QStringLiteral("Напоминания, которые уважают ваше время")},
    {QStringLiteral("*\nDeadliner starts gently, escalates only when needed, and keeps quiet hours configurable so reminders stay useful instead of annoying."), QStringLiteral("Deadliner начинает мягко и использует тихие часы, чтобы напоминания были полезными, а не раздражающими.")},
    {QStringLiteral("*\nDesktop-first workflow"), QStringLiteral("Удобство на десктопе")},
    {QStringLiteral("*\nMost daily actions are available from the system tray, while the main window keeps event, profile and statistics management organized."), QStringLiteral("Частые действия доступны из трея, а главное окно управляет событиями и статистикой.")},
    {QStringLiteral("*\nRecommended starter setup"), QStringLiteral("Рекомендуемый пресет")},
    {QStringLiteral("*\nYou can start with a safe eye-rest preset now and fine-tune it later in Profiles."), QStringLiteral("Вы можете начать с базового правила отдыха для глаз и настроить его позже.")},
    {QStringLiteral("*\nEnable the recommended 20-20-20 inspired eye-rest rule (20 min work / 1 min break)."), QStringLiteral("Включить рекомендуемое правило 20-20-20 (20 мин работы / 1 мин отдыха глаз).")},
    {QStringLiteral("*\nBack"), QStringLiteral("Назад")},
    {QStringLiteral("*\nNext"), QStringLiteral("Далее")},
    {QStringLiteral("*\nFinish"), QStringLiteral("Завершить")},
    {QStringLiteral("*\nEvents inherit reminder mode and snooze policy from the selected profile."), QStringLiteral("События наследуют режим напоминания и лимиты от профиля.")},
    {QStringLiteral("*\nProfile behavior"), QStringLiteral("Поведение")},
    {QStringLiteral("*\nStart at / anchor"), QStringLiteral("Время начала / отсчёта")},
    {QStringLiteral("*\nRecurrence"), QStringLiteral("Повторение")},
    {QStringLiteral("*\n%1 mode, snooze %2 x %3 min, break %4 min"), QStringLiteral("Режим %1, отложить %2 раз по %3 мин, перерыв %4 мин")},
    {QStringLiteral("*\nRequire post-break confirmation"), QStringLiteral("Требовать подтверждения возвращения")},
    {QStringLiteral("*\nInterval (minutes)"), QStringLiteral("Интервал (минуты)")},
    {QStringLiteral("*\nBreak duration (minutes)"), QStringLiteral("Длительность перерыва (мин)")},
    {QStringLiteral("*\nMax snoozes"), QStringLiteral("Макс. откладываний")},
    {QStringLiteral("*\nSnooze minutes"), QStringLiteral("Минут при откладывании")},
    {QStringLiteral("*\nQuiet hours policy"), QStringLiteral("Правило тихих часов")},
    {QStringLiteral("*\nReminder mode"), QStringLiteral("Режим напоминания")},
    {QStringLiteral("*\nKind"), QStringLiteral("Тип профиля")},
    {QStringLiteral("*\nDatabase error"), QStringLiteral("Ошибка базы данных")},
    {QStringLiteral("*\nManual break"), QStringLiteral("Ручной перерыв")},
    {QStringLiteral("*\nStarted from the tray menu."), QStringLiteral("Запущено из меню трея.")},
    {QStringLiteral("*\nReminders paused until %1"), QStringLiteral("Напоминания на паузе до %1")},
    {QStringLiteral("*\nNo upcoming reminders to skip."), QStringLiteral("Нет предстоящих напоминаний для пропуска.")},
    {QStringLiteral("*\nThe next reminder cannot be skipped automatically."), QStringLiteral("Следующее напоминание нельзя пропустить автоматически.")},
    {QStringLiteral("*\nSkipped next reminder: %1"), QStringLiteral("Пропущено следующее напоминание: %1")},
    {QStringLiteral("*\nGood morning"), QStringLiteral("Доброе утро")},
    {QStringLiteral("*\nGood afternoon"), QStringLiteral("Добрый день")},
    {QStringLiteral("*\nGood evening"), QStringLiteral("Добрый вечер")},
    {QStringLiteral("*\n%1. Here's your day at a glance."), QStringLiteral("%1. Вот план на сегодня.")},
    {QStringLiteral("*\n20-20-20 eye rest"), QStringLiteral("Отдых для глаз 20-20-20")},
    {QStringLiteral("*\nShort eye-rest reminder suggested during onboarding."), QStringLiteral("Короткий перерыв для глаз, добавленный при настройке.")},
    {QStringLiteral("*\nOverview of upcoming reminders, active break profiles and today's events."), QStringLiteral("Обзор предстоящих напоминаний, активных профилей и событий.")},
    {QStringLiteral("*\nManage one-time and recurring reminders."), QStringLiteral("Управление разовыми и регулярными напоминаниями.")},
    {QStringLiteral("*\nTune reminder modes, break duration and snooze rules."), QStringLiteral("Настройка режимов напоминаний, перерывов и правил откладывания.")},
    {QStringLiteral("*\nCheck local reminder outcomes and adherence trends."), QStringLiteral("Локальная статистика выполнения напоминаний.")},
    {QStringLiteral("*\nAdjust startup behavior, quiet hours, theme and language."), QStringLiteral("Настройка автозагрузки, тихих часов, темы и языка.")},
    {QStringLiteral("*\nActive profiles"), QStringLiteral("Активные профили")},
    {QStringLiteral("*\nToday's events"), QStringLiteral("События сегодня")},
    {QStringLiteral("*\nManage profiles"), QStringLiteral("Управление профилями")},
    {QStringLiteral("*\nManage events"), QStringLiteral("Управление событиями")},
    {QStringLiteral("*\nProfile"), QStringLiteral("Профиль")},
    {QStringLiteral("*\nMode"), QStringLiteral("Режим")},
    {QStringLiteral("*\nCadence"), QStringLiteral("Интервал")},
    {QStringLiteral("*\nTime"), QStringLiteral("Время")},
    {QStringLiteral("*\nTitle"), QStringLiteral("Название")},
    {QStringLiteral("*\nRepeat"), QStringLiteral("Повтор")},
    {QStringLiteral("*\nNext break"), QStringLiteral("Следующий перерыв")},
    {QStringLiteral("*\nNo break scheduled"), QStringLiteral("Нет перерывов")},
    {QStringLiteral("*\n%1 min work / %2 min break"), QStringLiteral("%1 мин работы / %2 мин отдыха")},
    {QStringLiteral("*\nEnabled"), QStringLiteral("Включено")},
    {QStringLiteral("*\nPaused"), QStringLiteral("На паузе")},
    {QStringLiteral("*\nEvents"), QStringLiteral("События")},
    {QStringLiteral("*\nOne-time and recurring reminders with profile-based behavior."), QStringLiteral("Разовые и повторяющиеся напоминания с привязкой к профилям.")},
    {QStringLiteral("*\nNext trigger"), QStringLiteral("Следующее")},
    {QStringLiteral("*\nState"), QStringLiteral("Статус")},
    {QStringLiteral("*\nProfiles"), QStringLiteral("Профили")},
    {QStringLiteral("*\nProfiles define reminder intensity, snooze limits and break behavior."), QStringLiteral("Профили задают интенсивность, лимиты откладывания и перерывы.")},
    {QStringLiteral("*\nCreate at least one profile to control reminder behavior."), QStringLiteral("Создайте хотя бы один профиль для управления напоминаниями.")},
    {QStringLiteral("*\nName"), QStringLiteral("Название")},
    {QStringLiteral("*\nScope"), QStringLiteral("Назначение")},
    {QStringLiteral("*\nOrigin"), QStringLiteral("Источник")},
    {QStringLiteral("*\n%1 / %2 min"), QStringLiteral("%1 / %2 мин")},
    {QStringLiteral("*\n%1 x %2 min"), QStringLiteral("%1 раз по %2 мин")},
    {QStringLiteral("*\nBuilt-in"), QStringLiteral("Встроенный")},
    {QStringLiteral("*\nCustom"), QStringLiteral("Кастомный")},
    {QStringLiteral("*\nCompleted"), QStringLiteral("Выполнено")},
    {QStringLiteral("*\nSkipped"), QStringLiteral("Пропущено")},
    {QStringLiteral("*\nSnoozed"), QStringLiteral("Отложено")},
    {QStringLiteral("*\nDate"), QStringLiteral("Дата")},
    {QStringLiteral("*\nLightweight local analytics for completed, skipped and snoozed reminders."), QStringLiteral("Локальная статистика по выполненным и пропущенным напоминаниям.")},
    {QStringLiteral("*\nStatistics will appear after reminders start running."), QStringLiteral("Статистика появится после запуска напоминаний.")},
    {QStringLiteral("*\nBreak completed"), QStringLiteral("Сделано перерывов")},
    {QStringLiteral("*\nBreak missed"), QStringLiteral("Пропущено перерывов")},
    {QStringLiteral("*\nControl startup behavior, quiet hours, theme and interface language."), QStringLiteral("Управление запуском, режимом тишины, темой и языком интерфейса.")},
    {QStringLiteral("*\nDefault profile"), QStringLiteral("Профиль по умолчанию")},
    {QStringLiteral("*\nPause reminders until"), QStringLiteral("Пауза до")},
    {QStringLiteral("*\nQuiet hours start"), QStringLiteral("Начало тихих часов")},
    {QStringLiteral("*\nQuiet hours end"), QStringLiteral("Конец тихих часов")},
    {QStringLiteral("*\nSoft reminders"), QStringLiteral("Мягкие напоминания")},
    {QStringLiteral("*\nPersistent reminders"), QStringLiteral("Настойчивые напоминания")},
    {QStringLiteral("*\nStrict breaks"), QStringLiteral("Строгие перерывы")},
    {QStringLiteral("*\nTray integration is available on this desktop environment."), QStringLiteral("Интеграция с треем доступна в вашей среде.")},
    {QStringLiteral("*\nTray integration is not available right now, so the main window stays as the primary control surface."), QStringLiteral("Трей сейчас недоступен, поэтому главное окно остаётся открытым.")},
    {QStringLiteral("*\nWelcome to Deadliner"), QStringLiteral("Добро пожаловать в Deadliner")},
    {QStringLiteral("*\nSet up your reminder workspace"), QStringLiteral("Настройте ваше рабочее пространство")},
    {QStringLiteral("*\nChoose interface language"), QStringLiteral("Выберите язык интерфейса")},
    {QStringLiteral("*\nThe application supports Russian and English out of the box, and you can switch later in Settings without restarting."), QStringLiteral("Приложение поддерживает русский и английский, язык всегда можно сменить в настройках.")},
    {QStringLiteral("*\nBreak reminders that respect your day"), QStringLiteral("Напоминания, которые уважают ваше время")},
    {QStringLiteral("*\nDeadliner starts gently, escalates only when needed, and keeps quiet hours configurable so reminders stay useful instead of annoying."), QStringLiteral("Deadliner начинает мягко и использует тихие часы, чтобы напоминания были полезными, а не раздражающими.")},
    {QStringLiteral("*\nDesktop-first workflow"), QStringLiteral("Удобство на десктопе")},
    {QStringLiteral("*\nMost daily actions are available from the system tray, while the main window keeps event, profile and statistics management organized."), QStringLiteral("Частые действия доступны из трея, а главное окно управляет событиями и статистикой.")},
    {QStringLiteral("*\nRecommended starter setup"), QStringLiteral("Рекомендуемый пресет")},
    {QStringLiteral("*\nYou can start with a safe eye-rest preset now and fine-tune it later in Profiles."), QStringLiteral("Вы можете начать с базового правила отдыха для глаз и настроить его позже.")},
    {QStringLiteral("*\nEnable the recommended 20-20-20 inspired eye-rest rule (20 min work / 1 min break)."), QStringLiteral("Включить рекомендуемое правило 20-20-20 (20 мин работы / 1 мин отдыха глаз).")},
    {QStringLiteral("*\nBack"), QStringLiteral("Назад")},
    {QStringLiteral("*\nNext"), QStringLiteral("Далее")},
    {QStringLiteral("*\nFinish"), QStringLiteral("Завершить")},
    {QStringLiteral("*\nEvents inherit reminder mode and snooze policy from the selected profile."), QStringLiteral("События наследуют режим напоминания и лимиты от профиля.")},
    {QStringLiteral("*\nProfile behavior"), QStringLiteral("Поведение")},
    {QStringLiteral("*\nStart at / anchor"), QStringLiteral("Время начала / отсчёта")},
    {QStringLiteral("*\nRecurrence"), QStringLiteral("Повторение")},
    {QStringLiteral("*\n%1 mode, snooze %2 x %3 min, break %4 min"), QStringLiteral("Режим %1, отложить %2 раз по %3 мин, перерыв %4 мин")},
    {QStringLiteral("*\nRequire post-break confirmation"), QStringLiteral("Требовать подтверждения возвращения")},
    {QStringLiteral("*\nInterval (minutes)"), QStringLiteral("Интервал (минуты)")},
    {QStringLiteral("*\nBreak duration (minutes)"), QStringLiteral("Длительность перерыва (мин)")},
    {QStringLiteral("*\nMax snoozes"), QStringLiteral("Макс. откладываний")},
    {QStringLiteral("*\nSnooze minutes"), QStringLiteral("Минут при откладывании")},
    {QStringLiteral("*\nQuiet hours policy"), QStringLiteral("Правило тихих часов")},
    {QStringLiteral("*\nReminder mode"), QStringLiteral("Режим напоминания")},
    {QStringLiteral("*\nKind"), QStringLiteral("Тип профиля")},
    {QStringLiteral("*\nDatabase error"), QStringLiteral("Ошибка базы данных")},
    {QStringLiteral("*\nManual break"), QStringLiteral("Ручной перерыв")},
    {QStringLiteral("*\nStarted from the tray menu."), QStringLiteral("Запущено из меню трея.")},
    {QStringLiteral("*\nReminders paused until %1"), QStringLiteral("Напоминания на паузе до %1")},
    {QStringLiteral("*\nNo upcoming reminders to skip."), QStringLiteral("Нет предстоящих напоминаний для пропуска.")},
    {QStringLiteral("*\nThe next reminder cannot be skipped automatically."), QStringLiteral("Следующее напоминание нельзя пропустить автоматически.")},
    {QStringLiteral("*\nSkipped next reminder: %1"), QStringLiteral("Пропущено следующее напоминание: %1")},
    {QStringLiteral("*\nGood morning"), QStringLiteral("Доброе утро")},
    {QStringLiteral("*\nGood afternoon"), QStringLiteral("Добрый день")},
    {QStringLiteral("*\nGood evening"), QStringLiteral("Добрый вечер")},
    {QStringLiteral("*\n%1. Here's your day at a glance."), QStringLiteral("%1. Вот план на сегодня.")},
    {QStringLiteral("*\n20-20-20 eye rest"), QStringLiteral("Отдых для глаз 20-20-20")},
    {QStringLiteral("*\nShort eye-rest reminder suggested during onboarding."), QStringLiteral("Короткий перерыв для глаз, добавленный при настройке.")},
            {QStringLiteral("*\nDefault profile"), QStringLiteral("Профиль по умолчанию")},
            {QStringLiteral("*\nPause reminders until"), QStringLiteral("Пауза до")},
            {QStringLiteral("*\nQuiet hours start"), QStringLiteral("Начало тихих часов")},
            {QStringLiteral("*\nQuiet hours end"), QStringLiteral("Конец тихих часов")},
            {QStringLiteral("*\nSoft reminders"), QStringLiteral("Мягкие напоминания")},
            {QStringLiteral("*\nPersistent reminders"), QStringLiteral("Настойчивые напоминания")},
            {QStringLiteral("*\nStrict breaks"), QStringLiteral("Строгие перерывы")},
            {QStringLiteral("*\nControl startup behavior, quiet hours, theme and interface language."),
             QStringLiteral("Управление запуском, тихими часами, темой и языком интерфейса.")},
            {QStringLiteral("*\nTray integration is available on this desktop environment."),
             QStringLiteral("Интеграция с треем доступна в вашей среде рабочего стола.")},
            {QStringLiteral("*\nTray integration is not available right now, so the main window stays as the primary control surface."),
             QStringLiteral("Трей сейчас недоступен — главное окно остаётся основным интерфейсом управления.")},

            // Today page
            {QStringLiteral("*\nActive profiles"), QStringLiteral("Активные профили")},
            {QStringLiteral("*\nToday's events"), QStringLiteral("События сегодня")},
            {QStringLiteral("*\nManage profiles"), QStringLiteral("Управление профилями")},
            {QStringLiteral("*\nManage events"), QStringLiteral("Управление событиями")},
            {QStringLiteral("*\nProfile"), QStringLiteral("Профиль")},
            {QStringLiteral("*\nType"), QStringLiteral("Тип")},
            {QStringLiteral("*\nMode"), QStringLiteral("Режим")},
            {QStringLiteral("*\nCadence"), QStringLiteral("Интервал")},
            {QStringLiteral("*\nTime"), QStringLiteral("Время")},
            {QStringLiteral("*\nTitle"), QStringLiteral("Название")},
            {QStringLiteral("*\nRepeat"), QStringLiteral("Повтор")},
            {QStringLiteral("*\nNext break"), QStringLiteral("Следующий перерыв")},
            {QStringLiteral("*\nNo break scheduled"), QStringLiteral("Нет запланированных перерывов")},
            {QStringLiteral("*\n%1 min work / %2 min break"), QStringLiteral("%1 мин работы / %2 мин отдыха")},
            {QStringLiteral("*\nEnabled"), QStringLiteral("Включено")},
            {QStringLiteral("*\nPaused"), QStringLiteral("На паузе")},

            // Events page
            {QStringLiteral("*\nOne-time and recurring reminders with profile-based behavior."),
             QStringLiteral("Разовые и повторяющиеся напоминания с привязкой к профилям.")},
            {QStringLiteral("*\nNext trigger"), QStringLiteral("Следующее")},
            {QStringLiteral("*\nState"), QStringLiteral("Статус")},

            // Profiles page
            {QStringLiteral("*\nProfiles define reminder intensity, snooze limits and break behavior."),
             QStringLiteral("Профили задают интенсивность, лимиты откладывания и поведение перерывов.")},
            {QStringLiteral("*\nCreate at least one profile to control reminder behavior."),
             QStringLiteral("Создайте хотя бы один профиль для управления напоминаниями.")},
            {QStringLiteral("*\nName"), QStringLiteral("Название")},
            {QStringLiteral("*\nScope"), QStringLiteral("Назначение")},
            {QStringLiteral("*\nOrigin"), QStringLiteral("Источник")},
            {QStringLiteral("*\n%1 / %2 min"), QStringLiteral("%1 / %2 мин")},
            {QStringLiteral("*\n%1 x %2 min"), QStringLiteral("%1 раз по %2 мин")},
            {QStringLiteral("*\nBuilt-in"), QStringLiteral("Встроенный")},
            {QStringLiteral("*\nCustom"), QStringLiteral("Пользовательский")},
            {QStringLiteral("*\nMixed"), QStringLiteral("Смешанный")},

            // Statistics page
            {QStringLiteral("*\nCompleted"), QStringLiteral("Выполнено")},
            {QStringLiteral("*\nSkipped"), QStringLiteral("Пропущено")},
            {QStringLiteral("*\nSnoozed"), QStringLiteral("Отложено")},
            {QStringLiteral("*\nDate"), QStringLiteral("Дата")},
            {QStringLiteral("*\nLightweight local analytics for completed, skipped and snoozed reminders."),
             QStringLiteral("Локальная статистика по выполненным и пропущенным напоминаниям.")},
            {QStringLiteral("*\nStatistics will appear after reminders start running."),
             QStringLiteral("Статистика появится после первых сработавших напоминаний.")},
            {QStringLiteral("*\nBreak completed"), QStringLiteral("Сделано перерывов")},
            {QStringLiteral("*\nBreak missed"), QStringLiteral("Пропущено перерывов")},

            // Onboarding
            {QStringLiteral("*\nWelcome to Deadliner"), QStringLiteral("Добро пожаловать в Deadliner")},
            {QStringLiteral("*\nSet up your reminder workspace"),
             QStringLiteral("Настройте рабочее пространство")},
            {QStringLiteral("*\nChoose interface language"),
             QStringLiteral("Выберите язык интерфейса")},
            {QStringLiteral("*\nThe application supports Russian and English out of the box, and you can switch later in Settings without restarting."),
             QStringLiteral("Приложение поддерживает русский и английский языки; сменить язык можно в настройках без перезапуска.")},
            {QStringLiteral("*\nBreak reminders that respect your day"),
             QStringLiteral("Напоминания, которые уважают ваше время")},
            {QStringLiteral("*\nDeadliner starts gently, escalates only when needed, and keeps quiet hours configurable so reminders stay useful instead of annoying."),
             QStringLiteral("Deadliner начинает мягко, усиливает только при необходимости и соблюдает тихие часы — чтобы напоминания оставались полезными.")},
            {QStringLiteral("*\nDesktop-first workflow"),
             QStringLiteral("Удобство на рабочем столе")},
            {QStringLiteral("*\nMost daily actions are available from the system tray, while the main window keeps event, profile and statistics management organized."),
             QStringLiteral("Частые действия доступны из трея; главное окно хранит события, профили и статистику.")},
            {QStringLiteral("*\nRecommended starter setup"),
             QStringLiteral("Рекомендуемый стартовый набор")},
            {QStringLiteral("*\nYou can start with a safe eye-rest preset now and fine-tune it later in Profiles."),
             QStringLiteral("Можно начать с готового правила отдыха для глаз и настроить его позже в Профилях.")},
            {QStringLiteral("*\nEnable the recommended 20-20-20 inspired eye-rest rule (20 min work / 1 min break)."),
             QStringLiteral("Включить правило 20-20-20 (20 мин работы / 1 мин отдыха для глаз).")},

            // Event / profile dialog fields
            {QStringLiteral("*\nDescription"), QStringLiteral("Описание")},
            {QStringLiteral("*\nEvents inherit reminder mode and snooze policy from the selected profile."),
             QStringLiteral("События наследуют режим напоминания и политику откладывания от профиля.")},
            {QStringLiteral("*\nProfile behavior"), QStringLiteral("Поведение профиля")},
            {QStringLiteral("*\nStart at / anchor"), QStringLiteral("Время начала / отсчёта")},
            {QStringLiteral("*\nRecurrence"), QStringLiteral("Повторение")},
            {QStringLiteral("*\nOne-time event"), QStringLiteral("Разовое событие")},
            {QStringLiteral("*\nMode: %1\nSnooze: %2 x %3 min\nBreak: %4 min"),
             QStringLiteral("Режим: %1\nОткладывание: %2 x %3 мин\nПерерыв: %4 мин")},
            {QStringLiteral("*\nRequire post-break confirmation"),
             QStringLiteral("Требовать подтверждения возвращения")},
            {QStringLiteral("*\nAllow skip"), QStringLiteral("Разрешить пропуск")},
            {QStringLiteral("*\nInterval (minutes)"), QStringLiteral("Интервал (минуты)")},
            {QStringLiteral("*\nBreak duration (minutes)"), QStringLiteral("Длительность перерыва (мин)")},
            {QStringLiteral("*\nMax snoozes"), QStringLiteral("Макс. откладываний")},
            {QStringLiteral("*\nSnooze minutes"), QStringLiteral("Минут при откладывании")},
            {QStringLiteral("*\nQuiet hours policy"), QStringLiteral("Правило тихих часов")},
            {QStringLiteral("*\nReminder mode"), QStringLiteral("Режим напоминания")},
            {QStringLiteral("*\nKind"), QStringLiteral("Тип профиля")},

            // Tray / controller messages
            {QStringLiteral("*\nManual break"), QStringLiteral("Ручной перерыв")},
            {QStringLiteral("*\nStarted from the tray menu."),
             QStringLiteral("Запущено из меню трея.")},
            {QStringLiteral("*\nReminders paused until %1"),
             QStringLiteral("Напоминания на паузе до %1")},
            {QStringLiteral("*\nNo upcoming reminders to skip."),
             QStringLiteral("Нет предстоящих напоминаний для пропуска.")},
            {QStringLiteral("*\nThe next reminder cannot be skipped automatically."),
             QStringLiteral("Следующее напоминание невозможно пропустить автоматически.")},
            {QStringLiteral("*\nSkip next reminder"),
             QStringLiteral("Пропустить следующее напоминание")},
            {QStringLiteral("*\nSkipped next reminder: %1"),
             QStringLiteral("Пропущено следующее напоминание: %1")},

            // Test event descriptions
            {QStringLiteral("*\nQuick soft reminder for notification popup testing."),
             QStringLiteral("Быстрое мягкое напоминание для проверки уведомления.")},
            {QStringLiteral("*\nQuick persistent reminder for dialog testing."),
             QStringLiteral("Быстрое настойчивое напоминание для проверки диалога.")},
            {QStringLiteral("*\nQuick break reminder for fullscreen break testing."),
             QStringLiteral("Быстрое напоминание-перерыв для проверки полноэкранного режима.")},

            // Greetings
            {QStringLiteral("*\nGood morning"), QStringLiteral("Доброе утро")},
            {QStringLiteral("*\nGood afternoon"), QStringLiteral("Добрый день")},
            {QStringLiteral("*\nGood evening"), QStringLiteral("Добрый вечер")},
            {QStringLiteral("*\n%1. Here's your day at a glance."),
             QStringLiteral("%1. Вот план на сегодня.")},

            // Onboarding preset
            {QStringLiteral("*\n20-20-20 eye rest"), QStringLiteral("Правило 20-20-20")},
            {QStringLiteral("*\nShort eye-rest reminder suggested during onboarding."),
             QStringLiteral("Короткий перерыв для глаз, предложенный при настройке приложения.")},
        };

    } // namespace

    LanguageManager::LanguageManager(QObject *parent)
        : QObject(parent)
    {
    }

    LanguageManager::~LanguageManager()
    {
        if (m_translator != nullptr)
        {
            qApp->removeTranslator(m_translator);
            delete m_translator;
        }
    }

    QString LanguageManager::systemDefaultLanguage()
    {
        return QLocale::system().language() == QLocale::Russian ? QStringLiteral("ru") : QStringLiteral("en");
    }

    QString LanguageManager::normalizedLanguage(const QString &languageCode)
    {
        const QString canonical = languageCode.trimmed().toLower();
        if (canonical.isEmpty() || canonical == QStringLiteral("system"))
        {
            return systemDefaultLanguage();
        }

        if (canonical.startsWith(QStringLiteral("ru"))
            || canonical == QStringLiteral("russian")
            || canonical == QStringLiteral("русский"))
        {
            return QStringLiteral("ru");
        }

        return QStringLiteral("en");
    }

    QString LanguageManager::canonicalLanguageSetting(const QString &languageCode) const
    {
        const QString canonical = languageCode.trimmed().toLower();
        if (canonical.isEmpty() || canonical == QStringLiteral("system"))
        {
            return QStringLiteral("system");
        }

        if (canonical.startsWith(QStringLiteral("ru"))
            || canonical == QStringLiteral("russian")
            || canonical == QStringLiteral("русский"))
        {
            return QStringLiteral("ru");
        }

        if (canonical.startsWith(QStringLiteral("en"))
            || canonical == QStringLiteral("english")
            || canonical == QStringLiteral("английский"))
        {
            return QStringLiteral("en");
        }

        return QStringLiteral("system");
    }

    QString LanguageManager::resolveStartupLanguage(const QString &storedLanguage) const
    {
        return normalizedLanguage(canonicalLanguageSetting(storedLanguage));
    }

    QString LanguageManager::currentLanguage() const
    {
        return m_currentLanguage;
    }

    bool LanguageManager::applyLanguage(const QString &languageCode)
    {
        const QString normalized = normalizedLanguage(languageCode);
        if (normalized == m_currentLanguage && m_translator != nullptr)
        {
            return true;
        }

        if (m_translator != nullptr)
        {
            qApp->removeTranslator(m_translator);
            delete m_translator;
            m_translator = nullptr;
        }

        if (normalized == QStringLiteral("ru"))
        {
            auto *translator = new DictionaryTranslator(normalized, this);
            qApp->installTranslator(translator);
            m_translator = translator;
        }

        m_currentLanguage = normalized;
        emit languageApplied(m_currentLanguage);
        return true;
    }

} // namespace deadliner::ui
