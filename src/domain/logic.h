#pragma once

#include "domain/models.h"

#include <QList>

namespace deadliner::domain
{

    bool isOverdueEvent(const ReminderEvent &event, const ReminderProfile &profile, const QDateTime &now);
    bool isSchedulableEvent(const ReminderEvent &event, const ReminderProfile &profile, const QDateTime &now);
    bool isMissedRecurringEvent(const ReminderEvent &event, const ReminderProfile &profile, const QDateTime &now);

    QDateTime computeNextDateTimeOccurrence(const ReminderEvent &event, const QDateTime &reference);
    QDateTime computeNextBreakOccurrence(const ReminderEvent &event, const ReminderProfile &profile, const QDateTime &reference);
    QDateTime normalizeNextTrigger(const ReminderEvent &event, const ReminderProfile &profile, const QDateTime &reference);

    bool isInQuietHours(const QuietHoursPolicy &policy, const QDateTime &timestamp);
    QDateTime quietHoursEnd(const QuietHoursPolicy &policy, const QDateTime &timestamp);
    QuietHoursDecision applyQuietHours(const QuietHoursPolicy &policy, SeverityMode mode, bool oneTimeEvent, const QDateTime &timestamp);

    bool canSnooze(const ReminderProfile &profile, int currentSnoozeCount);
    QDateTime snoozeUntil(const ReminderProfile &profile, const QDateTime &from);

    StatsDaily applyLogToDailyStats(const StatsDaily &current, const OccurrenceLogEntry &entry);

    QStringList supportedRecurrenceRules();

} // namespace deadliner::domain
