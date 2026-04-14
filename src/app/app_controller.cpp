#include "app/app_controller.h"

#include "domain/logic.h"
#include "ui/break_window.h"
#include "ui/event_dialog.h"
#include "ui/onboarding/onboarding_dialog.h"
#include "ui/profile_dialog.h"
#include "ui/reminder_dialog.h"
#include "ui/settings/theme_manager.h"

#include <QApplication>
#include <QDate>
#include <QDateTime>
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QMessageBox>
#include <QStandardPaths>
#include <QSystemTrayIcon>
#include <QTimer>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif

namespace deadliner::app
{

    namespace
    {

        constexpr auto kTestSoftProfileName = "[Test] Soft popup";
        constexpr auto kTestPersistentProfileName = "[Test] Persistent dialog";
        constexpr auto kTestBreakProfileName = "[Test] Break overlay";

        constexpr auto kTestSoftEventTitle = "[Test] Soft popup in 15s";
        constexpr auto kTestPersistentEventTitle = "[Test] Persistent dialog in 30s";
        constexpr auto kTestBreakEventTitle = "[Test] Break overlay in 45s";

        QHash<qint64, domain::ReminderProfile> profileMap(const QList<domain::ReminderProfile> &profiles)
        {
            QHash<qint64, domain::ReminderProfile> map;
            for (const auto &profile : profiles)
            {
                map.insert(profile.id, profile);
            }
            return map;
        }

        QString dataPath(const QString &fileName)
        {
            const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            QDir().mkpath(base);
            return QDir(base).filePath(fileName);
        }

        bool shouldSeedStartupTestData()
        {
            return qEnvironmentVariableIsSet("DEADLINER_ENABLE_TEST_DATA");
        }

    } // namespace

    AppController::AppController(QObject *parent)
        : QObject(parent)
    {
        connect(&m_scheduler, &application::ReminderScheduler::occurrenceDue, this, &AppController::handleOccurrence);
        connect(&m_trayController, &ui::TrayController::openMainWindowRequested, this, &AppController::showMainWindow);
        connect(&m_trayController, &ui::TrayController::openSectionRequested, this, &AppController::showSection);
        connect(&m_trayController, &ui::TrayController::startBreakRequested, this, &AppController::startBreakNow);
        connect(&m_trayController, &ui::TrayController::snoozeAllRequested, this, &AppController::snoozeAll);
        connect(&m_trayController, &ui::TrayController::pauseRequested, this, &AppController::pauseReminders);
        connect(&m_trayController, &ui::TrayController::skipNextRequested, this, &AppController::skipNextReminder);
        connect(&m_trayController, &ui::TrayController::quitRequested, this, &AppController::quitApplication);

        connect(&m_mainWindow, &ui::MainWindow::addEventRequested, this, &AppController::addEvent);
        connect(&m_mainWindow, &ui::MainWindow::editEventRequested, this, &AppController::editEvent);
        connect(&m_mainWindow, &ui::MainWindow::deleteEventRequested, this, &AppController::deleteEvent);
        connect(&m_mainWindow, &ui::MainWindow::addProfileRequested, this, &AppController::addProfile);
        connect(&m_mainWindow, &ui::MainWindow::editProfileRequested, this, &AppController::editProfile);
        connect(&m_mainWindow, &ui::MainWindow::deleteProfileRequested, this, &AppController::deleteProfile);
        connect(&m_mainWindow, &ui::MainWindow::saveSettingsRequested, this, &AppController::saveSettings);
        connect(&m_mainWindow, &ui::MainWindow::closeRequested, this, &AppController::quitApplication);
        connect(&m_languageManager, &ui::LanguageManager::languageApplied, this, [this]()
                { m_trayController.retranslateUi(); });
        installRuntimeResyncHooks();
    }

