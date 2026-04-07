#include "ui/today/today_page.h"

#include "ui/common/display_strings.h"

#include <QDateTime>
#include <QEvent>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QAbstractItemView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>

namespace deadliner::ui
{

    namespace
    {

        QTableWidget *createTable(int columns, QWidget *parent)
        {
            auto *table = new QTableWidget(parent);
            table->setColumnCount(columns);
            table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            table->verticalHeader()->setVisible(false);
            table->setSelectionMode(QAbstractItemView::NoSelection);
            table->setFocusPolicy(Qt::NoFocus);
            table->setEditTriggers(QAbstractItemView::NoEditTriggers);
            table->setAlternatingRowColors(true);
            return table;
        }

        QString greetingForTime(const QTime &time, const QObject *context)
        {
            if (time.hour() < 12)
            {
                return context->tr("Good morning");
            }
            if (time.hour() < 18)
            {
                return context->tr("Good afternoon");
            }
            return context->tr("Good evening");
        }

    } // namespace

    TodayPage::TodayPage(QWidget *parent)
        : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(24, 24, 24, 24);
        layout->setSpacing(18);

        auto *heroLayout = new QVBoxLayout();
        heroLayout->setSpacing(6);

        m_pauseStatusLabel = new QLabel(this);
        m_pauseStatusLabel->setWordWrap(true);
        m_greetingLabel = new QLabel(this);
        m_greetingLabel->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 600;"));
        m_nextBreakCaptionLabel = new QLabel(this);
        m_nextBreakValueLabel = new QLabel(this);
        m_nextBreakValueLabel->setStyleSheet(QStringLiteral("font-size: 30px; font-weight: 700;"));

        heroLayout->addWidget(m_pauseStatusLabel);
        heroLayout->addWidget(m_greetingLabel);
        heroLayout->addSpacing(4);
        heroLayout->addWidget(m_nextBreakCaptionLabel);
        heroLayout->addWidget(m_nextBreakValueLabel);

        auto *rulesHeader = new QHBoxLayout();
        m_rulesCaptionLabel = new QLabel(this);
        m_rulesCaptionLabel->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 600;"));
        m_manageProfilesButton = new QPushButton(this);
        rulesHeader->addWidget(m_rulesCaptionLabel);
        rulesHeader->addStretch();
        rulesHeader->addWidget(m_manageProfilesButton);

        m_rulesTable = createTable(3, this);

        auto *eventsHeader = new QHBoxLayout();
        m_eventsCaptionLabel = new QLabel(this);
        m_eventsCaptionLabel->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 600;"));
        m_manageEventsButton = new QPushButton(this);
        eventsHeader->addWidget(m_eventsCaptionLabel);
        eventsHeader->addStretch();
        eventsHeader->addWidget(m_manageEventsButton);

        m_eventsTable = createTable(4, this);

        layout->addLayout(heroLayout);
        layout->addSpacing(8);
        layout->addLayout(rulesHeader);
        layout->addWidget(m_rulesTable);
        layout->addLayout(eventsHeader);
        layout->addWidget(m_eventsTable, 1);

        connect(m_manageProfilesButton, &QPushButton::clicked, this, &TodayPage::openProfilesRequested);
        connect(m_manageEventsButton, &QPushButton::clicked, this, &TodayPage::openEventsRequested);

        m_updateTimer = new QTimer(this);
        m_updateTimer->setInterval(1000);
        connect(m_updateTimer, &QTimer::timeout, this, &TodayPage::refreshDynamicContent);

