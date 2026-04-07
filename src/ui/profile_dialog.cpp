#include "ui/profile_dialog.h"

#include "ui/common/display_strings.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QEvent>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace deadliner::ui {

namespace {

QLabel *createFormLabel(QWidget *parent)
{
    auto *label = new QLabel(parent);
    label->setStyleSheet(QStringLiteral("color: palette(window-text);"));
    return label;
}

} // namespace

ProfileDialog::ProfileDialog(const QList<domain::QuietHoursPolicy> &policies,
                             const domain::ReminderProfile *profile,
                             QWidget *parent)
    : QDialog(parent)
{
    resize(460, 400);

    auto *layout = new QVBoxLayout(this);
    m_form = new QFormLayout();
    m_form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_nameEdit = new QLineEdit(this);
    m_kindCombo = new QComboBox(this);
    m_kindCombo->addItem(QString(), QVariant::fromValue(domain::ProfileKind::Break));
    m_kindCombo->addItem(QString(), QVariant::fromValue(domain::ProfileKind::Generic));
    m_kindCombo->addItem(QString(), QVariant::fromValue(domain::ProfileKind::Mixed));

    m_intervalSpin = new QSpinBox(this);
    m_intervalSpin->setRange(5, 240);
    m_intervalSpin->setValue(50);
    m_breakDurationSpin = new QSpinBox(this);
    m_breakDurationSpin->setRange(1, 60);
    m_breakDurationSpin->setValue(10);

    m_modeCombo = new QComboBox(this);
    m_modeCombo->addItem(QString(), QVariant::fromValue(domain::SeverityMode::Soft));
    m_modeCombo->addItem(QString(), QVariant::fromValue(domain::SeverityMode::Persistent));
    m_modeCombo->addItem(QString(), QVariant::fromValue(domain::SeverityMode::Break));

    m_maxSnoozeSpin = new QSpinBox(this);
    m_maxSnoozeSpin->setRange(0, 10);
    m_maxSnoozeSpin->setValue(2);
    m_snoozeSpin = new QSpinBox(this);
    m_snoozeSpin->setRange(1, 120);
    m_snoozeSpin->setValue(5);

    m_quietCombo = new QComboBox(this);
    m_quietCombo->addItem(QString(), 0);
    for (const auto &policy : policies) {
        m_quietCombo->addItem(policy.name, policy.id);
    }

    m_requireConfirmationCheck = new QCheckBox(this);
    m_requireConfirmationCheck->setChecked(true);
    m_allowSkipCheck = new QCheckBox(this);
    m_allowSkipCheck->setChecked(true);
    m_enabledCheck = new QCheckBox(this);
    m_enabledCheck->setChecked(true);

    m_nameLabel = createFormLabel(this);
    m_kindLabel = createFormLabel(this);
    m_intervalLabel = createFormLabel(this);
    m_breakDurationLabel = createFormLabel(this);
    m_modeLabel = createFormLabel(this);
    m_maxSnoozeLabel = createFormLabel(this);
    m_snoozeLabel = createFormLabel(this);
    m_quietLabel = createFormLabel(this);

    m_form->addRow(m_nameLabel, m_nameEdit);
    m_form->addRow(m_kindLabel, m_kindCombo);
    m_form->addRow(m_intervalLabel, m_intervalSpin);
    m_form->addRow(m_breakDurationLabel, m_breakDurationSpin);
    m_form->addRow(m_modeLabel, m_modeCombo);
    m_form->addRow(m_maxSnoozeLabel, m_maxSnoozeSpin);
    m_form->addRow(m_snoozeLabel, m_snoozeSpin);
    m_form->addRow(m_quietLabel, m_quietCombo);
    m_form->addRow(QString(), m_requireConfirmationCheck);
    m_form->addRow(QString(), m_allowSkipCheck);
    m_form->addRow(QString(), m_enabledCheck);
    layout->addLayout(m_form);

    auto *buttons = new QDialogButtonBox(this);
    m_saveButton = buttons->addButton(tr("Save"), QDialogButtonBox::AcceptRole);
    m_cancelButton = buttons->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    if (profile) {
        m_existingId = profile->id;
        m_builtIn = profile->builtIn;
        m_nameEdit->setText(profile->name);
        m_kindCombo->setCurrentIndex(m_kindCombo->findData(QVariant::fromValue(profile->kind)));
        m_intervalSpin->setValue(profile->intervalMinutes);
        m_breakDurationSpin->setValue(profile->breakDurationMinutes);
        m_modeCombo->setCurrentIndex(m_modeCombo->findData(QVariant::fromValue(profile->severityMode)));
        m_maxSnoozeSpin->setValue(profile->maxSnoozeCount);
        m_snoozeSpin->setValue(profile->snoozeMinutes);
        m_quietCombo->setCurrentIndex(m_quietCombo->findData(profile->quietHoursPolicyId));
        m_requireConfirmationCheck->setChecked(profile->requirePostBreakConfirmation);
        m_allowSkipCheck->setChecked(profile->allowSkip);
        m_enabledCheck->setChecked(profile->enabled);
    }

    retranslateUi();
}

