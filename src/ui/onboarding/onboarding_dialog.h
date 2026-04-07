#pragma once

#include <QEvent>
#include <QDialog>

class QCheckBox;
class QComboBox;
class QLabel;
class QStackedWidget;
class QPushButton;

namespace deadliner::ui
{

    class OnboardingDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit OnboardingDialog(QWidget *parent = nullptr);

        QString selectedLanguage() const;
        bool enableRecommendedBreakRule() const;

    signals:
        void languagePreviewRequested(const QString &languageCode);

    protected:
        void changeEvent(QEvent *event) override;

    private:
        void retranslateUi();
        void updateButtons();

        QStackedWidget *m_pages = nullptr;
        QComboBox *m_languageCombo = nullptr;
        QCheckBox *m_enableRuleCheck = nullptr;
        QPushButton *m_backButton = nullptr;
        QPushButton *m_nextButton = nullptr;
        QPushButton *m_finishButton = nullptr;
        QLabel *m_titleLabel = nullptr;
        QLabel *m_page0Title = nullptr;
        QLabel *m_page0Body = nullptr;
        QLabel *m_page1Title = nullptr;
        QLabel *m_page1Body = nullptr;
        QLabel *m_page2Title = nullptr;
        QLabel *m_page2Body = nullptr;
        QLabel *m_page3Title = nullptr;
        QLabel *m_page3Body = nullptr;
    };

} // namespace deadliner::ui
