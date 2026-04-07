#include "domain/logic.h"

#include <QtTest>

using namespace deadliner::domain;

class DomainTests : public QObject {
    Q_OBJECT

private slots:
    void dailyRecurrence();
    void weekdaysRecurrence();
    void quietHoursAcrossMidnight();
    void snoozeLimits();
    void statsAggregation();
};

void DomainTests::dailyRecurrence()
{
    ReminderEvent event;
    event.enabled = true;
    event.startAt = QDateTime(QDate(2026, 4, 5), QTime(9, 0));
    event.recurrenceRule = QStringLiteral("daily");
    event.isOneTime = false;

    const auto next = computeNextDateTimeOccurrence(event, QDateTime(QDate(2026, 4, 5), QTime(10, 0)));
    QCOMPARE(next, QDateTime(QDate(2026, 4, 6), QTime(9, 0)));
}

void DomainTests::weekdaysRecurrence()
{
    ReminderEvent event;
    event.enabled = true;
    event.startAt = QDateTime(QDate(2026, 4, 3), QTime(11, 30));
    event.recurrenceRule = QStringLiteral("weekdays");
    event.isOneTime = false;

    const auto next = computeNextDateTimeOccurrence(event, QDateTime(QDate(2026, 4, 4), QTime(9, 0)));
    QCOMPARE(next.date().dayOfWeek(), 1);
    QCOMPARE(next.time(), QTime(11, 30));
}

void DomainTests::quietHoursAcrossMidnight()
{
    QuietHoursPolicy policy;
    policy.startTime = QTime(22, 0);
    policy.endTime = QTime(8, 0);
    policy.behaviorSoft = QuietBehavior::Suppress;

    QVERIFY(isInQuietHours(policy, QDateTime(QDate(2026, 4, 5), QTime(23, 30))));
    QVERIFY(isInQuietHours(policy, QDateTime(QDate(2026, 4, 6), QTime(7, 30))));
    QVERIFY(!isInQuietHours(policy, QDateTime(QDate(2026, 4, 6), QTime(9, 0))));

    const auto end = quietHoursEnd(policy, QDateTime(QDate(2026, 4, 5), QTime(23, 30)));
    QCOMPARE(end, QDateTime(QDate(2026, 4, 6), QTime(8, 0)));
}

void DomainTests::snoozeLimits()
{
    ReminderProfile profile;
    profile.maxSnoozeCount = 2;
    QVERIFY(canSnooze(profile, 0));
    QVERIFY(canSnooze(profile, 1));
    QVERIFY(!canSnooze(profile, 2));
}

void DomainTests::statsAggregation()
{
    OccurrenceLogEntry entry;
    entry.triggeredAt = QDateTime(QDate(2026, 4, 5), QTime(12, 0));
    entry.shownMode = SeverityMode::Break;
    entry.result = ReminderResult::Completed;

    const auto stats = applyLogToDailyStats({}, entry);
    QCOMPARE(stats.completedCount, 1);
    QCOMPARE(stats.breakCompletedCount, 1);
}

QTEST_MAIN(DomainTests)
#include "test_domain.moc"
