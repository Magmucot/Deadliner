#pragma once

#include "domain/models.h"

#include <QSqlDatabase>
#include <QString>

#include <functional>

class QSqlQuery;

namespace deadliner::infrastructure {

struct RepositoryState {
    QList<domain::ReminderProfile> profiles;
    QList<domain::ReminderEvent> events;
    QList<domain::QuietHoursPolicy> quietHoursPolicies;
    QList<domain::StatsDaily> stats;
};

class Repository {
public:
    Repository();
    ~Repository();

    bool open(const QString &databasePath, QString *errorMessage = nullptr);
    RepositoryState loadState() const;

    QList<domain::ReminderProfile> loadProfiles() const;
    QList<domain::ReminderEvent> loadEvents() const;
    QList<domain::QuietHoursPolicy> loadQuietHoursPolicies() const;
    QList<domain::StatsDaily> loadDailyStats(int limit = 30) const;

    qint64 saveProfile(const domain::ReminderProfile &profile);
    bool deleteProfile(qint64 id);
    bool profileExists(const QString &name, qint64 excludeId = 0) const;

    qint64 saveEvent(const domain::ReminderEvent &event);
    bool deleteEvent(qint64 id);
    bool eventExists(const QString &title, const QDateTime &startAt, qint64 profileId, qint64 excludeId = 0) const;

    qint64 saveQuietHoursPolicy(const domain::QuietHoursPolicy &policy);
    domain::QuietHoursPolicy loadQuietHoursPolicy(qint64 id) const;

    bool logOccurrence(const domain::OccurrenceLogEntry &entry);
    bool upsertDailyStats(const domain::StatsDaily &stats);

    bool ensureSeedData();

private:
    bool runMigrations();
    bool cleanupDuplicates();
    qint64 insertOrUpdate(const QString &sqlInsert, const QString &sqlUpdate, qint64 id, const std::function<void(QSqlQuery &)> &binder) const;
    void bindProfile(QSqlQuery &query, const domain::ReminderProfile &profile) const;
    void bindEvent(QSqlQuery &query, const domain::ReminderEvent &event) const;
    void bindQuietPolicy(QSqlQuery &query, const domain::QuietHoursPolicy &policy) const;

    QSqlDatabase m_db;
    QString m_connectionName;
};

}  // namespace deadliner::infrastructure