    bool AppController::initialize()
    {
#ifdef Q_OS_WIN
        SetCurrentProcessExplicitAppUserModelID(L"Deadliner.Deadliner");
#endif

        QString error;
        if (!m_repository.open(dataPath(QStringLiteral("deadliner.sqlite")), &error))
        {
            QMessageBox::critical(nullptr, tr("Database error"), error);
            return false;
        }

        m_settings = m_settingsStore.load();
        m_settings.language = m_languageManager.canonicalLanguageSetting(m_settings.language);
        m_languageManager.applyLanguage(m_settings.language);
        applyAppearanceSettings();

        m_autostartManager.setEnabled(m_settings.launchOnStartup, QCoreApplication::applicationFilePath());
        refreshState();

        m_trayController.setIcon(m_settings.trayIcon);

        // Always defer tray initialisation to the event loop on all platforms.
        QTimer::singleShot(0, this, [this]()
                           {
            m_trayController.show();
            
            if (m_trayController.isAvailable())
            {
                refreshState();
            }
            else if (m_settings.startMinimized)
            {
                // Tray unavailable and window suppressed — show it as last resort.
                m_mainWindow.show();
            } });

        if (!runOnboardingIfNeeded())
        {
            return false;
        }

        if (shouldSeedStartupTestData())
        {
            ensureStartupTestEvents();
            refreshState();
        }

        if (!m_settings.lastWindowGeometry.isEmpty())
        {
            m_mainWindow.restoreGeometry(m_settings.lastWindowGeometry);
        }

        if (!m_settings.startMinimized)
        {
            m_mainWindow.show();
        }

        return true;
    }

    void AppController::refreshState()
    {
        m_state = m_repository.loadState();

        QList<domain::ReminderEvent> normalizedEvents;
        QList<domain::ReminderEvent> overdueEvents;
        const auto profiles = profileMap(m_state.profiles);
        const QDateTime now = QDateTime::currentDateTime();
        for (auto event : m_state.events)
        {
            const auto profileIt = profiles.constFind(event.profileId);
            if (profileIt == profiles.constEnd())
            {
                continue;
            }

            const auto &profile = profileIt.value();

            if (domain::isOverdueEvent(event, profile, now))
            {
                overdueEvents.push_back(event);
                normalizedEvents.push_back(event);
                continue;
            }

            if (domain::isMissedRecurringEvent(event, profile, now))
            {
                // Never fire a missed recurring occurrence on open — advance to the
                // next future trigger regardless of skipMissedOccurrences.  The flag
                // only controls whether the missed slot is counted as skipped; the
                // scheduling logic must always produce a future nextTriggerAt.
                const QDateTime next = domain::normalizeNextTrigger(event, profile, now);
                if (next != event.nextTriggerAt)
                {
                    event.nextTriggerAt = next;
                    saveEventOrWarn(event, tr("The event state could not be refreshed. Check for a duplicate title/date/profile combination."));
                }
                normalizedEvents.push_back(event);
                continue;
            }

            if (!event.nextTriggerAt.isValid())
            {
                const QDateTime next = domain::normalizeNextTrigger(event, profile, now);
                if (next != event.nextTriggerAt)
                {
                    event.nextTriggerAt = next;
                    saveEventOrWarn(event, tr("The event state could not be refreshed. Check for a duplicate title/date/profile combination."));
                }
            }

            normalizedEvents.push_back(event);
        }
        m_state.events = normalizedEvents;

        m_scheduler.setState(m_state.events, profiles);
        m_mainWindow.setState(m_state.events,
                              overdueEvents,
                              m_state.profiles,
                              m_state.stats,
                              m_scheduler.upcomingOccurrences(),
                              m_state.quietHoursPolicies,
                              m_settings,
                              m_autostartManager.isEnabled(),
                              m_trayController.isAvailable());
    }

