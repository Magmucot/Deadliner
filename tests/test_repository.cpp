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
    void savesEventSkipMissedOccurrencesFlag();
    void saveDuplicateEventReturnsFailure();
    void deletesEventLogsWithEvent();
    void builtInProfileCanBeDeleted();
    void duplicateProfilesAreNotRemovedByNameOnly();
    void duplicateEventsAreRemovedOnOpen();
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

void RepositoryTests::saveDuplicateEventReturnsFailure()
{
    QTemporaryDir dir;
    infrastructure::Repository repository;
    QString error;
    QVERIFY(repository.open(dir.filePath(QStringLiteral("deadliner.sqlite")), &error));

    const auto state = repository.loadState();
    QVERIFY(!state.profiles.isEmpty());

    domain::ReminderEvent event;
    event.title = QStringLiteral("Duplicate check");
    event.type = domain::ReminderType::DateTime;
    event.profileId = state.profiles.constFirst().id;
    event.startAt = QDateTime::currentDateTime().addDays(1);
    event.recurrenceRule = QStringLiteral("none");
    event.isOneTime = true;
    event.enabled = true;
    event.nextTriggerAt = event.startAt;

    QVERIFY(repository.saveEvent(event) > 0);
    QCOMPARE(repository.saveEvent(event), qint64{0});
}

void RepositoryTests::savesEventSkipMissedOccurrencesFlag()
{
    QTemporaryDir dir;
    infrastructure::Repository repository;
    QString error;
    QVERIFY(repository.open(dir.filePath(QStringLiteral("deadliner.sqlite")), &error));

    const auto state = repository.loadState();
    QVERIFY(!state.profiles.isEmpty());

    domain::ReminderEvent event;
    event.title = QStringLiteral("Recurring catch-up setting");
    event.type = domain::ReminderType::DateTime;
    event.profileId = state.profiles.constFirst().id;
    event.startAt = QDateTime::currentDateTime().addDays(-1);
    event.recurrenceRule = QStringLiteral("daily");
    event.isOneTime = false;
    event.skipMissedOccurrences = false;
    event.enabled = true;
    event.nextTriggerAt = event.startAt;

    const qint64 id = repository.saveEvent(event);
    QVERIFY(id > 0);

    bool found = false;
    for (const auto &item : repository.loadState().events) {
        if (item.id == id) {
            found = true;
            QVERIFY(!item.skipMissedOccurrences);
        }
    }
    QVERIFY(found);
}

void RepositoryTests::builtInProfileCanBeDeleted()
{
    QTemporaryDir dir;
    infrastructure::Repository repository;
    QString error;
    QVERIFY(repository.open(dir.filePath(QStringLiteral("deadliner.sqlite")), &error));

    const auto initialState = repository.loadState();
    QVERIFY(!initialState.profiles.isEmpty());
    qint64 profileId = 0;
    for (const auto &profile : initialState.profiles) {
        if (profile.builtIn) {
            profileId = profile.id;
            break;
        }
    }
    QVERIFY(profileId > 0);

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
    bool foundProfile = false;
    for (const auto &profile : stateAfterDelete.profiles) {
        if (profile.id == profileId) {
            foundProfile = true;
        }
    }
    QVERIFY(!foundProfile);

    bool foundEvent = false;
    for (const auto &item : stateAfterDelete.events) {
        if (item.title == event.title && item.profileId == profileId) {
            foundEvent = true;
        }
    }
    QVERIFY(!foundEvent);
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

    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("delete_event_logs_check"));
        db.setDatabaseName(databasePath);
        QVERIFY(db.open());

        QSqlQuery countBefore(db);
        countBefore.prepare(QStringLiteral("SELECT COUNT(*) FROM occurrence_log WHERE event_id=?"));
        countBefore.addBindValue(eventId);
        QVERIFY(countBefore.exec());
        QVERIFY(countBefore.next());
        QCOMPARE(countBefore.value(0).toInt(), 1);

        db.close();
    }
    QSqlDatabase::removeDatabase(QStringLiteral("delete_event_logs_check"));

    QVERIFY(repository.deleteEvent(eventId));

    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("delete_event_logs_check_after"));
        db.setDatabaseName(databasePath);
        QVERIFY(db.open());

        QSqlQuery countAfter(db);
        countAfter.prepare(QStringLiteral("SELECT COUNT(*) FROM occurrence_log WHERE event_id=?"));
        countAfter.addBindValue(eventId);
        QVERIFY(countAfter.exec());
        QVERIFY(countAfter.next());
        QCOMPARE(countAfter.value(0).toInt(), 0);

        db.close();
    }
    QSqlDatabase::removeDatabase(QStringLiteral("delete_event_logs_check_after"));
}

void RepositoryTests::duplicateProfilesAreNotRemovedByNameOnly()
{
    QTemporaryDir dir;
    const QString databasePath = dir.filePath(QStringLiteral("deadliner.sqlite"));
    QString duplicateName;

    {
        infrastructure::Repository repository;
        QString error;
        QVERIFY(repository.open(databasePath, &error));

        const auto state = repository.loadState();
        QVERIFY(!state.profiles.isEmpty());
        const auto templateProfile = state.profiles.constFirst();
        duplicateName = templateProfile.name;

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

            QSqlQuery updateProfile(db);
            updateProfile.prepare(QStringLiteral("UPDATE reminder_profiles SET interval_minutes=? WHERE id=?"));
            updateProfile.addBindValue(templateProfile.intervalMinutes + 5);
            updateProfile.addBindValue(duplicateProfileId);
            QVERIFY(updateProfile.exec());

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
            if (profile.name == duplicateName) {
                ++profileCount;
            }
        }
        QCOMPARE(profileCount, 2);
    }
}

void RepositoryTests::duplicateEventsAreRemovedOnOpen()
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
            QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("duplicate_events_setup"));
            db.setDatabaseName(databasePath);
            QVERIFY(db.open());

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
        QSqlDatabase::removeDatabase(QStringLiteral("duplicate_events_setup"));
    }

    {
        infrastructure::Repository repository;
        QString error;
        QVERIFY(repository.open(databasePath, &error));
        const auto state = repository.loadState();

        int profileCount = 0;
        for (const auto &event : state.events) {
            if (event.title == QStringLiteral("Duplicate event")) {
                ++profileCount;
            }
        }
        QCOMPARE(profileCount, 1);
    }
}

QTEST_GUILESS_MAIN(RepositoryTests)
#include "test_repository.moc"
