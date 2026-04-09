#include "domain/logic.h"

#include <QPair>
#include <QStringList>
#include <QTimeZone>

namespace deadliner::domain
{

    namespace
    {

        QTimeZone resolveEventTimeZone(const ReminderEvent &event, const QDateTime &fallback)
        {
            if (event.startAt.timeZone().isValid())
            {
                return event.startAt.timeZone();
            }

            if (event.timezoneMode == QStringLiteral("local"))
            {
                return fallback.timeZone().isValid() ? fallback.timeZone() : QTimeZone::systemTimeZone();
            }

            return QTimeZone::systemTimeZone();
        }

        template <typename Enum>
        QString enumToString(Enum value, const QList<QPair<Enum, QString>> &mapping)
        {
            for (const auto &[candidate, text] : mapping)
            {
                if (candidate == value)
                {
                    return text;
                }
            }
            return {};
        }

        template <typename Enum>
        Enum enumFromString(const QString &value, const QList<QPair<Enum, QString>> &mapping, Enum fallback)
        {
            for (const auto &[candidate, text] : mapping)
            {
                if (text == value)
                {
                    return candidate;
                }
            }
            return fallback;
        }

        QList<int> parseWeekdays(const QString &rule)
        {
            QList<int> days;
            const QStringList chunks = rule.section(':', 1).split(',', Qt::SkipEmptyParts);
            for (const QString &chunk : chunks)
            {
                bool ok = false;
                const int day = chunk.toInt(&ok);
                if (ok && day >= 1 && day <= 7)
                {
                    days.push_back(day);
                }
            }
            return days;
        }

        QDateTime advanceMonthly(const QDateTime &reference, int dayOfMonth)
        {
            QDate month = reference.date();
            while (true)
            {
                const int safeDay = qMin(dayOfMonth, month.daysInMonth());
                const QDate candidateDate(month.year(), month.month(), safeDay);
                const QDateTime candidate(candidateDate, reference.time(), reference.timeZone());
                if (candidate >= reference)
                {
                    return candidate;
                }
                month = month.addMonths(1);
            }
        }

        QDateTime nextDateForWeekdays(const QDateTime &start,
                                      const QDateTime &reference,
                                      const QList<int> &days,
                                      const QTimeZone &zone)
        {
            QDateTime candidate(reference.date(), start.time(), zone);
            if (candidate < reference)
            {
                candidate = candidate.addDays(1);
            }

            for (int i = 0; i < 14; ++i)
            {
                if (days.contains(candidate.date().dayOfWeek()))
                {
                    return candidate;
                }
                candidate = candidate.addDays(1);
            }
            return {};
        }

        QDateTime rollEveryNDays(const QDateTime &start, const QDateTime &reference, int n)
        {
            if (n <= 0)
            {
                return {};
            }

            if (start >= reference)
            {
                return start;
            }

            qint64 days = start.daysTo(reference);
            qint64 steps = days / n;
            QDateTime candidate = start.addDays(steps * n);
            while (candidate < reference)
            {
                candidate = candidate.addDays(n);
            }
            return candidate;
        }

    } // namespace

    bool isOverdueEvent(const ReminderEvent &event,
                        const ReminderProfile &profile,
                        const QDateTime &now)
    {
        Q_UNUSED(profile);

        if (!event.enabled || event.type != ReminderType::DateTime)
        {
            return false;
        }

        if (!event.isOneTime)
        {
            return false;
        }

        if (!event.startAt.isValid())
        {
            return false;
        }

        if (event.pendingOriginalTriggerAt.isValid())
        {
            return false;
        }

        return event.startAt < now;
    }

    bool isSchedulableEvent(const ReminderEvent &event,
                            const ReminderProfile &profile,
                            const QDateTime &now)
    {
        return !isOverdueEvent(event, profile, now);
    }

    bool isMissedRecurringEvent(const ReminderEvent &event,
                                const ReminderProfile &profile,
                                const QDateTime &now)
    {
        Q_UNUSED(profile);

        if (!event.enabled || event.type != ReminderType::DateTime)
        {
            return false;
        }

        if (event.isOneTime || event.pendingOriginalTriggerAt.isValid())
        {
            return false;
        }

        const QDateTime scheduledAt = event.nextTriggerAt.isValid() ? event.nextTriggerAt : event.startAt;
        return scheduledAt.isValid() && scheduledAt < now;
    }

    QString toString(ReminderType value)
    {
        return enumToString(value, {
                                       {ReminderType::BreakInterval, QStringLiteral("break_interval")},
                                       {ReminderType::DateTime, QStringLiteral("datetime")},
                                   });
    }

    QString toString(SeverityMode value)
    {
        return enumToString(value, {
                                       {SeverityMode::Soft, QStringLiteral("soft")},
                                       {SeverityMode::Persistent, QStringLiteral("persistent")},
                                       {SeverityMode::Break, QStringLiteral("break")},
                                   });
    }

    QString toString(ProfileKind value)
    {
        return enumToString(value, {
                                       {ProfileKind::Break, QStringLiteral("break")},
                                       {ProfileKind::Generic, QStringLiteral("generic")},
                                       {ProfileKind::Mixed, QStringLiteral("mixed")},
                                   });
    }

