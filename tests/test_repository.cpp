#include "infrastructure/storage/repository.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QVariant>
#include <QtTest>

using namespace deadliner;

class RepositoryTests : public QObject {
    Q_OBJECT

private slots:
    void opensAndSeeds();
    void savesAndLoadsEvent();
    void deletesEventLogsWithEvent();
    void deletesBuiltInProfileWithEvents();
    void removesDuplicateProfilesAndEventsOnOpen();
};

void RepositoryTests::opensAndSeeds()
{
    QTemporaryDir dir;
    infrastructure::Repository repository;
    QString error;
    QVERIFY(repository.open(dir.filePath(QStringLiteral("deadliner.sqlite")), &error));
    const auto state = repository.loadState();
    QVERIFY(!state.profiles.isEmpty());
    QVERIFY(!state.quietHoursPolicies.isEmpty());
}

void RepositoryTests::savesAndLoadsEvent()
{
    QTemporaryDir dir;
    infrastructure::Repository repository;
    QString error;
    QVERIFY(repository.open(dir.filePath(QStringLiteral("deadliner.sqlite")), &error));

    auto state = repository.loadState();
    QVERIFY(!state.profiles.isEmpty());

    domain::ReminderEvent event;
    event.title = QStringLiteral("Doctor call");
    event.type = domain::ReminderType::DateTime;
    event.profileId = state.profiles.constFirst().id;
    event.startAt = QDateTime::currentDateTime().addDays(1);
    event.recurrenceRule = QStringLiteral("none");
    event.isOneTime = true;
    event.enabled = true;
    event.nextTriggerAt = event.startAt;

    const qint64 id = repository.saveEvent(event);
    QVERIFY(id > 0);

    state = repository.loadState();
    bool found = false;
    for (const auto &item : state.events) {
        if (item.id == id && item.title == event.title) {
            found = true;
            break;
        }
    }
    QVERIFY(found);
}

void RepositoryTests::deletesBuiltInProfileWithEvents()
{
    QTemporaryDir dir;
    infrastructure::Repository repository;
    QString error;
    QVERIFY(repository.open(dir.filePath(QStringLiteral("deadliner.sqlite")), &error));

    const auto initialState = repository.loadState();
    QVERIFY(!initialState.profiles.isEmpty());
    const qint64 profileId = initialState.profiles.constFirst().id;

    domain::ReminderEvent event;
    event.title = QStringLiteral("Delete with profile");
    event.type = domain::ReminderType::DateTime;
    event.profileId = profileId;
    event.startAt = QDateTime::currentDateTime().addDays(1);
    event.recurrenceRule = QStringLiteral("none");
    event.isOneTime = true;
    event.enabled = true;
    event.nextTriggerAt = event.startAt;
    QVERIFY(repository.saveEvent(event) > 0);

    QVERIFY(repository.deleteProfile(profileId));

    const auto stateAfterDelete = repository.loadState();
    for (const auto &profile : stateAfterDelete.profiles) {
        QVERIFY(profile.id != profileId);
    }
    for (const auto &item : stateAfterDelete.events) {
        QVERIFY(item.profileId != profileId);
    }
}

void RepositoryTests::deletesEventLogsWithEvent()
{
    QTemporaryDir dir;
    const QString databasePath = dir.filePath(QStringLiteral("deadliner.sqlite"));
    infrastructure::Repository repository;
    QString error;
    QVERIFY(repository.open(databasePath, &error));

    const auto state = repository.loadState();
    QVERIFY(!state.profiles.isEmpty());

    domain::ReminderEvent event;
    event.title = QStringLiteral("Delete event logs");
    event.type = domain::ReminderType::DateTime;
    event.profileId = state.profiles.constFirst().id;
    event.startAt = QDateTime::currentDateTime().addDays(1);
    event.recurrenceRule = QStringLiteral("none");
    event.isOneTime = true;
    event.enabled = true;
    event.nextTriggerAt = event.startAt;

    const qint64 eventId = repository.saveEvent(event);
    QVERIFY(eventId > 0);

    domain::OccurrenceLogEntry entry;
    entry.eventId = eventId;
    entry.triggeredAt = QDateTime::currentDateTime();
    entry.shownMode = domain::SeverityMode::Soft;
    entry.result = domain::ReminderResult::Completed;
    QVERIFY(repository.logOccurrence(entry));

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("delete_event_logs_check"));
    db.setDatabaseName(databasePath);
    QVERIFY(db.open());

    QSqlQuery countBefore(db);
    countBefore.prepare(QStringLiteral("SELECT COUNT(*) FROM occurrence_log WHERE event_id=?"));
    countBefore.addBindValue(eventId);
    QVERIFY(countBefore.exec());
    QVERIFY(countBefore.next());
    QCOMPARE(countBefore.value(0).toInt(), 1);

    QVERIFY(repository.deleteEvent(eventId));

    QSqlQuery countAfter(db);
    countAfter.prepare(QStringLiteral("SELECT COUNT(*) FROM occurrence_log WHERE event_id=?"));
    countAfter.addBindValue(eventId);
    QVERIFY(countAfter.exec());
    QVERIFY(countAfter.next());
    QCOMPARE(countAfter.value(0).toInt(), 0);

    db.close();
    QSqlDatabase::removeDatabase(QStringLiteral("delete_event_logs_check"));
}

