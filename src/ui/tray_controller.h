#pragma once

#include "ui/common/navigation.h"

#include <QObject>
#include <QString>

class QAction;
class QMenu;
class QSystemTrayIcon;

namespace deadliner::ui {

class TrayController : public QObject {
    Q_OBJECT

public:
    explicit TrayController(QObject *parent = nullptr);
    ~TrayController() override;

    bool isAvailable() const;
    bool supportsMessages() const;
    void show();
    void showMessage(const QString &title, const QString &message);
    void retranslateUi();
    void setIcon(const QString &iconVariant);

signals:
    void openMainWindowRequested();
    void openSectionRequested(deadliner::ui::MainSection section);
    void startBreakRequested();
    void snoozeAllRequested(int minutes);
    void pauseRequested(int minutes);
    void skipNextRequested();
    void quitRequested();

private:
    QString iconResourcePath(const QString &iconVariant) const;
    bool desktopNotificationsAvailable() const;
    bool showDesktopMessage(const QString &title, const QString &message) const;

    QAction *m_showAction = nullptr;
    QAction *m_eventsAction = nullptr;
    QAction *m_profilesAction = nullptr;
    QAction *m_statisticsAction = nullptr;
    QAction *m_settingsAction = nullptr;
    QAction *m_breakAction = nullptr;
    QAction *m_pause60Action = nullptr;
    QAction *m_pause180Action = nullptr;
    QAction *m_pauseTomorrowAction = nullptr;
    QAction *m_skipNextAction = nullptr;
    QAction *m_quitAction = nullptr;
    QSystemTrayIcon *m_tray = nullptr;
    QMenu *m_menu = nullptr;
};

}  // namespace deadliner::ui
