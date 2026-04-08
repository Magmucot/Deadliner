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

        QString fallbackKey(const char *context, const char *sourceText)
        {
            const QString ctx = QString::fromUtf8(context ? context : "");
            return ctx + QLatin1Char('\n') + QString::fromUtf8(sourceText ? sourceText : "");
        }

        class HybridTranslator final : public QTranslator
        {
        public:
            explicit HybridTranslator(QString languageCode, QObject *parent = nullptr)
                : QTranslator(parent), m_languageCode(std::move(languageCode))
            {
            }

            bool loadCatalog()
            {
                if (m_languageCode != QStringLiteral("ru"))
                {
                    return false;
                }

                return m_catalog.load(QLocale(QStringLiteral("ru_RU")),
                                      QStringLiteral("deadliner"),
                                      QStringLiteral("_"),
                                      QStringLiteral(":/i18n"));
            }

            QString translate(const char *context,
                              const char *sourceText,
                              const char *disambiguation,
                              int n) const override
            {
                const QString fromCatalog = m_catalog.translate(context, sourceText, disambiguation, n);
                if (!fromCatalog.isEmpty())
                {
                    return fromCatalog;
                }

                const QString fromSharedCatalog = m_catalog.translate("deadliner", sourceText, disambiguation, n);
                if (!fromSharedCatalog.isEmpty())
                {
                    return fromSharedCatalog;
                }

                if (m_languageCode != QStringLiteral("ru") || sourceText == nullptr)
                {
                    return {};
                }

                const auto it = s_fallback.constFind(fallbackKey(context, sourceText));
                if (it != s_fallback.constEnd())
                {
                    return it.value();
                }

                const auto fallbackIt = s_fallback.constFind(QStringLiteral("*\n") + QString::fromUtf8(sourceText));
                return fallbackIt != s_fallback.constEnd() ? fallbackIt.value() : QString{};
            }

        private:
            QString m_languageCode;
            QTranslator m_catalog;

            static const QHash<QString, QString> s_fallback;
        };

        const QHash<QString, QString> HybridTranslator::s_fallback = {
            {QStringLiteral("*\nTray icon"), QStringLiteral("Иконка в трее")},
            {QStringLiteral("*\nVariant 1"), QStringLiteral("Вариант 1")},
            {QStringLiteral("*\nVariant 2"), QStringLiteral("Вариант 2")},
            {QStringLiteral("*\nSave failed"), QStringLiteral("Не удалось сохранить")},
            {QStringLiteral("*\nSelection required"), QStringLiteral("Нужно выбрать элемент")},
            {QStringLiteral("*\nBuilt-in profile"), QStringLiteral("Встроенный профиль")},
            {QStringLiteral("*\nBuilt-in profiles cannot be deleted."), QStringLiteral("Встроенные профили нельзя удалить.")},
            {QStringLiteral("*\nDatabase error"), QStringLiteral("Ошибка базы данных")},
            {QStringLiteral("*\nReminder is due"), QStringLiteral("Напоминание наступило")},
            {QStringLiteral("*\nManual break"), QStringLiteral("Ручной перерыв")},
            {QStringLiteral("*\nStarted from the tray menu."), QStringLiteral("Запущено из меню трея.")},
            {QStringLiteral("*\nReminders paused until %1"), QStringLiteral("Напоминания на паузе до %1")},
            {QStringLiteral("*\nNo upcoming reminders to skip."), QStringLiteral("Нет предстоящих напоминаний для пропуска.")},
            {QStringLiteral("*\nThe next reminder cannot be skipped automatically."), QStringLiteral("Следующее напоминание нельзя пропустить автоматически.")},
            {QStringLiteral("*\nSkipped next reminder: %1"), QStringLiteral("Пропущено следующее напоминание: %1")},
            {QStringLiteral("*\nThe event state could not be refreshed. Check for a duplicate title/date/profile combination."),
             QStringLiteral("Не удалось обновить состояние события. Проверьте, нет ли дубликата по названию, дате и профилю.")},
            {QStringLiteral("*\nThe reminder could not be updated while reminders are paused."),
             QStringLiteral("Не удалось обновить напоминание, пока напоминания стоят на паузе.")},
            {QStringLiteral("*\nThe reminder could not be updated after applying quiet hours."),
             QStringLiteral("Не удалось обновить напоминание после применения тихих часов.")},
            {QStringLiteral("*\nThe reminder could not be updated after it was handled."),
             QStringLiteral("Не удалось обновить напоминание после обработки.")},
            {QStringLiteral("*\nThe event could not be saved. Check for a duplicate title/date/profile combination."),
             QStringLiteral("Не удалось сохранить событие. Проверьте, нет ли дубликата по названию, дате и профилю.")},
            {QStringLiteral("*\nThe profile could not be saved. Check for a duplicate profile name."),
             QStringLiteral("Не удалось сохранить профиль. Проверьте, нет ли профиля с таким именем.")},
            {QStringLiteral("*\nA reminder could not be updated while snoozing all reminders."),
             QStringLiteral("Не удалось обновить одно из напоминаний при массовом откладывании.")},
            {QStringLiteral("*\nA reminder could not be updated while pausing reminders."),
             QStringLiteral("Не удалось обновить одно из напоминаний при постановке на паузу.")},
            {QStringLiteral("*\nThe reminder could not be updated after skipping it."),
             QStringLiteral("Не удалось обновить напоминание после пропуска.")},
            {QStringLiteral("*\nThe onboarding profile could not be saved."),
             QStringLiteral("Не удалось сохранить профиль первичной настройки.")},
            {QStringLiteral("*\nThe onboarding event could not be saved."),
             QStringLiteral("Не удалось сохранить событие первичной настройки.")},
            {QStringLiteral("*\nA test profile could not be updated."),
             QStringLiteral("Не удалось обновить тестовый профиль.")},
            {QStringLiteral("*\nA test profile could not be saved."),
             QStringLiteral("Не удалось сохранить тестовый профиль.")},
            {QStringLiteral("*\nA test event could not be saved."),
             QStringLiteral("Не удалось сохранить тестовое событие.")},
            {QStringLiteral("*\nQuick soft reminder for notification popup testing."),
             QStringLiteral("Быстрое мягкое напоминание для проверки всплывающего уведомления.")},
            {QStringLiteral("*\nQuick persistent reminder for dialog testing."),
             QStringLiteral("Быстрое настойчивое напоминание для проверки диалога.")},
            {QStringLiteral("*\nQuick break reminder for fullscreen break testing."),
             QStringLiteral("Быстрое напоминание-перерыв для проверки полноэкранного режима.")},
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

        if (canonical.startsWith(QStringLiteral("ru")) || canonical == QStringLiteral("russian") || canonical == QStringLiteral("русский"))
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

        if (canonical.startsWith(QStringLiteral("ru")) || canonical == QStringLiteral("russian") || canonical == QStringLiteral("русский"))
        {
            return QStringLiteral("ru");
        }

        if (canonical.startsWith(QStringLiteral("en")) || canonical == QStringLiteral("english") || canonical == QStringLiteral("английский"))
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
            auto *translator = new HybridTranslator(normalized, this);
            translator->loadCatalog();
            qApp->installTranslator(translator);
            m_translator = translator;
        }

        m_currentLanguage = normalized;
        emit languageApplied(m_currentLanguage);
        return true;
    }

} // namespace deadliner::ui
