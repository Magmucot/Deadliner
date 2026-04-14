#include "ui/tray_controller.h"

#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QMenu>
#include <QOperatingSystemVersion>
#include <QStyle>
#include <QSystemTrayIcon>

#ifdef Q_OS_LINUX
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QVariantMap>
#endif

namespace deadliner::ui
{

    TrayController::TrayController(QObject *parent)
        : QObject(parent), m_tray(new QSystemTrayIcon(this)), m_menu(new QMenu())
    {
        setIcon(QStringLiteral("variant1"));
        m_showAction = m_menu->addAction(QString());
        m_eventsAction = m_menu->addAction(QString());
        m_profilesAction = m_menu->addAction(QString());
        m_statisticsAction = m_menu->addAction(QString());
        m_settingsAction = m_menu->addAction(QString());
        m_menu->addSeparator();
        m_breakAction = m_menu->addAction(QString());
        m_pause60Action = m_menu->addAction(QString());
        m_pause180Action = m_menu->addAction(QString());
        m_pauseTomorrowAction = m_menu->addAction(QString());
        m_skipNextAction = m_menu->addAction(QString());
        m_menu->addSeparator();
        m_quitAction = m_menu->addAction(QString());

        connect(m_showAction, &QAction::triggered, this, &TrayController::openMainWindowRequested);
        connect(m_eventsAction, &QAction::triggered, this, [this]()
                { emit openSectionRequested(MainSection::Events); });
        connect(m_profilesAction, &QAction::triggered, this, [this]()
                { emit openSectionRequested(MainSection::Profiles); });
        connect(m_statisticsAction, &QAction::triggered, this, [this]()
                { emit openSectionRequested(MainSection::Statistics); });
        connect(m_settingsAction, &QAction::triggered, this, [this]()
                { emit openSectionRequested(MainSection::Settings); });
        connect(m_breakAction, &QAction::triggered, this, &TrayController::startBreakRequested);
        connect(m_pause60Action, &QAction::triggered, this, [this]()
                { emit pauseRequested(60); });
        connect(m_pause180Action, &QAction::triggered, this, [this]()
                { emit pauseRequested(180); });
        connect(m_pauseTomorrowAction, &QAction::triggered, this, [this]()
                { emit pauseRequested(12 * 60); });
        connect(m_skipNextAction, &QAction::triggered, this, &TrayController::skipNextRequested);
        connect(m_quitAction, &QAction::triggered, this, &TrayController::quitRequested);
        connect(m_tray, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason)
                {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
            emit openMainWindowRequested();
        } });

        m_tray->setContextMenu(m_menu);
        retranslateUi();
    }

    TrayController::~TrayController()
    {
        delete m_menu;
    }

    bool TrayController::isAvailable() const
    {
        return QSystemTrayIcon::isSystemTrayAvailable();
    }

    bool TrayController::supportsMessages() const
    {
        return desktopNotificationsAvailable() || QSystemTrayIcon::supportsMessages();
    }

    void TrayController::show()
    {
        m_tray->show();
    }

    void TrayController::showMessage(const QString &title, const QString &message)
    {
        if (showDesktopMessage(title, message))
        {
            return;
        }

        if (QSystemTrayIcon::supportsMessages())
        {
            m_tray->showMessage(title, message, QSystemTrayIcon::Information, 10000);
        }
    }

    bool TrayController::desktopNotificationsAvailable() const
    {
#ifdef Q_OS_LINUX
        auto *interface = QDBusConnection::sessionBus().interface();
        if (interface == nullptr)
        {
            return false;
        }

        const QDBusReply<bool> reply = interface->isServiceRegistered(QStringLiteral("org.freedesktop.Notifications"));
        return reply.isValid() && reply.value();
#else
        return false;
#endif
    }

    bool TrayController::showDesktopMessage(const QString &title, const QString &message) const
    {
#ifdef Q_OS_LINUX
        if (!desktopNotificationsAvailable())
        {
            return false;
        }

        QVariantMap hints;
        hints.insert(QStringLiteral("desktop-entry"), QStringLiteral("deadliner"));

        QDBusMessage dbusMessage = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.Notifications"),
                                                                  QStringLiteral("/org/freedesktop/Notifications"),
                                                                  QStringLiteral("org.freedesktop.Notifications"),
                                                                  QStringLiteral("Notify"));
        dbusMessage << QStringLiteral("Deadliner")
                    << static_cast<uint>(0)
                    << QStringLiteral("deadliner")
                    << title
                    << message
                    << QStringList{}
                    << hints
                    << 10000;

        const QDBusMessage reply = QDBusConnection::sessionBus().call(dbusMessage);
        return reply.type() != QDBusMessage::ErrorMessage;
#else
        Q_UNUSED(title);
        Q_UNUSED(message);
        return false;
#endif
    }

    void TrayController::retranslateUi()
    {
        m_tray->setToolTip(tr("Deadliner"));
        m_showAction->setText(tr("Open dashboard"));
        m_eventsAction->setText(tr("Events"));
        m_profilesAction->setText(tr("Profiles"));
        m_statisticsAction->setText(tr("Statistics"));
        m_settingsAction->setText(tr("Settings"));
        m_breakAction->setText(tr("Start break now"));
        m_pause60Action->setText(tr("Pause for 1 hour"));
        m_pause180Action->setText(tr("Pause for 3 hours"));
        m_pauseTomorrowAction->setText(tr("Pause until tomorrow"));
        m_skipNextAction->setText(tr("Skip next reminder"));
        m_quitAction->setText(tr("Quit"));
    }

    void TrayController::setIcon(const QString &iconVariant)
    {
        QIcon icon(iconResourcePath(iconVariant));
        if (!icon.isNull())
        {
            m_tray->setIcon(icon);
        }
        else
        {
            m_tray->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
        }
    }

    QString TrayController::iconResourcePath(const QString &iconVariant) const
    {
        const QString baseName = iconVariant == QStringLiteral("variant2")
                                     ? QStringLiteral("icon_variant2")
                                     : QStringLiteral("icon_variant1");
        const bool preferIco = QOperatingSystemVersion::currentType() == QOperatingSystemVersion::Windows;
        return QStringLiteral(":/icons/icons/%1.%2")
            .arg(baseName, preferIco ? QStringLiteral("ico") : QStringLiteral("png"));
    }

} // namespace deadliner::ui
