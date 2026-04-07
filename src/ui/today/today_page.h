#pragma once

#include "domain/models.h"

#include <QEvent>
#include <QWidget>

class QLabel;
class QTableWidget;
class QPushButton;

namespace deadliner::ui
{

    class TodayPage : public QWidget
    {
        Q_OBJECT

    public:
        explicit TodayPage(QWidget *parent = nullptr);

        void setState(const QList<domain::ReminderEvent> &events,
                      const QList<domain::ReminderProfile> &profiles,
                      const QList<domain::ReminderOccurrence> &upcoming,
                      const domain::AppSettings &settings);

    signals:
        void openEventsRequested();
        void openProfilesRequested();

    protected:
        void changeEvent(QEvent *event) override;

    private:
        void retranslateUi();
        void refreshDynamicContent();

        QList<domain::ReminderEvent> m_events;
        QList<domain::ReminderProfile> m_profiles;
        QList<domain::ReminderOccurrence> m_upcoming;
        domain::AppSettings m_settings;

        QLabel *m_greetingLabel = nullptr;
        QLabel *m_pauseStatusLabel = nullptr;
        QLabel *m_nextBreakCaptionLabel = nullptr;
        QLabel *m_nextBreakValueLabel = nullptr;
        QLabel *m_rulesCaptionLabel = nullptr;
        QLabel *m_eventsCaptionLabel = nullptr;
        QTableWidget *m_rulesTable = nullptr;
        QTableWidget *m_eventsTable = nullptr;
        QPushButton *m_manageProfilesButton = nullptr;
        QPushButton *m_manageEventsButton = nullptr;
    };

} // namespace deadliner::ui