domain::ReminderProfile ProfileDialog::profile() const
{
    domain::ReminderProfile item;
    item.id = m_existingId;
    item.builtIn = m_builtIn;
    item.name = m_nameEdit->text().trimmed();
    item.kind = m_kindCombo->currentData().value<domain::ProfileKind>();
    item.intervalMinutes = m_intervalSpin->value();
    item.breakDurationMinutes = m_breakDurationSpin->value();
    item.severityMode = m_modeCombo->currentData().value<domain::SeverityMode>();
    item.maxSnoozeCount = m_maxSnoozeSpin->value();
    item.snoozeMinutes = m_snoozeSpin->value();
    item.quietHoursPolicyId = m_quietCombo->currentData().toLongLong();
    item.requirePostBreakConfirmation = m_requireConfirmationCheck->isChecked();
    item.allowSkip = m_allowSkipCheck->isChecked();
    item.enabled = m_enabledCheck->isChecked();
    return item;
}

void ProfileDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QDialog::changeEvent(event);
}

void ProfileDialog::retranslateUi()
{
    setWindowTitle(m_existingId == 0 ? tr("Create profile") : tr("Edit profile"));
    m_nameLabel->setText(tr("Name"));
    m_kindLabel->setText(tr("Kind"));
    m_intervalLabel->setText(tr("Interval (minutes)"));
    m_breakDurationLabel->setText(tr("Break duration (minutes)"));
    m_modeLabel->setText(tr("Reminder mode"));
    m_maxSnoozeLabel->setText(tr("Max snoozes"));
    m_snoozeLabel->setText(tr("Snooze minutes"));
    m_quietLabel->setText(tr("Quiet hours policy"));
    m_kindCombo->setItemText(0, displayProfileKind(domain::ProfileKind::Break, this));
    m_kindCombo->setItemText(1, displayProfileKind(domain::ProfileKind::Generic, this));
    m_kindCombo->setItemText(2, displayProfileKind(domain::ProfileKind::Mixed, this));
    m_modeCombo->setItemText(0, displaySeverityMode(domain::SeverityMode::Soft, this));
    m_modeCombo->setItemText(1, displaySeverityMode(domain::SeverityMode::Persistent, this));
    m_modeCombo->setItemText(2, displaySeverityMode(domain::SeverityMode::Break, this));
    if (m_quietCombo->count() > 0) {
        m_quietCombo->setItemText(0, tr("No quiet hours"));
    }
    m_requireConfirmationCheck->setText(tr("Require post-break confirmation"));
    m_allowSkipCheck->setText(tr("Allow skip"));
    m_enabledCheck->setText(tr("Enabled"));
    m_saveButton->setText(tr("Save"));
    m_cancelButton->setText(tr("Cancel"));
}

}  // namespace deadliner::ui