    void AppController::handleOccurrence(const domain::ReminderOccurrence &occurrence)
    {
        if (m_handlingOccurrence)
        {
            return;
        }

        m_handlingOccurrence = true;
        const auto resetHandlingOccurrence = qScopeGuard([this]()
                                                         { m_handlingOccurrence = false; });

        const bool manualOccurrence = occurrence.eventId <= 0;
        auto event = eventForId(occurrence.eventId);
        const auto profile = profileForId(occurrence.profileId);
        const auto policy = quietPolicyForId(profile.quietHoursPolicyId);

        if (m_settings.pauseUntil.isValid() && m_settings.pauseUntil > QDateTime::currentDateTime())
        {
            if (manualOccurrence)
            {
                return;
            }
            event.nextTriggerAt = m_settings.pauseUntil;
            event.pendingOriginalTriggerAt = occurrence.originalTriggerAt;
            event.pendingMode = occurrence.mode;
            event.pendingSnoozeCount = occurrence.snoozeCount;
            if (!saveEventOrWarn(event, tr("The reminder could not be updated while reminders are paused.")))
            {
                return;
            }
            refreshState();
            return;
        }

        const auto quietDecision = domain::applyQuietHours(policy, occurrence.mode, event.isOneTime, QDateTime::currentDateTime());
        if (!quietDecision.shouldShow && !manualOccurrence)
        {
            if (quietDecision.shouldSuppress)
            {
                persistOutcome(occurrence, occurrence.mode, domain::ReminderResult::Dismissed);
                event.lastTriggeredAt = occurrence.originalTriggerAt;
                event.pendingOriginalTriggerAt = {};
                event.pendingMode.reset();
                event.pendingSnoozeCount = 0;
                event.nextTriggerAt = domain::normalizeNextTrigger(event, profile, QDateTime::currentDateTime());
            }
            else
            {
                event.nextTriggerAt = quietDecision.deferUntil;
                event.pendingOriginalTriggerAt = occurrence.originalTriggerAt;
                event.pendingMode = quietDecision.effectiveMode;
                event.pendingSnoozeCount = occurrence.snoozeCount;
            }
            if (!saveEventOrWarn(event, tr("The reminder could not be updated after applying quiet hours.")))
            {
                return;
            }
            refreshState();
            return;
        }

        domain::ReminderAction action = domain::ReminderAction::Dismissed;
        const domain::SeverityMode effectiveMode = quietDecision.effectiveMode;

#ifdef Q_OS_WIN
        QWidget *dialogParent = nullptr;
#else
        QWidget *dialogParent = &m_mainWindow;
#endif

        if (effectiveMode == domain::SeverityMode::Soft)
        {
            if (m_trayController.supportsMessages())
            {
                m_trayController.showMessage(occurrence.title, occurrence.description.isEmpty() ? tr("Reminder is due") : occurrence.description);
                action = domain::ReminderAction::Dismissed;
                persistOutcome(occurrence, effectiveMode, domain::ReminderResult::Dismissed);
            }
            else
            {
                ui::ReminderDialog dialog(occurrence, dialogParent);
                dialog.show();
                dialog.raise();
                dialog.activateWindow();
                dialog.exec();
                action = dialog.action();

                switch (action)
                {
                case domain::ReminderAction::Completed:
                    persistOutcome(occurrence, effectiveMode, domain::ReminderResult::Completed);
                    break;
                case domain::ReminderAction::Snoozed:
                    persistOutcome(occurrence, effectiveMode, domain::ReminderResult::Snoozed);
                    break;
                case domain::ReminderAction::Skipped:
                    persistOutcome(occurrence, effectiveMode, domain::ReminderResult::Skipped);
                    break;
                case domain::ReminderAction::Dismissed:
                    persistOutcome(occurrence, effectiveMode, domain::ReminderResult::Dismissed);
                    break;
                }
            }
        }
        else if (effectiveMode == domain::SeverityMode::Break)
        {
            ui::BreakWindow window(profile.breakDurationMinutes,
                                   domain::canSnooze(profile, occurrence.snoozeCount),
                                   profile.allowSkip,
                                   dialogParent);
            window.show();
            window.raise();
            window.activateWindow();
            const int code = window.exec();
            const auto result = window.result();
            if (code == QDialog::Accepted && result.completed)
            {
                action = domain::ReminderAction::Completed;
                persistOutcome(occurrence, effectiveMode, domain::ReminderResult::Completed, result.actualSeconds);
            }
            else if (result.snoozed)
            {
                action = domain::ReminderAction::Snoozed;
                persistOutcome(occurrence, effectiveMode, domain::ReminderResult::Snoozed, result.actualSeconds);
            }
            else
            {
                action = domain::ReminderAction::Skipped;
                persistOutcome(occurrence, effectiveMode, domain::ReminderResult::Expired, result.actualSeconds);
            }
        }
        else
        {
            ui::ReminderDialog dialog(occurrence, dialogParent);
            dialog.show();
            dialog.raise();
            dialog.activateWindow();
            dialog.exec();
            action = dialog.action();
            switch (action)
            {
            case domain::ReminderAction::Completed:
                persistOutcome(occurrence, effectiveMode, domain::ReminderResult::Completed);
                break;
            case domain::ReminderAction::Snoozed:
                persistOutcome(occurrence, effectiveMode, domain::ReminderResult::Snoozed);
                break;
            case domain::ReminderAction::Skipped:
                persistOutcome(occurrence, effectiveMode, domain::ReminderResult::Skipped);
                break;
            case domain::ReminderAction::Dismissed:
                persistOutcome(occurrence, effectiveMode, domain::ReminderResult::Dismissed);
                break;
            }
        }

        if (manualOccurrence)
        {
            refreshState();
            return;
        }

        if (action == domain::ReminderAction::Snoozed && domain::canSnooze(profile, occurrence.snoozeCount))
        {
            event.pendingSnoozeCount = occurrence.snoozeCount + 1;
            event.pendingOriginalTriggerAt = occurrence.originalTriggerAt;
            event.pendingMode = effectiveMode;
            event.nextTriggerAt = domain::snoozeUntil(profile, QDateTime::currentDateTime());
        }
        else
        {
            event.lastTriggeredAt = QDateTime::currentDateTime();
            event.pendingOriginalTriggerAt = {};
            event.pendingMode.reset();
            event.pendingSnoozeCount = 0;

            if (action == domain::ReminderAction::Dismissed && effectiveMode == domain::SeverityMode::Persistent)
            {
                event.pendingOriginalTriggerAt = occurrence.originalTriggerAt;
                event.pendingMode = effectiveMode;
                event.pendingSnoozeCount = occurrence.snoozeCount;
                event.nextTriggerAt = QDateTime::currentDateTime().addSecs(profile.snoozeMinutes * 60);
            }
            else if (action == domain::ReminderAction::Snoozed && !domain::canSnooze(profile, occurrence.snoozeCount))
            {
                if (effectiveMode == domain::SeverityMode::Persistent)
                {
                    event.pendingOriginalTriggerAt = occurrence.originalTriggerAt;
                    event.pendingMode = domain::SeverityMode::Break;
                    event.nextTriggerAt = QDateTime::currentDateTime().addSecs(60);
                }
                else
                {
                    event.nextTriggerAt = domain::normalizeNextTrigger(event, profile, QDateTime::currentDateTime());
                }
            }
            else
            {
                event.nextTriggerAt = domain::normalizeNextTrigger(event, profile, QDateTime::currentDateTime());
            }
        }

        if (!saveEventOrWarn(event, tr("The reminder could not be updated after it was handled.")))
        {
            return;
        }
        refreshState();
    }

