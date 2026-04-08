#include "ui/overdue/overdue_page.h"

#include "ui/common/display_strings.h"
#include "ui/widgets/empty_state_widget.h"

#include <QAbstractItemView>
#include <QEvent>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace deadliner::ui {

namespace {

QString formatOverdueDuration(const QDateTime &dueAt, const QDateTime &now, const QObject *context)
{
    if (!dueAt.isValid() || dueAt >= now) {
        return context->tr("Due now");
    }

    qint64 seconds = dueAt.secsTo(now);
    const qint64 days = seconds / 86400;
    seconds %= 86400;
    const qint64 hours = seconds / 3600;
    seconds %= 3600;
    const qint64 minutes = qMax<qint64>(1, (seconds + 59) / 60);

    if (days > 0) {
        return context->tr("%1 d %2 h late").arg(days).arg(hours);
    }
    if (hours > 0) {
        return context->tr("%1 h %2 min late").arg(hours).arg(minutes);
    }
    return context->tr("%1 min late").arg(minutes);
}

} // namespace

OverduePage::OverduePage(QWidget *parent)
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
    m_editButton = new QPushButton(this);
    m_deleteButton = new QPushButton(this);
    actionsLayout->addWidget(m_editButton);
    actionsLayout->addWidget(m_deleteButton);
    actionsLayout->addStretch();

    m_stack = new QStackedWidget(this);
    m_emptyState = new EmptyStateWidget(this);
    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
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

    connect(m_editButton, &QPushButton::clicked, this, [this]() { emit editRequested(selectedEventId()); });
    connect(m_deleteButton, &QPushButton::clicked, this, [this]() { emit deleteRequested(selectedEventId()); });

    retranslateUi();
}

void OverduePage::setState(const QList<domain::ReminderEvent> &events,
                           const QList<domain::ReminderProfile> &profiles,
                           const QDateTime &now)
{
    m_events = events;
    m_profiles = profiles;
    m_now = now;

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
        m_table->setItem(row, 1, new QTableWidgetItem(profileNames.value(event.profileId)));
        m_table->setItem(row, 2, new QTableWidgetItem(formatDateTimeShort(event.startAt, locale)));
        m_table->setItem(row, 3, new QTableWidgetItem(formatOverdueDuration(event.startAt, m_now, this)));
    }
}

qint64 OverduePage::selectedEventId() const
{
    if (m_table->currentRow() < 0 || m_table->item(m_table->currentRow(), 0) == nullptr) {
        return 0;
    }

    return m_table->item(m_table->currentRow(), 0)->data(Qt::UserRole).toLongLong();
}

void OverduePage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
        setState(m_events, m_profiles, m_now);
    }

    QWidget::changeEvent(event);
}

void OverduePage::retranslateUi()
{
    m_titleLabel->setText(tr("Overdue"));
    m_subtitleLabel->setText(tr("One-time reminders that were missed and now need manual review."));
    m_editButton->setText(tr("Edit"));
    m_deleteButton->setText(tr("Delete"));
    m_emptyState->setTitle(tr("No overdue reminders."));
    m_emptyState->setDescription(tr("When a one-time reminder is missed, it will appear here instead of firing automatically."));
    m_emptyState->setActionText({});
    m_table->setHorizontalHeaderLabels({tr("Title"), tr("Profile"), tr("Due at"), tr("Overdue by")});
}

} // namespace deadliner::ui
