#include "Theme.h"
#include "FontLoader.h"

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

// ============== Opaque colors (V4) ==============

QColor Theme::bgPage() const {
    // Light: warmer paper (#F2EEE5); Dark: paper-warm dark (#26241F)
    return m_mode == Light ? QColor("#F2EEE5") : QColor("#26241F");
}

QColor Theme::bgContainer() const {
    // Cards: pure white in Light for clear hierarchy. Dark: warm container.
    return m_mode == Light ? QColor("#FFFFFF") : QColor("#2E2C26");
}

QColor Theme::bgComponent() const {
    return m_mode == Light ? QColor("#EAE5D8") : QColor("#363229");
}

QColor Theme::bgHover() const {
    return m_mode == Light ? QColor("#EDE8DF") : QColor("#3A362E");
}

QColor Theme::stroke() const {
    // 极淡描边：浅色 0.08 / 深色 0.06
    return m_mode == Light ? QColor(60, 50, 40, 20) : QColor(240, 230, 210, 16);
}

QColor Theme::textPrimary() const {
    return m_mode == Light ? QColor("#1D1C16") : QColor(247, 247, 245);
}

QColor Theme::textSecondary() const {
    return m_mode == Light ? QColor("#6B645A") : QColor(194, 182, 182);
}

QColor Theme::textPlaceholder() const {
    return m_mode == Light ? QColor("#A39B8E") : QColor(146, 148, 138);
}

QColor Theme::brand() const {
    // Light deepened to #C26646 (WCAG AA); Dark keeps #E08A6E
    return m_mode == Light ? QColor("#C26646") : QColor("#E08A6E");
}

QColor Theme::brandLight() const {
    return m_mode == Light ? QColor("#F4DDD0") : QColor("#4A3328");
}

QColor Theme::accent() const {
    return m_mode == Light ? QColor("#6F8B7E") : QColor("#8FA59A");
}

QColor Theme::todayHighlight() const {
    // 保留接口；MonthView 已不再用作背景填充
    return m_mode == Light ? QColor(194, 102, 70, 0) : QColor(224, 138, 110, 0);
}

QColor Theme::nowLine() const { return brand(); }

QColor Theme::success() const {
    return m_mode == Light ? QColor("#6B7F47") : QColor("#92A66B");
}

QColor Theme::danger() const {
    return m_mode == Light ? QColor("#B8453E") : QColor("#D26C66");
}

// ============== Semi-transparent rgba (QSS) ==============

QString Theme::cardBgRgba() const {
    // 浅色卡片直接拉到纯白；深色保持原暗色卡片
    return m_mode == Light
        ? QStringLiteral("rgba(255, 255, 255, 1.0)")
        : QStringLiteral("rgba(46, 44, 38, 1.0)");
}

QString Theme::cardBgHoverRgba() const {
    return m_mode == Light
        ? QStringLiteral("rgba(248, 244, 237, 1.0)")
        : QStringLiteral("rgba(58, 54, 46, 1.0)");
}

QString Theme::sidebarBgRgba() const {
    // V4：侧栏与页面相同的纸色，靠右侧 1px 描边分隔；深色比页面略暗
    return m_mode == Light
        ? QStringLiteral("rgba(242, 238, 229, 1.0)")
        : QStringLiteral("rgba(34, 32, 28, 1.0)");
}

QString Theme::componentBgRgba() const {
    return m_mode == Light
        ? QStringLiteral("rgba(234, 229, 216, 1.0)")
        : QStringLiteral("rgba(54, 50, 41, 1.0)");
}

QString Theme::strokeRgba() const {
    return m_mode == Light
        ? QStringLiteral("rgba(60, 50, 40, 0.08)")
        : QStringLiteral("rgba(240, 230, 210, 0.06)");
}

QString Theme::shadowRgba() const {
    return m_mode == Light
        ? QStringLiteral("rgba(60, 45, 30, 0.04)")
        : QStringLiteral("rgba(0, 0, 0, 0.20)");
}

