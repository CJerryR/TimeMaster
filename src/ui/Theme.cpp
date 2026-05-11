#include "Theme.h"

namespace timemaster {

Theme &Theme::instance() {
    static Theme t;
    return t;
}

Theme::Theme(QObject *parent) : QObject(parent) {
    int saved = m_settings.value("theme_mode", int(Light)).toInt();
    m_mode = (saved == Dark) ? Dark : Light;
}

void Theme::setMode(Mode m) {
    if (m == m_mode) return;
    m_mode = m;
    m_settings.setValue("theme_mode", int(m));
    emit changed();
}

void Theme::toggle() { setMode(m_mode == Light ? Dark : Light); }

// ============== 不透明色 ==============

QColor Theme::bgPage() const {
    // 浅色：自然纸张白（#F5F2ED），少约 15% 蓝光
    // 深色：暖调棕黑（#262521），不发青
    return m_mode == Light ? QColor("#F5F2ED") : QColor("#262521");
}

QColor Theme::bgPageTop() const {
    // 渐变左上：略亮于底色，模拟左上散射光
    return m_mode == Light ? QColor("#F8F5F0") : QColor("#2C2B26");
}

QColor Theme::bgPageBottom() const {
    // 渐变右下：略暗，纸张的自然落影
    return m_mode == Light ? QColor("#EEEAE2") : QColor("#1F1E1B");
}

QColor Theme::bgContainer() const {
    // 卡片不透明色，用于饼图等需要纯色填充的场景
    return m_mode == Light ? QColor("#FBF9F5") : QColor("#2E2D28");
}

QColor Theme::bgComponent() const {
    return m_mode == Light ? QColor("#F0EDE6") : QColor("#33312B");
}

QColor Theme::bgHover() const {
    return m_mode == Light ? QColor("#EDE8DF") : QColor("#36342E");
}

QColor Theme::stroke() const {
    // 实色描边（用于绘制非 QSS 部分，如日历网格线）
    return m_mode == Light ? QColor(60, 50, 40, 28) : QColor(240, 230, 210, 30);
}

QColor Theme::textPrimary() const {
    // #1D1C16 — 黑里带暖棕调
    return m_mode == Light ? QColor("#1D1C16") : QColor("#F0ECE0");
}

QColor Theme::textSecondary() const {
    return m_mode == Light ? QColor("#6B645A") : QColor("#B5AEA3");
}

QColor Theme::textPlaceholder() const {
    return m_mode == Light ? QColor("#A39B8E") : QColor("#7A736A");
}

QColor Theme::brand() const {
    // #D97757 — 暖橙褐色调
    // 深色模式略微提亮以保对比
    return m_mode == Light ? QColor("#D97757") : QColor("#E08A6E");
}

QColor Theme::brandLight() const {
    return m_mode == Light ? QColor("#F4DDD0") : QColor("#4A3328");
}

QColor Theme::accent() const {
    // 茶绿，与橙互补，用于"日历附带"等次级强调
    return m_mode == Light ? QColor("#6F8B7E") : QColor("#8FA59A");
}

QColor Theme::todayHighlight() const {
    return m_mode == Light ? QColor(217, 119, 87, 22) : QColor(224, 138, 110, 28);
}

QColor Theme::nowLine() const {
    return brand();
}

QColor Theme::success() const {
    return m_mode == Light ? QColor("#6B7F47") : QColor("#92A66B");
}

QColor Theme::danger() const {
    // 暖砖红，不用纯红
    return m_mode == Light ? QColor("#B8453E") : QColor("#D26C66");
}

// ============== 半透明 rgba 字符串 ==============

QString Theme::cardBgRgba() const {
    // 卡片：纸张白半透明叠加底层渐变
    return m_mode == Light
        ? QStringLiteral("rgba(252, 250, 245, 0.78)")
        : QStringLiteral("rgba(50, 48, 42, 0.72)");
}

QString Theme::cardBgHoverRgba() const {
    return m_mode == Light
        ? QStringLiteral("rgba(245, 240, 232, 0.92)")
        : QStringLiteral("rgba(62, 59, 52, 0.88)");
}

QString Theme::sidebarBgRgba() const {
    return m_mode == Light
        ? QStringLiteral("rgba(248, 244, 237, 0.62)")
        : QStringLiteral("rgba(38, 36, 32, 0.62)");
}

QString Theme::componentBgRgba() const {
    return m_mode == Light
        ? QStringLiteral("rgba(240, 235, 226, 0.55)")
        : QStringLiteral("rgba(46, 43, 38, 0.55)");
}

QString Theme::strokeRgba() const {
    // 暖调极低对比描边
    return m_mode == Light
        ? QStringLiteral("rgba(60, 50, 40, 0.10)")
        : QStringLiteral("rgba(240, 230, 210, 0.10)");
}

QString Theme::shadowRgba() const {
    return m_mode == Light
        ? QStringLiteral("rgba(60, 45, 30, 0.04)")
        : QStringLiteral("rgba(0, 0, 0, 0.20)");
}

QString Theme::pageGradient() const {
    QString top = bgPageTop().name();
    QString bot = bgPageBottom().name();
    return QString("qlineargradient(x1:0, y1:0, x2:1, y2:1, "
                   "stop:0 %1, stop:1 %2)").arg(top, bot);
}

QHash<EventColor, ColorPalette> Theme::palette() const {
    // 事件颜色保持多样（用户自选），但调暖了一档以呼应整体氛围
    QHash<EventColor, ColorPalette> p;
    if (m_mode == Light) {
        p[EventColor::Red]    = {QColor("#FDE3DC"), QColor("#B8453E"), QColor("#E8A89F"), "砖红"};
        p[EventColor::Orange] = {QColor("#FBE3D0"), QColor("#C2702E"), QColor("#E8B383"), "琥珀"};
        p[EventColor::Yellow] = {QColor("#FAEBC9"), QColor("#9E7A1F"), QColor("#DCC078"), "麦黄"};
        p[EventColor::Green]  = {QColor("#DDE7D3"), QColor("#5C7140"), QColor("#A8BC8E"), "茶绿"};
        p[EventColor::Teal]   = {QColor("#D4E3DE"), QColor("#3F6E60"), QColor("#8DB3A6"), "湖青"};
        p[EventColor::Blue]   = {QColor("#D8E2EC"), QColor("#3D5F7E"), QColor("#90A7BF"), "墨蓝"};
        p[EventColor::Indigo] = {QColor("#DBD9EA"), QColor("#504878"), QColor("#A39CC2"), "暮霭"};
        p[EventColor::Purple] = {QColor("#E5D8E3"), QColor("#6B4368"), QColor("#B59AB0"), "紫檀"};
        p[EventColor::Pink]   = {QColor("#F0DAD9"), QColor("#9B4D4F"), QColor("#D2A2A2"), "霞粉"};
    } else {
        p[EventColor::Red]    = {QColor(184, 69, 62, 38),    QColor("#E29089"), QColor("#7A3530"), "砖红"};
        p[EventColor::Orange] = {QColor(194, 112, 46, 38),   QColor("#E8B074"), QColor("#7A4A1F"), "琥珀"};
        p[EventColor::Yellow] = {QColor(158, 122, 31, 38),   QColor("#D4B66B"), QColor("#6E5A20"), "麦黄"};
        p[EventColor::Green]  = {QColor(92, 113, 64, 38),    QColor("#A6BC85"), QColor("#465A33"), "茶绿"};
        p[EventColor::Teal]   = {QColor(63, 110, 96, 38),    QColor("#8FB5A7"), QColor("#2E5749"), "湖青"};
        p[EventColor::Blue]   = {QColor(61, 95, 126, 38),    QColor("#9AB1C8"), QColor("#2D4A66"), "墨蓝"};
        p[EventColor::Indigo] = {QColor(80, 72, 133, 38),    QColor("#A8A3C8"), QColor("#3D375E"), "暮霭"};
        p[EventColor::Purple] = {QColor(107, 67, 104, 38),   QColor("#BB9DB7"), QColor("#523350"), "紫檀"};
        p[EventColor::Pink]   = {QColor(155, 77, 79, 38),    QColor("#D5A8A8"), QColor("#7A3A3C"), "霞粉"};
    }
    return p;
}

// ============== 全局 QSS ==============

QString Theme::globalStylesheet() const {
    QString brand = this->brand().name();
    QString brandHover = m_mode == Light ? "#C26646" : "#D97757";
    QString textPrim = textPrimary().name();
    QString textSec = textSecondary().name();
    QString placeholder = textPlaceholder().name();
    QString stroke = strokeRgba();
    QString cardBg = cardBgRgba();
    QString cardHover = cardBgHoverRgba();
    QString componentBg = componentBgRgba();
    QString bgContainer = this->bgContainer().name();
    QString hoverBg = bgHover().name();

    // 基准：所有按钮/输入框 8px 圆角，符合 8pt 网格
    return QString(R"(
        /* ============ 基础类型 ============ */
        QWidget {
            color: %1;
            font-family: "Microsoft YaHei UI", "PingFang SC", "Noto Sans CJK SC", sans-serif;
        }
        QLabel { background: transparent; color: %1; }
        QLabel[class="title"] {
            font-size: 19px;
            font-weight: 700;
            color: %1;
            letter-spacing: 0.2px;
        }
        QLabel[class="subtitle"] {
            font-size: 14px;
            font-weight: 600;
            color: %1;
            letter-spacing: 0.2px;
        }
        QLabel[class="caption"] {
            color: %2;
            font-size: 12px;
        }

        /* ============ 输入控件 ============ */
        QLineEdit, QPlainTextEdit, QTextEdit,
        QDateTimeEdit, QSpinBox, QComboBox {
            background-color: %4;
            color: %1;
            border: 1px solid %3;
            border-radius: 8px;
            padding: 6px 10px;
            selection-background-color: rgba(217, 119, 87, 0.22);
            selection-color: %1;
        }
        QLineEdit:focus, QPlainTextEdit:focus, QTextEdit:focus,
        QDateTimeEdit:focus, QSpinBox:focus, QComboBox:focus {
            border: 1px solid %5;
        }
        QLineEdit::placeholder, QPlainTextEdit::placeholder {
            color: %6;
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 22px;
            border: none;
        }
        QComboBox QAbstractItemView {
            background-color: %7;
            color: %1;
            border: 1px solid %3;
            border-radius: 8px;
            padding: 4px;
            selection-background-color: %8;
            selection-color: %1;
            outline: 0;
        }

        /* ============ 按钮 ============ */
        QPushButton {
            background-color: %4;
            color: %1;
            border: 1px solid %3;
            border-radius: 8px;
            padding: 6px 14px;
        }
        QPushButton:hover { background-color: %9; }
        QPushButton:disabled {
            color: %6;
            border-color: %3;
        }
        QPushButton[class="primary"] {
            background-color: %5;
            color: white;
            border: none;
            font-weight: 600;
            padding: 6px 18px;
        }
        QPushButton[class="primary"]:hover {
            background-color: %10;
        }
        QPushButton[class="primary"]:disabled {
            background-color: %4;
            color: %6;
        }
        QPushButton[class="ghost"] {
            background-color: transparent;
            color: %2;
            border: 1px solid %3;
        }
        QPushButton[class="ghost"]:hover {
            background-color: %9;
            color: %1;
        }

        /* ============ 滚动条 ============ */
        QScrollBar:vertical {
            background: transparent;
            width: 10px;
            margin: 4px 2px 4px 2px;
        }
        QScrollBar::handle:vertical {
            background: %3;
            min-height: 32px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical:hover { background: %2; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }

        QScrollBar:horizontal {
            background: transparent;
            height: 10px;
            margin: 2px 4px;
        }
        QScrollBar::handle:horizontal {
            background: %3;
            min-width: 32px;
            border-radius: 4px;
        }
        QScrollBar::handle:horizontal:hover { background: %2; }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
        QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: transparent; }

        /* ============ Group / Tooltip ============ */
        QToolTip {
            background-color: %7;
            color: %1;
            border: 1px solid %3;
            border-radius: 8px;
            padding: 6px 10px;
        }

        /* ============ 复选框 ============ */
        QCheckBox { color: %1; spacing: 8px; }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border: 1px solid %3;
            border-radius: 4px;
            background: %4;
        }
        QCheckBox::indicator:hover { border-color: %2; }
        QCheckBox::indicator:checked {
            background: %5;
            border-color: %5;
            image: none;
        }

        /* ============ 日历弹窗（QDateTimeEdit popup） ============ */
        QCalendarWidget QToolButton {
            background: transparent;
            color: %1;
            border: none;
            padding: 4px 8px;
        }
        QCalendarWidget QToolButton:hover { background: %9; border-radius: 6px; }
        QCalendarWidget QMenu { background: %7; color: %1; }
        QCalendarWidget QAbstractItemView:enabled {
            background: %7;
            color: %1;
            selection-background-color: %5;
            selection-color: white;
        }
        QCalendarWidget QWidget#qt_calendar_navigationbar {
            background: %7;
        }
    )")
    /*1*/.arg(textPrim)
    /*2*/.arg(textSec)
    /*3*/.arg(stroke)
    /*4*/.arg(componentBg)
    /*5*/.arg(brand)
    /*6*/.arg(placeholder)
    /*7*/.arg(bgContainer)
    /*8*/.arg(hoverBg)
    /*9*/.arg(cardHover)
    /*10*/.arg(brandHover);
}

} // namespace timemaster
