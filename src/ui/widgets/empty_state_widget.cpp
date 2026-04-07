#include "ui/widgets/empty_state_widget.h"

#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

namespace deadliner::ui
{

    EmptyStateWidget::EmptyStateWidget(QWidget *parent)
        : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(32, 32, 32, 32);
        layout->setSpacing(10);

        m_iconLabel = new QLabel(this);
        m_iconLabel->setAlignment(Qt::AlignCenter);
        m_iconLabel->setPixmap(qApp->style()->standardIcon(QStyle::SP_FileDialogContentsView).pixmap(48, 48));

        m_titleLabel = new QLabel(this);
        m_titleLabel->setAlignment(Qt::AlignCenter);
        m_titleLabel->setWordWrap(true);
        m_titleLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 600;"));

        m_descriptionLabel = new QLabel(this);
        m_descriptionLabel->setAlignment(Qt::AlignCenter);
        m_descriptionLabel->setWordWrap(true);

        m_actionButton = new QPushButton(this);
        m_actionButton->hide();

        layout->addStretch();
        layout->addWidget(m_iconLabel, 0, Qt::AlignCenter);
        layout->addWidget(m_titleLabel);
        layout->addWidget(m_descriptionLabel);
        layout->addWidget(m_actionButton, 0, Qt::AlignHCenter);
        layout->addStretch();

        connect(m_actionButton, &QPushButton::clicked, this, &EmptyStateWidget::actionTriggered);
    }

    void EmptyStateWidget::setTitle(const QString &title)
    {
        m_titleLabel->setText(title);
    }

    void EmptyStateWidget::setDescription(const QString &description)
    {
        m_descriptionLabel->setText(description);
    }

    void EmptyStateWidget::setActionText(const QString &text)
    {
        m_actionButton->setVisible(!text.isEmpty());
        m_actionButton->setText(text);
    }

} // namespace deadliner::ui
