#pragma once

#include "domain/models.h"

#include <QEvent>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QDateTimeEdit;
class QFormLayout;
class QLabel;
class QPushButton;
class QRadioButton;
class QTimeEdit;

namespace deadliner::ui
{

    class SettingsPage : public QWidget
    {
        Q_OBJECT

    public:
        explicit SettingsPage(QWidget *parent = nullptr);

        void setState(const QList<domain::ReminderProfile> &profiles,
                      const QList<domain::QuietHoursPolicy> &policies,
                      const domain::AppSettings &settings,
                      bool autostartEnabled,
                      bool hasTray);

    signals:
        void saveRequested(domain::AppSettings settings, domain::QuietHoursPolicy policy);

    protected:
        void changeEvent(QEvent *event) override;

    private:
        void retranslateUi();
        void repopulateCombos();

        QList<domain::ReminderProfile> m_profiles;
        QList<domain::QuietHoursPolicy> m_policies;
        domain::AppSettings m_settings;
        bool m_autostartEnabled = false;
        bool m_hasTray = false;
        QFormLayout *m_form = nullptr;

        QLabel *m_titleLabel = nullptr;
        QLabel *m_subtitleLabel = nullptr;
        QLabel *m_platformNoteLabel = nullptr;
        QLabel *m_trayIconLabel = nullptr;
        QCheckBox *m_launchCheckbox = nullptr;
        QCheckBox *m_startMinimizedCheckbox = nullptr;
        QCheckBox *m_minimizeToTrayCheckbox = nullptr;
        QCheckBox *m_closeToTrayCheckbox = nullptr;
        QComboBox *m_languageCombo = nullptr;
        QComboBox *m_themeCombo = nullptr;
        QRadioButton *m_iconVariant1Radio = nullptr;
        QRadioButton *m_iconVariant2Radio = nullptr;
        QComboBox *m_defaultProfileCombo = nullptr;
        QDateTimeEdit *m_pauseUntilEdit = nullptr;
        QTimeEdit *m_quietStartEdit = nullptr;
        QTimeEdit *m_quietEndEdit = nullptr;
        QComboBox *m_softBehaviorCombo = nullptr;
        QComboBox *m_persistentBehaviorCombo = nullptr;
        QComboBox *m_breakBehaviorCombo = nullptr;
        QPushButton *m_saveButton = nullptr;

        QLabel *m_aboutTitleLabel = nullptr;
        QLabel *m_aboutVersionLabel = nullptr;
        QLabel *m_aboutAuthorLabel = nullptr;
        QLabel *m_aboutGithubLabel = nullptr;
    };

} // namespace deadliner::ui