void RepositoryTests::removesDuplicateProfilesAndEventsOnOpen()
{
    QTemporaryDir dir;
    const QString databasePath = dir.filePath(QStringLiteral("deadliner.sqlite"));

    {
        infrastructure::Repository repository;
        QString error;
        QVERIFY(repository.open(databasePath, &error));

        const auto state = repository.loadState();
        QVERIFY(!state.profiles.isEmpty());
        const auto templateProfile = state.profiles.constFirst();

        {
            QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("duplicates_setup"));
            db.setDatabaseName(databasePath);
            QVERIFY(db.open());

            QSqlQuery profileInsert(db);
            QVERIFY(profileInsert.prepare(QStringLiteral(
                "INSERT INTO reminder_profiles (name, kind, interval_minutes, break_duration_minutes, severity_mode, max_snooze_count, snooze_minutes, quiet_hours_policy_id, require_post_break_confirmation, allow_skip, enabled, built_in, created_at, updated_at) "
                "SELECT name, kind, interval_minutes, break_duration_minutes, severity_mode, max_snooze_count, snooze_minutes, quiet_hours_policy_id, require_post_break_confirmation, allow_skip, enabled, built_in, created_at, updated_at "
                "FROM reminder_profiles WHERE id=?")));
            profileInsert.addBindValue(templateProfile.id);
            QVERIFY(profileInsert.exec());
            const qint64 duplicateProfileId = profileInsert.lastInsertId().toLongLong();
            QVERIFY(duplicateProfileId > 0);

            QSqlQuery eventInsert(db);
            QVERIFY(eventInsert.prepare(QStringLiteral(
                "INSERT INTO reminder_events (title, description, type, profile_id, start_at, timezone_mode, recurrence_rule, is_one_time, enabled, last_triggered_at, next_trigger_at, pending_snooze_count, pending_original_trigger_at, pending_mode, created_at, updated_at) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)")));
            eventInsert.addBindValue(QStringLiteral("Duplicate event"));
            eventInsert.addBindValue(QStringLiteral("Duplicate event description"));
            eventInsert.addBindValue(QStringLiteral("datetime"));
            eventInsert.addBindValue(templateProfile.id);
            const QDateTime startAt = QDateTime::currentDateTime().addDays(2);
            eventInsert.addBindValue(startAt);
            eventInsert.addBindValue(QStringLiteral("local"));
            eventInsert.addBindValue(QStringLiteral("none"));
            eventInsert.addBindValue(true);
            eventInsert.addBindValue(true);
            eventInsert.addBindValue(QVariant());
            eventInsert.addBindValue(startAt);
            eventInsert.addBindValue(0);
            eventInsert.addBindValue(QVariant());
            eventInsert.addBindValue(QVariant());
            eventInsert.addBindValue(QDateTime::currentDateTime());
            eventInsert.addBindValue(QDateTime::currentDateTime());
            QVERIFY(eventInsert.exec());

            QSqlQuery duplicateEventInsert(db);
            QVERIFY(duplicateEventInsert.prepare(QStringLiteral(
                "INSERT INTO reminder_events (title, description, type, profile_id, start_at, timezone_mode, recurrence_rule, is_one_time, enabled, last_triggered_at, next_trigger_at, pending_snooze_count, pending_original_trigger_at, pending_mode, created_at, updated_at) "
                "SELECT title, description, type, profile_id, start_at, timezone_mode, recurrence_rule, is_one_time, enabled, last_triggered_at, next_trigger_at, pending_snooze_count, pending_original_trigger_at, pending_mode, created_at, updated_at "
                "FROM reminder_events WHERE id=?")));
            duplicateEventInsert.addBindValue(eventInsert.lastInsertId());
            QVERIFY(duplicateEventInsert.exec());

            db.close();
        }
        QSqlDatabase::removeDatabase(QStringLiteral("duplicates_setup"));
    }

    {
        infrastructure::Repository repository;
        QString error;
        QVERIFY(repository.open(databasePath, &error));
        const auto state = repository.loadState();

        int profileCount = 0;
        for (const auto &profile : state.profiles) {
            if (profile.name == QStringLiteral("Gentle Breaks")) {
                ++profileCount;
            }
        }
        QCOMPARE(profileCount, 1);

        int eventCount = 0;
        for (const auto &event : state.events) {
            if (event.title == QStringLiteral("Duplicate event")) {
                ++eventCount;
            }
        }
        QCOMPARE(eventCount, 1);
    }
}

QTEST_GUILESS_MAIN(RepositoryTests)
#include "test_repository.moc"
