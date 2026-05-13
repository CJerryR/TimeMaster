//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include <QDialog>

class QLineEdit;
class QLabel;
class QPushButton;
class QSpinBox;
class QCheckBox;
class QWidget;

namespace timemaster {

class DeepSeekClient;

// 设置对话框：API Key、AI 日历上下文、AI 操作权限、外观、语言、周起始日、存储路径、版本信息
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    // 构造函数
    explicit SettingsDialog(DeepSeekClient *ai, const QString &dbPath, QWidget *parent = nullptr);

protected:
    // 窗口尺寸变化：重新定位密码可见按钮
    void resizeEvent(QResizeEvent *e) override;
    // 窗口显示事件：重新定位密码可见按钮
    void showEvent(QShowEvent *e) override;

private slots:
    // 应用主题样式：刷新全局 QSS
    void applyTheme();
    // 保存设置：持久化 API Key、AI 上下文、周起始日、权限等配置
    void onSave();
    // 切换 API Key 可见性：明文/密码模式互切
    void onToggleVisibility();

private:
    // 定位密码可见按钮到输入框右侧
    void repositionEyeBtn();

    DeepSeekClient *m_ai;
    QLineEdit *m_apiKeyEdit;
    QLabel *m_statusLabel;
    QPushButton *m_eyeBtn = nullptr;
    QWidget *m_keyHost = nullptr;
    bool m_keyVisible = false;

    // V4.2 #10 — AI calendar context configuration
    QSpinBox *m_pastDaysSpin   = nullptr;
    QSpinBox *m_futureDaysSpin = nullptr;
    QCheckBox *m_aiSeesCalendarChk = nullptr;

    // V4.3 #8 — 周起始日切换（周一 vs 周日）
    QPushButton *m_weekStartMonBtn = nullptr;
    QPushButton *m_weekStartSunBtn = nullptr;

    // V4.3 #7 — AI 操作日历的"总是允许"开关：增 / 删 / 改
    QCheckBox *m_autoApproveAddChk    = nullptr;
    QCheckBox *m_autoApproveDeleteChk = nullptr;
    QCheckBox *m_autoApproveUpdateChk = nullptr;
};

} // namespace timemaster
