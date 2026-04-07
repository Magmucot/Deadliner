#include "ui/main_window.h"

#include "ui/events/events_page.h"
#include "ui/profiles/profiles_page.h"
#include "ui/settings/settings_page.h"
#include "ui/statistics/statistics_page.h"
#include "ui/today/today_page.h"

#include <QCloseEvent>
#include <QEvent>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

namespace deadliner::ui
{

    namespace
    {

        QString subtitleForSection(MainSection section, const QObject *context)
        {
            switch (section)
            {
            case MainSection::Today:
                return context->tr("Overview of upcoming reminders, active break profiles and today's events.");
            case MainSection::Events:
                return context->tr("Manage one-time and recurring reminders.");
            case MainSection::Profiles:
                return context->tr("Tune reminder modes, break duration and snooze rules.");
            case MainSection::Statistics:
                return context->tr("Check local reminder outcomes and adherence trends.");
            case MainSection::Settings:
                return context->tr("Adjust startup behavior, quiet hours, theme and language.");
            }
            return {};
        }

    } // namespace

    MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
    {
        setWindowTitle(tr("Deadliner"));
        resize(1180, 760);
        setMinimumSize(980, 640);

        auto *central = new QWidget(this);
        auto *rootLayout = new QHBoxLayout(central);
        rootLayout->setContentsMargins(0, 0, 0, 0);
        rootLayout->setSpacing(0);

        // ── Sidebar ─────────────────────────────────────────────────────────────
        auto *sidebarWidget = new QWidget(central);
        sidebarWidget->setObjectName(QStringLiteral("sidebarWidget"));
        sidebarWidget->setMinimumWidth(220);
        sidebarWidget->setMaximumWidth(280);
        // Border drawn via palette so it responds to theme changes.
        sidebarWidget->setStyleSheet(QStringLiteral(
            "#sidebarWidget { border-right: 1px solid palette(midlight); }"));

        auto *sidebarLayout = new QVBoxLayout(sidebarWidget);
        sidebarLayout->setContentsMargins(16, 20, 16, 20);
        sidebarLayout->setSpacing(12);

        auto *brandLabel = new QLabel(tr("Deadliner"), sidebarWidget);
        brandLabel->setStyleSheet(QStringLiteral(
            "font-size: 20px; font-weight: 700; padding: 4px 6px 12px 6px;"));

        m_navigationList = new QListWidget(sidebarWidget);
        m_navigationList->setObjectName(QStringLiteral("navigationList"));
        // Frame and scrollbar handled by global QSS; disable the built-in frame.
        m_navigationList->setFrameShape(QFrame::NoFrame);
        m_navigationList->setSpacing(2);
        m_navigationList->setUniformItemSizes(true);
        m_navigationList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        sidebarLayout->addWidget(brandLabel);
        sidebarLayout->addWidget(m_navigationList, 1);

        // ── Content area ────────────────────────────────────────────────────────
        auto *contentWidget = new QWidget(central);
        auto *contentLayout = new QVBoxLayout(contentWidget);
        contentLayout->setContentsMargins(28, 24, 28, 24);
        contentLayout->setSpacing(6);

        m_titleLabel = new QLabel(contentWidget);
        m_titleLabel->setStyleSheet(QStringLiteral("font-size: 26px; font-weight: 700;"));
        m_subtitleLabel = new QLabel(contentWidget);
        m_subtitleLabel->setWordWrap(true);
        m_subtitleLabel->setStyleSheet(QStringLiteral("color: palette(mid); margin-bottom: 8px;"));

        m_pages = new QStackedWidget(contentWidget);
        m_todayPage = new TodayPage(m_pages);
        m_eventsPage = new EventsPage(m_pages);
        m_profilesPage = new ProfilesPage(m_pages);
        m_statisticsPage = new StatisticsPage(m_pages);
        m_settingsPage = new SettingsPage(m_pages);

        m_pages->addWidget(m_todayPage);
        m_pages->addWidget(m_eventsPage);
        m_pages->addWidget(m_profilesPage);
        m_pages->addWidget(m_statisticsPage);
        m_pages->addWidget(m_settingsPage);

        contentLayout->addWidget(m_titleLabel);
        contentLayout->addWidget(m_subtitleLabel);
        contentLayout->addWidget(m_pages, 1);

        rootLayout->addWidget(sidebarWidget);
        rootLayout->addWidget(contentWidget, 1);
        setCentralWidget(central);

        // ── Connections ─────────────────────────────────────────────────────────
        connect(m_navigationList, &QListWidget::currentRowChanged, this, [this](int row)
                {
        if (row < 0 || row >= m_pages->count()) {
            return;
        }
        m_currentSection = static_cast<MainSection>(row);
        m_pages->setCurrentIndex(row);
        retranslateUi(); });

        connect(m_todayPage, &TodayPage::openEventsRequested, this, [this]()
                { setCurrentSection(MainSection::Events); });
        connect(m_todayPage, &TodayPage::openProfilesRequested, this, [this]()
                { setCurrentSection(MainSection::Profiles); });
        connect(m_eventsPage, &EventsPage::addRequested, this, &MainWindow::addEventRequested);
        connect(m_eventsPage, &EventsPage::editRequested, this, &MainWindow::editEventRequested);
        connect(m_eventsPage, &EventsPage::deleteRequested, this, &MainWindow::deleteEventRequested);
        connect(m_profilesPage, &ProfilesPage::addRequested, this, &MainWindow::addProfileRequested);
        connect(m_profilesPage, &ProfilesPage::editRequested, this, &MainWindow::editProfileRequested);
        connect(m_profilesPage, &ProfilesPage::deleteRequested, this, &MainWindow::deleteProfileRequested);
        connect(m_settingsPage, &SettingsPage::saveRequested, this, &MainWindow::saveSettingsRequested);

        retranslateUi();
        setCurrentSection(MainSection::Today);
    }

