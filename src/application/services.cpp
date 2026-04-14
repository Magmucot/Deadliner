#include "application/services.h"

#include "domain/logic.h"

#include <algorithm>
#include <limits>

namespace deadliner::application
{

    ReminderScheduler::ReminderScheduler(QObject *parent)
        : QObject(parent)
    {
        m_timer.setSingleShot(true);
        connect(&m_timer, &QTimer::timeout, this, &ReminderScheduler::onTimeout);
    }

    void ReminderScheduler::setState(const QList<domain::ReminderEvent> &events,
                                     const QHash<qint64, domain::ReminderProfile> &profiles)
    {
        m_events = events;
        m_profiles = profiles;
        rebuild();
    }

    QList<domain::ReminderOccurrence> ReminderScheduler::upcomingOccurrences() const
    {
        return buildOccurrences(QDateTime::currentDateTime());
    }

    void ReminderScheduler::rebuild()
    {
        armNext();
    }

    QList<domain::ReminderOccurrence> ReminderScheduler::buildOccurrences(const QDateTime &now) const
    {
        QList<domain::ReminderOccurrence> result;
        for (const auto &event : m_events)
        {
            const auto profileIt = m_profiles.constFind(event.profileId);
            if (profileIt == m_profiles.constEnd())
            {
                continue;
            }

            const auto &profile = profileIt.value();
            if (!event.enabled || !profile.enabled)
            {
                continue;
            }

            if (!domain::isSchedulableEvent(event, profile, now))
            {
                continue;
            }

            const QDateTime dueAt = event.nextTriggerAt.isValid()
                                        ? event.nextTriggerAt
                                        : domain::normalizeNextTrigger(event, profile, now);

            if (!dueAt.isValid())
            {
                continue;
            }

            domain::ReminderOccurrence occurrence;
            occurrence.eventId = event.id;
            occurrence.profileId = event.profileId;
            occurrence.title = event.title;
            occurrence.description = event.description;
            occurrence.dueAt = dueAt;
            occurrence.originalTriggerAt = event.pendingOriginalTriggerAt.isValid() ? event.pendingOriginalTriggerAt : dueAt;
            occurrence.mode = event.pendingMode.value_or(profile.severityMode);
            occurrence.snoozeCount = event.pendingSnoozeCount;
            occurrence.allowSkip = profile.allowSkip;
            occurrence.breakDurationMinutes = profile.breakDurationMinutes;
            occurrence.requirePostBreakConfirmation = profile.requirePostBreakConfirmation;
            result.push_back(occurrence);
        }

        std::sort(result.begin(), result.end(), [](const auto &left, const auto &right)
                  { return left.dueAt < right.dueAt; });
        return result;
    }

    void ReminderScheduler::armNext()
    {
        m_timer.stop();
        m_armedFor = {};
        const auto occurrences = buildOccurrences(QDateTime::currentDateTime());
        if (occurrences.isEmpty())
        {
            return;
        }

        m_armedFor = occurrences.constFirst().dueAt;
        const qint64 ms = qMax<qint64>(0, QDateTime::currentDateTime().msecsTo(m_armedFor));
        m_timer.start(static_cast<int>(qMin<qint64>(ms, std::numeric_limits<int>::max())));
    }

    bool ReminderScheduler::needsResync(const QDateTime &now) const
    {
        if (!m_armedFor.isValid())
        {
            return false;
        }

        if (now >= m_armedFor.addSecs(2))
        {
            return true;
        }

        if (!m_timer.isActive())
        {
            return false;
        }

        const qint64 remainingMs = m_timer.remainingTimeAsDuration().count();
        if (remainingMs < 0)
        {
            return false;
        }

        const QDateTime expected = now.addMSecs(remainingMs);
        return qAbs(expected.msecsTo(m_armedFor)) > 2000;
    }

    void ReminderScheduler::onTimeout()
    {
        const QDateTime now = QDateTime::currentDateTime();
        const auto occurrences = buildOccurrences(now);
        for (const auto &occurrence : occurrences)
        {
            if (occurrence.dueAt <= now.addSecs(1))
            {
                emit occurrenceDue(occurrence);
            }
            else
            {
                break;
            }
        }
        armNext();
    }

    domain::StatsDaily StatisticsService::updateDaily(const QList<domain::StatsDaily> &current,
                                                      const domain::OccurrenceLogEntry &entry) const
    {
        for (const auto &stats : current)
        {
            if (stats.date == entry.triggeredAt.date())
            {
                return domain::applyLogToDailyStats(stats, entry);
            }
        }
        return domain::applyLogToDailyStats({}, entry);
    }

} // namespace deadliner::application
