#pragma once

#include "domain/models.h"

#include <QDateTime>
#include <QEvent>
#include <QWidget>

class QLabel;
class QPushButton;
class QStackedWidget;
class QTableWidget;

namespace deadliner::ui {

class EmptyStateWidget;

class OverduePage : public QWidget {
    Q_OBJECT

public:
    explicit OverduePage(QWidget *parent = nullptr);

    void setState(const QList<domain::ReminderEvent> &events,
                  const QList<domain::ReminderProfile> &profiles,
                  const QDateTime &now);
    qint64 selectedEventId() const;

signals:
    void editRequested(qint64 id);
    void deleteRequested(qint64 id);

protected:
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();

    QList<domain::ReminderEvent> m_events;
    QList<domain::ReminderProfile> m_profiles;
    QDateTime m_now;

    QLabel *m_titleLabel = nullptr;
    QLabel *m_subtitleLabel = nullptr;
    QPushButton *m_editButton = nullptr;
    QPushButton *m_deleteButton = nullptr;
    QStackedWidget *m_stack = nullptr;
    EmptyStateWidget *m_emptyState = nullptr;
    QTableWidget *m_table = nullptr;
};

} // namespace deadliner::ui