    void AppController::showMainWindow()
    {
        m_mainWindow.showNormal();
        m_mainWindow.raise();
        m_mainWindow.activateWindow();
    }

    void AppController::showSection(ui::MainSection section)
    {
        showMainWindow();
        m_mainWindow.setCurrentSection(section);
    }

    void AppController::addEvent()
    {
        ui::EventDialog dialog(m_state.profiles, nullptr, &m_mainWindow);
        if (dialog.exec() != QDialog::Accepted)
        {
            return;
        }

        auto event = dialog.event();
        const auto profile = profileForId(event.profileId);
        const QDateTime now = QDateTime::currentDateTime();
        event.nextTriggerAt = domain::isMissedRecurringEvent(event, profile, now) && !event.skipMissedOccurrences
                                  ? event.startAt
                                  : domain::normalizeNextTrigger(event, profile, now);
        if (!saveEventOrWarn(event, tr("The event could not be saved. Check for a duplicate title/date/profile combination.")))
        {
            return;
        }
        refreshState();
    }

    void AppController::editEvent(qint64 id)
    {
        if (id == 0)
        {
            QMessageBox::information(&m_mainWindow, tr("Selection required"), tr("Select an item first."));
            return;
        }
        auto existing = eventForId(id);
        ui::EventDialog dialog(m_state.profiles, &existing, &m_mainWindow);
        if (dialog.exec() != QDialog::Accepted)
        {
            return;
        }

        auto event = dialog.event();
        event.id = existing.id;
        event.timezoneMode = existing.timezoneMode;
        event.lastTriggeredAt = existing.lastTriggeredAt;
        event.nextTriggerAt = existing.nextTriggerAt;
        event.pendingSnoozeCount = existing.pendingSnoozeCount;
        event.pendingOriginalTriggerAt = existing.pendingOriginalTriggerAt;
        event.pendingMode = existing.pendingMode;
        event.createdAt = existing.createdAt;
        const auto profile = profileForId(event.profileId);
        if (!event.pendingOriginalTriggerAt.isValid() && !event.pendingMode.has_value() && event.pendingSnoozeCount == 0)
        {
            const QDateTime now = QDateTime::currentDateTime();
            event.nextTriggerAt = domain::isMissedRecurringEvent(event, profile, now) && !event.skipMissedOccurrences
                                      ? event.startAt
                                      : domain::normalizeNextTrigger(event, profile, now);
        }
        if (!saveEventOrWarn(event, tr("The event could not be saved. Check for a duplicate title/date/profile combination.")))
        {
            return;
        }
        refreshState();
    }

