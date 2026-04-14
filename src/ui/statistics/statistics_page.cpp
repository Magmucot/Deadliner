#include "ui/statistics/statistics_page.h"

#include "ui/widgets/empty_state_widget.h"

#include <QEvent>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QAbstractItemView>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace deadliner::ui
{

    namespace
    {

        QWidget *createSummaryCard(const QString &title, QLabel **valueLabel, QWidget *parent)
        {
            auto *card = new QWidget(parent);
            card->setStyleSheet(QStringLiteral("QWidget { border: 1px solid palette(midlight); border-radius: 8px; }"));
            auto *layout = new QVBoxLayout(card);
            layout->setContentsMargins(16, 16, 16, 16);
            auto *titleLabel = new QLabel(title, card);
            titleLabel->setProperty("summaryTitle", true);
            auto *value = new QLabel(card);
            value->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 700;"));
            layout->addWidget(titleLabel);
            layout->addWidget(value);
            *valueLabel = value;
            return card;
        }

    } // namespace

    StatisticsPage::StatisticsPage(QWidget *parent)
        : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(24, 24, 24, 24);
        layout->setSpacing(16);

        m_titleLabel = new QLabel(this);
        m_titleLabel->setStyleSheet(QStringLiteral("font-size: 22px; font-weight: 600;"));
        m_subtitleLabel = new QLabel(this);
        m_subtitleLabel->setWordWrap(true);

        auto *summaryLayout = new QGridLayout();
        summaryLayout->setHorizontalSpacing(12);
        summaryLayout->addWidget(createSummaryCard(tr("Completed"), &m_completedValueLabel, this), 0, 0);
        summaryLayout->addWidget(createSummaryCard(tr("Skipped"), &m_skippedValueLabel, this), 0, 1);
        summaryLayout->addWidget(createSummaryCard(tr("Snoozed"), &m_snoozedValueLabel, this), 0, 2);

        m_stack = new QStackedWidget(this);
        m_emptyState = new EmptyStateWidget(this);
        m_table = new QTableWidget(this);
        m_table->setColumnCount(6);
        m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_table->verticalHeader()->setVisible(false);
        m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_table->setSelectionMode(QAbstractItemView::NoSelection);
        m_table->setAlternatingRowColors(true);

        m_stack->addWidget(m_emptyState);
        m_stack->addWidget(m_table);

        layout->addWidget(m_titleLabel);
        layout->addWidget(m_subtitleLabel);
        layout->addLayout(summaryLayout);
        layout->addWidget(m_stack, 1);

        retranslateUi();
    }

    void StatisticsPage::setState(const QList<domain::StatsDaily> &stats)
    {
        retranslateUi();
        m_stats = stats;
        int completed = 0;
        int skipped = 0;
        int snoozed = 0;
        for (const auto &day : stats)
        {
            completed += day.completedCount;
            skipped += day.skippedCount;
            snoozed += day.snoozedCount;
        }

        m_completedValueLabel->setText(QString::number(completed));
        m_skippedValueLabel->setText(QString::number(skipped));
        m_snoozedValueLabel->setText(QString::number(snoozed));

        m_stack->setCurrentWidget(stats.isEmpty() ? static_cast<QWidget *>(m_emptyState) : static_cast<QWidget *>(m_table));
        m_table->setRowCount(stats.size());
        const QLocale locale;
        for (int row = 0; row < stats.size(); ++row)
        {
            const auto &day = stats.at(row);
            m_table->setItem(row, 0, new QTableWidgetItem(locale.toString(day.date, QLocale::ShortFormat)));
            m_table->setItem(row, 1, new QTableWidgetItem(QString::number(day.completedCount)));
            m_table->setItem(row, 2, new QTableWidgetItem(QString::number(day.skippedCount)));
            m_table->setItem(row, 3, new QTableWidgetItem(QString::number(day.snoozedCount)));
            m_table->setItem(row, 4, new QTableWidgetItem(QString::number(day.breakCompletedCount)));
            m_table->setItem(row, 5, new QTableWidgetItem(QString::number(day.breakMissedCount)));
        }
    }

    void StatisticsPage::changeEvent(QEvent *event)
    {
        if (event->type() == QEvent::LanguageChange)
        {
            retranslateUi();
            setState(m_stats);
        }
        QWidget::changeEvent(event);
    }

    void StatisticsPage::retranslateUi()
    {
        m_titleLabel->setText(tr("Statistics"));
        m_subtitleLabel->setText(tr("Lightweight local analytics for completed, skipped and snoozed reminders."));
        m_emptyState->setTitle(tr("No statistics yet."));
        m_emptyState->setDescription(tr("Statistics will appear after reminders start running."));
        m_emptyState->setActionText(QString());
        m_table->setHorizontalHeaderLabels({tr("Date"), tr("Completed"), tr("Skipped"), tr("Snoozed"), tr("Break completed"), tr("Break missed")});
    }

} // namespace deadliner::ui
