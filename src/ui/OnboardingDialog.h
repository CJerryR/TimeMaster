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
// 首次引导弹窗（3 步）：功能介绍 → 语言选择 → API Key 输入，可跳过
class OnboardingDialog : public QDialog {
    Q_OBJECT
public:
    // 构造函数
    explicit OnboardingDialog(DeepSeekClient *ai, QWidget *parent = nullptr);

private slots:
    // 应用主题样式：刷新 QSS
    void applyTheme();
    // 更新界面文字到当前语言
    void applyLanguage();
    // 下一步：进入下一引导页或触发完成
    void goNext();
    // 上一步：返回上一引导页
    void goBack();
    // 跳过引导：标记已引导并关闭弹窗
    void onSkip();
    // 完成引导：保存 API Key、标记已引导
    void onDone();
    // 更新按钮状态：根据当前步骤显示/隐藏上一步、切换文字
    void updateButtons();

private:
    // 构建 UI 布局：步骤指示点、堆叠页面、底部按钮行
    void buildUi();
    // 构建步骤 1：功能介绍页（标题 + 说明文字）
    void buildStep1(QWidget *page);
    // 构建步骤 2：语言选择页（中/英文切换按钮）
    void buildStep2(QWidget *page);
    // 构建步骤 3：API Key 输入页（密码输入框）
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

    // 更新步骤指示点的激活状态
    void updateStepDots();
};

} // namespace timemaster
