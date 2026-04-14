#include "infrastructure/storage/repository.h"

#include "domain/logic.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace deadliner::infrastructure
{

    namespace
    {

        using namespace deadliner::domain;

        QString nowText()
        {
            return QDateTime::currentDateTime().toString(Qt::ISODate);
        }

        bool columnExists(QSqlDatabase db, const QString &tableName, const QString &columnName)
        {
            QSqlQuery query(db);
            query.prepare(QStringLiteral("PRAGMA table_info(%1)").arg(tableName));
            if (!query.exec())
            {
                return false;
            }

            while (query.next())
            {
                if (query.value(1).toString() == columnName)
                {
                    return true;
                }
            }

            return false;
        }

        ReminderProfile profileFromQuery(const QSqlQuery &query)
        {
            ReminderProfile profile;
            profile.id = query.value(QStringLiteral("id")).toLongLong();
            profile.name = query.value(QStringLiteral("name")).toString();
            profile.kind = profileKindFromString(query.value(QStringLiteral("kind")).toString());
            profile.intervalMinutes = query.value(QStringLiteral("interval_minutes")).toInt();
            profile.breakDurationMinutes = query.value(QStringLiteral("break_duration_minutes")).toInt();
            profile.severityMode = severityModeFromString(query.value(QStringLiteral("severity_mode")).toString());
            profile.maxSnoozeCount = query.value(QStringLiteral("max_snooze_count")).toInt();
            profile.snoozeMinutes = query.value(QStringLiteral("snooze_minutes")).toInt();
            profile.quietHoursPolicyId = query.value(QStringLiteral("quiet_hours_policy_id")).toLongLong();
            profile.requirePostBreakConfirmation = query.value(QStringLiteral("require_post_break_confirmation")).toBool();
            profile.allowSkip = query.value(QStringLiteral("allow_skip")).toBool();
            profile.enabled = query.value(QStringLiteral("enabled")).toBool();
            profile.builtIn = query.value(QStringLiteral("built_in")).toBool();
            profile.createdAt = query.value(QStringLiteral("created_at")).toDateTime();
            profile.updatedAt = query.value(QStringLiteral("updated_at")).toDateTime();
            return profile;
        }

        ReminderEvent eventFromQuery(const QSqlQuery &query)
        {
            ReminderEvent event;
            event.id = query.value(QStringLiteral("id")).toLongLong();
            event.title = query.value(QStringLiteral("title")).toString();
            event.description = query.value(QStringLiteral("description")).toString();
            event.type = reminderTypeFromString(query.value(QStringLiteral("type")).toString());
            event.profileId = query.value(QStringLiteral("profile_id")).toLongLong();
            event.startAt = query.value(QStringLiteral("start_at")).toDateTime();
            event.timezoneMode = query.value(QStringLiteral("timezone_mode")).toString();
            event.recurrenceRule = query.value(QStringLiteral("recurrence_rule")).toString();
            event.isOneTime = query.value(QStringLiteral("is_one_time")).toBool();
            event.skipMissedOccurrences = query.value(QStringLiteral("skip_missed_occurrences")).toBool();
            event.enabled = query.value(QStringLiteral("enabled")).toBool();
            event.lastTriggeredAt = query.value(QStringLiteral("last_triggered_at")).toDateTime();
            event.nextTriggerAt = query.value(QStringLiteral("next_trigger_at")).toDateTime();
            event.pendingSnoozeCount = query.value(QStringLiteral("pending_snooze_count")).toInt();
            event.pendingOriginalTriggerAt = query.value(QStringLiteral("pending_original_trigger_at")).toDateTime();
            const QString pendingMode = query.value(QStringLiteral("pending_mode")).toString();
            if (!pendingMode.isEmpty())
            {
                event.pendingMode = severityModeFromString(pendingMode);
            }
            event.createdAt = query.value(QStringLiteral("created_at")).toDateTime();
            event.updatedAt = query.value(QStringLiteral("updated_at")).toDateTime();
            return event;
        }

        QuietHoursPolicy quietPolicyFromQuery(const QSqlQuery &query)
        {
            QuietHoursPolicy policy;
            policy.id = query.value(QStringLiteral("id")).toLongLong();
            policy.name = query.value(QStringLiteral("name")).toString();
            policy.startTime = query.value(QStringLiteral("start_time")).toTime();
            policy.endTime = query.value(QStringLiteral("end_time")).toTime();
            policy.behaviorSoft = quietBehaviorFromString(query.value(QStringLiteral("behavior_soft")).toString());
            policy.behaviorPersistent = quietBehaviorFromString(query.value(QStringLiteral("behavior_persistent")).toString());
            policy.behaviorBreak = quietBehaviorFromString(query.value(QStringLiteral("behavior_break")).toString());
            policy.createdAt = query.value(QStringLiteral("created_at")).toDateTime();
            policy.updatedAt = query.value(QStringLiteral("updated_at")).toDateTime();
            return policy;
        }

        StatsDaily statsFromQuery(const QSqlQuery &query)
        {
            StatsDaily stats;
            stats.date = query.value(QStringLiteral("date")).toDate();
            stats.completedCount = query.value(QStringLiteral("completed_count")).toInt();
            stats.skippedCount = query.value(QStringLiteral("skipped_count")).toInt();
            stats.snoozedCount = query.value(QStringLiteral("snoozed_count")).toInt();
            stats.breakCompletedCount = query.value(QStringLiteral("break_completed_count")).toInt();
            stats.breakMissedCount = query.value(QStringLiteral("break_missed_count")).toInt();
            return stats;
        }

    } // namespace

    Repository::Repository()
    {
        m_connectionName = QStringLiteral("deadliner_%1").arg(reinterpret_cast<quintptr>(this));
    }

    Repository::~Repository()
    {
        if (m_db.isValid())
        {
            m_db.close();
        }
        m_db = {};
        QSqlDatabase::removeDatabase(m_connectionName);
    }

    bool Repository::open(const QString &databasePath, QString *errorMessage)
    {
        QDir().mkpath(QFileInfo(databasePath).absolutePath());
        m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
        m_db.setDatabaseName(databasePath);
        if (!m_db.open())
        {
            if (errorMessage)
            {
                *errorMessage = m_db.lastError().text();
            }
            return false;
        }
        return runMigrations();
    }

    bool Repository::runMigrations()
    {
        const QStringList migrations = {
            QStringLiteral(
                "CREATE TABLE IF NOT EXISTS reminder_profiles ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "name TEXT NOT NULL,"
                "kind TEXT NOT NULL,"
                "interval_minutes INTEGER NOT NULL,"
                "break_duration_minutes INTEGER NOT NULL,"
                "severity_mode TEXT NOT NULL,"
                "max_snooze_count INTEGER NOT NULL,"
                "snooze_minutes INTEGER NOT NULL,"
                "quiet_hours_policy_id INTEGER NOT NULL,"
                "require_post_break_confirmation INTEGER NOT NULL,"
                "allow_skip INTEGER NOT NULL,"
                "enabled INTEGER NOT NULL,"
                "built_in INTEGER NOT NULL DEFAULT 0,"
                "created_at TEXT NOT NULL,"
                "updated_at TEXT NOT NULL"
                ")"),
            QStringLiteral(
                "CREATE TABLE IF NOT EXISTS reminder_events ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "title TEXT NOT NULL,"
                "description TEXT,"
                "type TEXT NOT NULL,"
                "profile_id INTEGER NOT NULL,"
                "start_at TEXT NOT NULL,"
                "timezone_mode TEXT NOT NULL,"
                "recurrence_rule TEXT NOT NULL,"
                "is_one_time INTEGER NOT NULL,"
                "skip_missed_occurrences INTEGER NOT NULL DEFAULT 1,"
                "enabled INTEGER NOT NULL,"
                "last_triggered_at TEXT,"
                "next_trigger_at TEXT,"
                "pending_snooze_count INTEGER NOT NULL DEFAULT 0,"
                "pending_original_trigger_at TEXT,"
                "pending_mode TEXT,"
                "created_at TEXT NOT NULL,"
                "updated_at TEXT NOT NULL"
                ")"),
            QStringLiteral(
                "CREATE TABLE IF NOT EXISTS quiet_hours_policies ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "name TEXT NOT NULL,"
                "start_time TEXT NOT NULL,"
                "end_time TEXT NOT NULL,"
                "behavior_soft TEXT NOT NULL,"
                "behavior_persistent TEXT NOT NULL,"
                "behavior_break TEXT NOT NULL,"
                "created_at TEXT NOT NULL,"
                "updated_at TEXT NOT NULL"
                ")"),
            QStringLiteral(
                "CREATE TABLE IF NOT EXISTS occurrence_log ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "event_id INTEGER NOT NULL,"
                "triggered_at TEXT NOT NULL,"
                "shown_mode TEXT NOT NULL,"
                "result TEXT NOT NULL,"
                "snooze_count INTEGER NOT NULL,"
                "break_expected_seconds INTEGER NOT NULL,"
                "break_actual_seconds INTEGER NOT NULL,"
                "completed_at TEXT"
                ")"),
            QStringLiteral(
                "CREATE TABLE IF NOT EXISTS stats_daily ("
                "date TEXT PRIMARY KEY,"
                "completed_count INTEGER NOT NULL,"
                "skipped_count INTEGER NOT NULL,"
                "snoozed_count INTEGER NOT NULL,"
                "break_completed_count INTEGER NOT NULL,"
                "break_missed_count INTEGER NOT NULL"
                ")"),
        };

        for (const QString &migration : migrations)
        {
            QSqlQuery query(m_db);
            if (!query.exec(migration))
            {
                qWarning() << "Migration failed" << query.lastError().text();
                return false;
            }
        }

        if (!columnExists(m_db, QStringLiteral("reminder_events"), QStringLiteral("skip_missed_occurrences")))
        {
            QSqlQuery query(m_db);
            if (!query.exec(QStringLiteral("ALTER TABLE reminder_events ADD COLUMN skip_missed_occurrences INTEGER NOT NULL DEFAULT 1")))
            {
                qWarning() << "Migration failed" << query.lastError().text();
                return false;
            }
        }

        if (!cleanupDuplicates())
        {
            return false;
        }

        return ensureSeedData();
    }

    bool Repository::cleanupDuplicates()
    {
        if (!m_db.transaction())
        {
            qWarning() << "Failed to start duplicate cleanup transaction" << m_db.lastError().text();
            return false;
        }

        QSqlQuery duplicateEventsQuery(m_db);
        duplicateEventsQuery.prepare(QStringLiteral(
            "SELECT id FROM reminder_events "
            "WHERE id NOT IN ("
            "  SELECT MIN(id) FROM reminder_events GROUP BY title, start_at, profile_id"
            ") ORDER BY id ASC"));
        if (!duplicateEventsQuery.exec())
        {
            qWarning() << "Failed to load duplicate events" << duplicateEventsQuery.lastError().text();
            m_db.rollback();
            return false;
        }

        while (duplicateEventsQuery.next())
        {
            const qint64 duplicateId = duplicateEventsQuery.value(0).toLongLong();

            QSqlQuery deleteLogQuery(m_db);
            deleteLogQuery.prepare(QStringLiteral("DELETE FROM occurrence_log WHERE event_id=?"));
            deleteLogQuery.addBindValue(duplicateId);
            if (!deleteLogQuery.exec())
            {
                qWarning() << "Failed to delete duplicate event logs" << deleteLogQuery.lastError().text();
                m_db.rollback();
                return false;
            }

            QSqlQuery deleteEventQuery(m_db);
            deleteEventQuery.prepare(QStringLiteral("DELETE FROM reminder_events WHERE id=?"));
            deleteEventQuery.addBindValue(duplicateId);
            if (!deleteEventQuery.exec())
            {
                qWarning() << "Failed to delete duplicate event" << deleteEventQuery.lastError().text();
                m_db.rollback();
                return false;
            }
        }

        if (!m_db.commit())
        {
            qWarning() << "Failed to commit duplicate cleanup transaction" << m_db.lastError().text();
            m_db.rollback();
            return false;
        }

        return true;
    }

    bool Repository::ensureSeedData()
    {
        QSqlQuery countPolicies(m_db);
        countPolicies.exec(QStringLiteral("SELECT COUNT(*) FROM quiet_hours_policies"));
        countPolicies.next();
        if (countPolicies.value(0).toInt() == 0)
        {
            QuietHoursPolicy policy;
            policy.name = QStringLiteral("Night");
            policy.startTime = QTime(22, 0);
            policy.endTime = QTime(8, 0);
            policy.behaviorSoft = QuietBehavior::Suppress;
            policy.behaviorPersistent = QuietBehavior::Defer;
            policy.behaviorBreak = QuietBehavior::Downgrade;
            saveQuietHoursPolicy(policy);
        }

        QSqlQuery countProfiles(m_db);
        countProfiles.exec(QStringLiteral("SELECT COUNT(*) FROM reminder_profiles"));
        countProfiles.next();
        if (countProfiles.value(0).toInt() == 0)
        {
            const qint64 quietId = loadQuietHoursPolicies().constFirst().id;

            ReminderProfile gentle;
            gentle.name = QStringLiteral("Gentle reminder");
            gentle.kind = ProfileKind::Break;
            gentle.intervalMinutes = 50;
            gentle.breakDurationMinutes = 10;
            gentle.severityMode = SeverityMode::Soft;
            gentle.maxSnoozeCount = 2;
            gentle.snoozeMinutes = 5;
            gentle.quietHoursPolicyId = quietId;
            gentle.requirePostBreakConfirmation = true;
            gentle.allowSkip = true;
            gentle.builtIn = true;
            const qint64 gentleId = saveProfile(gentle);

            ReminderProfile focus = gentle;
            focus.name = QStringLiteral("Focus Session");
            focus.intervalMinutes = 60;
            focus.breakDurationMinutes = 10;
            focus.severityMode = SeverityMode::Persistent;
            focus.maxSnoozeCount = 1;
            focus.allowSkip = false;
            focus.builtIn = true;
            saveProfile(focus);

            ReminderProfile custom = gentle;
            custom.name = QStringLiteral("Break reminder");
            custom.kind = ProfileKind::Break;
            custom.severityMode = SeverityMode::Break;
            custom.intervalMinutes = 45;
            custom.breakDurationMinutes = 8;
            custom.maxSnoozeCount = 2;
            custom.builtIn = false;
            const qint64 customId = saveProfile(custom);

            ReminderEvent event;
            event.title = QStringLiteral("Regular break");
            event.description = QStringLiteral("Take a short rest away from the screen.");
            event.type = ReminderType::BreakInterval;
            event.profileId = gentleId;
            event.startAt = QDateTime::currentDateTime().addSecs(gentle.intervalMinutes * 60);
            event.recurrenceRule = QStringLiteral("none");
            event.isOneTime = false;
            event.nextTriggerAt = event.startAt;
            saveEvent(event);

            ReminderEvent hydration;
            hydration.title = QStringLiteral("Drink water");
            hydration.description = QStringLiteral("Stay hydrated during the day.");
            hydration.type = ReminderType::DateTime;
            hydration.profileId = customId;
            hydration.startAt = QDateTime(QDate::currentDate(), QTime(11, 30));
            if (hydration.startAt < QDateTime::currentDateTime())
            {
                hydration.startAt = hydration.startAt.addDays(1);
            }
            hydration.recurrenceRule = QStringLiteral("weekdays");
            hydration.isOneTime = false;
            hydration.nextTriggerAt = hydration.startAt;
            saveEvent(hydration);
        }

        return true;
    }

    RepositoryState Repository::loadState() const
    {
        return {loadProfiles(), loadEvents(), loadQuietHoursPolicies(), loadDailyStats()};
    }

    QList<ReminderProfile> Repository::loadProfiles() const
    {
        QList<ReminderProfile> items;
        QSqlQuery query(m_db);
        query.exec(QStringLiteral("SELECT * FROM reminder_profiles ORDER BY built_in DESC, name ASC"));
        while (query.next())
        {
            items.push_back(profileFromQuery(query));
        }
        return items;
    }

    QList<ReminderEvent> Repository::loadEvents() const
    {
        QList<ReminderEvent> items;
        QSqlQuery query(m_db);
        query.exec(QStringLiteral("SELECT * FROM reminder_events ORDER BY enabled DESC, title ASC"));
        while (query.next())
        {
            items.push_back(eventFromQuery(query));
        }
        return items;
    }

    QList<QuietHoursPolicy> Repository::loadQuietHoursPolicies() const
    {
        QList<QuietHoursPolicy> items;
        QSqlQuery query(m_db);
        query.exec(QStringLiteral("SELECT * FROM quiet_hours_policies ORDER BY name ASC"));
        while (query.next())
        {
            items.push_back(quietPolicyFromQuery(query));
        }
        return items;
    }

    QList<StatsDaily> Repository::loadDailyStats(int limit) const
    {
        QList<StatsDaily> items;
        QSqlQuery query(m_db);
        query.prepare(QStringLiteral("SELECT * FROM stats_daily ORDER BY date DESC LIMIT ?"));
        query.addBindValue(limit);
        query.exec();
        while (query.next())
        {
            items.push_back(statsFromQuery(query));
        }
        return items;
    }

    qint64 Repository::insertOrUpdate(const QString &sqlInsert,
                                      const QString &sqlUpdate,
                                      qint64 id,
                                      const std::function<void(QSqlQuery &)> &binder) const
    {
        QSqlQuery query(m_db);
        query.prepare(id == 0 ? sqlInsert : sqlUpdate);
        binder(query);
        if (id != 0)
        {
            query.addBindValue(id);
        }
        if (!query.exec())
        {
            qWarning() << "Database write failed" << query.lastError().text();
            return 0;
        }
        return id == 0 ? query.lastInsertId().toLongLong() : id;
    }

    void Repository::bindProfile(QSqlQuery &query, const ReminderProfile &profile) const
    {
        query.addBindValue(profile.name);
        query.addBindValue(toString(profile.kind));
        query.addBindValue(profile.intervalMinutes);
        query.addBindValue(profile.breakDurationMinutes);
        query.addBindValue(toString(profile.severityMode));
        query.addBindValue(profile.maxSnoozeCount);
        query.addBindValue(profile.snoozeMinutes);
        query.addBindValue(profile.quietHoursPolicyId);
        query.addBindValue(profile.requirePostBreakConfirmation);
        query.addBindValue(profile.allowSkip);
        query.addBindValue(profile.enabled);
        query.addBindValue(profile.builtIn);
        query.addBindValue(profile.createdAt.isValid() ? profile.createdAt : QDateTime::currentDateTime());
        query.addBindValue(QDateTime::currentDateTime());
    }

    void Repository::bindEvent(QSqlQuery &query, const ReminderEvent &event) const
    {
        query.addBindValue(event.title);
        query.addBindValue(event.description);
        query.addBindValue(toString(event.type));
        query.addBindValue(event.profileId);
        query.addBindValue(event.startAt);
        query.addBindValue(event.timezoneMode);
        query.addBindValue(event.recurrenceRule);
        query.addBindValue(event.isOneTime);
        query.addBindValue(event.skipMissedOccurrences);
        query.addBindValue(event.enabled);
        query.addBindValue(event.lastTriggeredAt);
        query.addBindValue(event.nextTriggerAt);
        query.addBindValue(event.pendingSnoozeCount);
        query.addBindValue(event.pendingOriginalTriggerAt);
        query.addBindValue(event.pendingMode ? toString(*event.pendingMode) : QVariant{});
        query.addBindValue(event.createdAt.isValid() ? event.createdAt : QDateTime::currentDateTime());
        query.addBindValue(QDateTime::currentDateTime());
    }

    void Repository::bindQuietPolicy(QSqlQuery &query, const QuietHoursPolicy &policy) const
    {
        query.addBindValue(policy.name);
        query.addBindValue(policy.startTime);
        query.addBindValue(policy.endTime);
        query.addBindValue(toString(policy.behaviorSoft));
        query.addBindValue(toString(policy.behaviorPersistent));
        query.addBindValue(toString(policy.behaviorBreak));
        query.addBindValue(policy.createdAt.isValid() ? policy.createdAt : QDateTime::currentDateTime());
        query.addBindValue(QDateTime::currentDateTime());
    }

    qint64 Repository::saveProfile(const ReminderProfile &profile)
    {
        if (profileExists(profile.name, profile.id))
        {
            return 0;
        }
        return insertOrUpdate(
            QStringLiteral("INSERT INTO reminder_profiles (name, kind, interval_minutes, break_duration_minutes, severity_mode, max_snooze_count, snooze_minutes, quiet_hours_policy_id, require_post_break_confirmation, allow_skip, enabled, built_in, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"),
            QStringLiteral("UPDATE reminder_profiles SET name=?, kind=?, interval_minutes=?, break_duration_minutes=?, severity_mode=?, max_snooze_count=?, snooze_minutes=?, quiet_hours_policy_id=?, require_post_break_confirmation=?, allow_skip=?, enabled=?, built_in=?, created_at=?, updated_at=? WHERE id=?"),
            profile.id,
            [this, &profile](QSqlQuery &query)
            { bindProfile(query, profile); });
    }

    bool Repository::deleteProfile(qint64 id)
    {
        if (!m_db.transaction())
        {
            qWarning() << "Failed to start profile deletion transaction" << m_db.lastError().text();
            return false;
        }

        QSqlQuery deleteLogsQuery(m_db);
        deleteLogsQuery.prepare(QStringLiteral(
            "DELETE FROM occurrence_log WHERE event_id IN (SELECT id FROM reminder_events WHERE profile_id=?)"));
        deleteLogsQuery.addBindValue(id);
        if (!deleteLogsQuery.exec())
        {
            qWarning() << "Failed to delete profile occurrence logs" << deleteLogsQuery.lastError().text();
            m_db.rollback();
            return false;
        }

        QSqlQuery deleteEventsQuery(m_db);
        deleteEventsQuery.prepare(QStringLiteral("DELETE FROM reminder_events WHERE profile_id=?"));
        deleteEventsQuery.addBindValue(id);
        if (!deleteEventsQuery.exec())
        {
            qWarning() << "Failed to delete profile events" << deleteEventsQuery.lastError().text();
            m_db.rollback();
            return false;
        }

        QSqlQuery deleteProfileQuery(m_db);
        deleteProfileQuery.prepare(QStringLiteral("DELETE FROM reminder_profiles WHERE id=?"));
        deleteProfileQuery.addBindValue(id);
        if (!deleteProfileQuery.exec())
        {
            qWarning() << "Failed to delete profile" << deleteProfileQuery.lastError().text();
            m_db.rollback();
            return false;
        }

        if (!m_db.commit())
        {
            qWarning() << "Failed to commit profile deletion transaction" << m_db.lastError().text();
            m_db.rollback();
            return false;
        }

        return true;
    }

    bool Repository::profileExists(const QString &name, qint64 excludeId) const
    {
        QSqlQuery query(m_db);
        if (excludeId > 0)
        {
            query.prepare(QStringLiteral("SELECT COUNT(*) FROM reminder_profiles WHERE name=? AND id != ?"));
            query.addBindValue(name);
            query.addBindValue(excludeId);
        }
        else
        {
            query.prepare(QStringLiteral("SELECT COUNT(*) FROM reminder_profiles WHERE name=?"));
            query.addBindValue(name);
        }
        if (!query.exec() || !query.next())
        {
            return false;
        }
        return query.value(0).toInt() > 0;
    }

    qint64 Repository::saveEvent(const ReminderEvent &event)
    {
        if (eventExists(event.title, event.startAt, event.profileId, event.id))
        {
            return 0;
        }
        return insertOrUpdate(
            QStringLiteral("INSERT INTO reminder_events (title, description, type, profile_id, start_at, timezone_mode, recurrence_rule, is_one_time, skip_missed_occurrences, enabled, last_triggered_at, next_trigger_at, pending_snooze_count, pending_original_trigger_at, pending_mode, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"),
            QStringLiteral("UPDATE reminder_events SET title=?, description=?, type=?, profile_id=?, start_at=?, timezone_mode=?, recurrence_rule=?, is_one_time=?, skip_missed_occurrences=?, enabled=?, last_triggered_at=?, next_trigger_at=?, pending_snooze_count=?, pending_original_trigger_at=?, pending_mode=?, created_at=?, updated_at=? WHERE id=?"),
            event.id,
            [this, &event](QSqlQuery &query)
            { bindEvent(query, event); });
    }

    bool Repository::deleteEvent(qint64 id)
    {
        if (!m_db.transaction())
        {
            qWarning() << "Failed to start event deletion transaction" << m_db.lastError().text();
            return false;
        }

        QSqlQuery deleteLogsQuery(m_db);
        deleteLogsQuery.prepare(QStringLiteral("DELETE FROM occurrence_log WHERE event_id=?"));
        deleteLogsQuery.addBindValue(id);
        if (!deleteLogsQuery.exec())
        {
            qWarning() << "Failed to delete event occurrence logs" << deleteLogsQuery.lastError().text();
            m_db.rollback();
            return false;
        }

        QSqlQuery deleteEventQuery(m_db);
        deleteEventQuery.prepare(QStringLiteral("DELETE FROM reminder_events WHERE id=?"));
        deleteEventQuery.addBindValue(id);
        if (!deleteEventQuery.exec())
        {
            qWarning() << "Failed to delete event" << deleteEventQuery.lastError().text();
            m_db.rollback();
            return false;
        }

        if (!m_db.commit())
        {
            qWarning() << "Failed to commit event deletion transaction" << m_db.lastError().text();
            m_db.rollback();
            return false;
        }

        return true;
    }

    bool Repository::eventExists(const QString &title, const QDateTime &startAt, qint64 profileId, qint64 excludeId) const
    {
        QSqlQuery query(m_db);
        if (excludeId > 0)
        {
            query.prepare(QStringLiteral("SELECT COUNT(*) FROM reminder_events WHERE title=? AND start_at=? AND profile_id=? AND id != ?"));
            query.addBindValue(title);
            query.addBindValue(startAt);
            query.addBindValue(profileId);
            query.addBindValue(excludeId);
        }
        else
        {
            query.prepare(QStringLiteral("SELECT COUNT(*) FROM reminder_events WHERE title=? AND start_at=? AND profile_id=?"));
            query.addBindValue(title);
            query.addBindValue(startAt);
            query.addBindValue(profileId);
        }
        if (!query.exec() || !query.next())
        {
            return false;
        }
        return query.value(0).toInt() > 0;
    }

    qint64 Repository::saveQuietHoursPolicy(const QuietHoursPolicy &policy)
    {
        return insertOrUpdate(
            QStringLiteral("INSERT INTO quiet_hours_policies (name, start_time, end_time, behavior_soft, behavior_persistent, behavior_break, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?)"),
            QStringLiteral("UPDATE quiet_hours_policies SET name=?, start_time=?, end_time=?, behavior_soft=?, behavior_persistent=?, behavior_break=?, created_at=?, updated_at=? WHERE id=?"),
            policy.id,
            [this, &policy](QSqlQuery &query)
            { bindQuietPolicy(query, policy); });
    }

    QuietHoursPolicy Repository::loadQuietHoursPolicy(qint64 id) const
    {
        QSqlQuery query(m_db);
        query.prepare(QStringLiteral("SELECT * FROM quiet_hours_policies WHERE id=?"));
        query.addBindValue(id);
        if (query.exec() && query.next())
        {
            return quietPolicyFromQuery(query);
        }
        return {};
    }

    bool Repository::logOccurrence(const OccurrenceLogEntry &entry)
    {
        QSqlQuery query(m_db);
        query.prepare(QStringLiteral("INSERT INTO occurrence_log (event_id, triggered_at, shown_mode, result, snooze_count, break_expected_seconds, break_actual_seconds, completed_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?)"));
        query.addBindValue(entry.eventId);
        query.addBindValue(entry.triggeredAt);
        query.addBindValue(toString(entry.shownMode));
        query.addBindValue(toString(entry.result));
        query.addBindValue(entry.snoozeCount);
        query.addBindValue(entry.breakExpectedSeconds);
        query.addBindValue(entry.breakActualSeconds);
        query.addBindValue(entry.completedAt);
        return query.exec();
    }

    bool Repository::upsertDailyStats(const StatsDaily &stats)
    {
        QSqlQuery query(m_db);
        query.prepare(QStringLiteral("INSERT INTO stats_daily (date, completed_count, skipped_count, snoozed_count, break_completed_count, break_missed_count) VALUES (?, ?, ?, ?, ?, ?) ON CONFLICT(date) DO UPDATE SET completed_count=excluded.completed_count, skipped_count=excluded.skipped_count, snoozed_count=excluded.snoozed_count, break_completed_count=excluded.break_completed_count, break_missed_count=excluded.break_missed_count"));
        query.addBindValue(stats.date);
        query.addBindValue(stats.completedCount);
        query.addBindValue(stats.skippedCount);
        query.addBindValue(stats.snoozedCount);
        query.addBindValue(stats.breakCompletedCount);
        query.addBindValue(stats.breakMissedCount);
        return query.exec();
    }

} // namespace deadliner::infrastructure