QHash<EventColor, ColorPalette> Theme::palette() const {
    QHash<EventColor, ColorPalette> p;
    if (m_mode == Light) {
        p[EventColor::Red]    = {QColor("#FDE3DC"), QColor("#B8453E"), QColor("#E8A89F"), "Red"};
        p[EventColor::Orange] = {QColor("#FBE3D0"), QColor("#C2702E"), QColor("#E8B383"), "Orange"};
        p[EventColor::Yellow] = {QColor("#FAEBC9"), QColor("#9E7A1F"), QColor("#DCC078"), "Yellow"};
        p[EventColor::Green]  = {QColor("#DDE7D3"), QColor("#5C7140"), QColor("#A8BC8E"), "Green"};
        p[EventColor::Teal]   = {QColor("#D4E3DE"), QColor("#3F6E60"), QColor("#8DB3A6"), "Teal"};
        p[EventColor::Blue]   = {QColor("#D8E2EC"), QColor("#3D5F7E"), QColor("#90A7BF"), "Blue"};
        p[EventColor::Indigo] = {QColor("#DBD9EA"), QColor("#504878"), QColor("#A39CC2"), "Indigo"};
        p[EventColor::Purple] = {QColor("#E5D8E3"), QColor("#6B4368"), QColor("#B59AB0"), "Purple"};
        p[EventColor::Pink]   = {QColor("#F0DAD9"), QColor("#9B4D4F"), QColor("#D2A2A2"), "Pink"};
    } else {
        p[EventColor::Red]    = {QColor(184, 69, 62, 38),    QColor("#E29089"), QColor("#7A3530"), "Red"};
        p[EventColor::Orange] = {QColor(194, 112, 46, 38),   QColor("#E8B074"), QColor("#7A4A1F"), "Orange"};
        p[EventColor::Yellow] = {QColor(158, 122, 31, 38),   QColor("#D4B66B"), QColor("#6E5A20"), "Yellow"};
        p[EventColor::Green]  = {QColor(92, 113, 64, 38),    QColor("#A6BC85"), QColor("#465A33"), "Green"};
        p[EventColor::Teal]   = {QColor(63, 110, 96, 38),    QColor("#8FB5A7"), QColor("#2E5749"), "Teal"};
        p[EventColor::Blue]   = {QColor(61, 95, 126, 38),    QColor("#9AB1C8"), QColor("#2D4A66"), "Blue"};
        p[EventColor::Indigo] = {QColor(80, 72, 133, 38),    QColor("#A8A3C8"), QColor("#3D375E"), "Indigo"};
        p[EventColor::Purple] = {QColor(107, 67, 104, 38),   QColor("#BB9DB7"), QColor("#523350"), "Purple"};
        p[EventColor::Pink]   = {QColor(155, 77, 79, 38),    QColor("#D5A8A8"), QColor("#7A3A3C"), "Pink"};
    }
    return p;
}

// ============== Global QSS (V4 typography) ==============

QString Theme::globalStylesheet() const {
    QString brand = this->brand().name();
    QString brandHover = m_mode == Light ? "#A85638" : "#D97757";
    QString textPrim = textPrimary().name();
    QString textSec = textSecondary().name();
    QString placeholder = textPlaceholder().name();
    QString stroke = strokeRgba();
    QString cardBg = cardBgRgba();
    QString cardHover = cardBgHoverRgba();
    QString componentBg = componentBgRgba();
    QString bgContainer = this->bgContainer().name();
    QString hoverBg = bgHover().name();

    QString fontFamily = FontLoader::familyChain();

    // Typography hierarchy (V4 § 3.1):
    //   page title   22px / 600 / -0.2px tracking
    //   section      15px / 600
    //   body         14px / 400
    //   secondary    13px / 400
    //   caption      12px / 400
    //
    // Geometry (V4 § 6.5):
    //   cards 12px, buttons & inputs 8px, chips 6px
    return QString(R"(
        /* ============ Base type ============ */
        QWidget {
            color: %1;
            font-family: )" + fontFamily + R"(;
            font-size: 14px;
        }
        QLabel { background: transparent; color: %1; }
        QLabel[class="title"] {
            font-size: 22px;
            font-weight: 600;
            color: %1;
            letter-spacing: -0.2px;
        }
        QLabel[class="section"] {
            font-size: 15px;
            font-weight: 600;
            color: %1;
        }
        QLabel[class="subtitle"] {
            font-size: 15px;
            font-weight: 600;
            color: %1;
        }
        QLabel[class="caption"] {
            color: %2;
            font-size: 13px;
        }
        QLabel[class="hint"] {
            color: %6;
            font-size: 12px;
        }

        /* ============ Inputs ============ */
        QLineEdit, QPlainTextEdit, QTextEdit,
        QDateTimeEdit, QSpinBox, QComboBox {
            background-color: %4;
            color: %1;
            border: 1px solid %3;
            border-radius: 8px;
            padding: 6px 10px;
            selection-background-color: rgba(194, 102, 70, 0.22);
            selection-color: %1;
        }
        QLineEdit:focus, QPlainTextEdit:focus, QTextEdit:focus,
        QDateTimeEdit:focus, QSpinBox:focus, QComboBox:focus {
            border: 1px solid %5;
        }
        QLineEdit::placeholder, QPlainTextEdit::placeholder { color: %6; }
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

        /* ============ Buttons ============ */
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
        QPushButton[class="primary"]:hover { background-color: %10; }
        QPushButton[class="primary"]:disabled {
            background-color: %4;
            color: %6;
        }
        QPushButton[class="ghost"] {
            background-color: transparent;
            color: %2;
            border: 1px solid transparent;
        }
        QPushButton[class="ghost"]:hover {
            background-color: %9;
            color: %1;
            border-color: %3;
        }

        /* ============ Scrollbar ============ */
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

        QToolTip {
            background-color: %7;
            color: %1;
            border: 1px solid %3;
            border-radius: 6px;
            padding: 6px 10px;
        }

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
        QCalendarWidget QWidget#qt_calendar_navigationbar { background: %7; }
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