    void AppController::deleteEvent(qint64 id)
    {
        if (id == 0)
        {
            QMessageBox::information(&m_mainWindow, tr("Selection required"), tr("Select an item first."));
            return;
        }
        m_repository.deleteEvent(id);
        refreshState();
    }

    void AppController::addProfile()
    {
        ui::ProfileDialog dialog(m_state.quietHoursPolicies, nullptr, &m_mainWindow);
        if (dialog.exec() != QDialog::Accepted)
        {
            return;
        }
        if (!saveProfileOrWarn(dialog.profile(), tr("The profile could not be saved. Check for a duplicate profile name.")))
        {
            return;
        }
        refreshState();
    }

    void AppController::editProfile(qint64 id)
    {
        if (id == 0)
        {
            QMessageBox::information(&m_mainWindow, tr("Selection required"), tr("Select an item first."));
            return;
        }
        auto existing = profileForId(id);
        ui::ProfileDialog dialog(m_state.quietHoursPolicies, &existing, &m_mainWindow);
        if (dialog.exec() != QDialog::Accepted)
        {
            return;
        }
        auto profile = dialog.profile();
        profile.id = existing.id;
        profile.builtIn = existing.builtIn;
        if (!saveProfileOrWarn(profile, tr("The profile could not be saved. Check for a duplicate profile name.")))
        {
            return;
        }
        refreshState();
    }

    void AppController::deleteProfile(qint64 id)
    {
        if (id == 0)
        {
            QMessageBox::information(&m_mainWindow, tr("Selection required"), tr("Select an item first."));
            return;
        }

        m_repository.deleteProfile(id);
        refreshState();
    }

    void AppController::saveSettings(domain::AppSettings settings, domain::QuietHoursPolicy policy)
    {
        settings.lastWindowGeometry = m_mainWindow.saveGeometry();
        settings.onboardingCompleted = m_settings.onboardingCompleted;
        settings.language = m_languageManager.canonicalLanguageSetting(settings.language);
        m_settings = settings;
        m_settingsStore.save(m_settings);
        applyAppearanceSettings();

        m_autostartManager.setEnabled(m_settings.launchOnStartup, QCoreApplication::applicationFilePath());

        if (!m_state.quietHoursPolicies.isEmpty())
        {
            auto current = m_state.quietHoursPolicies.constFirst();
            current.startTime = policy.startTime;
            current.endTime = policy.endTime;
            current.behaviorSoft = policy.behaviorSoft;
            current.behaviorPersistent = policy.behaviorPersistent;
            current.behaviorBreak = policy.behaviorBreak;
            m_repository.saveQuietHoursPolicy(current);
        }

        refreshState();
    }

    void AppController::startBreakNow()
    {
        const qint64 profileId = m_settings.defaultProfileId != 0 && profileForId(m_settings.defaultProfileId).id != 0
                                     ? m_settings.defaultProfileId
                                     : (m_state.profiles.isEmpty() ? 0 : m_state.profiles.constFirst().id);
        if (profileId == 0)
        {
            return;
        }

        domain::ReminderOccurrence occurrence;
        occurrence.eventId = -1;
        occurrence.profileId = profileId;
        occurrence.title = tr("Manual break");
        occurrence.description = tr("Started from the tray menu.");
        occurrence.dueAt = QDateTime::currentDateTime();
        occurrence.originalTriggerAt = occurrence.dueAt;
        occurrence.mode = domain::SeverityMode::Break;
        occurrence.allowSkip = profileForId(profileId).allowSkip;
        occurrence.breakDurationMinutes = profileForId(profileId).breakDurationMinutes;
        occurrence.requirePostBreakConfirmation = profileForId(profileId).requirePostBreakConfirmation;
        handleOccurrence(occurrence);
    }

    void AppController::snoozeAll(int minutes)
    {
        const QDateTime target = QDateTime::currentDateTime().addSecs(minutes * 60);
        for (auto event : m_state.events)
        {
            const QDateTime previousTrigger = event.nextTriggerAt;
            event.nextTriggerAt = target;
            if (!event.pendingOriginalTriggerAt.isValid())
            {
                event.pendingOriginalTriggerAt = previousTrigger.isValid() ? previousTrigger : target;
            }
            saveEventOrWarn(event, tr("A reminder could not be updated while snoozing all reminders."));
        }
        refreshState();
    }

