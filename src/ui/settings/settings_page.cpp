#include "ui/settings/settings_page.h"

#include "ui/common/display_strings.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QEvent>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSizePolicy>
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

        void setGrowingField(QWidget *field)
        {
            field->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        }

    } // namespace

    SettingsPage::SettingsPage(QWidget *parent)
        : QWidget(parent)
    {
        auto *rootLayout = new QVBoxLayout(this);
        rootLayout->setContentsMargins(0, 0, 0, 0);
        rootLayout->setSpacing(0);

        auto *scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        rootLayout->addWidget(scrollArea);

        auto *content = new QWidget(scrollArea);
        scrollArea->setWidget(content);

        auto *layout = new QVBoxLayout(content);
        layout->setContentsMargins(24, 24, 24, 24);
        layout->setSpacing(16);

        m_titleLabel = new QLabel(content);
        m_titleLabel->setStyleSheet(QStringLiteral("font-size: 22px; font-weight: 600;"));
        m_subtitleLabel = new QLabel(content);
        m_subtitleLabel->setWordWrap(true);
        m_platformNoteLabel = new QLabel(content);
        m_platformNoteLabel->setWordWrap(true);

        m_form = new QFormLayout();
        m_form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        m_form->setRowWrapPolicy(QFormLayout::WrapLongRows);
        m_form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
        m_form->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
        m_form->setHorizontalSpacing(16);
        m_form->setVerticalSpacing(10);

        m_launchCheckbox = new QCheckBox(content);
        m_startMinimizedCheckbox = new QCheckBox(content);
        m_minimizeToTrayCheckbox = new QCheckBox(content);
        m_closeToTrayCheckbox = new QCheckBox(content);
        m_languageCombo = new QComboBox(content);
        m_themeCombo = new QComboBox(content);

        m_iconVariant1Radio = new QRadioButton(content);
        m_iconVariant1Radio->setIcon(QIcon(QStringLiteral(":/icons/icons/icon_variant1_256.png")));
        m_iconVariant1Radio->setIconSize(QSize(64, 64));
        m_iconVariant1Radio->setChecked(true);

        m_iconVariant2Radio = new QRadioButton(content);
        m_iconVariant2Radio->setIcon(QIcon(QStringLiteral(":/icons/icons/icon_variant2_256.png")));
        m_iconVariant2Radio->setIconSize(QSize(64, 64));

        auto *iconWidget = new QWidget(content);
        auto *iconLayout = new QHBoxLayout(iconWidget);
        iconLayout->setContentsMargins(0, 0, 0, 0);
        iconLayout->setSpacing(12);
        iconLayout->addWidget(m_iconVariant1Radio);
        iconLayout->addWidget(m_iconVariant2Radio);
        iconLayout->addStretch();

        m_trayIconLabel = createFormLabel(content);
        m_defaultProfileCombo = new QComboBox(content);
        m_pauseUntilEdit = new QDateTimeEdit(content);
        m_pauseUntilEdit->setCalendarPopup(true);
        m_quietStartEdit = new QTimeEdit(content);
        m_quietEndEdit = new QTimeEdit(content);
        m_softBehaviorCombo = createBehaviorCombo(content);
        m_persistentBehaviorCombo = createBehaviorCombo(content);
        m_breakBehaviorCombo = createBehaviorCombo(content);
        m_saveButton = new QPushButton(content);

        setGrowingField(m_languageCombo);
        setGrowingField(m_themeCombo);
        setGrowingField(m_defaultProfileCombo);
        setGrowingField(m_pauseUntilEdit);
        setGrowingField(m_quietStartEdit);
        setGrowingField(m_quietEndEdit);
        setGrowingField(m_softBehaviorCombo);
        setGrowingField(m_persistentBehaviorCombo);
        setGrowingField(m_breakBehaviorCombo);
        iconWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

        m_form->addRow(m_launchCheckbox);
        m_form->addRow(m_startMinimizedCheckbox);
        m_form->addRow(m_minimizeToTrayCheckbox);
        m_form->addRow(m_closeToTrayCheckbox);
        m_form->addRow(createFormLabel(content), m_languageCombo);
        m_form->addRow(createFormLabel(content), m_themeCombo);
        m_form->addRow(m_trayIconLabel, iconWidget);
        m_form->addRow(createFormLabel(content), m_defaultProfileCombo);
        m_form->addRow(createFormLabel(content), m_pauseUntilEdit);
        m_form->addRow(createFormLabel(content), m_quietStartEdit);
        m_form->addRow(createFormLabel(content), m_quietEndEdit);
        m_form->addRow(createFormLabel(content), m_softBehaviorCombo);
        m_form->addRow(createFormLabel(content), m_persistentBehaviorCombo);
        m_form->addRow(createFormLabel(content), m_breakBehaviorCombo);

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
                    settings.trayIcon = m_iconVariant2Radio->isChecked()
                                            ? QStringLiteral("variant2")
                                            : QStringLiteral("variant1");
                    settings.defaultProfileId = m_defaultProfileCombo->currentData().toLongLong();
                    settings.pauseUntil = m_pauseUntilEdit->dateTime();

                    domain::QuietHoursPolicy policy =
                        m_policies.isEmpty() ? domain::QuietHoursPolicy {} : m_policies.constFirst();
                    policy.startTime = m_quietStartEdit->time();
                    policy.endTime = m_quietEndEdit->time();
                    policy.behaviorSoft =
                        m_softBehaviorCombo->currentData().value<domain::QuietBehavior>();
                    policy.behaviorPersistent =
                        m_persistentBehaviorCombo->currentData().value<domain::QuietBehavior>();
                    policy.behaviorBreak =
                        m_breakBehaviorCombo->currentData().value<domain::QuietBehavior>();

                    emit saveRequested(settings, policy); });

        retranslateUi();
    }

} // namespace deadliner::ui