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

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(DeepSeekClient *ai, const QString &dbPath, QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *e) override;
    void showEvent(QShowEvent *e) override;

private slots:
    void applyTheme();
    void onSave();
    void onToggleVisibility();

private:
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