    void AppController::pauseReminders(int minutes)
    {
        QDateTime target;
        if (minutes >= 12 * 60)
        {
            target = QDateTime(QDate::currentDate().addDays(1), QTime(8, 0));
        }
        else
        {
            target = QDateTime::currentDateTime().addSecs(minutes * 60);
        }

        m_settings.pauseUntil = target;
        m_settings.lastWindowGeometry = m_mainWindow.saveGeometry();
        m_settingsStore.save(m_settings);

        for (auto event : m_state.events)
        {
            if (event.nextTriggerAt.isValid() && event.nextTriggerAt < target)
            {
                const QDateTime previousTrigger = event.nextTriggerAt;
                event.nextTriggerAt = target;
                if (!event.pendingOriginalTriggerAt.isValid())
                {
                    event.pendingOriginalTriggerAt = previousTrigger;
                }
                saveEventOrWarn(event, tr("A reminder could not be updated while pausing reminders."));
            }
        }

        m_trayController.showMessage(tr("Deadliner"), tr("Reminders paused until %1").arg(QLocale().toString(target, QLocale::ShortFormat)));
        refreshState();
    }

    void AppController::skipNextReminder()
    {
        const auto upcoming = m_scheduler.upcomingOccurrences();
        if (upcoming.isEmpty())
        {
            m_trayController.showMessage(tr("Deadliner"), tr("No upcoming reminders to skip."));
            return;
        }

        const auto nextOccurrence = upcoming.constFirst();
        if (nextOccurrence.eventId <= 0)
        {
            m_trayController.showMessage(tr("Deadliner"), tr("The next reminder cannot be skipped automatically."));
            return;
        }

        auto event = eventForId(nextOccurrence.eventId);
        const auto profile = profileForId(event.profileId);

        persistOutcome(nextOccurrence, nextOccurrence.mode, domain::ReminderResult::Skipped);
        event.lastTriggeredAt = nextOccurrence.dueAt;
        event.pendingOriginalTriggerAt = {};
        event.pendingMode.reset();
        event.pendingSnoozeCount = 0;
        event.nextTriggerAt = domain::normalizeNextTrigger(event, profile, nextOccurrence.dueAt.addSecs(1));
        if (!saveEventOrWarn(event, tr("The reminder could not be updated after skipping it.")))
        {
            return;
        }

        m_trayController.showMessage(tr("Deadliner"), tr("Skipped next reminder: %1").arg(nextOccurrence.title));
        refreshState();
    }

    void AppController::quitApplication()
    {
        m_settings.lastWindowGeometry = m_mainWindow.saveGeometry();
        m_settingsStore.save(m_settings);
        qApp->quit();
    }

    void AppController::applyAppearanceSettings()
    {
        m_languageManager.applyLanguage(m_settings.language);
        ui::applyTheme(*qobject_cast<QApplication *>(qApp), m_settings.theme);
        m_trayController.setIcon(m_settings.trayIcon);
        m_trayController.retranslateUi();
    }

    void AppController::installRuntimeResyncHooks()
    {
        connect(qApp, &QGuiApplication::applicationStateChanged, this, [this](Qt::ApplicationState state)
                {
            if (m_handlingOccurrence)
            {
                return;
            }

            if (state == Qt::ApplicationActive)
            {
                refreshState();
            } });

        m_runtimeResyncTimer = new QTimer(this);
        m_runtimeResyncTimer->setInterval(30000);
        connect(m_runtimeResyncTimer, &QTimer::timeout, this, [this]()
                {
            if (m_handlingOccurrence)
            {
                return;
            }

            if (m_scheduler.needsResync(QDateTime::currentDateTime()))
            {
                refreshState();
            } });
        m_runtimeResyncTimer->start();
    }

    bool AppController::runOnboardingIfNeeded()
    {
        if (m_settings.onboardingCompleted)
        {
            return true;
        }

        const QString originalLanguage = m_settings.language;
        ui::OnboardingDialog dialog(&m_mainWindow);
        connect(&dialog, &ui::OnboardingDialog::languagePreviewRequested, this, [this](const QString &languageCode)
                {
        m_settings.language = m_languageManager.canonicalLanguageSetting(languageCode);
        applyAppearanceSettings(); });

        if (dialog.exec() != QDialog::Accepted)
        {
            m_settings.language = originalLanguage;
            applyAppearanceSettings();
            return false;
        }

        m_settings.language = m_languageManager.canonicalLanguageSetting(dialog.selectedLanguage());
        m_settings.onboardingCompleted = true;
        applyAppearanceSettings();
        ensureRecommendedOnboardingPreset(dialog.enableRecommendedBreakRule());
        m_settingsStore.save(m_settings);
        refreshState();
        return true;
    }

