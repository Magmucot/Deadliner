#pragma once

#include <QWidget>

class QLabel;
class QPushButton;

namespace deadliner::ui
{

    class EmptyStateWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit EmptyStateWidget(QWidget *parent = nullptr);

        void setTitle(const QString &title);
        void setDescription(const QString &description);
        void setActionText(const QString &text);

    signals:
        void actionTriggered();

    private:
        QLabel *m_iconLabel = nullptr;
        QLabel *m_titleLabel = nullptr;
        QLabel *m_descriptionLabel = nullptr;
        QPushButton *m_actionButton = nullptr;
    };

} // namespace deadliner::ui
