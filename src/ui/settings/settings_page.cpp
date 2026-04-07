#include "ui/settings/settings_page.h"

#include "ui/common/display_strings.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QEvent>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTimeEdit>
#include <QVariant>
#include <QVBoxLayout>

namespace deadliner::ui
{

    namespace
    {

        QComboBox *createBehaviorCombo(QWidget *parent)
        {
            auto *combo = new QComboBox(parent);
            combo->addItem(QString(), QVariant::fromValue(domain::QuietBehavior::Suppress));
            combo->addItem(QString(), QVariant::fromValue(domain::QuietBehavior::Defer));
            combo->addItem(QString(), QVariant::fromValue(domain::QuietBehavior::Downgrade));
            combo->addItem(QString(), QVariant::fromValue(domain::QuietBehavior::Allow));
            return combo;
        }

        QLabel *createFormLabel(QWidget *parent)
        {
            auto *label = new QLabel(parent);
            label->setWordWrap(true);
            return label;
        }

    } // namespace

    SettingsPage::SettingsPage(QWidget *parent)
        : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(24, 24, 24, 24);
        layout->setSpacing(16);

        m_titleLabel = new QLabel(this);
        m_titleLabel->setStyleSheet(QStringLiteral("font-size: 22px; font-weight: 600;"));
        m_subtitleLabel = new QLabel(this);
        m_subtitleLabel->setWordWrap(true);
        m_platformNoteLabel = new QLabel(this);
        m_platformNoteLabel->setWordWrap(true);

        m_form = new QFormLayout();
        m_form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

        m_launchCheckbox = new QCheckBox(this);
        m_startMinimizedCheckbox = new QCheckBox(this);
        m_minimizeToTrayCheckbox = new QCheckBox(this);
        m_closeToTrayCheckbox = new QCheckBox(this);
        m_languageCombo = new QComboBox(this);
        m_themeCombo = new QComboBox(this);
        m_trayIconCombo = new QComboBox(this);
        m_trayIconCombo->addItem(QString(), QStringLiteral("variant1"));
        m_trayIconCombo->addItem(QString(), QStringLiteral("variant2"));
        m_defaultProfileCombo = new QComboBox(this);
        m_pauseUntilEdit = new QDateTimeEdit(this);
        m_pauseUntilEdit->setCalendarPopup(true);
        m_quietStartEdit = new QTimeEdit(this);
        m_quietEndEdit = new QTimeEdit(this);
        m_softBehaviorCombo = createBehaviorCombo(this);
        m_persistentBehaviorCombo = createBehaviorCombo(this);
        m_breakBehaviorCombo = createBehaviorCombo(this);
        m_saveButton = new QPushButton(this);

        m_form->addRow(m_launchCheckbox);
        m_form->addRow(m_startMinimizedCheckbox);
        m_form->addRow(m_minimizeToTrayCheckbox);
        m_form->addRow(m_closeToTrayCheckbox);
        m_form->addRow(createFormLabel(this), m_languageCombo);
        m_form->addRow(createFormLabel(this), m_themeCombo);
        m_form->addRow(createFormLabel(this), m_trayIconCombo);
        m_form->addRow(createFormLabel(this), m_defaultProfileCombo);
        m_form->addRow(createFormLabel(this), m_pauseUntilEdit);
        m_form->addRow(createFormLabel(this), m_quietStartEdit);
        m_form->addRow(createFormLabel(this), m_quietEndEdit);
        m_form->addRow(createFormLabel(this), m_softBehaviorCombo);
        m_form->addRow(createFormLabel(this), m_persistentBehaviorCombo);
        m_form->addRow(createFormLabel(this), m_breakBehaviorCombo);

        layout->addWidget(m_titleLabel);
        layout->addWidget(m_subtitleLabel);
        layout->addWidget(m_platformNoteLabel);
        layout->addLayout(m_form);
        layout->addWidget(m_saveButton, 0, Qt::AlignLeft);
        layout->addStretch();

        repopulateCombos();

