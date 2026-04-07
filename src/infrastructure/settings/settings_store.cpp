#include "infrastructure/settings/settings_store.h"

namespace deadliner::infrastructure {

namespace {

QString canonicalLanguage(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized.isEmpty() || normalized == QStringLiteral("system")) {
        return QStringLiteral("system");
    }
    if (normalized.startsWith(QStringLiteral("ru"))
        || normalized == QStringLiteral("russian")
        || normalized == QStringLiteral("русский")) {
        return QStringLiteral("ru");
    }
    if (normalized.startsWith(QStringLiteral("en"))
        || normalized == QStringLiteral("english")
        || normalized == QStringLiteral("английский")) {
        return QStringLiteral("en");
    }
    return QStringLiteral("system");
}

QString canonicalTheme(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized.isEmpty() || normalized == QStringLiteral("system")) {
        return QStringLiteral("system");
    }
    if (normalized == QStringLiteral("dark") || normalized == QStringLiteral("темная")
        || normalized == QStringLiteral("тёмная")) {
        return QStringLiteral("dark");
    }
    if (normalized == QStringLiteral("light") || normalized == QStringLiteral("светлая")) {
        return QStringLiteral("light");
    }
    return QStringLiteral("system");
}

bool hasExistingUserSetup(const QSettings &settings)
{
    if (settings.contains(QStringLiteral("general/onboarding_completed"))) {
        return true;
    }

    return settings.contains(QStringLiteral("ui/language"))
        || settings.contains(QStringLiteral("ui/minimize_to_tray"))
        || settings.contains(QStringLiteral("ui/close_to_tray"))
        || settings.contains(QStringLiteral("ui/theme"))
        || settings.contains(QStringLiteral("ui/last_window_geometry"))
        || settings.contains(QStringLiteral("general/launch_on_startup"))
        || settings.contains(QStringLiteral("general/start_minimized"))
        || settings.contains(QStringLiteral("general/default_profile_id"))
        || settings.contains(QStringLiteral("general/pause_until"));
}

} // namespace

SettingsStore::SettingsStore()
    : m_settings(QStringLiteral("Deadliner"), QStringLiteral("Deadliner"))
{
}

domain::AppSettings SettingsStore::load() const
{
    domain::AppSettings settings;
    bool needsMigrationSave = false;

    settings.minimizeToTray = m_settings.value(QStringLiteral("ui/minimize_to_tray"), true).toBool();
    settings.closeToTray = m_settings.value(QStringLiteral("ui/close_to_tray"), true).toBool();
    settings.lastWindowGeometry = m_settings.value(QStringLiteral("ui/last_window_geometry")).toByteArray();
    const QString storedLanguage = m_settings.value(QStringLiteral("ui/language"), settings.language).toString();
    const QString storedTheme = m_settings.value(QStringLiteral("ui/theme"), settings.theme).toString();
    settings.language = canonicalLanguage(storedLanguage);
    settings.theme = canonicalTheme(storedTheme);
    settings.launchOnStartup = m_settings.value(QStringLiteral("general/launch_on_startup"), false).toBool();
    settings.startMinimized = m_settings.value(QStringLiteral("general/start_minimized"), false).toBool();
    settings.onboardingCompleted = m_settings.value(QStringLiteral("general/onboarding_completed"), false).toBool();
    settings.defaultProfileId = m_settings.value(QStringLiteral("general/default_profile_id"), 0).toLongLong();
    settings.pauseUntil = m_settings.value(QStringLiteral("general/pause_until")).toDateTime();
    settings.logLevel = m_settings.value(QStringLiteral("debug/log_level"), settings.logLevel).toString();
    settings.trayIcon = m_settings.value(QStringLiteral("ui/tray_icon"), settings.trayIcon).toString();

    if (storedLanguage != settings.language || storedTheme != settings.theme) {
        needsMigrationSave = true;
    }

    if (!settings.onboardingCompleted && hasExistingUserSetup(m_settings)) {
        settings.onboardingCompleted = true;
        needsMigrationSave = true;
    }

    if (needsMigrationSave) {
        save(settings);
    }

    return settings;
}

void SettingsStore::save(const domain::AppSettings &settings) const
{
    m_settings.setValue(QStringLiteral("ui/minimize_to_tray"), settings.minimizeToTray);
    m_settings.setValue(QStringLiteral("ui/close_to_tray"), settings.closeToTray);
    m_settings.setValue(QStringLiteral("ui/last_window_geometry"), settings.lastWindowGeometry);
    m_settings.setValue(QStringLiteral("ui/language"), canonicalLanguage(settings.language));
    m_settings.setValue(QStringLiteral("ui/theme"), canonicalTheme(settings.theme));
    m_settings.setValue(QStringLiteral("ui/tray_icon"), settings.trayIcon);
    m_settings.setValue(QStringLiteral("general/launch_on_startup"), settings.launchOnStartup);
    m_settings.setValue(QStringLiteral("general/start_minimized"), settings.startMinimized);
    m_settings.setValue(QStringLiteral("general/onboarding_completed"), settings.onboardingCompleted);
    m_settings.setValue(QStringLiteral("general/default_profile_id"), settings.defaultProfileId);
    m_settings.setValue(QStringLiteral("general/pause_until"), settings.pauseUntil);
    m_settings.setValue(QStringLiteral("debug/log_level"), settings.logLevel);
    m_settings.sync();
}

}  // namespace deadliner::infrastructure
