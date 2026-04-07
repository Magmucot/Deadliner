#include "ui/onboarding/onboarding_dialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace deadliner::ui
{

    namespace
    {

        QLabel *createBodyLabel(QWidget *parent)
        {
            auto *bodyLabel = new QLabel(parent);
            bodyLabel->setWordWrap(true);
            return bodyLabel;
        }

        QWidget *createPage(QLabel **titleLabel, QLabel **bodyLabel, QWidget *extra, QWidget *parent)
        {
            auto *page = new QWidget(parent);
            auto *layout = new QVBoxLayout(page);
            layout->setSpacing(12);
            *titleLabel = new QLabel(page);
            (*titleLabel)->setStyleSheet(QStringLiteral("font-size: 22px; font-weight: 600;"));
            (*titleLabel)->setWordWrap(true);
            *bodyLabel = createBodyLabel(page);
            layout->addWidget(*titleLabel);
            layout->addWidget(*bodyLabel);
            if (extra != nullptr)
            {
                layout->addWidget(extra);
            }
            layout->addStretch();
            return page;
        }

    } // namespace

    OnboardingDialog::OnboardingDialog(QWidget *parent)
        : QDialog(parent)
    {
        setWindowTitle(tr("Welcome to Deadliner"));
        setMinimumSize(560, 360);

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(24, 24, 24, 24);
        layout->setSpacing(16);

        m_titleLabel = new QLabel(this);
        m_titleLabel->setStyleSheet(QStringLiteral("font-size: 26px; font-weight: 700;"));

        m_pages = new QStackedWidget(this);

        auto *languageControls = new QWidget(this);
        auto *languageLayout = new QVBoxLayout(languageControls);
        m_languageCombo = new QComboBox(languageControls);
        m_languageCombo->addItem(QString(), QStringLiteral("en"));
        m_languageCombo->addItem(QString(), QStringLiteral("ru"));
        const int defaultLanguageIndex = QLocale::system().language() == QLocale::Russian ? 1 : 0;
        m_languageCombo->setCurrentIndex(defaultLanguageIndex);
        languageLayout->addWidget(m_languageCombo, 0, Qt::AlignLeft);
        m_pages->addWidget(createPage(&m_page0Title, &m_page0Body, languageControls, this));
        m_pages->addWidget(createPage(&m_page1Title, &m_page1Body, nullptr, this));
        m_pages->addWidget(createPage(&m_page2Title, &m_page2Body, nullptr, this));

        auto *presetWidget = new QWidget(this);
        auto *presetLayout = new QVBoxLayout(presetWidget);
        m_enableRuleCheck = new QCheckBox(presetWidget);
        m_enableRuleCheck->setChecked(true);
        presetLayout->addWidget(m_enableRuleCheck);
        m_pages->addWidget(createPage(&m_page3Title, &m_page3Body, presetWidget, this));

        auto *buttonsLayout = new QHBoxLayout();
        m_backButton = new QPushButton(this);
        m_nextButton = new QPushButton(this);
        m_finishButton = new QPushButton(this);
        buttonsLayout->addWidget(m_backButton);
        buttonsLayout->addStretch();
        buttonsLayout->addWidget(m_nextButton);
        buttonsLayout->addWidget(m_finishButton);

        layout->addWidget(m_titleLabel);
        layout->addWidget(m_pages, 1);
        layout->addLayout(buttonsLayout);

        connect(m_backButton, &QPushButton::clicked, this, [this]()
                {
        m_pages->setCurrentIndex(qMax(0, m_pages->currentIndex() - 1));
        updateButtons(); });
        connect(m_nextButton, &QPushButton::clicked, this, [this]()
                {
        m_pages->setCurrentIndex(qMin(m_pages->count() - 1, m_pages->currentIndex() + 1));
        updateButtons(); });
        connect(m_finishButton, &QPushButton::clicked, this, &QDialog::accept);
        connect(m_languageCombo, &QComboBox::currentIndexChanged, this, [this]()
                { emit languagePreviewRequested(selectedLanguage()); });

        retranslateUi();
        updateButtons();
    }

    QString OnboardingDialog::selectedLanguage() const
    {
        return m_languageCombo->currentData().toString();
    }

    bool OnboardingDialog::enableRecommendedBreakRule() const
    {
        return m_enableRuleCheck->isChecked();
    }

    void OnboardingDialog::changeEvent(QEvent *event)
    {
        if (event->type() == QEvent::LanguageChange)
        {
            retranslateUi();
        }
        QDialog::changeEvent(event);
    }

    void OnboardingDialog::retranslateUi()
    {
        setWindowTitle(tr("Welcome to Deadliner"));
        m_titleLabel->setText(tr("Set up your reminder workspace"));
        m_languageCombo->setItemText(0, tr("English"));
        m_languageCombo->setItemText(1, tr("Russian"));
        m_enableRuleCheck->setText(tr("Enable the recommended 20-20-20 inspired eye-rest rule (20 min work / 1 min break)."));
        m_backButton->setText(tr("Back"));
        m_nextButton->setText(tr("Next"));
        m_finishButton->setText(tr("Finish"));

        m_page0Title->setText(tr("Choose interface language"));
        m_page0Body->setText(tr("The application supports Russian and English out of the box, and you can switch later in Settings without restarting."));
        m_page1Title->setText(tr("Break reminders that respect your day"));
        m_page1Body->setText(tr("Deadliner starts gently, escalates only when needed, and keeps quiet hours configurable so reminders stay useful instead of annoying."));
        m_page2Title->setText(tr("Desktop-first workflow"));
        m_page2Body->setText(tr("Most daily actions are available from the system tray, while the main window keeps event, profile and statistics management organized."));
        m_page3Title->setText(tr("Recommended starter setup"));
        m_page3Body->setText(tr("You can start with a safe eye-rest preset now and fine-tune it later in Profiles."));
    }

    void OnboardingDialog::updateButtons()
    {
        const int index = m_pages->currentIndex();
        m_backButton->setEnabled(index > 0);
        m_nextButton->setVisible(index < m_pages->count() - 1);
        m_finishButton->setVisible(index == m_pages->count() - 1);
    }

} // namespace deadliner::ui
