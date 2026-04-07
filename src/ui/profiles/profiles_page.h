#pragma once

#include "domain/models.h"

#include <QEvent>
#include <QWidget>

class QLabel;
class QPushButton;
class QStackedWidget;
class QTableWidget;

namespace deadliner::ui
{

    class EmptyStateWidget;

    class ProfilesPage : public QWidget
    {
        Q_OBJECT

    public:
        explicit ProfilesPage(QWidget *parent = nullptr);

        void setState(const QList<domain::ReminderProfile> &profiles);
        qint64 selectedProfileId() const;

    signals:
        void addRequested();
        void editRequested(qint64 id);
        void deleteRequested(qint64 id);

    protected:
        void changeEvent(QEvent *event) override;

    private:
        void retranslateUi();

        QList<domain::ReminderProfile> m_profiles;

        QLabel *m_titleLabel = nullptr;
        QLabel *m_subtitleLabel = nullptr;
        QPushButton *m_addButton = nullptr;
        QPushButton *m_editButton = nullptr;
        QPushButton *m_deleteButton = nullptr;
        QStackedWidget *m_stack = nullptr;
        EmptyStateWidget *m_emptyState = nullptr;
        QTableWidget *m_table = nullptr;
    };

} // namespace deadliner::ui
