#pragma once

#include "application/services.h"
#include "domain/models.h"
#include "infrastructure/platform/autostart_manager.h"
#include "infrastructure/settings/settings_store.h"
#include "infrastructure/storage/repository.h"
#include "ui/i18n/language_manager.h"
#include "ui/main_window.h"
#include "ui/common/navigation.h"
#include "ui/tray_controller.h"

#include <QHash>
#include <QObject>

class QTimer;

namespace deadliner::app {

class AppController : public QObject {
    Q_OBJECT

public:
    explicit AppController(QObject *parent = nullptr);

    bool initialize();
    void refreshState();
    void handleOccurrence(const domain::ReminderOccurrence &occurrence);

private slots:
    void showMainWindow();
    void showSection(ui::MainSection section);
    void addEvent();
    void editEvent(qint64 id);
    void deleteEvent(qint64 id);
    void addProfile();
    void editProfile(qint64 id);
    void deleteProfile(qint64 id);
    void saveSettings(domain::AppSettings settings, domain::QuietHoursPolicy policy);
    void startBreakNow();
    void snoozeAll(int minutes);
    void pauseReminders(int minutes);
    void skipNextReminder();
    void quitApplication();

private:
    void applyAppearanceSettings();
    void installRuntimeResyncHooks();
    bool runOnboardingIfNeeded();
    void ensureRecommendedOnboardingPreset(bool enabled);
    void ensureStartupTestEvents();
    domain::ReminderProfile profileForId(qint64 id) const;
    domain::ReminderEvent eventForId(qint64 id) const;
    domain::QuietHoursPolicy quietPolicyForId(qint64 id) const;
    bool saveEventOrWarn(const domain::ReminderEvent &event, const QString &message);
    bool saveProfileOrWarn(const domain::ReminderProfile &profile, const QString &message);
    void persistOutcome(const domain::ReminderOccurrence &occurrence,
                        domain::SeverityMode shownMode,
                        domain::ReminderResult result,
                        int breakActualSeconds = 0);

    infrastructure::Repository m_repository;
    infrastructure::SettingsStore m_settingsStore;
    infrastructure::AutostartManager m_autostartManager;
    application::ReminderScheduler m_scheduler;
    application::StatisticsService m_statisticsService;
    ui::LanguageManager m_languageManager;
    ui::MainWindow m_mainWindow;
    ui::TrayController m_trayController;
    domain::AppSettings m_settings;
    infrastructure::RepositoryState m_state;
    QTimer *m_runtimeResyncTimer = nullptr;
    bool m_handlingOccurrence = false;
};

}  // namespace deadliner::app
