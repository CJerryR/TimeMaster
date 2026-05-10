#include "SettingsDialog.h"
#include "Theme.h"
#include "../core/DeepSeekClient.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QGroupBox>

namespace timeplan {

SettingsDialog::SettingsDialog(DeepSeekClient *ai, const QString &dbPath, QWidget *parent)
    : QDialog(parent), m_ai(ai)
{
    setWindowTitle("设置");
    setMinimumWidth(520);
    setObjectName("SettingsDialog");

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(16);

    auto *title = new QLabel("应用设置");
    title->setObjectName("DialogTitle");
    root->addWidget(title);

    // ---- DeepSeek API ----
    auto *apiBox = new QGroupBox("DeepSeek API");
    apiBox->setObjectName("SettingsGroup");
    auto *apiLayout = new QVBoxLayout(apiBox);
    apiLayout->setSpacing(8);

    auto *hint = new QLabel("输入 DeepSeek API Key 后即可启用 AI 解析与对话功能。\n获取地址：https://platform.deepseek.com/api_keys");
    hint->setObjectName("FieldHint");
    hint->setWordWrap(true);
    apiLayout->addWidget(hint);

    auto *keyRow = new QHBoxLayout;
    keyRow->setSpacing(8);
    m_apiKeyEdit = new QLineEdit;
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    m_apiKeyEdit->setPlaceholderText("sk-xxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    m_apiKeyEdit->setText(m_ai->apiKey());

    auto *eyeBtn = new QPushButton("👁");
    eyeBtn->setObjectName("InlineBtn");
    eyeBtn->setFixedSize(36, 36);
    eyeBtn->setToolTip("显示 / 隐藏");
    connect(eyeBtn, &QPushButton::clicked, this, &SettingsDialog::onToggleVisibility);

    keyRow->addWidget(m_apiKeyEdit);
    keyRow->addWidget(eyeBtn);
    apiLayout->addLayout(keyRow);

    m_statusLabel = new QLabel(m_ai->hasApiKey() ? "状态：已配置 ✓" : "状态：未配置");
    m_statusLabel->setObjectName("FieldHint");
    apiLayout->addWidget(m_statusLabel);

    root->addWidget(apiBox);

    // ---- 主题 ----
    auto *themeBox = new QGroupBox("外观");
    themeBox->setObjectName("SettingsGroup");
    auto *themeLayout = new QHBoxLayout(themeBox);

    auto *themeLabel = new QLabel("主题模式");
    auto *lightBtn = new QPushButton("浅色");
    auto *darkBtn = new QPushButton("深色");
    lightBtn->setObjectName("PillBtn");
    darkBtn->setObjectName("PillBtn");
    lightBtn->setCheckable(true);
    darkBtn->setCheckable(true);
    lightBtn->setChecked(Theme::instance().mode() == Theme::Light);
    darkBtn->setChecked(Theme::instance().mode() == Theme::Dark);

    connect(lightBtn, &QPushButton::clicked, [lightBtn, darkBtn]{
        Theme::instance().setMode(Theme::Light);
        lightBtn->setChecked(true);
        darkBtn->setChecked(false);
    });
    connect(darkBtn, &QPushButton::clicked, [lightBtn, darkBtn]{
        Theme::instance().setMode(Theme::Dark);
        lightBtn->setChecked(false);
        darkBtn->setChecked(true);
    });

    themeLayout->addWidget(themeLabel);
    themeLayout->addStretch();
    themeLayout->addWidget(lightBtn);
    themeLayout->addWidget(darkBtn);
    root->addWidget(themeBox);

    // ---- 数据库 ----
    auto *dbBox = new QGroupBox("数据存储");
    dbBox->setObjectName("SettingsGroup");
    auto *dbLayout = new QVBoxLayout(dbBox);
    auto *dbInfo = new QLabel(QString("数据库文件：\n%1").arg(dbPath));
    dbInfo->setObjectName("FieldHint");
    dbInfo->setWordWrap(true);
    dbInfo->setTextInteractionFlags(Qt::TextSelectableByMouse);
    dbLayout->addWidget(dbInfo);
    root->addWidget(dbBox);

    root->addStretch();

    // ---- 按钮 ----
    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto *cancelBtn = new QPushButton("取消");
    auto *saveBtn = new QPushButton("保存");
    cancelBtn->setObjectName("SecondaryBtn");
    saveBtn->setObjectName("PrimaryBtn");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::onSave);
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(saveBtn);
    root->addLayout(btnRow);

    connect(&Theme::instance(), &Theme::changed, this, &SettingsDialog::applyTheme);
    applyTheme();
}

void SettingsDialog::onToggleVisibility() {
    m_keyVisible = !m_keyVisible;
    m_apiKeyEdit->setEchoMode(m_keyVisible ? QLineEdit::Normal : QLineEdit::Password);
}

void SettingsDialog::onSave() {
    QString key = m_apiKeyEdit->text().trimmed();
    QSettings s;
    s.setValue("deepseek_api_key", key);
    m_ai->setApiKey(key);
    accept();
}

void SettingsDialog::onTestConnection() {
    // 预留：可调用 sendChat 测试，本版本由设置后实际使用反馈替代
}

void SettingsDialog::applyTheme() {
    setStyleSheet(Theme::instance().globalStylesheet() + QString(R"(
        QGroupBox#SettingsGroup {
            border: 1px solid %1;
            border-radius: 10px;
            margin-top: 14px;
            padding: 14px 14px 12px 14px;
            font-weight: 500;
            color: %2;
        }
        QGroupBox#SettingsGroup::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 6px;
        }
        QLabel#DialogTitle {
            font-size: 18px;
            font-weight: 600;
            color: %2;
        }
        QLabel#FieldHint {
            color: %3;
            font-size: 12px;
        }
        QPushButton#PillBtn {
            border: 1px solid %1;
            border-radius: 14px;
            padding: 4px 14px;
            background-color: transparent;
            color: %3;
        }
        QPushButton#PillBtn:checked {
            background-color: %4;
            color: white;
            border-color: %4;
        }
        QPushButton#PrimaryBtn {
            background-color: %4;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 18px;
            font-weight: 500;
        }
        QPushButton#PrimaryBtn:hover { background-color: %5; }
        QPushButton#SecondaryBtn {
            background-color: transparent;
            color: %2;
            border: 1px solid %1;
            border-radius: 8px;
            padding: 8px 18px;
        }
        QPushButton#SecondaryBtn:hover { background-color: %6; }
        QPushButton#InlineBtn {
            background-color: %6;
            border: 1px solid %1;
            border-radius: 8px;
            color: %2;
        }
        QPushButton#InlineBtn:hover { background-color: %1; }
    )")
        .arg(Theme::instance().stroke().name())
        .arg(Theme::instance().textPrimary().name())
        .arg(Theme::instance().textSecondary().name())
        .arg(Theme::instance().brand().name())
        .arg(Theme::instance().brand().darker(110).name())
        .arg(Theme::instance().bgHover().name()));
}

} // namespace timeplan
