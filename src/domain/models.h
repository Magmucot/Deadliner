#pragma once

#include <QDate>
#include <QDateTime>
#include <QMetaType>
#include <QByteArray>
#include <QTime>
#include <QString>

#include <optional>

namespace deadliner::domain {

enum class ReminderType {
    BreakInterval,
    DateTime,
};

enum class SeverityMode {
    Soft,
    Persistent,
    Break,
};

enum class ProfileKind {
    Break,
    Generic,
    Mixed,
};

enum class QuietBehavior {
    Suppress,
    Defer,
    Downgrade,
    Allow,
};

enum class ReminderResult {
    None,
    Completed,
    Skipped,
    Snoozed,
    Dismissed,
    Expired,
};

enum class ReminderAction {
    Completed,
    Snoozed,
    Skipped,
    Dismissed,
};

struct QuietHoursPolicy {
    qint64 id = 0;
    QString name;
    QTime startTime;
    QTime endTime;
    QuietBehavior behaviorSoft = QuietBehavior::Suppress;
    QuietBehavior behaviorPersistent = QuietBehavior::Defer;
    QuietBehavior behaviorBreak = QuietBehavior::Downgrade;
    QDateTime createdAt;
    QDateTime updatedAt;
};

struct ReminderProfile {
    qint64 id = 0;
    QString name;
    ProfileKind kind = ProfileKind::Mixed;
    int intervalMinutes = 50;
    int breakDurationMinutes = 10;
    SeverityMode severityMode = SeverityMode::Soft;
    int maxSnoozeCount = 1;
    int snoozeMinutes = 5;
    qint64 quietHoursPolicyId = 0;
    bool requirePostBreakConfirmation = true;
    bool allowSkip = true;
    bool enabled = true;
    bool builtIn = false;
    QDateTime createdAt;
    QDateTime updatedAt;
};

struct ReminderEvent {
    qint64 id = 0;
    QString title;
    QString description;
    ReminderType type = ReminderType::DateTime;
    qint64 profileId = 0;
    QDateTime startAt;
    QString timezoneMode = QStringLiteral("local");
    QString recurrenceRule = QStringLiteral("none");
    bool isOneTime = true;
    bool enabled = true;
    QDateTime lastTriggeredAt;
    QDateTime nextTriggerAt;
    int pendingSnoozeCount = 0;
    QDateTime pendingOriginalTriggerAt;
    std::optional<SeverityMode> pendingMode;
    QDateTime createdAt;
    QDateTime updatedAt;
};

struct ReminderOccurrence {
    qint64 eventId = 0;
    qint64 profileId = 0;
    QString title;
    QString description;
    QDateTime dueAt;
    QDateTime originalTriggerAt;
    SeverityMode mode = SeverityMode::Soft;
    int snoozeCount = 0;
    bool allowSkip = true;
    int breakDurationMinutes = 0;
    bool requirePostBreakConfirmation = true;
};

struct OccurrenceLogEntry {
    qint64 id = 0;
    qint64 eventId = 0;
    QDateTime triggeredAt;
    SeverityMode shownMode = SeverityMode::Soft;
    ReminderResult result = ReminderResult::None;
    int snoozeCount = 0;
    int breakExpectedSeconds = 0;
    int breakActualSeconds = 0;
    QDateTime completedAt;
};

struct StatsDaily {
    QDate date;
    int completedCount = 0;
    int skippedCount = 0;
    int snoozedCount = 0;
    int breakCompletedCount = 0;
    int breakMissedCount = 0;
};

struct AppSettings {
    bool minimizeToTray = true;
    bool closeToTray = true;
    QByteArray lastWindowGeometry;
    QString language = QStringLiteral("system");
    QString theme = QStringLiteral("system");
    bool launchOnStartup = false;
    bool startMinimized = false;
    bool onboardingCompleted = false;
    qint64 defaultProfileId = 0;
    QDateTime pauseUntil;
    QString logLevel = QStringLiteral("info");
};

struct QuietHoursDecision {
    bool shouldShow = true;
    bool shouldSuppress = false;
    QDateTime deferUntil;
    SeverityMode effectiveMode = SeverityMode::Soft;
};

QString toString(ReminderType value);
QString toString(SeverityMode value);
QString toString(ProfileKind value);
QString toString(QuietBehavior value);
QString toString(ReminderResult value);

ReminderType reminderTypeFromString(const QString &value);
SeverityMode severityModeFromString(const QString &value);
ProfileKind profileKindFromString(const QString &value);
QuietBehavior quietBehaviorFromString(const QString &value);
ReminderResult reminderResultFromString(const QString &value);

}  // namespace deadliner::domain

Q_DECLARE_METATYPE(deadliner::domain::ReminderType)
Q_DECLARE_METATYPE(deadliner::domain::SeverityMode)
Q_DECLARE_METATYPE(deadliner::domain::ProfileKind)
Q_DECLARE_METATYPE(deadliner::domain::QuietBehavior)
Q_DECLARE_METATYPE(deadliner::domain::ReminderResult)
Q_DECLARE_METATYPE(deadliner::domain::ReminderAction)
