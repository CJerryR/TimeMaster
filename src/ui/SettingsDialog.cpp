//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#include "SettingsDialog.h"
#include "Theme.h"
#include "FontLoader.h"
#include "../core/DeepSeekClient.h"
#include "../core/I18n.h"
#include "../core/Preferences.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QSettings>
#include <QGroupBox>
#include <QMessageBox>
#include <QScrollArea>
#include <QFrame>
#include <QGuiApplication>
#include <QScreen>

namespace timemaster {

// V4.2 #10 — defaults match the legacy 7+14 chip on the chat page
static constexpr int kDefaultPastDays   = 7;
static constexpr int kDefaultFutureDays = 14;

// 构造函数：构建完整设置面板（滚动区域 + 固定按钮行），加载既有配置
SettingsDialog::SettingsDialog(DeepSeekClient *ai, const QString &dbPath, QWidget *parent)
    : QDialog(parent), m_ai(ai)
{
    setWindowTitle(I18n::t("settings.title"));
    setObjectName("SettingsDialog");

    // V4.3 #10 — 关键修复：响应屏幕高度，避免低分辨率 / 高 DPI 时弹窗高度超出
    // 屏幕导致保存/取消按钮被裁切到屏幕外。
    QSize avail = QSize(1200, 800);
    if (auto *scr = QGuiApplication::primaryScreen()) {
        avail = scr->availableSize();
    }
    int targetW = qMin(720, qMax(560, avail.width()  - 120));
    int targetH = qMin(720, qMax(540, avail.height() - 120));
    setMinimumWidth(qMin(640, targetW));
    setMinimumHeight(qMin(540, targetH));
    resize(targetW, targetH);

    // ============== 外层布局：scroll + 固定按钮行 ==============
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto *scroll = new QScrollArea(this);
    scroll->setObjectName("SettingsScroll");
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    outer->addWidget(scroll, 1);

    auto *content = new QWidget;
    content->setObjectName("SettingsContent");
    scroll->setWidget(content);

    auto *root = new QVBoxLayout(content);
    root->setContentsMargins(36, 32, 36, 22);
    root->setSpacing(20);

    // V4.2 #3 — bigger, bolder dialog title
    auto *title = new QLabel(I18n::t("settings.app"));
    title->setObjectName("DialogTitle");
    {
        QFont tf;
        tf.setPointSize(26);
        tf.setWeight(QFont::Bold);
        title->setFont(tf);
    }
    root->addWidget(title);

    QSettings s;

    // ===================== DeepSeek API =====================
    auto *apiBox = new QGroupBox(I18n::t("settings.api.group"));
    apiBox->setObjectName("SettingsGroup");
    auto *apiLayout = new QVBoxLayout(apiBox);
    apiLayout->setContentsMargins(22, 30, 22, 20);
    apiLayout->setSpacing(14);

    auto *hint = new QLabel(I18n::t("settings.api.hint"));
    hint->setObjectName("FieldHint");
    hint->setWordWrap(true);
    apiLayout->addWidget(hint);

    auto *keyHost = new QWidget;
    keyHost->setMinimumHeight(48);
    auto *keyRow = new QHBoxLayout(keyHost);
    keyRow->setContentsMargins(0, 0, 0, 0);
    keyRow->setSpacing(8);

    m_apiKeyEdit = new QLineEdit;
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    m_apiKeyEdit->setPlaceholderText("sk-xxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    m_apiKeyEdit->setText(m_ai->apiKey());
    m_apiKeyEdit->setMinimumHeight(44);
    m_apiKeyEdit->setObjectName("ApiKeyEdit");
    m_apiKeyEdit->setStyleSheet("QLineEdit#ApiKeyEdit { padding-right: 46px; }");
    keyRow->addWidget(m_apiKeyEdit);

    auto *clearKeyBtn = new QPushButton(I18n::t("settings.api.clear"));
    clearKeyBtn->setObjectName("SecondaryBtn");
    clearKeyBtn->setMinimumSize(86, 44);
    clearKeyBtn->setCursor(Qt::PointingHandCursor);
    clearKeyBtn->setFocusPolicy(Qt::NoFocus);
    connect(clearKeyBtn, &QPushButton::clicked, this, [this]{
        auto ret = QMessageBox::question(this, I18n::t("settings.api.clear_confirm_title"),
            I18n::t("settings.api.clear_confirm_msg"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret != QMessageBox::Yes) return;
        m_apiKeyEdit->clear();
        QSettings s;
        s.remove("deepseek_api_key");
        m_ai->setApiKey(QString());
        m_statusLabel->setText(I18n::t("settings.api.unconfigured"));
    });
    keyRow->addWidget(clearKeyBtn);

    auto *eyeBtn = new QPushButton(keyHost);
    eyeBtn->setObjectName("EyeBtn");
    eyeBtn->setFixedSize(34, 34);
    eyeBtn->setCursor(Qt::PointingHandCursor);
    eyeBtn->setToolTip(I18n::t("settings.api.toggle_tip"));
    eyeBtn->setText("👁");
    eyeBtn->setFocusPolicy(Qt::NoFocus);
    eyeBtn->raise();
    connect(eyeBtn, &QPushButton::clicked, this, &SettingsDialog::onToggleVisibility);
    m_eyeBtn = eyeBtn;
    m_keyHost = keyHost;

    apiLayout->addWidget(keyHost);

    m_statusLabel = new QLabel(m_ai->hasApiKey()
                               ? I18n::t("settings.api.configured")
                               : I18n::t("settings.api.unconfigured"));
    m_statusLabel->setObjectName("FieldHint");
    apiLayout->addWidget(m_statusLabel);

    root->addWidget(apiBox);

    // ===================== AI 对话上下文 =====================
    auto *ctxBox = new QGroupBox(I18n::t("settings.ai_context.group"));
    ctxBox->setObjectName("SettingsGroup");
    auto *ctxLayout = new QVBoxLayout(ctxBox);
    ctxLayout->setContentsMargins(22, 30, 22, 20);
    ctxLayout->setSpacing(12);

    auto *ctxHint = new QLabel(I18n::t("settings.ai_context.hint"));
    ctxHint->setObjectName("FieldHint");
    ctxHint->setWordWrap(true);
    ctxLayout->addWidget(ctxHint);

    m_aiSeesCalendarChk = new QCheckBox(I18n::t("settings.ai_context.enable"));
    m_aiSeesCalendarChk->setChecked(s.value("ai_sees_calendar", true).toBool());
    m_aiSeesCalendarChk->setFocusPolicy(Qt::NoFocus);
    ctxLayout->addWidget(m_aiSeesCalendarChk);

    auto *rangeRow = new QHBoxLayout;
    rangeRow->setSpacing(10);

    auto *pastLab = new QLabel(I18n::t("settings.ai_context.past"));
    pastLab->setObjectName("FieldLabel");
    rangeRow->addWidget(pastLab);

    m_pastDaysSpin = new QSpinBox;
    m_pastDaysSpin->setRange(0, 60);
    m_pastDaysSpin->setValue(s.value("ai_context_past_days", kDefaultPastDays).toInt());
    m_pastDaysSpin->setSuffix(I18n::t("settings.ai_context.days_suffix"));
    m_pastDaysSpin->setMinimumHeight(36);
    m_pastDaysSpin->setMinimumWidth(110);
    rangeRow->addWidget(m_pastDaysSpin);

    rangeRow->addSpacing(18);

    auto *futLab = new QLabel(I18n::t("settings.ai_context.future"));
    futLab->setObjectName("FieldLabel");
    rangeRow->addWidget(futLab);

    m_futureDaysSpin = new QSpinBox;
    m_futureDaysSpin->setRange(0, 60);
    m_futureDaysSpin->setValue(s.value("ai_context_future_days", kDefaultFutureDays).toInt());
    m_futureDaysSpin->setSuffix(I18n::t("settings.ai_context.days_suffix"));
    m_futureDaysSpin->setMinimumHeight(36);
    m_futureDaysSpin->setMinimumWidth(110);
    rangeRow->addWidget(m_futureDaysSpin);

    rangeRow->addStretch();
    ctxLayout->addLayout(rangeRow);

    auto syncEnabled = [this]{
        bool on = m_aiSeesCalendarChk->isChecked();
        m_pastDaysSpin->setEnabled(on);
        m_futureDaysSpin->setEnabled(on);
    };
    connect(m_aiSeesCalendarChk, &QCheckBox::toggled, this, syncEnabled);
    syncEnabled();

    root->addWidget(ctxBox);

    // ============= V4.3 #7 — AI 操作日历权限 =============
    // 类似 Claude Code 的权限模式：默认每次操作弹审批卡，用户也可以勾选"总是允许"
    auto *permBox = new QGroupBox(I18n::t("settings.ai_perm.group"));
    permBox->setObjectName("SettingsGroup");
    auto *permLayout = new QVBoxLayout(permBox);
    permLayout->setContentsMargins(22, 30, 22, 20);
    permLayout->setSpacing(10);

    auto *permHint = new QLabel(I18n::t("settings.ai_perm.hint"));
    permHint->setObjectName("FieldHint");
    permHint->setWordWrap(true);
    permLayout->addWidget(permHint);

    m_autoApproveAddChk    = new QCheckBox(I18n::t("settings.ai_perm.auto_add"));
    m_autoApproveDeleteChk = new QCheckBox(I18n::t("settings.ai_perm.auto_delete"));
    m_autoApproveUpdateChk = new QCheckBox(I18n::t("settings.ai_perm.auto_update"));
    for (auto *c : {m_autoApproveAddChk, m_autoApproveDeleteChk, m_autoApproveUpdateChk}) {
        c->setFocusPolicy(Qt::NoFocus);
        permLayout->addWidget(c);
    }
    m_autoApproveAddChk   ->setChecked(Preferences::instance().autoApproveAdd());
    m_autoApproveDeleteChk->setChecked(Preferences::instance().autoApproveDelete());
    m_autoApproveUpdateChk->setChecked(Preferences::instance().autoApproveUpdate());

    root->addWidget(permBox);

    // ===================== Appearance + Language + Week start =====================
    auto *appearanceBox = new QGroupBox(I18n::t("settings.appearance"));
    appearanceBox->setObjectName("SettingsGroup");
    auto *appearanceLayout = new QVBoxLayout(appearanceBox);
    appearanceLayout->setContentsMargins(22, 30, 22, 20);
    appearanceLayout->setSpacing(14);

    auto makePill = [](const QString &text){
        auto *b = new QPushButton(text);
        b->setObjectName("PillBtn");
        b->setCheckable(true);
        b->setMinimumSize(100, 40);
        b->setCursor(Qt::PointingHandCursor);
        b->setFocusPolicy(Qt::NoFocus);
        return b;
    };

    // -- Theme row
    {
        auto *row = new QHBoxLayout;
        row->setSpacing(10);
        auto *label = new QLabel(I18n::t("settings.theme"));
        label->setObjectName("FieldLabel");
        auto *lightBtn = makePill(I18n::t("settings.light"));
        auto *darkBtn  = makePill(I18n::t("settings.dark"));
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

    // -- Language row
    {
        auto *row = new QHBoxLayout;
        row->setSpacing(10);
        auto *label = new QLabel(I18n::t("settings.language"));
        label->setObjectName("FieldLabel");
        auto *enBtn = makePill(I18n::t("settings.lang.en"));
        auto *zhBtn = makePill(I18n::t("settings.lang.zh"));
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

    // -- V4.3 #8 Week start row
    {
        auto *row = new QHBoxLayout;
        row->setSpacing(10);
        auto *label = new QLabel(I18n::t("settings.week_start"));
        label->setObjectName("FieldLabel");
        m_weekStartMonBtn = makePill(I18n::t("settings.week_start.monday"));
        m_weekStartSunBtn = makePill(I18n::t("settings.week_start.sunday"));
        bool mon = Preferences::instance().weekStartsMonday();
        m_weekStartMonBtn->setChecked(mon);
        m_weekStartSunBtn->setChecked(!mon);
        connect(m_weekStartMonBtn, &QPushButton::clicked, this, [this]{
            m_weekStartMonBtn->setChecked(true);
            m_weekStartSunBtn->setChecked(false);
        });
        connect(m_weekStartSunBtn, &QPushButton::clicked, this, [this]{
            m_weekStartMonBtn->setChecked(false);
            m_weekStartSunBtn->setChecked(true);
        });
        row->addWidget(label);
        row->addStretch();
        row->addWidget(m_weekStartMonBtn);
        row->addWidget(m_weekStartSunBtn);
        appearanceLayout->addLayout(row);
    }

    root->addWidget(appearanceBox);

    // ===================== Storage =====================
    auto *dbBox = new QGroupBox(I18n::t("settings.storage"));
    dbBox->setObjectName("SettingsGroup");
    auto *dbLayout = new QVBoxLayout(dbBox);
    dbLayout->setContentsMargins(22, 30, 22, 20);
    auto *dbInfo = new QLabel(I18n::t("settings.db_file_fmt").arg(dbPath));
    dbInfo->setObjectName("FieldHint");
    dbInfo->setWordWrap(true);
    dbInfo->setTextInteractionFlags(Qt::TextSelectableByMouse);
    dbLayout->addWidget(dbInfo);
    root->addWidget(dbBox);

    // V4.3.2 #1 — 设置面板底部展示版本号 + 作者署名
    auto *versionLbl = new QLabel(I18n::t("settings.version_footer"));
    versionLbl->setObjectName("VersionFooter");
    versionLbl->setAlignment(Qt::AlignCenter);
    versionLbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    versionLbl->setOpenExternalLinks(true);
    root->addWidget(versionLbl);

    root->addStretch();

    // ============== 底部固定按钮行（不随滚动）==============
    auto *btnHost = new QWidget(this);
    btnHost->setObjectName("SettingsBtnRow");
    auto *btnRow = new QHBoxLayout(btnHost);
    btnRow->setContentsMargins(36, 14, 36, 18);
    btnRow->setSpacing(10);
    btnRow->addStretch();

    auto *cancelBtn = new QPushButton(I18n::t("settings.cancel"));
    auto *saveBtn   = new QPushButton(I18n::t("settings.save"));
    cancelBtn->setObjectName("SecondaryBtn");
    saveBtn->setObjectName("PrimaryBtn");
    cancelBtn->setMinimumSize(108, 42);
    saveBtn->setMinimumSize(108, 42);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setFocusPolicy(Qt::NoFocus);
    saveBtn->setFocusPolicy(Qt::NoFocus);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::onSave);
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(saveBtn);
    outer->addWidget(btnHost);

    connect(&Theme::instance(), &Theme::changed, this, &SettingsDialog::applyTheme);
    applyTheme();
}

// 窗口尺寸变化：重新定位眼睛按钮
void SettingsDialog::resizeEvent(QResizeEvent *e) {
    QDialog::resizeEvent(e);
    repositionEyeBtn();
}

// 窗口显示事件：重新定位眼睛按钮
void SettingsDialog::showEvent(QShowEvent *e) {
    QDialog::showEvent(e);
    repositionEyeBtn();
}

// 将眼睛按钮定位到 API Key 输入框右侧边缘
void SettingsDialog::repositionEyeBtn() {
    if (!m_eyeBtn || !m_apiKeyEdit || !m_keyHost) return;
    QRect editGeom = m_apiKeyEdit->geometry();
    int x = editGeom.right() - m_eyeBtn->width() - 6;
    int y = editGeom.center().y() - m_eyeBtn->height() / 2;
    m_eyeBtn->move(x, y);
    m_eyeBtn->raise();
}

// 切换 API Key 明文/密文显示
void SettingsDialog::onToggleVisibility() {
    m_keyVisible = !m_keyVisible;
    m_apiKeyEdit->setEchoMode(m_keyVisible ? QLineEdit::Normal : QLineEdit::Password);
}

// 保存所有配置项到 QSettings 和 Preferences
void SettingsDialog::onSave() {
    QString key = m_apiKeyEdit->text().trimmed();
    QSettings s;
    if (key.isEmpty()) {
        s.remove("deepseek_api_key");
    } else {
        s.setValue("deepseek_api_key", key);
    }
    m_ai->setApiKey(key);

    if (m_aiSeesCalendarChk) {
        s.setValue("ai_sees_calendar", m_aiSeesCalendarChk->isChecked());
    }
    if (m_pastDaysSpin) {
        s.setValue("ai_context_past_days", m_pastDaysSpin->value());
    }
    if (m_futureDaysSpin) {
        s.setValue("ai_context_future_days", m_futureDaysSpin->value());
    }

    // V4.3 #8 — 持久化周起始日
    if (m_weekStartMonBtn) {
        Preferences::instance().setWeekStartsMonday(m_weekStartMonBtn->isChecked());
    }

    // V4.3 #7 — 持久化自动审批开关
    if (m_autoApproveAddChk)    Preferences::instance().setAutoApproveAdd(m_autoApproveAddChk->isChecked());
    if (m_autoApproveDeleteChk) Preferences::instance().setAutoApproveDelete(m_autoApproveDeleteChk->isChecked());
    if (m_autoApproveUpdateChk) Preferences::instance().setAutoApproveUpdate(m_autoApproveUpdateChk->isChecked());

    accept();
}

// 应用主题：设置完整 QSS 样式并重定位眼睛按钮
void SettingsDialog::applyTheme() {
    auto &t = Theme::instance();
    setStyleSheet(t.globalStylesheet() + QString(R"(
        QDialog#SettingsDialog { background-color: %6; }
        QScrollArea#SettingsScroll {
            background-color: %6;
            border: none;
        }
        QWidget#SettingsContent {
            background-color: %6;
        }
        QWidget#SettingsBtnRow {
            background-color: %5;
            border-top: 1px solid %1;
        }

        QGroupBox#SettingsGroup {
            background-color: %5;
            border: 1px solid %1;
            border-radius: 14px;
            margin-top: 24px;
            padding-top: 10px;
            font-weight: 600;
            color: %2;
        }
        QGroupBox#SettingsGroup::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 18px;
            top: 6px;
            padding: 6px 18px;
            background-color: %6;
            border: 1px solid %1;
            border-radius: 10px;
            color: %2;
            font-size: 14px;
            font-weight: 600;
        }

        QLabel#DialogTitle {
            color: %2;
            background: transparent;
            letter-spacing: -0.3px;
        }
        QLabel#FieldLabel {
            color: %2;
            font-size: 15px;
            font-weight: 500;
        }
        QLabel#FieldHint {
            color: %3;
            font-size: 14px;
            line-height: 1.6;
        }

        QLineEdit#ApiKeyEdit {
            background-color: %6;
            color: %2;
            border: 1px solid %1;
            border-radius: 10px;
            padding: 0 12px;
            padding-right: 46px;
            font-size: 15px;
            selection-background-color: %4;
        }
        QLineEdit#ApiKeyEdit:focus { border: 1px solid %4; }

        QPushButton#EyeBtn {
            background-color: transparent;
            color: %3;
            border: none;
            border-radius: 8px;
            font-size: 16px;
            outline: 0;
        }
        QPushButton#EyeBtn:hover {
            background-color: %7;
            color: %2;
        }

        QSpinBox {
            background-color: %6;
            color: %2;
            border: 1px solid %1;
            border-radius: 8px;
            padding: 4px 10px;
            font-size: 15px;
        }
        QSpinBox:focus { border: 1px solid %4; }
        QSpinBox:disabled { color: %3; }

        QCheckBox {
            color: %2;
            font-size: 15px;
            spacing: 10px;
            outline: 0;
        }

        QPushButton#PillBtn {
            border: 1px solid %1;
            border-radius: 10px;
            padding: 6px 20px;
            background-color: transparent;
            color: %3;
            font-size: 15px;
            font-weight: 500;
            outline: 0;
        }
        QPushButton#PillBtn:hover { background-color: %7; }
        QPushButton#PillBtn:checked {
            background-color: %4;
            color: white;
            border-color: %4;
        }

        QPushButton#PrimaryBtn {
            background-color: %4;
            color: white;
            border: none;
            border-radius: 10px;
            padding: 10px 24px;
            font-size: 15px;
            font-weight: 600;
            outline: 0;
        }
        QPushButton#PrimaryBtn:hover { background-color: %8; }

        QPushButton#SecondaryBtn {
            background-color: transparent;
            color: %2;
            border: 1px solid %1;
            border-radius: 10px;
            padding: 10px 20px;
            font-size: 15px;
            outline: 0;
        }
        QPushButton#SecondaryBtn:hover { background-color: %7; }

        /* V4.3.2 #1 — 版本号 footer 弱化处理：text-placeholder 色，小字号 */
        QLabel#VersionFooter {
            color: %3;
            font-size: 12px;
            font-weight: 400;
            padding: 14px 0 4px 0;
            background: transparent;
        }
    )")
    /*1*/.arg(t.strokeRgba())
    /*2*/.arg(t.textPrimary().name())
    /*3*/.arg(t.textSecondary().name())
    /*4*/.arg(t.brand().name())
    /*5*/.arg(t.cardBgRgba())
    /*6*/.arg(t.bgContainer().name())
    /*7*/.arg(t.cardBgHoverRgba())
    /*8*/.arg(t.mode() == Theme::Light ? "#A85638" : "#D97757"));

    repositionEyeBtn();
}

} // namespace timemaster
