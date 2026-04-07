#pragma once

#include "domain/models.h"

#include <QDateTime>
#include <QString>

class QObject;
class QLocale;

namespace deadliner::ui {

QString displayReminderType(domain::ReminderType type, const QObject *context);
QString displaySeverityMode(domain::SeverityMode mode, const QObject *context);
QString displayProfileKind(domain::ProfileKind kind, const QObject *context);
QString displayQuietBehavior(domain::QuietBehavior behavior, const QObject *context);
QString displayRecurrenceRule(const QString &rule, const QObject *context);
QString formatRelativeDue(const QDateTime &dueAt, const QDateTime &now, const QObject *context);
QString formatDateTimeShort(const QDateTime &value, const QLocale &locale);
QString formatPauseUntil(const QDateTime &value, const QLocale &locale, const QObject *context);

}  // namespace deadliner::ui
