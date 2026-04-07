#pragma once

#include <QObject>
#include <QString>

class QTranslator;

namespace deadliner::ui
{

    class LanguageManager : public QObject
    {
        Q_OBJECT

    public:
        explicit LanguageManager(QObject *parent = nullptr);
        ~LanguageManager() override;

        QString canonicalLanguageSetting(const QString &languageCode) const;
        QString resolveStartupLanguage(const QString &storedLanguage) const;
        QString currentLanguage() const;
        bool applyLanguage(const QString &languageCode);

        static QString systemDefaultLanguage();
        static QString normalizedLanguage(const QString &languageCode);

    signals:
        void languageApplied(const QString &languageCode);

    private:
        QTranslator *m_translator = nullptr;
        QString m_currentLanguage = QStringLiteral("en");
    };

} // namespace deadliner::ui
