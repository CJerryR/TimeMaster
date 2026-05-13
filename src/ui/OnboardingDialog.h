//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include <QDialog>

class QStackedWidget;
class QPushButton;
class QLabel;
class QLineEdit;

namespace timemaster {

class DeepSeekClient;

/**
 * V4 § 5.4: first-run onboarding flow.
 *   · Step 1 — What can it do?
 *   · Step 2 — Pick a language
 *   · Step 3 — Optional API key
 *
 * Triggered from MainWindow when QSettings("onboarded") is false. Honours both
 * Skip (closes without saving) and Done (writes API key + sets onboarded=true).
 */
class OnboardingDialog : public QDialog {
    Q_OBJECT
public:
    explicit OnboardingDialog(DeepSeekClient *ai, QWidget *parent = nullptr);

private slots:
    void applyTheme();
    void applyLanguage();
    void goNext();
    void goBack();
    void onSkip();
    void onDone();
    void updateButtons();

private:
    void buildUi();
    void buildStep1(QWidget *page);
    void buildStep2(QWidget *page);
    void buildStep3(QWidget *page);

    DeepSeekClient *m_ai;
    QStackedWidget *m_stack       = nullptr;
    QPushButton    *m_backBtn     = nullptr;
    QPushButton    *m_nextBtn     = nullptr;
    QPushButton    *m_skipBtn     = nullptr;

    // Step 1
    QLabel *m_step1Title = nullptr;
    QLabel *m_step1Body  = nullptr;

    // Step 2
    QLabel *m_step2Title = nullptr;
    QLabel *m_step2Body  = nullptr;
    QPushButton *m_enBtn = nullptr;
    QPushButton *m_zhBtn = nullptr;

    // Step 3
    QLabel    *m_step3Title = nullptr;
    QLabel    *m_step3Body  = nullptr;
    QLineEdit *m_apiKeyEdit = nullptr;

    // Tracker pills
    QList<QLabel*> m_stepDots;

    void updateStepDots();
};

} // namespace timemaster
