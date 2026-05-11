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

namespace timemaster {

SettingsDialog::SettingsDialog(DeepSeekClient *ai, const QString &dbPath, QWidget *parent)
    : QDialog(parent), m_ai(ai)
{
    setWindowTitle("设置");
    setMinimumWidth(560);
    setObjectName("SettingsDialog");

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(26, 22, 26, 22);
    root->setSpacing(18);

    auto *title = new QLabel("应用设置");
    title->setObjectName("DialogTitle");
    root->addWidget(title);

    // ---- DeepSeek API ----
    auto *apiBox = new QGroupBox("DeepSeek API");
    apiBox->setObjectName("SettingsGroup");
    auto *apiLayout = new QVBoxLayout(apiBox);
    apiLayout->setSpacing(10);

    auto *hint = new QLabel("配置 API Key 后即可使用 AI 解析与对话。\n申请地址：https://platform.deepseek.com/api_keys");
    hint->setObjectName("FieldHint");
    hint->setWordWrap(true);
    apiLayout->addWidget(hint);

    auto *keyRow = new QHBoxLayout;
    keyRow->setSpacing(8);
    m_apiKeyEdit = new QLineEdit;
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    m_apiKeyEdit->setPlaceholderText("sk-xxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    m_apiKeyEdit->setText(m_ai->apiKey());
    m_apiKeyEdit->setMinimumHeight(38);

    auto *eyeBtn = new QPushButton("👁");
    eyeBtn->setObjectName("InlineBtn");
    eyeBtn->setFixedSize(38, 38);
    eyeBtn->setToolTip("显示 / 隐藏");
    eyeBtn->setCursor(Qt::PointingHandCursor);
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
    themeLayout->setSpacing(10);

    auto *themeLabel = new QLabel("主题模式");
    auto *lightBtn = new QPushButton("🌞 浅色");
    auto *darkBtn = new QPushButton("🌙 深色");
    lightBtn->setObjectName("PillBtn");
    darkBtn->setObjectName("PillBtn");
    lightBtn->setCheckable(true);
    darkBtn->setCheckable(true);
    lightBtn->setMinimumHeight(34);
    darkBtn->setMinimumHeight(34);
    lightBtn->setCursor(Qt::PointingHandCursor);
    darkBtn->setCursor(Qt::PointingHandCursor);
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

    // ---- 数据库路径 ----
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

    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto *cancelBtn = new QPushButton("取消");
    auto *saveBtn = new QPushButton("保存");
    cancelBtn->setObjectName("SecondaryBtn");
    saveBtn->setObjectName("PrimaryBtn");
    cancelBtn->setMinimumSize(86, 36);
    saveBtn->setMinimumSize(86, 36);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setCursor(Qt::PointingHandCursor);
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

void SettingsDialog::applyTheme() {
    auto &t = Theme::instance();
    setStyleSheet(t.globalStylesheet() + QString(R"(
        QGroupBox#SettingsGroup {
            background-color: %5;
            border: 1px solid %1;
            border-radius: 12px;
            margin-top: 16px;
            padding: 16px 16px 14px 16px;
            font-weight: 600;
            color: %2;
        }
        QGroupBox#SettingsGroup::title {
            subcontrol-origin: margin;
            left: 14px;
            padding: 0 8px;
            background-color: %6;
        }
        QLabel#DialogTitle {
            font-size: 19px;
            font-weight: 700;
            color: %2;
        }
        QLabel#FieldHint {
            color: %3;
            font-size: 12px;
        }
        QPushButton#PillBtn {
            border: 1px solid %1;
            border-radius: 16px;
            padding: 4px 18px;
            background-color: transparent;
            color: %3;
            font-weight: 500;
        }
        QPushButton#PillBtn:hover {
            background-color: %7;
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
            border-radius: 9px;
            padding: 8px 18px;
            font-weight: 600;
        }
        QPushButton#PrimaryBtn:hover { background-color: #dc2626; }
        QPushButton#SecondaryBtn {
            background-color: transparent;
            color: %2;
            border: 1px solid %1;
            border-radius: 9px;
            padding: 8px 18px;
        }
        QPushButton#SecondaryBtn:hover { background-color: %7; }
        QPushButton#InlineBtn {
            background-color: %7;
            border: 1px solid %1;
            border-radius: 9px;
            color: %2;
        }
        QPushButton#InlineBtn:hover { background-color: %1; }
    )")
    .arg(t.strokeRgba())
    .arg(t.textPrimary().name())
    .arg(t.textSecondary().name())
    .arg(t.brand().name())
    .arg(t.cardBgRgba())
    .arg(t.bgContainer().name())
    .arg(t.cardBgHoverRgba()));
}

} // namespace timemaster
