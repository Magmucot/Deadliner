#pragma once

#include "domain/models.h"

#include <QDialog>
#include <QEvent>

class QCheckBox;
class QComboBox;
class QFormLayout;
class QLineEdit;
class QPushButton;
class QSpinBox;

namespace deadliner::ui {

class ProfileDialog : public QDialog {
    Q_OBJECT

public:
    ProfileDialog(const QList<domain::QuietHoursPolicy> &policies,
                  const domain::ReminderProfile *profile,
                  QWidget *parent = nullptr);

    domain::ReminderProfile profile() const;

protected:
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();

    qint64 m_existingId = 0;
    bool m_builtIn = false;
    QFormLayout *m_form = nullptr;
    QLineEdit *m_nameEdit = nullptr;
    QComboBox *m_kindCombo = nullptr;
    QSpinBox *m_intervalSpin = nullptr;
    QSpinBox *m_breakDurationSpin = nullptr;
    QComboBox *m_modeCombo = nullptr;
    QSpinBox *m_maxSnoozeSpin = nullptr;
    QSpinBox *m_snoozeSpin = nullptr;
    QComboBox *m_quietCombo = nullptr;
    QCheckBox *m_requireConfirmationCheck = nullptr;
    QCheckBox *m_allowSkipCheck = nullptr;
    QCheckBox *m_enabledCheck = nullptr;
    QPushButton *m_saveButton = nullptr;
    QPushButton *m_cancelButton = nullptr;
};

}  // namespace deadliner::ui
