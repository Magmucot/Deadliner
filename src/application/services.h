#pragma once

#include "domain/models.h"

#include <QHash>
#include <QObject>
#include <QTimer>

namespace deadliner::application {

class ReminderScheduler : public QObject {
    Q_OBJECT

public:
    explicit ReminderScheduler(QObject *parent = nullptr);

    void setState(const QList<domain::ReminderEvent> &events,
                  const QHash<qint64, domain::ReminderProfile> &profiles);
    QList<domain::ReminderOccurrence> upcomingOccurrences() const;

public slots:
    void rebuild();

signals:
    void occurrenceDue(const domain::ReminderOccurrence &occurrence);

private slots:
    void onTimeout();

private:
    void armNext();
    QList<domain::ReminderOccurrence> buildOccurrences(const QDateTime &now) const;

    QList<domain::ReminderEvent> m_events;
    QHash<qint64, domain::ReminderProfile> m_profiles;
    QTimer m_timer;
};

class StatisticsService {
public:
    domain::StatsDaily updateDaily(const QList<domain::StatsDaily> &current, const domain::OccurrenceLogEntry &entry) const;
};

}  // namespace deadliner::application
