#include "application/services.h"

#include <QtTest>

using namespace deadliner;

class SchedulerTests : public QObject {
    Q_OBJECT

private slots:
    void overdueEventIsNotIncludedInUpcomingOccurrences();
    void futureEventIsIncludedInUpcomingOccurrences();
    void overdueEventDoesNotEmitOccurrenceDueAfterRebuild();
    void needsResyncIsFalseWithoutArmedOccurrence();
    void needsResyncIsTrueWhenDeadlineWasMissed();
};

void SchedulerTests::overdueEventIsNotIncludedInUpcomingOccurrences()
{
    application::ReminderScheduler scheduler;

    domain::ReminderProfile profile;
    profile.id = 1;
    profile.enabled = true;

    domain::ReminderEvent event;
    event.id = 1;
    event.profileId = profile.id;
    event.title = QStringLiteral("Past event");
    event.type = domain::ReminderType::DateTime;
    event.enabled = true;
    event.isOneTime = true;
    event.startAt = QDateTime::currentDateTime().addSecs(-120);

    scheduler.setState({event}, {{profile.id, profile}});
    QVERIFY(scheduler.upcomingOccurrences().isEmpty());
}

void SchedulerTests::futureEventIsIncludedInUpcomingOccurrences()
{
    application::ReminderScheduler scheduler;

    domain::ReminderProfile profile;
    profile.id = 1;
    profile.enabled = true;

    domain::ReminderEvent event;
    event.id = 1;
    event.profileId = profile.id;
    event.title = QStringLiteral("Future event");
    event.type = domain::ReminderType::DateTime;
    event.enabled = true;
    event.isOneTime = true;
    event.startAt = QDateTime::currentDateTime().addSecs(300);
    event.nextTriggerAt = event.startAt;

    scheduler.setState({event}, {{profile.id, profile}});

    const auto upcoming = scheduler.upcomingOccurrences();
    QCOMPARE(upcoming.size(), 1);
    QCOMPARE(upcoming.constFirst().eventId, event.id);
}

void SchedulerTests::overdueEventDoesNotEmitOccurrenceDueAfterRebuild()
{
    application::ReminderScheduler scheduler;

    domain::ReminderProfile profile;
    profile.id = 1;
    profile.enabled = true;

    domain::ReminderEvent event;
    event.id = 1;
    event.profileId = profile.id;
    event.title = QStringLiteral("Past event");
    event.type = domain::ReminderType::DateTime;
    event.enabled = true;
    event.isOneTime = true;
    event.startAt = QDateTime::currentDateTime().addSecs(-5);

    QSignalSpy spy(&scheduler, &application::ReminderScheduler::occurrenceDue);
    scheduler.setState({event}, {{profile.id, profile}});
    scheduler.rebuild();

    QTest::qWait(1500);
    QCOMPARE(spy.count(), 0);
}

void SchedulerTests::needsResyncIsFalseWithoutArmedOccurrence()
{
    application::ReminderScheduler scheduler;
    QVERIFY(!scheduler.needsResync(QDateTime::currentDateTime()));
}

void SchedulerTests::needsResyncIsTrueWhenDeadlineWasMissed()
{
    application::ReminderScheduler scheduler;

    domain::ReminderProfile profile;
    profile.id = 1;
    profile.enabled = true;

    domain::ReminderEvent event;
    event.id = 1;
    event.profileId = profile.id;
    event.title = QStringLiteral("Future event");
    event.type = domain::ReminderType::DateTime;
    event.enabled = true;
    event.isOneTime = true;
    event.startAt = QDateTime::currentDateTime().addSecs(60);
    event.nextTriggerAt = event.startAt;

    scheduler.setState({event}, {{profile.id, profile}});

    QVERIFY(scheduler.needsResync(event.nextTriggerAt.addSecs(3)));
}

QTEST_GUILESS_MAIN(SchedulerTests)
#include "test_scheduler.moc"