        connect(m_saveButton, &QPushButton::clicked, this, [this]()
                {
        domain::AppSettings settings = m_settings;
        settings.launchOnStartup = m_launchCheckbox->isChecked();
        settings.startMinimized = m_startMinimizedCheckbox->isChecked();
        settings.minimizeToTray = m_minimizeToTrayCheckbox->isChecked();
        settings.closeToTray = m_closeToTrayCheckbox->isChecked();
        settings.language = m_languageCombo->currentData().toString();
        settings.theme = m_themeCombo->currentData().toString();
        settings.trayIcon = m_trayIconCombo->currentData().toString();
        settings.defaultProfileId = m_defaultProfileCombo->currentData().toLongLong();
        settings.pauseUntil = m_pauseUntilEdit->dateTime();

        domain::QuietHoursPolicy policy = m_policies.isEmpty() ? domain::QuietHoursPolicy {} : m_policies.constFirst();
        policy.startTime = m_quietStartEdit->time();
        policy.endTime = m_quietEndEdit->time();
        policy.behaviorSoft = m_softBehaviorCombo->currentData().value<domain::QuietBehavior>();
        policy.behaviorPersistent = m_persistentBehaviorCombo->currentData().value<domain::QuietBehavior>();
        policy.behaviorBreak = m_breakBehaviorCombo->currentData().value<domain::QuietBehavior>();
        emit saveRequested(settings, policy); });

