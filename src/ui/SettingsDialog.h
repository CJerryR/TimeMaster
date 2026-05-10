#pragma once

#include <QDialog>

class QLineEdit;
class QLabel;

namespace timeplan {

class DeepSeekClient;

/**
 * 设置对话框
 * - DeepSeek API Key 输入（密码模式）
 * - 主题切换
 * - 显示数据库路径
 */
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(DeepSeekClient *ai, const QString &dbPath, QWidget *parent = nullptr);

private slots:
    void applyTheme();
    void onSave();
    void onToggleVisibility();
    void onTestConnection();

private:
    DeepSeekClient *m_ai;
    QLineEdit *m_apiKeyEdit;
    QLabel *m_statusLabel;
    bool m_keyVisible = false;
};

} // namespace timeplan
