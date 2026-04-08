#include "ui/profiles/profiles_page.h"

#include "ui/common/display_strings.h"
#include "ui/widgets/empty_state_widget.h"

#include <QEvent>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QAbstractItemView>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace deadliner::ui
{

    ProfileDialog::ProfileDialog(const QList<domain::QuietHoursPolicy> &policies,
                                 const domain::ReminderProfile *profile,
                                 QWidget *parent)
        : QDialog(parent)
    {
        resize(520, 420);

        auto *layout = new QVBoxLayout(this);
        m_form = new QFormLayout();
        m_form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        m_form->setRowWrapPolicy(QFormLayout::WrapLongRows);
        m_form->setHorizontalSpacing(12);
        m_form->setVerticalSpacing(10);
    }

    ProfilesPage::ProfilesPage(QWidget *parent)
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

        connect(m_addButton, &QPushButton::clicked, this, &ProfilesPage::addRequested);
        connect(m_editButton, &QPushButton::clicked, this, [this]()
                { emit editRequested(selectedProfileId()); });
        connect(m_deleteButton, &QPushButton::clicked, this, [this]()
                { emit deleteRequested(selectedProfileId()); });
        connect(m_emptyState, &EmptyStateWidget::actionTriggered, this, &ProfilesPage::addRequested);

        retranslateUi();
    }

    void ProfilesPage::setState(const QList<domain::ReminderProfile> &profiles)
    {
        m_profiles = profiles;
        m_stack->setCurrentWidget(profiles.isEmpty() ? static_cast<QWidget *>(m_emptyState) : static_cast<QWidget *>(m_table));
        m_table->setRowCount(profiles.size());

        for (int row = 0; row < profiles.size(); ++row)
        {
            const auto &profile = profiles.at(row);
            auto *nameItem = new QTableWidgetItem(profile.name);
            nameItem->setData(Qt::UserRole, profile.id);
            m_table->setItem(row, 0, nameItem);
            m_table->setItem(row, 1, new QTableWidgetItem(displayProfileKind(profile.kind, this)));
            m_table->setItem(row, 2, new QTableWidgetItem(displaySeverityMode(profile.severityMode, this)));
            m_table->setItem(row, 3, new QTableWidgetItem(tr("%1 / %2 min").arg(profile.intervalMinutes).arg(profile.breakDurationMinutes)));
            m_table->setItem(row, 4, new QTableWidgetItem(tr("%1 x %2 min").arg(profile.maxSnoozeCount).arg(profile.snoozeMinutes)));
            m_table->setItem(row, 5, new QTableWidgetItem(profile.builtIn ? tr("Built-in") : tr("Custom")));
        }
    }

    qint64 ProfilesPage::selectedProfileId() const
    {
        if (m_table->currentRow() < 0 || m_table->item(m_table->currentRow(), 0) == nullptr)
        {
            return 0;
        }
        return m_table->item(m_table->currentRow(), 0)->data(Qt::UserRole).toLongLong();
    }

    void ProfilesPage::changeEvent(QEvent *event)
    {
        if (event->type() == QEvent::LanguageChange)
        {
            retranslateUi();
            setState(m_profiles);
        }
        QWidget::changeEvent(event);
    }

    void ProfilesPage::retranslateUi()
    {
        m_titleLabel->setText(tr("Profiles"));
        m_subtitleLabel->setText(tr("Profiles define reminder intensity, snooze limits and break behavior."));
        m_addButton->setText(tr("Create profile"));
        m_editButton->setText(tr("Edit"));
        m_deleteButton->setText(tr("Delete"));
        m_emptyState->setTitle(tr("No reminder profiles yet."));
        m_emptyState->setDescription(tr("Create at least one profile to control reminder behavior."));
        m_emptyState->setActionText(tr("Create profile"));
        m_table->setHorizontalHeaderLabels({tr("Name"), tr("Scope"), tr("Mode"), tr("Cadence"), tr("Snooze"), tr("Origin")});
    }

} // namespace deadliner::ui
