#pragma once

#include "domain/models.h"

#include <QEvent>
#include <QWidget>

class QLabel;
class QStackedWidget;
class QTableWidget;

namespace deadliner::ui
{

    class EmptyStateWidget;

    class StatisticsPage : public QWidget
    {
        Q_OBJECT

    public:
        explicit StatisticsPage(QWidget *parent = nullptr);

        void setState(const QList<domain::StatsDaily> &stats);

    protected:
        void changeEvent(QEvent *event) override;

    private:
        void retranslateUi();

        QList<domain::StatsDaily> m_stats;

        QLabel *m_titleLabel = nullptr;
        QLabel *m_subtitleLabel = nullptr;
        QLabel *m_completedValueLabel = nullptr;
        QLabel *m_skippedValueLabel = nullptr;
        QLabel *m_snoozedValueLabel = nullptr;
        QStackedWidget *m_stack = nullptr;
        EmptyStateWidget *m_emptyState = nullptr;
        QTableWidget *m_table = nullptr;
    };

} // namespace deadliner::ui
