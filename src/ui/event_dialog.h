#pragma once

#include "domain/models.h"

#include <QDialog>
#include <QEvent>

class QCheckBox;
class QComboBox;
class QDateTimeEdit;
class QFormLayout;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

namespace deadliner::ui {

class EventDialog : public QDialog {
    Q_OBJECT

public:
    EventDialog(const QList<domain::ReminderProfile> &profiles,
                const domain::ReminderEvent *event,
                QWidget *parent = nullptr);

    domain::ReminderEvent event() const;

protected:
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();
    void updateProfileSummary();

    QList<domain::ReminderProfile> m_profiles;
    qint64 m_existingId = 0;
    QFormLayout *m_form = nullptr;
    QLabel *m_titleFieldLabel = nullptr;
    QLabel *m_descriptionFieldLabel = nullptr;
    QLabel *m_typeFieldLabel = nullptr;
    QLabel *m_profileFieldLabel = nullptr;
    QLabel *m_profileBehaviorFieldLabel = nullptr;
    QLabel *m_startFieldLabel = nullptr;
    QLabel *m_recurrenceFieldLabel = nullptr;
    QLineEdit *m_titleEdit = nullptr;
    QPlainTextEdit *m_descriptionEdit = nullptr;
    QComboBox *m_typeCombo = nullptr;
    QComboBox *m_profileCombo = nullptr;
    QDateTimeEdit *m_startEdit = nullptr;
    QComboBox *m_recurrenceCombo = nullptr;
    QCheckBox *m_oneTimeCheck = nullptr;
    QCheckBox *m_enabledCheck = nullptr;
    QLabel *m_profileHintLabel = nullptr;
    QLabel *m_profileInfoLabel = nullptr;
    QPushButton *m_saveButton = nullptr;
    QPushButton *m_cancelButton = nullptr;
};

}  // namespace deadliner::ui
