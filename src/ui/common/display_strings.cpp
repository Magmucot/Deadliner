#include "ui/common/display_strings.h"

#include <QCoreApplication>
#include <QLocale>
#include <QObject>

namespace deadliner::ui {

namespace {

QString translate(const QObject *context, const char *sourceText)
{
    const char *name = context ? context->metaObject()->className() : "UiText";
    return QCoreApplication::translate(name, sourceText);
}

}  // namespace

QString displayReminderType(domain::ReminderType type, const QObject *context)
{
    switch (type) {
    case domain::ReminderType::BreakInterval:
        return translate(context, "Break interval");
    case domain::ReminderType::DateTime:
        return translate(context, "Date and time");
    }
    return {};
}

QString displaySeverityMode(domain::SeverityMode mode, const QObject *context)
{
    switch (mode) {
    case domain::SeverityMode::Soft:
        return translate(context, "Soft");
    case domain::SeverityMode::Persistent:
        return translate(context, "Persistent");
    case domain::SeverityMode::Break:
        return translate(context, "Strict break");
    }
    return {};
}

QString displayProfileKind(domain::ProfileKind kind, const QObject *context)
{
    switch (kind) {
    case domain::ProfileKind::Break:
        return translate(context, "Breaks");
    case domain::ProfileKind::Generic:
        return translate(context, "Events");
    case domain::ProfileKind::Mixed:
        return translate(context, "Mixed");
    }
    return {};
}

QString displayQuietBehavior(domain::QuietBehavior behavior, const QObject *context)
{
    switch (behavior) {
    case domain::QuietBehavior::Suppress:
        return translate(context, "Suppress");
    case domain::QuietBehavior::Defer:
        return translate(context, "Defer");
    case domain::QuietBehavior::Downgrade:
        return translate(context, "Downgrade");
    case domain::QuietBehavior::Allow:
        return translate(context, "Allow");
    }
    return {};
}

QString displayRecurrenceRule(const QString &rule, const QObject *context)
{
    if (rule.isEmpty() || rule == QStringLiteral("none")) {
        return translate(context, "Does not repeat");
    }
    if (rule == QStringLiteral("daily")) {
        return translate(context, "Daily");
    }
    if (rule == QStringLiteral("weekdays")) {
        return translate(context, "Weekdays");
    }
    if (rule.startsWith(QStringLiteral("weekly:"))) {
        return translate(context, "Weekly custom");
    }
    if (rule.startsWith(QStringLiteral("monthly:"))) {
        return translate(context, "Monthly");
    }
    if (rule.startsWith(QStringLiteral("every_n_days:"))) {
        bool ok = false;
        const int count = rule.section(':', 1).toInt(&ok);
        return ok ? translate(context, "Every %1 days").arg(count) : rule;
    }
    return rule;
}

QString formatRelativeDue(const QDateTime &dueAt, const QDateTime &now, const QObject *context)
{
    if (!dueAt.isValid()) {
        return translate(context, "Not scheduled");
    }

    qint64 seconds = now.secsTo(dueAt);
    if (seconds <= 0) {
        return translate(context, "Due now");
    }

    const qint64 days = seconds / 86400;
    seconds %= 86400;
    const qint64 hours = seconds / 3600;
    seconds %= 3600;
    const qint64 minutes = (seconds + 59) / 60;

    if (days > 0) {
        return translate(context, "In %1 d %2 h").arg(days).arg(hours);
    }
    if (hours > 0) {
        return translate(context, "In %1 h %2 min").arg(hours).arg(minutes);
    }
    return translate(context, "In %1 min").arg(qMax<qint64>(1, minutes));
}

QString formatDateTimeShort(const QDateTime &value, const QLocale &locale)
{
    return value.isValid() ? locale.toString(value, QLocale::ShortFormat) : QString {};
}

QString formatPauseUntil(const QDateTime &value, const QLocale &locale, const QObject *context)
{
    if (!value.isValid() || value <= QDateTime::currentDateTime()) {
        return translate(context, "Reminders are active");
    }
    return translate(context, "Paused until %1").arg(locale.toString(value, QLocale::ShortFormat));
}

}  // namespace deadliner::ui