    void AppController::ensureRecommendedOnboardingPreset(bool enabled)
    {
        if (!enabled || m_state.profiles.isEmpty())
        {
            return;
        }

        for (const auto &event : m_state.events)
        {
            if (event.title == tr("Eye rest"))
            {
                return;
            }
        }

        const auto baseProfile = m_state.profiles.constFirst();
        domain::ReminderProfile presetProfile = baseProfile;
        presetProfile.id = 0;
        presetProfile.name = tr("Eye rest");
        presetProfile.intervalMinutes = 30;
        presetProfile.breakDurationMinutes = 1;
        presetProfile.severityMode = domain::SeverityMode::Soft;
        presetProfile.maxSnoozeCount = 1;
        const qint64 profileId = m_repository.saveProfile(presetProfile);
        if (profileId == 0)
        {
            QMessageBox::warning(&m_mainWindow, tr("Save failed"), tr("The onboarding profile could not be saved."));
            return;
        }

        domain::ReminderEvent presetEvent;
        presetEvent.title = tr("Eye rest");
        presetEvent.description = tr("Short eye-rest reminder suggested during onboarding.");
        presetEvent.type = domain::ReminderType::BreakInterval;
        presetEvent.profileId = profileId;
        presetEvent.startAt = QDateTime::currentDateTime().addSecs(20 * 60);
        presetEvent.isOneTime = false;
        presetEvent.recurrenceRule = QStringLiteral("none");
        presetEvent.nextTriggerAt = presetEvent.startAt;
        saveEventOrWarn(presetEvent, tr("The onboarding event could not be saved."));
    }

    void AppController::ensureStartupTestEvents()
    {
        if (m_state.quietHoursPolicies.isEmpty())
        {
            return;
        }

        const qint64 quietPolicyId = m_state.quietHoursPolicies.constFirst().id;
        const QDateTime now = QDateTime::currentDateTime();

        const auto upsertProfile = [this, quietPolicyId](const QString &name,
                                                         domain::ProfileKind kind,
                                                         domain::SeverityMode severityMode,
                                                         int intervalMinutes,
                                                         int breakDurationMinutes,
                                                         bool allowSkip)
        {
            domain::ReminderProfile profile;
            for (const auto &existing : m_state.profiles)
            {
                if (existing.name == name)
                {
                    profile = existing;
                    break;
                }
            }

            profile.name = name;
            profile.kind = kind;
            profile.intervalMinutes = intervalMinutes;
            profile.breakDurationMinutes = breakDurationMinutes;
            profile.severityMode = severityMode;
            profile.maxSnoozeCount = 1;
            profile.snoozeMinutes = 1;
            profile.quietHoursPolicyId = quietPolicyId;
            profile.requirePostBreakConfirmation = true;
            profile.allowSkip = allowSkip;
            profile.enabled = true;
            profile.builtIn = false;
            const qint64 savedId = m_repository.saveProfile(profile);
            if (savedId != 0)
            {
                return savedId;
            }

            if (profile.id != 0)
            {
                QMessageBox::warning(&m_mainWindow, tr("Save failed"), tr("A test profile could not be updated."));
                return profile.id;
            }

            QMessageBox::warning(&m_mainWindow, tr("Save failed"), tr("A test profile could not be saved."));
            return qint64{0};
        };

        const qint64 softProfileId = upsertProfile(QString::fromUtf8(kTestSoftProfileName),
                                                   domain::ProfileKind::Generic,
                                                   domain::SeverityMode::Soft,
                                                   720,
                                                   1,
                                                   true);
        const qint64 persistentProfileId = upsertProfile(QString::fromUtf8(kTestPersistentProfileName),
                                                         domain::ProfileKind::Generic,
                                                         domain::SeverityMode::Persistent,
                                                         720,
                                                         1,
                                                         true);
        const qint64 breakProfileId = upsertProfile(QString::fromUtf8(kTestBreakProfileName),
                                                    domain::ProfileKind::Break,
                                                    domain::SeverityMode::Break,
                                                    720,
                                                    1,
                                                    true);

        const auto upsertEvent = [this](const QString &title,
                                        const QString &description,
                                        domain::ReminderType type,
                                        qint64 profileId,
                                        const QDateTime &startAt,
                                        bool isOneTime)
        {
            domain::ReminderEvent event;
            for (const auto &existing : m_state.events)
            {
                if (existing.title == title)
                {
                    event = existing;
                    break;
                }
            }

            event.title = title;
            event.description = description;
            event.type = type;
            event.profileId = profileId;
            event.startAt = startAt;
            event.timezoneMode = QStringLiteral("local");
            event.recurrenceRule = QStringLiteral("none");
            event.isOneTime = isOneTime;
            event.enabled = true;
            event.lastTriggeredAt = {};
            event.nextTriggerAt = startAt;
            event.pendingSnoozeCount = 0;
            event.pendingOriginalTriggerAt = {};
            event.pendingMode.reset();
            saveEventOrWarn(event, tr("A test event could not be saved."));
        };

        upsertEvent(QString::fromUtf8(kTestSoftEventTitle),
                    tr("Quick soft reminder for notification popup testing."),
                    domain::ReminderType::DateTime,
                    softProfileId,
                    now.addSecs(15),
                    true);
        upsertEvent(QString::fromUtf8(kTestPersistentEventTitle),
                    tr("Quick persistent reminder for dialog testing."),
                    domain::ReminderType::DateTime,
                    persistentProfileId,
                    now.addSecs(30),
                    true);
        upsertEvent(QString::fromUtf8(kTestBreakEventTitle),
                    tr("Quick break reminder for fullscreen break testing."),
                    domain::ReminderType::BreakInterval,
                    breakProfileId,
                    now.addSecs(45),
                    false);
    }

