#include "OnboardingDialog.h"
#include "Theme.h"
#include "../core/DeepSeekClient.h"
#include "../core/I18n.h"

#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>

namespace timemaster {

OnboardingDialog::OnboardingDialog(DeepSeekClient *ai, QWidget *parent)
    : QDialog(parent), m_ai(ai)
{
    setModal(true);
    setMinimumSize(620, 520);
    setObjectName("OnboardingDialog");
    buildUi();
    applyLanguage();
    applyTheme();
    connect(&Theme::instance(), &Theme::changed,        this, &OnboardingDialog::applyTheme);
    connect(&I18n::instance(),  &I18n::languageChanged, this, &OnboardingDialog::applyLanguage);
    updateButtons();
}

void OnboardingDialog::buildUi() {
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(32, 28, 32, 24);
    root->setSpacing(20);

    // Step dots
    auto *dotsRow = new QHBoxLayout;
    dotsRow->setSpacing(8);
    dotsRow->addStretch();
    for (int i = 0; i < 3; ++i) {
        auto *dot = new QLabel;
        dot->setFixedSize(28, 4);
        dot->setObjectName(i == 0 ? "StepDotActive" : "StepDotInactive");
        m_stepDots.append(dot);
        dotsRow->addWidget(dot);
    }
    dotsRow->addStretch();
    root->addLayout(dotsRow);

    // Stack of pages
    m_stack = new QStackedWidget;
    auto *page1 = new QWidget;
    auto *page2 = new QWidget;
    auto *page3 = new QWidget;
    buildStep1(page1);
    buildStep2(page2);
    buildStep3(page3);
    m_stack->addWidget(page1);
    m_stack->addWidget(page2);
    m_stack->addWidget(page3);
    root->addWidget(m_stack, 1);

    // Footer
    auto *footer = new QHBoxLayout;
    m_skipBtn = new QPushButton;
    m_skipBtn->setObjectName("OnboardSkipBtn");
    m_skipBtn->setCursor(Qt::PointingHandCursor);
    footer->addWidget(m_skipBtn);
    footer->addStretch();
    m_backBtn = new QPushButton;
    m_backBtn->setObjectName("OnboardBackBtn");
    m_backBtn->setMinimumSize(96, 36);
    m_backBtn->setCursor(Qt::PointingHandCursor);
    footer->addWidget(m_backBtn);
    m_nextBtn = new QPushButton;
    m_nextBtn->setObjectName("OnboardNextBtn");
    m_nextBtn->setMinimumSize(120, 36);
    m_nextBtn->setCursor(Qt::PointingHandCursor);
    m_nextBtn->setDefault(true);
    footer->addWidget(m_nextBtn);
    root->addLayout(footer);

    connect(m_skipBtn, &QPushButton::clicked, this, &OnboardingDialog::onSkip);
    connect(m_backBtn, &QPushButton::clicked, this, &OnboardingDialog::goBack);
    connect(m_nextBtn, &QPushButton::clicked, this, &OnboardingDialog::goNext);
    connect(m_stack,   &QStackedWidget::currentChanged, this, [this]{
        updateStepDots();
        updateButtons();
    });
}

void OnboardingDialog::buildStep1(QWidget *page) {
    auto *lay = new QVBoxLayout(page);
    lay->setContentsMargins(0, 20, 0, 20);
    lay->setSpacing(16);
    lay->setAlignment(Qt::AlignCenter);

    auto *logoRow = new QHBoxLayout;
    logoRow->setAlignment(Qt::AlignCenter);
    auto *logo = new QLabel("🕰");
    QFont lf; lf.setPointSize(40);
    logo->setFont(lf);
    logo->setAlignment(Qt::AlignCenter);
    logoRow->addWidget(logo);
    lay->addLayout(logoRow);

    m_step1Title = new QLabel;
    m_step1Title->setObjectName("OnboardTitle");
    m_step1Title->setAlignment(Qt::AlignCenter);
    QFont tf; tf.setPointSize(20); tf.setWeight(QFont::DemiBold);
    m_step1Title->setFont(tf);
    lay->addWidget(m_step1Title);

    m_step1Body = new QLabel;
    m_step1Body->setObjectName("OnboardBody");
    m_step1Body->setAlignment(Qt::AlignCenter);
    m_step1Body->setWordWrap(true);
    m_step1Body->setMaximumWidth(480);
    lay->addWidget(m_step1Body, 0, Qt::AlignCenter);
    lay->addStretch();
}

void OnboardingDialog::buildStep2(QWidget *page) {
    auto *lay = new QVBoxLayout(page);
    lay->setContentsMargins(0, 32, 0, 20);
    lay->setSpacing(18);

    m_step2Title = new QLabel;
    m_step2Title->setObjectName("OnboardTitle");
    m_step2Title->setAlignment(Qt::AlignCenter);
    QFont tf; tf.setPointSize(20); tf.setWeight(QFont::DemiBold);
    m_step2Title->setFont(tf);
    lay->addWidget(m_step2Title);

    m_step2Body = new QLabel;
    m_step2Body->setObjectName("OnboardBody");
    m_step2Body->setAlignment(Qt::AlignCenter);
    m_step2Body->setWordWrap(true);
    lay->addWidget(m_step2Body);

    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(16);
    btnRow->addStretch();

    m_enBtn = new QPushButton;
    m_enBtn->setObjectName("OnboardLangBtn");
    m_enBtn->setMinimumSize(160, 60);
    m_enBtn->setCursor(Qt::PointingHandCursor);
    m_enBtn->setCheckable(true);
    btnRow->addWidget(m_enBtn);

    m_zhBtn = new QPushButton;
    m_zhBtn->setObjectName("OnboardLangBtn");
    m_zhBtn->setMinimumSize(160, 60);
    m_zhBtn->setCursor(Qt::PointingHandCursor);
    m_zhBtn->setCheckable(true);
    btnRow->addWidget(m_zhBtn);

    btnRow->addStretch();
    lay->addLayout(btnRow);
    lay->addStretch();

    m_enBtn->setChecked(I18n::instance().isEnglish());
    m_zhBtn->setChecked(!I18n::instance().isEnglish());

    connect(m_enBtn, &QPushButton::clicked, this, [this]{
        I18n::instance().setLanguage(I18n::English);
        m_enBtn->setChecked(true);
        m_zhBtn->setChecked(false);
        applyTheme();
    });
    connect(m_zhBtn, &QPushButton::clicked, this, [this]{
        I18n::instance().setLanguage(I18n::Chinese);
        m_enBtn->setChecked(false);
        m_zhBtn->setChecked(true);
        applyTheme();
    });
}

void OnboardingDialog::buildStep3(QWidget *page) {
    auto *lay = new QVBoxLayout(page);
    lay->setContentsMargins(0, 28, 0, 20);
    lay->setSpacing(16);

    m_step3Title = new QLabel;
    m_step3Title->setObjectName("OnboardTitle");
    m_step3Title->setAlignment(Qt::AlignCenter);
    QFont tf; tf.setPointSize(20); tf.setWeight(QFont::DemiBold);
    m_step3Title->setFont(tf);
    lay->addWidget(m_step3Title);

    m_step3Body = new QLabel;
    m_step3Body->setObjectName("OnboardBody");
    m_step3Body->setAlignment(Qt::AlignCenter);
    m_step3Body->setWordWrap(true);
    m_step3Body->setMaximumWidth(520);
    lay->addWidget(m_step3Body, 0, Qt::AlignCenter);

    auto *editRow = new QHBoxLayout;
    editRow->setSpacing(8);
    editRow->setContentsMargins(40, 8, 40, 8);
    m_apiKeyEdit = new QLineEdit;
    m_apiKeyEdit->setObjectName("OnboardApiInput");
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    m_apiKeyEdit->setMinimumHeight(40);
    if (m_ai && m_ai->hasApiKey()) m_apiKeyEdit->setText(m_ai->apiKey());
    editRow->addWidget(m_apiKeyEdit);
    lay->addLayout(editRow);

    lay->addStretch();
}

void OnboardingDialog::applyLanguage() {
    setWindowTitle(I18n::t("onboarding.title"));
    if (m_step1Title) m_step1Title->setText(I18n::t("onboarding.step1.title"));
    if (m_step1Body)  m_step1Body->setText(I18n::t("onboarding.step1.body"));
    if (m_step2Title) m_step2Title->setText(I18n::t("onboarding.step2.title"));
    if (m_step2Body)  m_step2Body->setText(I18n::t("onboarding.step2.body"));
    if (m_step3Title) m_step3Title->setText(I18n::t("onboarding.step3.title"));
    if (m_step3Body)  m_step3Body->setText(I18n::t("onboarding.step3.body"));
    if (m_apiKeyEdit) m_apiKeyEdit->setPlaceholderText(I18n::t("onboarding.step3.placeholder"));
    if (m_enBtn)      m_enBtn->setText("English");
    if (m_zhBtn)      m_zhBtn->setText("中文");
    if (m_skipBtn)    m_skipBtn->setText(I18n::t("onboarding.skip"));
    if (m_backBtn)    m_backBtn->setText(I18n::t("onboarding.back"));
    updateButtons();
}

void OnboardingDialog::updateButtons() {
    if (!m_stack || !m_nextBtn || !m_backBtn) return;
    int idx = m_stack->currentIndex();
    int last = m_stack->count() - 1;
    m_backBtn->setVisible(idx > 0);
    m_nextBtn->setText(idx == last ? I18n::t("onboarding.done")
                                   : I18n::t("onboarding.next"));
}

void OnboardingDialog::updateStepDots() {
    if (m_stepDots.isEmpty() || !m_stack) return;
    int active = m_stack->currentIndex();
    for (int i = 0; i < m_stepDots.size(); ++i) {
        m_stepDots[i]->setObjectName(i == active ? "StepDotActive" : "StepDotInactive");
    }
    // Re-apply to refresh styles after objectName changes
    applyTheme();
}

void OnboardingDialog::goNext() {
    if (!m_stack) return;
    int idx = m_stack->currentIndex();
    int last = m_stack->count() - 1;
    if (idx == last) {
        onDone();
        return;
    }
    m_stack->setCurrentIndex(idx + 1);
}

void OnboardingDialog::goBack() {
    if (!m_stack) return;
    int idx = m_stack->currentIndex();
    if (idx > 0) m_stack->setCurrentIndex(idx - 1);
}

void OnboardingDialog::onSkip() {
    QSettings s;
    s.setValue("onboarded", true);
    reject();
}

void OnboardingDialog::onDone() {
    QString key = m_apiKeyEdit ? m_apiKeyEdit->text().trimmed() : QString();
    if (!key.isEmpty() && m_ai) {
        QSettings().setValue("deepseek_api_key", key);
        m_ai->setApiKey(key);
    }
    QSettings().setValue("onboarded", true);
    accept();
}

void OnboardingDialog::applyTheme() {
    auto &t = Theme::instance();
    setStyleSheet(t.globalStylesheet() + QString(R"(
        QDialog#OnboardingDialog {
            background-color: %1;
        }
        QLabel#OnboardTitle {
            color: %2;
            background: transparent;
            letter-spacing: -0.3px;
        }
        QLabel#OnboardBody {
            color: %3;
            background: transparent;
            font-size: 14px;
            line-height: 1.7;
        }
        QLabel#StepDotActive {
            background-color: %4;
            border-radius: 2px;
        }
        QLabel#StepDotInactive {
            background-color: %5;
            border-radius: 2px;
        }
        QPushButton#OnboardLangBtn {
            background-color: %6;
            color: %2;
            border: 1px solid %5;
            border-radius: 12px;
            font-size: 16px;
            font-weight: 600;
        }
        QPushButton#OnboardLangBtn:hover {
            background-color: %7;
        }
        QPushButton#OnboardLangBtn:checked {
            background-color: %4;
            color: white;
            border-color: %4;
        }
        QLineEdit#OnboardApiInput {
            background-color: %6;
            color: %2;
            border: 1px solid %5;
            border-radius: 8px;
            padding: 0 12px;
            font-size: 14px;
        }
        QLineEdit#OnboardApiInput:focus { border-color: %4; }
        QPushButton#OnboardNextBtn {
            background-color: %4;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 22px;
            font-weight: 600;
        }
        QPushButton#OnboardNextBtn:hover { background-color: %8; }
        QPushButton#OnboardBackBtn {
            background-color: transparent;
            color: %3;
            border: 1px solid %5;
            border-radius: 8px;
            padding: 8px 18px;
        }
        QPushButton#OnboardBackBtn:hover {
            background-color: %7;
            color: %2;
        }
        QPushButton#OnboardSkipBtn {
            background-color: transparent;
            color: %3;
            border: none;
            padding: 6px 8px;
            font-size: 13px;
        }
        QPushButton#OnboardSkipBtn:hover { color: %2; }
    )")
    /*1*/.arg(t.bgContainer().name())
    /*2*/.arg(t.textPrimary().name())
    /*3*/.arg(t.textSecondary().name())
    /*4*/.arg(t.brand().name())
    /*5*/.arg(t.strokeRgba())
    /*6*/.arg(t.cardBgRgba())
    /*7*/.arg(t.cardBgHoverRgba())
    /*8*/.arg(t.mode() == Theme::Light ? "#A85638" : "#D97757"));
}

} // namespace timemaster