    QString toString(QuietBehavior value)
    {
        return enumToString(value, {
                                       {QuietBehavior::Suppress, QStringLiteral("suppress")},
                                       {QuietBehavior::Defer, QStringLiteral("defer")},
                                       {QuietBehavior::Downgrade, QStringLiteral("downgrade")},
                                       {QuietBehavior::Allow, QStringLiteral("allow")},
                                   });
    }

    QString toString(ReminderResult value)
    {
        return enumToString(value, {
                                       {ReminderResult::None, QStringLiteral("none")},
                                       {ReminderResult::Completed, QStringLiteral("completed")},
                                       {ReminderResult::Skipped, QStringLiteral("skipped")},
                                       {ReminderResult::Snoozed, QStringLiteral("snoozed")},
                                       {ReminderResult::Dismissed, QStringLiteral("dismissed")},
                                       {ReminderResult::Expired, QStringLiteral("expired")},
                                   });
    }

    ReminderType reminderTypeFromString(const QString &value)
    {
        return enumFromString(value, {
                                         {ReminderType::BreakInterval, QStringLiteral("break_interval")},
                                         {ReminderType::DateTime, QStringLiteral("datetime")},
                                     },
                              ReminderType::DateTime);
    }

    SeverityMode severityModeFromString(const QString &value)
    {
        return enumFromString(value, {
                                         {SeverityMode::Soft, QStringLiteral("soft")},
                                         {SeverityMode::Persistent, QStringLiteral("persistent")},
                                         {SeverityMode::Break, QStringLiteral("break")},
                                     },
                              SeverityMode::Soft);
    }

    ProfileKind profileKindFromString(const QString &value)
    {
        return enumFromString(value, {
                                         {ProfileKind::Break, QStringLiteral("break")},
                                         {ProfileKind::Generic, QStringLiteral("generic")},
                                         {ProfileKind::Mixed, QStringLiteral("mixed")},
                                     },
                              ProfileKind::Mixed);
    }

    QuietBehavior quietBehaviorFromString(const QString &value)
    {
        return enumFromString(value, {
                                         {QuietBehavior::Suppress, QStringLiteral("suppress")},
                                         {QuietBehavior::Defer, QStringLiteral("defer")},
                                         {QuietBehavior::Downgrade, QStringLiteral("downgrade")},
                                         {QuietBehavior::Allow, QStringLiteral("allow")},
                                     },
                              QuietBehavior::Allow);
    }

    ReminderResult reminderResultFromString(const QString &value)
    {
        return enumFromString(value, {
                                         {ReminderResult::None, QStringLiteral("none")},
                                         {ReminderResult::Completed, QStringLiteral("completed")},
                                         {ReminderResult::Skipped, QStringLiteral("skipped")},
                                         {ReminderResult::Snoozed, QStringLiteral("snoozed")},
                                         {ReminderResult::Dismissed, QStringLiteral("dismissed")},
                                         {ReminderResult::Expired, QStringLiteral("expired")},
                                     },
                              ReminderResult::None);
    }

    QDateTime computeNextDateTimeOccurrence(const ReminderEvent &event, const QDateTime &reference)
    {
        if (!event.enabled || !event.startAt.isValid())
        {
            return {};
        }

        if (event.pendingOriginalTriggerAt.isValid() && event.nextTriggerAt.isValid())
        {
            return event.nextTriggerAt;
        }

        if (event.recurrenceRule.isEmpty() || event.recurrenceRule == QStringLiteral("none") || event.isOneTime)
        {
            return event.startAt >= reference ? event.startAt : QDateTime{};
        }

        const QTimeZone zone = resolveEventTimeZone(event, reference);

        if (event.recurrenceRule == QStringLiteral("daily"))
        {
            QDateTime candidate(reference.date(), event.startAt.time(), zone);
            if (candidate < reference)
            {
                candidate = candidate.addDays(1);
            }
            return candidate;
        }

        if (event.recurrenceRule == QStringLiteral("weekdays"))
        {
            return nextDateForWeekdays(event.startAt, reference, {1, 2, 3, 4, 5}, zone);
        }

        if (event.recurrenceRule.startsWith(QStringLiteral("weekly:")))
        {
            return nextDateForWeekdays(event.startAt, reference, parseWeekdays(event.recurrenceRule), zone);
        }

        if (event.recurrenceRule.startsWith(QStringLiteral("monthly:")))
        {
            bool ok = false;
            const int day = event.recurrenceRule.section(':', 1).toInt(&ok);
            if (!ok || day <= 0 || day > 31)
            {
                return {};
            }

            const QDateTime normalized(reference.date(), event.startAt.time(), zone);
            return advanceMonthly(normalized, day);
        }

        if (event.recurrenceRule.startsWith(QStringLiteral("every_n_days:")))
        {
            bool ok = false;
            const int n = event.recurrenceRule.section(':', 1).toInt(&ok);
            return ok ? rollEveryNDays(event.startAt, reference, n) : QDateTime{};
        }

        return {};
    }

