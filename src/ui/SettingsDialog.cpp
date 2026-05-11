#include "SettingsDialog.h"
#include "Theme.h"
#include "../core/DeepSeekClient.h"
#include "../core/I18n.h"

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
    setWindowTitle(I18n::t("settings.title"));
    setMinimumWidth(580);
    setObjectName("SettingsDialog");

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(26, 22, 26, 22);
    root->setSpacing(18);

    auto *title = new QLabel(I18n::t("settings.app"));
    title->setObjectName("DialogTitle");
    root->addWidget(title);

    // ---- DeepSeek API ----
    auto *apiBox = new QGroupBox(I18n::t("settings.api.group"));
    apiBox->setObjectName("SettingsGroup");
    auto *apiLayout = new QVBoxLayout(apiBox);
    apiLayout->setSpacing(10);

    auto *hint = new QLabel(I18n::t("settings.api.hint"));
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
    eyeBtn->setToolTip(I18n::t("settings.api.toggle_tip"));
    eyeBtn->setCursor(Qt::PointingHandCursor);
    connect(eyeBtn, &QPushButton::clicked, this, &SettingsDialog::onToggleVisibility);

    keyRow->addWidget(m_apiKeyEdit);
    keyRow->addWidget(eyeBtn);
    apiLayout->addLayout(keyRow);

    m_statusLabel = new QLabel(m_ai->hasApiKey()
                               ? I18n::t("settings.api.configured")
                               : I18n::t("settings.api.unconfigured"));
    m_statusLabel->setObjectName("FieldHint");
    apiLayout->addWidget(m_statusLabel);

    root->addWidget(apiBox);

    // ---- Appearance + Language ----
    auto *appearanceBox = new QGroupBox(I18n::t("settings.appearance"));
    appearanceBox->setObjectName("SettingsGroup");
    auto *appearanceLayout = new QVBoxLayout(appearanceBox);
    appearanceLayout->setSpacing(12);

    // Theme row
    {
        auto *row = new QHBoxLayout;
        row->setSpacing(10);
        auto *label = new QLabel(I18n::t("settings.theme"));
        auto *lightBtn = new QPushButton(I18n::t("settings.light"));
        auto *darkBtn  = new QPushButton(I18n::t("settings.dark"));
        for (auto *b : {lightBtn, darkBtn}) {
            b->setObjectName("PillBtn");
            b->setCheckable(true);
            b->setMinimumHeight(32);
            b->setCursor(Qt::PointingHandCursor);
        }
        lightBtn->setChecked(Theme::instance().mode() == Theme::Light);
        darkBtn->setChecked(Theme::instance().mode() == Theme::Dark);
        connect(lightBtn, &QPushButton::clicked, [lightBtn, darkBtn]{
            Theme::instance().setMode(Theme::Light);
            lightBtn->setChecked(true); darkBtn->setChecked(false);
        });
        connect(darkBtn, &QPushButton::clicked, [lightBtn, darkBtn]{
            Theme::instance().setMode(Theme::Dark);
            lightBtn->setChecked(false); darkBtn->setChecked(true);
        });
        row->addWidget(label);
        row->addStretch();
        row->addWidget(lightBtn);
        row->addWidget(darkBtn);
        appearanceLayout->addLayout(row);
    }

    // Language row (V4 § 0)
    {
        auto *row = new QHBoxLayout;
        row->setSpacing(10);
        auto *label = new QLabel(I18n::t("settings.language"));
        auto *enBtn = new QPushButton(I18n::t("settings.lang.en"));
        auto *zhBtn = new QPushButton(I18n::t("settings.lang.zh"));
        for (auto *b : {enBtn, zhBtn}) {
            b->setObjectName("PillBtn");
            b->setCheckable(true);
            b->setMinimumHeight(32);
            b->setCursor(Qt::PointingHandCursor);
        }
        enBtn->setChecked(I18n::instance().isEnglish());
        zhBtn->setChecked(!I18n::instance().isEnglish());
        connect(enBtn, &QPushButton::clicked, [enBtn, zhBtn]{
            I18n::instance().setLanguage(I18n::English);
            enBtn->setChecked(true); zhBtn->setChecked(false);
        });
        connect(zhBtn, &QPushButton::clicked, [enBtn, zhBtn]{
            I18n::instance().setLanguage(I18n::Chinese);
            enBtn->setChecked(false); zhBtn->setChecked(true);
        });
        row->addWidget(label);
        row->addStretch();
        row->addWidget(enBtn);
        row->addWidget(zhBtn);
        appearanceLayout->addLayout(row);
    }

    root->addWidget(appearanceBox);

    // ---- DB path ----
    auto *dbBox = new QGroupBox(I18n::t("settings.storage"));
    dbBox->setObjectName("SettingsGroup");
    auto *dbLayout = new QVBoxLayout(dbBox);
    auto *dbInfo = new QLabel(I18n::t("settings.db_file_fmt").arg(dbPath));
    dbInfo->setObjectName("FieldHint");
    dbInfo->setWordWrap(true);
    dbInfo->setTextInteractionFlags(Qt::TextSelectableByMouse);
    dbLayout->addWidget(dbInfo);
    root->addWidget(dbBox);

    root->addStretch();

    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto *cancelBtn = new QPushButton(I18n::t("settings.cancel"));
    auto *saveBtn   = new QPushButton(I18n::t("settings.save"));
    cancelBtn->setObjectName("SecondaryBtn");
    saveBtn->setObjectName("PrimaryBtn");
    cancelBtn->setMinimumSize(86, 34);
    saveBtn->setMinimumSize(86, 34);
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
            font-size: 22px;
            font-weight: 600;
            color: %2;
            letter-spacing: -0.2px;
        }
        QLabel#FieldHint {
            color: %3;
            font-size: 12px;
        }
        QPushButton#PillBtn {
            border: 1px solid %1;
            border-radius: 8px;
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
            border-radius: 8px;
            padding: 8px 18px;
            font-weight: 600;
        }
        QPushButton#PrimaryBtn:hover { background-color: %8; }
        QPushButton#SecondaryBtn {
            background-color: transparent;
            color: %2;
            border: 1px solid %1;
            border-radius: 8px;
            padding: 8px 18px;
        }
        QPushButton#SecondaryBtn:hover { background-color: %7; }
        QPushButton#InlineBtn {
            background-color: %7;
            border: 1px solid %1;
            border-radius: 8px;
            color: %2;
        }
        QPushButton#InlineBtn:hover { background-color: %1; }
    )")
    /*1*/.arg(t.strokeRgba())
    /*2*/.arg(t.textPrimary().name())
    /*3*/.arg(t.textSecondary().name())
    /*4*/.arg(t.brand().name())
    /*5*/.arg(t.cardBgRgba())
    /*6*/.arg(t.bgContainer().name())
    /*7*/.arg(t.cardBgHoverRgba())
    /*8*/.arg(t.mode() == Theme::Light ? "#A85638" : "#D97757"));
}

} // namespace timemaster