    domain::ReminderProfile AppController::profileForId(qint64 id) const
    {
        for (const auto &profile : m_state.profiles)
        {
            if (profile.id == id)
            {
                return profile;
            }
        }
        return {};
    }

    domain::ReminderEvent AppController::eventForId(qint64 id) const
    {
        for (const auto &event : m_state.events)
        {
            if (event.id == id)
            {
                return event;
            }
        }
        return {};
    }

    domain::QuietHoursPolicy AppController::quietPolicyForId(qint64 id) const
    {
        if (id <= 0)
        {
            return {};
        }
        for (const auto &policy : m_state.quietHoursPolicies)
        {
            if (policy.id == id)
            {
                return policy;
            }
        }
        return m_state.quietHoursPolicies.isEmpty() ? domain::QuietHoursPolicy{} : m_state.quietHoursPolicies.constFirst();
    }

    void AppController::persistOutcome(const domain::ReminderOccurrence &occurrence,
                                       domain::SeverityMode shownMode,
                                       domain::ReminderResult result,
                                       int breakActualSeconds)
    {
        if (occurrence.eventId <= 0)
        {
            return;
        }

        domain::OccurrenceLogEntry entry;
        entry.eventId = occurrence.eventId;
        const QDateTime loggedAt = occurrence.originalTriggerAt.isValid()
                                       ? occurrence.originalTriggerAt
                                       : occurrence.dueAt;
        entry.triggeredAt = loggedAt;
        entry.shownMode = shownMode;
        entry.result = result;
        entry.snoozeCount = occurrence.snoozeCount;
        entry.breakExpectedSeconds = occurrence.breakDurationMinutes * 60;
        entry.breakActualSeconds = breakActualSeconds;
        entry.completedAt = result == domain::ReminderResult::Completed ? QDateTime::currentDateTime() : QDateTime{};
        m_repository.logOccurrence(entry);

        const auto updated = m_statisticsService.updateDaily(m_state.stats, entry);
        m_repository.upsertDailyStats(updated);
    }

    bool AppController::saveEventOrWarn(const domain::ReminderEvent &event, const QString &message)
    {
        const qint64 savedId = m_repository.saveEvent(event);
        if (savedId != 0)
        {
            return true;
        }

        QMessageBox::warning(&m_mainWindow, tr("Save failed"), message);
        return false;
    }

    bool AppController::saveProfileOrWarn(const domain::ReminderProfile &profile, const QString &message)
    {
        const qint64 savedId = m_repository.saveProfile(profile);
        if (savedId != 0)
        {
            return true;
        }

        QMessageBox::warning(&m_mainWindow, tr("Save failed"), message);
        return false;
    }

} // namespace deadliner::app
