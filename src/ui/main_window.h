#pragma once

#include "domain/models.h"
#include "ui/common/navigation.h"

#include <QCloseEvent>
#include <QEvent>
#include <QMainWindow>

class QListWidget;
class QListWidgetItem;
class QLabel;
class QStackedWidget;

namespace deadliner::ui {

class EventsPage;
class OverduePage;
class ProfilesPage;
class SettingsPage;
class StatisticsPage;
class TodayPage;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    void setState(const QList<domain::ReminderEvent> &events,
                  const QList<domain::ReminderEvent> &overdueEvents,
                  const QList<domain::ReminderProfile> &profiles,
                  const QList<domain::StatsDaily> &stats,
                  const QList<domain::ReminderOccurrence> &upcoming,
                  const QList<domain::QuietHoursPolicy> &policies,
                  const domain::AppSettings &settings,
                  bool autostartEnabled,
                  bool hasTray);
    void setCurrentSection(MainSection section);

signals:
    void addEventRequested();
    void editEventRequested(qint64 id);
    void deleteEventRequested(qint64 id);
    void addProfileRequested();
    void editProfileRequested(qint64 id);
    void deleteProfileRequested(qint64 id);
    void saveSettingsRequested(domain::AppSettings settings, domain::QuietHoursPolicy policy);
    void closeRequested();

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();
    void syncNavigationSelection();

    QList<domain::ReminderEvent> m_events;
    QList<domain::ReminderEvent> m_overdueEvents;
    QList<domain::ReminderProfile> m_profiles;
    QList<domain::StatsDaily> m_stats;
    QList<domain::ReminderOccurrence> m_upcoming;
    QList<domain::QuietHoursPolicy> m_policies;
    domain::AppSettings m_settings;
    bool m_autostartEnabled = false;
    bool m_hasTray = false;
    MainSection m_currentSection = MainSection::Today;

    QLabel *m_titleLabel = nullptr;
    QLabel *m_subtitleLabel = nullptr;
    QListWidget *m_navigationList = nullptr;
    QStackedWidget *m_pages = nullptr;
    TodayPage *m_todayPage = nullptr;
    EventsPage *m_eventsPage = nullptr;
    OverduePage *m_overduePage = nullptr;
    ProfilesPage *m_profilesPage = nullptr;
    StatisticsPage *m_statisticsPage = nullptr;
    SettingsPage *m_settingsPage = nullptr;
};

}  // namespace deadliner::ui
