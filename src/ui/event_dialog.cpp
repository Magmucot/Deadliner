#include "ui/event_dialog.h"

#include "domain/logic.h"

#include "ui/common/display_strings.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDialogButtonBox>
#include <QEvent>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>

namespace deadliner::ui {

EventDialog::EventDialog(const QList<domain::ReminderProfile> &profiles,
                         const domain::ReminderEvent *event,
                         QWidget *parent)
    : QDialog(parent)
    , m_profiles(profiles)
{
    resize(560, 420);

    auto *layout = new QVBoxLayout(this);
    m_form = new QFormLayout();

    m_titleEdit = new QLineEdit(this);
    m_descriptionEdit = new QPlainTextEdit(this);
    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem(QString(), QVariant::fromValue(domain::ReminderType::DateTime));
    m_typeCombo->addItem(QString(), QVariant::fromValue(domain::ReminderType::BreakInterval));

    m_profileCombo = new QComboBox(this);
    for (const auto &profile : m_profiles) {
        m_profileCombo->addItem(profile.name, profile.id);
    }

    m_startEdit = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(3600), this);
    m_startEdit->setCalendarPopup(true);
    m_recurrenceCombo = new QComboBox(this);
    for (const auto &rule : domain::supportedRecurrenceRules()) {
        m_recurrenceCombo->addItem(QString(), rule);
    }
    m_oneTimeCheck = new QCheckBox(this);
    m_enabledCheck = new QCheckBox(this);
    m_enabledCheck->setChecked(true);
    m_profileHintLabel = new QLabel(this);
    m_profileHintLabel->setWordWrap(true);
    m_profileInfoLabel = new QLabel(this);
    m_profileInfoLabel->setWordWrap(true);

    m_form->addRow(QString(), m_profileHintLabel);
    m_form->addRow(QString(), m_titleEdit);
    m_form->addRow(QString(), m_descriptionEdit);
    m_form->addRow(QString(), m_typeCombo);
    m_form->addRow(QString(), m_profileCombo);
    m_form->addRow(QString(), m_profileInfoLabel);
    m_form->addRow(QString(), m_startEdit);
    m_form->addRow(QString(), m_recurrenceCombo);
    m_form->addRow(QString(), m_oneTimeCheck);
    m_form->addRow(QString(), m_enabledCheck);

    layout->addLayout(m_form);

    auto *buttons = new QDialogButtonBox(this);
    m_saveButton = buttons->addButton(tr("Save"), QDialogButtonBox::AcceptRole);
    m_cancelButton = buttons->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    if (event) {
        m_existingId = event->id;
        m_titleEdit->setText(event->title);
        m_descriptionEdit->setPlainText(event->description);
        m_typeCombo->setCurrentIndex(m_typeCombo->findData(QVariant::fromValue(event->type)));
        m_profileCombo->setCurrentIndex(m_profileCombo->findData(event->profileId));
        m_startEdit->setDateTime(event->startAt);
        m_recurrenceCombo->setCurrentIndex(m_recurrenceCombo->findData(event->recurrenceRule));
        m_oneTimeCheck->setChecked(event->isOneTime);
        m_enabledCheck->setChecked(event->enabled);
    }

    connect(m_profileCombo, &QComboBox::currentIndexChanged, this, [this]() { updateProfileSummary(); });
    retranslateUi();
    updateProfileSummary();
}

domain::ReminderEvent EventDialog::event() const
{
    domain::ReminderEvent item;
    item.id = m_existingId;
    item.title = m_titleEdit->text().trimmed();
    item.description = m_descriptionEdit->toPlainText().trimmed();
    item.type = m_typeCombo->currentData().value<domain::ReminderType>();
    item.profileId = m_profileCombo->currentData().toLongLong();
    item.startAt = m_startEdit->dateTime();
    item.recurrenceRule = m_recurrenceCombo->currentData().toString();
    item.isOneTime = m_oneTimeCheck->isChecked();
    item.enabled = m_enabledCheck->isChecked();
    if (item.isOneTime) {
        item.recurrenceRule = QStringLiteral("none");
    }
    return item;
}

void EventDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
        updateProfileSummary();
    }
    QDialog::changeEvent(event);
}

void EventDialog::retranslateUi()
{
    const auto setLabelText = [this](QWidget *field, const QString &text) {
        if (auto *label = qobject_cast<QLabel *>(m_form->labelForField(field))) {
            label->setText(text);
        }
    };

    setWindowTitle(m_existingId == 0 ? tr("Create event") : tr("Edit event"));
    m_profileHintLabel->setText(tr("Events inherit reminder mode and snooze policy from the selected profile."));
    setLabelText(m_titleEdit, tr("Title"));
    setLabelText(m_descriptionEdit, tr("Description"));
    setLabelText(m_typeCombo, tr("Type"));
    setLabelText(m_profileCombo, tr("Profile"));
    setLabelText(m_profileInfoLabel, tr("Profile behavior"));
    setLabelText(m_startEdit, tr("Start at / anchor"));
    setLabelText(m_recurrenceCombo, tr("Recurrence"));
    m_oneTimeCheck->setText(tr("One-time event"));
    m_enabledCheck->setText(tr("Enabled"));
    m_saveButton->setText(tr("Save"));
    m_cancelButton->setText(tr("Cancel"));

    m_typeCombo->setItemText(0, displayReminderType(domain::ReminderType::DateTime, this));
    m_typeCombo->setItemText(1, displayReminderType(domain::ReminderType::BreakInterval, this));

    for (int index = 0; index < m_recurrenceCombo->count(); ++index) {
        m_recurrenceCombo->setItemText(index, displayRecurrenceRule(m_recurrenceCombo->itemData(index).toString(), this));
    }
}

void EventDialog::updateProfileSummary()
{
    const qint64 profileId = m_profileCombo->currentData().toLongLong();
    for (const auto &profile : m_profiles) {
        if (profile.id == profileId) {
            m_profileInfoLabel->setText(tr("%1 mode, snooze %2 x %3 min, break %4 min")
                                            .arg(displaySeverityMode(profile.severityMode, this))
                                            .arg(profile.maxSnoozeCount)
                                            .arg(profile.snoozeMinutes)
                                            .arg(profile.breakDurationMinutes));
            return;
        }
    }
    m_profileInfoLabel->clear();
}

}  // namespace deadliner::ui
