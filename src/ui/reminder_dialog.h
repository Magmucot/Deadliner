#pragma once

#include "domain/models.h"

#include <QDialog>
#include <QEvent>

class QLabel;
class QPushButton;
class QWidget;

namespace deadliner::ui {

class ReminderDialog : public QDialog {
    Q_OBJECT

public:
    explicit ReminderDialog(const domain::ReminderOccurrence &occurrence, QWidget *parent = nullptr);

    domain::ReminderAction action() const;

protected:
    void changeEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void reject() override;

private:
    void retranslateUi();
    bool isPersistentOverlay() const;

    domain::ReminderOccurrence m_occurrence;
    domain::ReminderAction m_action = domain::ReminderAction::Dismissed;
    QWidget *m_card = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_descriptionLabel = nullptr;
    QLabel *m_modeLabel = nullptr;
    QPushButton *m_doneButton = nullptr;
    QPushButton *m_snoozeButton = nullptr;
    QPushButton *m_skipButton = nullptr;
};

}  // namespace deadliner::ui
