#pragma once

#include <QDialog>

class QLineEdit;
class QLabel;

namespace timemaster {

class DeepSeekClient;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(DeepSeekClient *ai, const QString &dbPath, QWidget *parent = nullptr);

private slots:
    void applyTheme();
    void onSave();
    void onToggleVisibility();

private:
    DeepSeekClient *m_ai;
    QLineEdit *m_apiKeyEdit;
    QLabel *m_statusLabel;
    bool m_keyVisible = false;
};

} // namespace timemaster
