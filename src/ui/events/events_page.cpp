#include "ui/events/events_page.h"

#include "ui/common/display_strings.h"
#include "ui/widgets/empty_state_widget.h"

#include <QEvent>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QAbstractItemView>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace deadliner::ui {

EventsPage::EventsPage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet(QStringLiteral("font-size: 22px; font-weight: 600;"));
    m_subtitleLabel = new QLabel(this);
    m_subtitleLabel->setWordWrap(true);

    auto *actionsLayout = new QHBoxLayout();
    m_addButton = new QPushButton(this);
    m_editButton = new QPushButton(this);
    m_deleteButton = new QPushButton(this);
    actionsLayout->addWidget(m_addButton);
    actionsLayout->addWidget(m_editButton);
    actionsLayout->addWidget(m_deleteButton);
    actionsLayout->addStretch();

    m_stack = new QStackedWidget(this);
    m_emptyState = new EmptyStateWidget(this);
    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);

    m_stack->addWidget(m_emptyState);
    m_stack->addWidget(m_table);

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_subtitleLabel);
    layout->addLayout(actionsLayout);
    layout->addWidget(m_stack, 1);

    connect(m_addButton, &QPushButton::clicked, this, &EventsPage::addRequested);
    connect(m_editButton, &QPushButton::clicked, this, [this]() { emit editRequested(selectedEventId()); });
    connect(m_deleteButton, &QPushButton::clicked, this, [this]() { emit deleteRequested(selectedEventId()); });
    connect(m_emptyState, &EmptyStateWidget::actionTriggered, this, &EventsPage::addRequested);

    retranslateUi();
}

void EventsPage::setState(const QList<domain::ReminderEvent> &events,
                          const QList<domain::ReminderProfile> &profiles)
{
    retranslateUi();
    m_events = events;
    m_profiles = profiles;

    QHash<qint64, QString> profileNames;
    for (const auto &profile : profiles) {
        profileNames.insert(profile.id, profile.name);
    }

    m_stack->setCurrentWidget(events.isEmpty() ? static_cast<QWidget *>(m_emptyState) : static_cast<QWidget *>(m_table));
    m_table->setRowCount(events.size());
    const QLocale locale;
    for (int row = 0; row < events.size(); ++row) {
        const auto &event = events.at(row);
        auto *titleItem = new QTableWidgetItem(event.title);
        titleItem->setData(Qt::UserRole, event.id);
        m_table->setItem(row, 0, titleItem);
        m_table->setItem(row, 1, new QTableWidgetItem(displayReminderType(event.type, this)));
        m_table->setItem(row, 2, new QTableWidgetItem(displayRecurrenceRule(event.recurrenceRule, this)));
        m_table->setItem(row, 3, new QTableWidgetItem(profileNames.value(event.profileId)));
        m_table->setItem(row, 4, new QTableWidgetItem(formatDateTimeShort(event.nextTriggerAt, locale)));
        m_table->setItem(row, 5, new QTableWidgetItem(event.enabled ? tr("Enabled") : tr("Paused")));
    }
}

qint64 EventsPage::selectedEventId() const
{
    if (m_table->currentRow() < 0 || m_table->item(m_table->currentRow(), 0) == nullptr) {
        return 0;
    }
    return m_table->item(m_table->currentRow(), 0)->data(Qt::UserRole).toLongLong();
}

void EventsPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
        setState(m_events, m_profiles);
    }
    QWidget::changeEvent(event);
}

void EventsPage::retranslateUi()
{
    m_titleLabel->setText(tr("Events"));
    m_subtitleLabel->setText(tr("One-time and recurring reminders with profile-based behavior."));
    m_addButton->setText(tr("Create event"));
    m_editButton->setText(tr("Edit"));
    m_deleteButton->setText(tr("Delete"));
    m_emptyState->setTitle(tr("No upcoming events."));
    m_emptyState->setDescription(tr("You have no upcoming events."));
    m_emptyState->setActionText(tr("Create event"));
    m_table->setHorizontalHeaderLabels({tr("Title"), tr("Type"), tr("Repeat"), tr("Profile"), tr("Next trigger"), tr("State")});
}

}  // namespace deadliner::ui
