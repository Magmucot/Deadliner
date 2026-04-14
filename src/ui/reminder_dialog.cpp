#include "ui/reminder_dialog.h"

#include "ui/common/display_strings.h"

#include <QDialogButtonBox>
#include <QEvent>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QVBoxLayout>
#include <QWindow>
#include <QWidget>

namespace deadliner::ui
{

    namespace
    {

        // Overlay backdrop for the Persistent fullscreen mode.
        constexpr auto kOverlayQss =
            "QDialog {"
            "  background-color: rgba(8, 8, 10, 200);"
            "}";

        // Card panel that holds the actual content on the overlay.
        constexpr auto kCardQss =
            "QWidget#reminderCard {"
            "  background-color: rgba(30, 30, 34, 245);"
            "  border: 1px solid rgba(255, 255, 255, 0.12);"
            "  border-radius: 16px;"
            "}";

        // Override button QSS inside the overlay to match the dark card.
        constexpr auto kCardButtonQss =
            "QPushButton {"
            "  border-radius: 8px;"
            "  padding: 8px 22px;"
            "  font-size: 14px;"
            "  font-weight: 500;"
            "  min-width: 110px;"
            "  min-height: 38px;"
            "  background-color: rgba(255, 255, 255, 0.10);"
            "  color: white;"
            "  border: 1px solid rgba(255, 255, 255, 0.18);"
            "}"
            "QPushButton:hover {"
            "  background-color: rgba(255, 255, 255, 0.18);"
            "}"
            "QPushButton:pressed {"
            "  background-color: rgba(255, 255, 255, 0.06);"
            "}";

        // Accent style applied to the primary "Done" button on the overlay.
        constexpr auto kCardDoneButtonQss =
            "QPushButton {"
            "  background-color: rgba(74, 144, 226, 0.85);"
            "  border-color: rgba(74, 144, 226, 0.5);"
            "}"
            "QPushButton:hover {"
            "  background-color: rgba(90, 160, 240, 1.0);"
            "}";

    } // namespace

