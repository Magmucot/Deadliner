#include <QApplication>
#include <algorithm>
#include <QTemporaryDir>
#include <QTimer>
#include <QtTest>

#include <sstream>

#define private public
#include "app/app_controller.h"
#undef private

#include "ui/reminder_dialog.h"

using namespace deadliner;

namespace {

int visibleReminderDialogCount()
{
    int count = 0;
    for (QWidget *widget : QApplication::topLevelWidgets())
    {
        auto *dialog = qobject_cast<ui::ReminderDialog *>(widget);
        if (dialog != nullptr && dialog->isVisible())
        {
            ++count;
        }
    }
    return count;
}

void closeReminderDialogs()
{
    for (QWidget *widget : QApplication::topLevelWidgets())
    {
        auto *dialog = qobject_cast<ui::ReminderDialog *>(widget);
        if (dialog != nullptr)
        {
            QMetaObject::invokeMethod(dialog, "reject", Qt::QueuedConnection);
        }
    }
}

} // namespace

class AppControllerTests : public QObject {
    Q_OBJECT

private slots:
    void persistentReminderIgnoresReentrantOccurrence();
    void refreshStatePreservesFutureRecurringTrigger();
};

void AppControllerTests::persistentReminderIgnoresReentrantOccurrence()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    app::AppController controller;

    QString error;
    QVERIFY(controller.m_repository.open(dir.filePath(QStringLiteral("deadliner.sqlite")), &error));

    domain::ReminderProfile profile;
    profile.id = 1;
    profile.enabled = true;
    profile.allowSkip = true;
    profile.snoozeMinutes = 1;
    profile.severityMode = domain::SeverityMode::Persistent;
    controller.m_state.profiles = {profile};

    domain::ReminderOccurrence occurrence;
    occurrence.eventId = -1;
    occurrence.profileId = profile.id;
    occurrence.title = QStringLiteral("Persistent test reminder");
    occurrence.mode = domain::SeverityMode::Persistent;
    occurrence.allowSkip = true;

    int maxDialogsSeen = 0;
    QTimer monitor;
    monitor.setInterval(10);
    connect(&monitor, &QTimer::timeout, this, [&maxDialogsSeen]()
            { maxDialogsSeen = qMax(maxDialogsSeen, visibleReminderDialogCount()); });
    monitor.start();

    QTimer::singleShot(50, &controller, [&controller, occurrence]()
                       { controller.handleOccurrence(occurrence); });
    QTimer::singleShot(150, this, []()
                       { closeReminderDialogs(); });

    controller.handleOccurrence(occurrence);

    monitor.stop();
    closeReminderDialogs();

    QCOMPARE(maxDialogsSeen, 1);
    QVERIFY(!controller.m_handlingOccurrence);
}

void AppControllerTests::refreshStatePreservesFutureRecurringTrigger()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    app::AppController controller;

    QString error;
    QVERIFY(controller.m_repository.open(dir.filePath(QStringLiteral("deadliner.sqlite")), &error));

    domain::ReminderProfile profile;
    profile.name = QStringLiteral("Test profile");
    profile.enabled = true;
    profile.allowSkip = true;
    profile.severityMode = domain::SeverityMode::Soft;
    const qint64 profileId = controller.m_repository.saveProfile(profile);
    QVERIFY(profileId > 0);

    const QDateTime now = QDateTime::currentDateTime();

    domain::ReminderEvent event;
    event.title = QStringLiteral("Future recurring reminder");
    event.type = domain::ReminderType::DateTime;
    event.profileId = profileId;
    event.enabled = true;
    event.isOneTime = false;
    event.recurrenceRule = QStringLiteral("daily");
    event.startAt = now.addDays(-3).addSecs(-3600);
    event.nextTriggerAt = now.addDays(2);
    const qint64 eventId = controller.m_repository.saveEvent(event);
    QVERIFY(eventId > 0);

    controller.refreshState();

    const auto storedEvents = controller.m_repository.loadEvents();
    const auto storedIt = std::find_if(storedEvents.cbegin(), storedEvents.cend(), [eventId](const domain::ReminderEvent &candidate)
                                       { return candidate.id == eventId; });
    QVERIFY(storedIt != storedEvents.cend());
    QCOMPARE(storedIt->nextTriggerAt, event.nextTriggerAt);

    const auto stateIt = std::find_if(controller.m_state.events.cbegin(), controller.m_state.events.cend(), [eventId](const domain::ReminderEvent &candidate)
                                      { return candidate.id == eventId; });
    QVERIFY(stateIt != controller.m_state.events.cend());
    QCOMPARE(stateIt->nextTriggerAt, event.nextTriggerAt);
}

int main(int argc, char **argv)
{
    qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    QApplication app(argc, argv);
    AppControllerTests tests;
    return QTest::qExec(&tests, argc, argv);
}

#include "test_app_controller.moc"