        retranslateUi();
    }

    void SettingsPage::setState(const QList<domain::ReminderProfile> &profiles,
                                const QList<domain::QuietHoursPolicy> &policies,
                                const domain::AppSettings &settings,
                                bool autostartEnabled,
                                bool hasTray)
    {
        m_profiles = profiles;
        m_policies = policies;
        m_settings = settings;
        m_autostartEnabled = autostartEnabled;
        m_hasTray = hasTray;

        repopulateCombos();
        retranslateUi();

        m_launchCheckbox->setChecked(autostartEnabled || settings.launchOnStartup);
        m_startMinimizedCheckbox->setChecked(settings.startMinimized);
        m_minimizeToTrayCheckbox->setChecked(settings.minimizeToTray);
        m_closeToTrayCheckbox->setChecked(settings.closeToTray);
        m_closeToTrayCheckbox->setEnabled(hasTray);
        m_minimizeToTrayCheckbox->setEnabled(hasTray);
        m_pauseUntilEdit->setDateTime(settings.pauseUntil.isValid() ? settings.pauseUntil : QDateTime::currentDateTime());

        if (!policies.isEmpty())
        {
            const auto &policy = policies.constFirst();
            m_quietStartEdit->setTime(policy.startTime);
            m_quietEndEdit->setTime(policy.endTime);
            m_softBehaviorCombo->setCurrentIndex(m_softBehaviorCombo->findData(QVariant::fromValue(policy.behaviorSoft)));
            m_persistentBehaviorCombo->setCurrentIndex(m_persistentBehaviorCombo->findData(QVariant::fromValue(policy.behaviorPersistent)));
            m_breakBehaviorCombo->setCurrentIndex(m_breakBehaviorCombo->findData(QVariant::fromValue(policy.behaviorBreak)));
        }

        m_platformNoteLabel->setText(hasTray
                                         ? tr("Tray integration is available on this desktop environment.")
                                         : tr("Tray integration is not available right now, so the main window stays as the primary control surface."));
    }

    void SettingsPage::changeEvent(QEvent *event)
    {
        if (event->type() == QEvent::LanguageChange)
        {
            retranslateUi();
        }
        QWidget::changeEvent(event);
    }

    void SettingsPage::retranslateUi()
    {
        const auto setLabelText = [this](QWidget *field, const QString &text)
        {
            if (auto *label = qobject_cast<QLabel *>(m_form->labelForField(field)))
            {
                label->setText(text);
            }
        };

        m_titleLabel->setText(tr("Settings"));
        m_subtitleLabel->setText(tr("Control startup behavior, quiet hours, theme and interface language."));
        m_launchCheckbox->setText(tr("Launch on startup"));
        m_startMinimizedCheckbox->setText(tr("Start minimized"));
        m_minimizeToTrayCheckbox->setText(tr("Minimize to tray"));
        m_closeToTrayCheckbox->setText(tr("Close to tray"));
        m_saveButton->setText(tr("Save"));
        setLabelText(m_languageCombo, tr("Language"));
        setLabelText(m_themeCombo, tr("Theme"));
        setLabelText(m_trayIconCombo, tr("Tray icon"));
        setLabelText(m_defaultProfileCombo, tr("Default profile"));
        setLabelText(m_pauseUntilEdit, tr("Pause reminders until"));
        setLabelText(m_quietStartEdit, tr("Quiet hours start"));
        setLabelText(m_quietEndEdit, tr("Quiet hours end"));
        setLabelText(m_softBehaviorCombo, tr("Soft reminders"));
        setLabelText(m_persistentBehaviorCombo, tr("Persistent reminders"));
        setLabelText(m_breakBehaviorCombo, tr("Strict breaks"));
        m_platformNoteLabel->setText(m_hasTray
                                         ? tr("Tray integration is available on this desktop environment.")
                                         : tr("Tray integration is not available right now, so the main window stays as the primary control surface."));

        if (m_languageCombo->count() >= 3)
        {
            m_languageCombo->setItemText(0, tr("System default"));
            m_languageCombo->setItemText(1, tr("English"));
            m_languageCombo->setItemText(2, tr("Russian"));
        }

        if (m_themeCombo->count() >= 3)
        {
            m_themeCombo->setItemText(0, tr("System"));
            m_themeCombo->setItemText(1, tr("Light"));
            m_themeCombo->setItemText(2, tr("Dark"));
        }

        if (m_trayIconCombo->count() >= 2)
        {
            m_trayIconCombo->setItemText(0, tr("Icon variant 1"));
            m_trayIconCombo->setItemText(1, tr("Icon variant 2"));
        }

        const QList<QComboBox *> combos = {m_softBehaviorCombo, m_persistentBehaviorCombo, m_breakBehaviorCombo};
        for (auto *combo : combos)
        {
            for (int index = 0; index < combo->count(); ++index)
            {
                combo->setItemText(index, displayQuietBehavior(combo->itemData(index).value<domain::QuietBehavior>(), this));
            }
        }
    }

    void SettingsPage::repopulateCombos()
    {
        if (m_languageCombo->count() == 0)
        {
            m_languageCombo->addItem(QString(), QStringLiteral("system"));
            m_languageCombo->addItem(QString(), QStringLiteral("en"));
            m_languageCombo->addItem(QString(), QStringLiteral("ru"));
        }
        if (m_themeCombo->count() == 0)
        {
            m_themeCombo->addItem(QString(), QStringLiteral("system"));
            m_themeCombo->addItem(QString(), QStringLiteral("light"));
            m_themeCombo->addItem(QString(), QStringLiteral("dark"));
        }

        const QSignalBlocker languageBlocker(m_languageCombo);
        const QSignalBlocker themeBlocker(m_themeCombo);
        const QSignalBlocker iconBlocker(m_trayIconCombo);
        const QSignalBlocker profileBlocker(m_defaultProfileCombo);

        const int languageIndex = m_languageCombo->findData(m_settings.language);
        m_languageCombo->setCurrentIndex(languageIndex >= 0 ? languageIndex : 0);

        const int themeIndex = m_themeCombo->findData(m_settings.theme);
        m_themeCombo->setCurrentIndex(themeIndex >= 0 ? themeIndex : 0);

        const int iconIndex = m_trayIconCombo->findData(m_settings.trayIcon);
        m_trayIconCombo->setCurrentIndex(iconIndex >= 0 ? iconIndex : 0);

        m_defaultProfileCombo->clear();
        for (const auto &profile : m_profiles)
        {
            m_defaultProfileCombo->addItem(profile.name, profile.id);
        }
        const int profileIndex = m_defaultProfileCombo->findData(m_settings.defaultProfileId);
        if (profileIndex >= 0)
        {
            m_defaultProfileCombo->setCurrentIndex(profileIndex);
        }
    }

} // namespace deadliner::ui