    void MainWindow::setState(const QList<domain::ReminderEvent> &events,
                              const QList<domain::ReminderProfile> &profiles,
                              const QList<domain::StatsDaily> &stats,
                              const QList<domain::ReminderOccurrence> &upcoming,
                              const QList<domain::QuietHoursPolicy> &policies,
                              const domain::AppSettings &settings,
                              bool autostartEnabled,
                              bool hasTray)
    {
        m_events = events;
        m_profiles = profiles;
        m_stats = stats;
        m_upcoming = upcoming;
        m_policies = policies;
        m_settings = settings;
        m_autostartEnabled = autostartEnabled;
        m_hasTray = hasTray;

        m_todayPage->setState(events, profiles, upcoming, settings);
        m_eventsPage->setState(events, profiles);
        m_profilesPage->setState(profiles);
        m_statisticsPage->setState(stats);
        m_settingsPage->setState(profiles, policies, settings, autostartEnabled, hasTray);
        retranslateUi();
    }

    void MainWindow::setCurrentSection(MainSection section)
    {
        m_currentSection = section;
        syncNavigationSelection();
        m_pages->setCurrentIndex(static_cast<int>(section));
        retranslateUi();
    }

    void MainWindow::closeEvent(QCloseEvent *event)
    {
        if (m_settings.closeToTray && m_hasTray)
        {
            hide();
            event->ignore();
            return;
        }

        emit closeRequested();
        event->ignore();
    }

    void MainWindow::changeEvent(QEvent *event)
    {
        if (event->type() == QEvent::LanguageChange)
        {
            retranslateUi();
        }
        QMainWindow::changeEvent(event);
    }

    void MainWindow::retranslateUi()
    {
        setWindowTitle(tr("Deadliner"));

        const QStringList titles = {
            tr("Today"),
            tr("Events"),
            tr("Profiles"),
            tr("Statistics"),
            tr("Settings"),
        };

        if (m_navigationList->count() == 0)
        {
            for (const auto &title : titles)
            {
                m_navigationList->addItem(title);
            }
        }
        else
        {
            for (int i = 0; i < titles.size(); ++i)
            {
                m_navigationList->item(i)->setText(titles.at(i));
            }
        }

        m_titleLabel->setText(titles.at(static_cast<int>(m_currentSection)));
        m_subtitleLabel->setText(subtitleForSection(m_currentSection, this));
    }

    void MainWindow::syncNavigationSelection()
    {
        const int row = static_cast<int>(m_currentSection);
        if (m_navigationList->currentRow() != row)
        {
            m_navigationList->setCurrentRow(row);
        }
    }

} // namespace deadliner::ui