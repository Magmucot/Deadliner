#pragma once

#include <QDialog>
#include <QElapsedTimer>
#include <QEvent>

class QLabel;
class QPushButton;
class QTimer;
class QCloseEvent;
class QKeyEvent;

namespace deadliner::ui {

class BreakWindow : public QDialog {
    Q_OBJECT

public:
    struct Result {
        bool completed = false;
        bool snoozed = false;
        bool skipped = false;
        int actualSeconds = 0;
    };

    BreakWindow(int breakDurationMinutes, bool allowSnooze, bool allowSkip, QWidget *parent = nullptr);

    Result result() const;

protected:
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void updateCountdown();
    void retranslateUi();

    int m_remainingSeconds = 0;
    int m_skipUnlockSeconds = 10;
    QTimer *m_timer = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_tipsLabel = nullptr;
    QLabel *m_skipHintLabel = nullptr;
    QLabel *m_timerLabel = nullptr;
    QPushButton *m_startButton = nullptr;
    QPushButton *m_snoozeButton = nullptr;
    QPushButton *m_skipButton = nullptr;
    QPushButton *m_returnButton = nullptr;
    QElapsedTimer m_elapsed;
    bool m_started = false;
    bool m_allowSkip = false;
    Result m_result;
};

}  // namespace deadliner::ui
