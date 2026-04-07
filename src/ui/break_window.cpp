#include "ui/break_window.h"

#include <QCloseEvent>
#include <QEvent>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QTimer>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace deadliner::ui
{

    namespace
    {

        // Semi-transparent dark overlay for the fullscreen break window.
        constexpr auto kWindowQss =
            "QDialog {"
            "  background-color: rgba(10, 10, 12, 220);"
            "  color: white;"
            "}";

        // Override global button QSS for the dark overlay context.
        constexpr auto kButtonQss =
            "QPushButton {"
            "  min-width: 140px;"
            "  min-height: 42px;"
            "  border-radius: 8px;"
            "  padding: 8px 24px;"
            "  font-size: 15px;"
            "  font-weight: 500;"
            "  background-color: rgba(255, 255, 255, 0.12);"
            "  color: white;"
            "  border: 1px solid rgba(255, 255, 255, 0.20);"
            "}"
            "QPushButton:hover {"
            "  background-color: rgba(255, 255, 255, 0.20);"
            "}"
            "QPushButton:pressed {"
            "  background-color: rgba(255, 255, 255, 0.08);"
            "}"
            "QPushButton:disabled {"
            "  background-color: rgba(255, 255, 255, 0.06);"
            "  color: rgba(255, 255, 255, 0.35);"
            "  border-color: rgba(255, 255, 255, 0.08);"
            "}";

        // Accent style for the primary action button ("I am back").
        constexpr auto kPrimaryButtonQss =
            "QPushButton {"
            "  background-color: rgba(74, 144, 226, 0.85);"
            "  border-color: rgba(74, 144, 226, 0.6);"
            "}"
            "QPushButton:hover {"
            "  background-color: rgba(74, 144, 226, 1.0);"
            "}"
            "QPushButton:enabled:hover {"
            "  background-color: rgba(90, 160, 240, 1.0);"
            "}";

    } // namespace

    BreakWindow::BreakWindow(int breakDurationMinutes, bool allowSnooze, bool allowSkip, QWidget *parent)
        : QDialog(parent), m_remainingSeconds(qMax(1, breakDurationMinutes) * 60), m_timer(new QTimer(this)), m_allowSkip(allowSkip)
    {
        setWindowTitle(tr("Break time"));
        setWindowFlag(Qt::WindowStaysOnTopHint, true);
        setWindowState(Qt::WindowFullScreen);
        setModal(true);
        setWindowFlag(Qt::FramelessWindowHint, true);
        setStyleSheet(QString::fromUtf8(kWindowQss));

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(48, 0, 48, 64);

        auto *buttonLayout = new QHBoxLayout();
        buttonLayout->setSpacing(16);

        m_titleLabel = new QLabel(this);
        m_titleLabel->setAlignment(Qt::AlignCenter);
        m_titleLabel->setStyleSheet(QStringLiteral(
            "font-size: 30px; font-weight: 700; color: white;"));

        m_timerLabel = new QLabel(this);
        m_timerLabel->setAlignment(Qt::AlignCenter);
        m_timerLabel->setStyleSheet(QStringLiteral(
            "font-size: 80px; font-weight: 200; color: white; letter-spacing: 4px;"));

        m_tipsLabel = new QLabel(this);
        m_tipsLabel->setAlignment(Qt::AlignCenter);
        m_tipsLabel->setWordWrap(true);
        m_tipsLabel->setStyleSheet(QStringLiteral(
            "font-size: 16px; color: rgba(255, 255, 255, 0.65); margin-top: 8px;"));

        m_skipHintLabel = new QLabel(this);
        m_skipHintLabel->setAlignment(Qt::AlignCenter);
        m_skipHintLabel->setStyleSheet(QStringLiteral(
            "font-size: 13px; color: rgba(255, 255, 255, 0.45); margin-top: 4px;"));

        m_startButton = new QPushButton(this);
        m_snoozeButton = new QPushButton(this);
        m_skipButton = new QPushButton(this);
        m_returnButton = new QPushButton(this);

        for (auto *btn : {m_startButton, m_snoozeButton, m_skipButton, m_returnButton})
        {
            btn->setStyleSheet(QString::fromUtf8(kButtonQss));
        }
        m_returnButton->setStyleSheet(QString::fromUtf8(kButtonQss) + QString::fromUtf8(kPrimaryButtonQss));
        m_returnButton->setEnabled(false);
        m_skipButton->setEnabled(false);

        layout->addStretch(2);
        layout->addWidget(m_titleLabel);
        layout->addSpacing(16);
        layout->addWidget(m_timerLabel);
        layout->addWidget(m_tipsLabel);
        layout->addWidget(m_skipHintLabel);
        layout->addStretch(1);

        buttonLayout->setAlignment(Qt::AlignCenter);
        buttonLayout->addWidget(m_startButton);
        if (allowSnooze)
        {
            buttonLayout->addWidget(m_snoozeButton);
        }
        else
        {
            m_snoozeButton->hide();
        }
        if (m_allowSkip)
        {
            buttonLayout->addWidget(m_skipButton);
        }
        else
        {
            m_skipButton->hide();
            m_skipHintLabel->hide();
        }
        buttonLayout->addWidget(m_returnButton);
        layout->addLayout(buttonLayout);

        updateCountdown();

        connect(m_startButton, &QPushButton::clicked, this, [this]()
                {
        if (m_started) {
            return;
        }
        m_started = true;
        m_elapsed.start();
        m_startButton->setEnabled(false);
        m_timer->start(1000); });
        connect(m_timer, &QTimer::timeout, this, &BreakWindow::updateCountdown);
        connect(m_snoozeButton, &QPushButton::clicked, this, [this]()
                {
        m_result.snoozed = true;
        m_result.actualSeconds = m_elapsed.isValid() ? static_cast<int>(m_elapsed.elapsed() / 1000) : 0;
        reject(); });
        connect(m_skipButton, &QPushButton::clicked, this, [this]()
                {
        m_result.skipped = true;
        m_result.actualSeconds = m_elapsed.isValid() ? static_cast<int>(m_elapsed.elapsed() / 1000) : 0;
        reject(); });
        connect(m_returnButton, &QPushButton::clicked, this, [this]()
                {
        m_result.completed = true;
        m_result.actualSeconds = m_elapsed.isValid() ? static_cast<int>(m_elapsed.elapsed() / 1000) : 0;
        accept(); });

        if (QScreen *screen = QGuiApplication::primaryScreen())
        {
            setGeometry(screen->geometry());
        }

        retranslateUi();
        m_snoozeButton->setFocus();
    }

    BreakWindow::Result BreakWindow::result() const
    {
        return m_result;
    }

    void BreakWindow::changeEvent(QEvent *event)
    {
        if (event->type() == QEvent::LanguageChange)
        {
            retranslateUi();
        }
        QDialog::changeEvent(event);
    }

    void BreakWindow::closeEvent(QCloseEvent *event)
    {
        if (!m_result.completed && !m_result.snoozed)
        {
            m_result.skipped = true;
            m_result.actualSeconds = m_elapsed.isValid() ? static_cast<int>(m_elapsed.elapsed() / 1000) : 0;
        }
        QDialog::closeEvent(event);
    }

    void BreakWindow::keyPressEvent(QKeyEvent *event)
    {
        // Prevent accidental Escape dismissal of a fullscreen break.
        if (event->key() == Qt::Key_Escape)
        {
            event->ignore();
            return;
        }
        QDialog::keyPressEvent(event);
    }

    void BreakWindow::updateCountdown()
    {
        if (!m_started)
        {
            // Show the initial time before the break starts.
            const int minutes = m_remainingSeconds / 60;
            const int seconds = m_remainingSeconds % 60;
            m_timerLabel->setText(QStringLiteral("%1:%2")
                                      .arg(minutes, 2, 10, QLatin1Char('0'))
                                      .arg(seconds, 2, 10, QLatin1Char('0')));
            if (m_allowSkip)
            {
                m_skipHintLabel->setText(tr("Skip unlocks in %1 s").arg(m_skipUnlockSeconds));
            }
            return;
        }

        // Decrement first so the display goes 1→0 then transitions to "Break complete".
        --m_remainingSeconds;

        if (m_remainingSeconds <= 0)
        {
            m_timer->stop();
            m_timerLabel->setText(tr("Break complete"));
            m_returnButton->setEnabled(true);
            m_skipHintLabel->clear();
            return;
        }

        const int minutes = m_remainingSeconds / 60;
        const int seconds = m_remainingSeconds % 60;
        m_timerLabel->setText(QStringLiteral("%1:%2")
                                  .arg(minutes, 2, 10, QLatin1Char('0'))
                                  .arg(seconds, 2, 10, QLatin1Char('0')));

        if (m_allowSkip && !m_skipButton->isEnabled())
        {
            const int elapsed = static_cast<int>(m_elapsed.elapsed() / 1000);
            const int remaining = qMax(0, m_skipUnlockSeconds - elapsed);
            if (remaining == 0)
            {
                m_skipButton->setEnabled(true);
                m_skipHintLabel->clear();
            }
            else
            {
                m_skipHintLabel->setText(tr("Skip unlocks in %1 s").arg(remaining));
            }
        }
    }

    void BreakWindow::retranslateUi()
    {
        setWindowTitle(tr("Break time"));
        m_titleLabel->setText(tr("Take a break away from the screen"));
        m_tipsLabel->setText(tr("Look into the distance, stretch your shoulders, or walk for a minute."));
        m_startButton->setText(tr("Start now"));
        m_snoozeButton->setText(tr("Snooze"));
        m_skipButton->setText(tr("Skip"));
        m_returnButton->setText(tr("I am back"));
        if (m_allowSkip && !m_skipButton->isEnabled())
        {
            m_skipHintLabel->setText(tr("Skip unlocks in %1 s").arg(m_skipUnlockSeconds));
        }
    }

} // namespace deadliner::ui