        retranslateUi();
    }

    void TodayPage::setState(const QList<domain::ReminderEvent> &events,
                             const QList<domain::ReminderProfile> &profiles,
                             const QList<domain::ReminderOccurrence> &upcoming,
                             const domain::AppSettings &settings)
    {
        m_events = events;
        m_profiles = profiles;
        m_upcoming = upcoming;
        m_settings = settings;
        refreshDynamicContent();
    }

    void TodayPage::changeEvent(QEvent *event)
    {
        if (event->type() == QEvent::LanguageChange)
        {
            retranslateUi();
            refreshDynamicContent();
        }
        QWidget::changeEvent(event);
    }

    void TodayPage::showEvent(QShowEvent *event)
    {
        QWidget::showEvent(event);
        if (m_updateTimer && !m_updateTimer->isActive())
        {
            m_updateTimer->start();
        }
    }

    void TodayPage::hideEvent(QHideEvent *event)
    {
        QWidget::hideEvent(event);
        if (m_updateTimer && m_updateTimer->isActive())
        {
            m_updateTimer->stop();
        }
    }

    void TodayPage::retranslateUi()
    {
        m_nextBreakCaptionLabel->setText(tr("Next break"));
        m_rulesCaptionLabel->setText(tr("Active profiles"));
        m_eventsCaptionLabel->setText(tr("Today's events"));
        m_manageProfilesButton->setText(tr("Manage profiles"));
        m_manageEventsButton->setText(tr("Manage events"));
        m_rulesTable->setHorizontalHeaderLabels({tr("Profile"), tr("Mode"), tr("Cadence")});
        m_eventsTable->setHorizontalHeaderLabels({tr("Time"), tr("Title"), tr("Profile"), tr("Repeat")});
    }

    void TodayPage::refreshDynamicContent()
    {
        const QLocale locale;
        const QDateTime now = QDateTime::currentDateTime();

        m_pauseStatusLabel->setText(formatPauseUntil(m_settings.pauseUntil, locale, this));
        m_greetingLabel->setText(tr("%1. Here's your day at a glance.").arg(greetingForTime(now.time(), this)));

        QString nextBreak = tr("No break scheduled");
        for (const auto &occurrence : m_upcoming)
        {
            if (occurrence.mode == domain::SeverityMode::Break || occurrence.breakDurationMinutes > 0)
            {
                nextBreak = formatRelativeDue(occurrence.dueAt, now, this);
                break;
            }
        }
        m_nextBreakValueLabel->setText(nextBreak);

        QList<domain::ReminderProfile> activeProfiles;
        for (const auto &profile : m_profiles)
        {
            if (profile.enabled)
            {
                activeProfiles.push_back(profile);
            }
        }

        m_rulesTable->setRowCount(activeProfiles.size());
        for (int row = 0; row < activeProfiles.size(); ++row)
        {
            const auto &profile = activeProfiles.at(row);
            m_rulesTable->setItem(row, 0, new QTableWidgetItem(profile.name));
            m_rulesTable->setItem(row, 1, new QTableWidgetItem(displaySeverityMode(profile.severityMode, this)));
            m_rulesTable->setItem(row, 2, new QTableWidgetItem(tr("%1 min work / %2 min break").arg(profile.intervalMinutes).arg(profile.breakDurationMinutes)));
        }

        QList<domain::ReminderEvent> todaysEvents;
        for (const auto &event : m_events)
        {
            const QDateTime candidate = event.nextTriggerAt.isValid() ? event.nextTriggerAt : event.startAt;
            if (candidate.isValid() && candidate.date() == now.date())
            {
                todaysEvents.push_back(event);
            }
        }

        QHash<qint64, QString> profileNames;
        for (const auto &profile : m_profiles)
        {
            profileNames.insert(profile.id, profile.name);
        }

        m_eventsTable->setRowCount(todaysEvents.size());
        for (int row = 0; row < todaysEvents.size(); ++row)
        {
            const auto &event = todaysEvents.at(row);
            const QDateTime when = event.nextTriggerAt.isValid() ? event.nextTriggerAt : event.startAt;
            m_eventsTable->setItem(row, 0, new QTableWidgetItem(formatDateTimeShort(when, locale)));
            m_eventsTable->setItem(row, 1, new QTableWidgetItem(event.title));
            m_eventsTable->setItem(row, 2, new QTableWidgetItem(profileNames.value(event.profileId)));
            m_eventsTable->setItem(row, 3, new QTableWidgetItem(displayRecurrenceRule(event.recurrenceRule, this)));
        }
    }

} // namespace deadliner::ui