    QDateTime computeNextBreakOccurrence(const ReminderEvent &event, const ReminderProfile &profile, const QDateTime &reference)
    {
        if (!event.enabled || !profile.enabled || profile.intervalMinutes <= 0)
        {
            return {};
        }

        if (event.pendingOriginalTriggerAt.isValid() && event.nextTriggerAt.isValid())
        {
            return event.nextTriggerAt;
        }

        if (!event.lastTriggeredAt.isValid())
        {
            if (event.startAt.isValid() && event.startAt >= reference)
            {
                return event.startAt;
            }
            return reference.addSecs(profile.intervalMinutes * 60);
        }

        QDateTime candidate = event.lastTriggeredAt.addSecs(profile.intervalMinutes * 60);
        // If the app was closed longer than one interval the candidate is already in
        // the past.  Reschedule from `reference` so the event does not fire
        // immediately on the next scheduler tick.
        if (candidate <= reference)
        {
            candidate = reference.addSecs(profile.intervalMinutes * 60);
        }
        return candidate;
    }

    QDateTime normalizeNextTrigger(const ReminderEvent &event, const ReminderProfile &profile, const QDateTime &reference)
    {
        return event.type == ReminderType::BreakInterval
                   ? computeNextBreakOccurrence(event, profile, reference)
                   : computeNextDateTimeOccurrence(event, reference);
    }

    bool isInQuietHours(const QuietHoursPolicy &policy, const QDateTime &timestamp)
    {
        if (!policy.startTime.isValid() || !policy.endTime.isValid())
        {
            return false;
        }

        const QTime current = timestamp.time();
        if (policy.startTime == policy.endTime)
        {
            return false;
        }

        if (policy.startTime < policy.endTime)
        {
            return current >= policy.startTime && current < policy.endTime;
        }

        return current >= policy.startTime || current < policy.endTime;
    }

    QDateTime quietHoursEnd(const QuietHoursPolicy &policy, const QDateTime &timestamp)
    {
        if (!isInQuietHours(policy, timestamp))
        {
            return timestamp;
        }

        QDate date = timestamp.date();
        if (policy.startTime > policy.endTime && timestamp.time() >= policy.startTime)
        {
            date = date.addDays(1);
        }
        return QDateTime(date, policy.endTime, timestamp.timeZone());
    }

    QuietHoursDecision applyQuietHours(const QuietHoursPolicy &policy, SeverityMode mode, bool oneTimeEvent, const QDateTime &timestamp)
    {
        QuietHoursDecision decision;
        decision.effectiveMode = mode;

        if (!isInQuietHours(policy, timestamp))
        {
            return decision;
        }

        const QuietBehavior behavior = [mode, &policy]()
        {
            switch (mode)
            {
            case SeverityMode::Soft:
                return policy.behaviorSoft;
            case SeverityMode::Persistent:
                return policy.behaviorPersistent;
            case SeverityMode::Break:
                return policy.behaviorBreak;
            }
            return QuietBehavior::Allow;
        }();

        if (behavior == QuietBehavior::Allow)
        {
            return decision;
        }

        if (behavior == QuietBehavior::Downgrade)
        {
            decision.effectiveMode = SeverityMode::Soft;
            return decision;
        }

        decision.shouldShow = false;
        decision.deferUntil = quietHoursEnd(policy, timestamp);
        if (behavior == QuietBehavior::Suppress && !oneTimeEvent)
        {
            decision.shouldSuppress = true;
            decision.deferUntil = {};
        }
        return decision;
    }

    bool canSnooze(const ReminderProfile &profile, int currentSnoozeCount)
    {
        return currentSnoozeCount < profile.maxSnoozeCount;
    }

    QDateTime snoozeUntil(const ReminderProfile &profile, const QDateTime &from)
    {
        return from.addSecs(profile.snoozeMinutes * 60);
    }

    StatsDaily applyLogToDailyStats(const StatsDaily &current, const OccurrenceLogEntry &entry)
    {
        StatsDaily next = current;
        if (next.date.isNull())
        {
            next.date = entry.triggeredAt.date();
        }

        switch (entry.result)
        {
        case ReminderResult::Completed:
            ++next.completedCount;
            if (entry.shownMode == SeverityMode::Break)
            {
                ++next.breakCompletedCount;
            }
            break;
        case ReminderResult::Skipped:
        case ReminderResult::Dismissed:
        case ReminderResult::Expired:
            ++next.skippedCount;
            if (entry.shownMode == SeverityMode::Break)
            {
                ++next.breakMissedCount;
            }
            break;
        case ReminderResult::Snoozed:
            ++next.snoozedCount;
            break;
        case ReminderResult::None:
            break;
        }

        return next;
    }

    QStringList supportedRecurrenceRules()
    {
        return {
            QStringLiteral("none"),
            QStringLiteral("daily"),
            QStringLiteral("weekdays"),
            QStringLiteral("weekly:1,3,5"),
            QStringLiteral("monthly:1"),
            QStringLiteral("every_n_days:2"),
        };
    }

} // namespace deadliner::domain