    ReminderDialog::ReminderDialog(const domain::ReminderOccurrence &occurrence, QWidget *parent)
        : QDialog(parent), m_occurrence(occurrence)
    {
        setWindowTitle(occurrence.title);
        setModal(true);

        auto *rootLayout = new QVBoxLayout(this);

        if (isPersistentOverlay())
        {
            setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
            setWindowModality(Qt::WindowModal);
            setStyleSheet(QString::fromUtf8(kOverlayQss));
            rootLayout->setContentsMargins(0, 0, 0, 0);

            rootLayout->addStretch();

            auto *centerRow = new QHBoxLayout();
            centerRow->addStretch();

            m_card = new QWidget(this);
            m_card->setObjectName(QStringLiteral("reminderCard"));
            m_card->setMinimumWidth(520);
            m_card->setMaximumWidth(720);
            m_card->setStyleSheet(QString::fromUtf8(kCardQss));

            auto *cardLayout = new QVBoxLayout(m_card);
            cardLayout->setContentsMargins(36, 32, 36, 32);
            cardLayout->setSpacing(14);

            m_titleLabel = new QLabel(m_card);
            m_descriptionLabel = new QLabel(m_card);
            m_modeLabel = new QLabel(m_card);

            m_titleLabel->setWordWrap(true);
            m_titleLabel->setAlignment(Qt::AlignCenter);
            m_titleLabel->setStyleSheet(QStringLiteral(
                "font-size: 28px; font-weight: 700; color: white;"));
            m_descriptionLabel->setWordWrap(true);
            m_descriptionLabel->setAlignment(Qt::AlignCenter);
            m_descriptionLabel->setStyleSheet(QStringLiteral(
                "font-size: 15px; color: rgba(255, 255, 255, 0.75);"));
            m_modeLabel->setAlignment(Qt::AlignCenter);
            m_modeLabel->setStyleSheet(QStringLiteral(
                "font-size: 12px; color: rgba(255, 255, 255, 0.45); margin-top: 2px;"));

            auto *buttons = new QDialogButtonBox(m_card);
            buttons->setCenterButtons(true);
            m_doneButton = buttons->addButton(QString(), QDialogButtonBox::AcceptRole);
            m_snoozeButton = buttons->addButton(QString(), QDialogButtonBox::ActionRole);
            if (occurrence.allowSkip)
            {
                m_skipButton = buttons->addButton(QString(), QDialogButtonBox::RejectRole);
            }

            // Apply overlay-aware button styles.
            for (auto *btn : buttons->buttons())
            {
                btn->setStyleSheet(QString::fromUtf8(kCardButtonQss));
            }
            m_doneButton->setStyleSheet(
                QString::fromUtf8(kCardButtonQss) + QString::fromUtf8(kCardDoneButtonQss));

            cardLayout->addWidget(m_titleLabel);
            cardLayout->addWidget(m_descriptionLabel);
            cardLayout->addWidget(m_modeLabel);
            cardLayout->addSpacing(8);
            cardLayout->addWidget(buttons);

            centerRow->addWidget(m_card);
            centerRow->addStretch();
            rootLayout->addLayout(centerRow);
            rootLayout->addStretch();
        }
        else
        {
            // Soft mode: compact dialog, inherits app theme.
            resize(440, 200);
            rootLayout->setContentsMargins(20, 20, 20, 20);
            rootLayout->setSpacing(8);

            m_titleLabel = new QLabel(this);
            m_titleLabel->setWordWrap(true);
            m_titleLabel->setStyleSheet(QStringLiteral("font-size: 17px; font-weight: 600;"));
            m_descriptionLabel = new QLabel(this);
            m_descriptionLabel->setWordWrap(true);
            m_modeLabel = new QLabel(this);
            m_modeLabel->setStyleSheet(QStringLiteral("color: palette(mid); font-size: 12px;"));

            auto *buttons = new QDialogButtonBox(this);
            m_doneButton = buttons->addButton(QString(), QDialogButtonBox::AcceptRole);
            m_snoozeButton = buttons->addButton(QString(), QDialogButtonBox::ActionRole);
            if (occurrence.allowSkip)
            {
                m_skipButton = buttons->addButton(QString(), QDialogButtonBox::RejectRole);
            }

            rootLayout->addWidget(m_titleLabel);
            rootLayout->addWidget(m_descriptionLabel);
            rootLayout->addWidget(m_modeLabel);
            rootLayout->addStretch();
            rootLayout->addWidget(buttons);
        }

        connect(m_doneButton, &QPushButton::clicked, this, [this]()
                {
        m_action = domain::ReminderAction::Completed;
        accept(); });
        connect(m_snoozeButton, &QPushButton::clicked, this, [this]()
                {
        m_action = domain::ReminderAction::Snoozed;
        accept(); });
        if (m_skipButton != nullptr)
        {
            connect(m_skipButton, &QPushButton::clicked, this, [this]()
                    {
            m_action = domain::ReminderAction::Skipped;
            reject(); });
        }

        if (isPersistentOverlay())
        {
            QWidget *anchor = parentWidget() != nullptr ? parentWidget()->window() : nullptr;
            if (anchor != nullptr)
            {
                setGeometry(anchor->geometry());
                if (windowHandle() != nullptr && anchor->windowHandle() != nullptr)
                {
                    windowHandle()->setTransientParent(anchor->windowHandle());
                }
            }
            else if (QScreen *screen = QGuiApplication::primaryScreen())
            {
                setGeometry(screen->geometry());
            }
        }

        retranslateUi();
        m_snoozeButton->setFocus();
    }

    domain::ReminderAction ReminderDialog::action() const
    {
        return m_action;
    }

    void ReminderDialog::changeEvent(QEvent *event)
    {
        if (event->type() == QEvent::LanguageChange)
        {
            retranslateUi();
        }
        QDialog::changeEvent(event);
    }

    void ReminderDialog::keyPressEvent(QKeyEvent *event)
    {
        // Prevent accidental dismissal of a persistent overlay with Escape.
        if (isPersistentOverlay() && event->key() == Qt::Key_Escape)
        {
            event->ignore();
            return;
        }
        QDialog::keyPressEvent(event);
    }

    void ReminderDialog::reject()
    {
        // If rejected without an explicit action (e.g. Alt-F4 on soft dialog),
        // treat as Snoozed rather than silent Dismissed so the reminder resurfaces.
        if (m_action == domain::ReminderAction::Dismissed)
        {
            m_action = domain::ReminderAction::Snoozed;
        }
        QDialog::reject();
    }

    void ReminderDialog::retranslateUi()
    {
        setWindowTitle(m_occurrence.title);
        m_titleLabel->setText(m_occurrence.title);
        m_descriptionLabel->setText(
            m_occurrence.description.isEmpty() ? tr("Reminder is due.") : m_occurrence.description);
        m_modeLabel->setText(tr("Mode: %1").arg(displaySeverityMode(m_occurrence.mode, this)));
        m_doneButton->setText(tr("Done"));
        m_snoozeButton->setText(tr("Snooze"));
        if (m_skipButton != nullptr)
        {
            m_skipButton->setText(tr("Skip"));
        }
    }

    bool ReminderDialog::isPersistentOverlay() const
    {
        return m_occurrence.mode == domain::SeverityMode::Persistent;
    }

} // namespace deadliner::ui
