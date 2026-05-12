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

// ============== Opaque colors (V4.2) ==============
// V4.2 #7: both modes get a slightly DARKER page background. Light goes from
// #F2EEE5 -> #E8E1CF (warm paper, noticeably deeper); Dark drops to V3.2 spec
// #1F1F1E for status bar feeling. Containers (cards) keep a clear hierarchy
// step above the page.

QColor Theme::bgPage() const {
    return m_mode == Light ? QColor("#E8E1CF") : QColor("#1F1F1E");
}

QColor Theme::bgContainer() const {
    // Cards: warm off-white in Light for clear hierarchy against deeper page.
    // Dark: container slightly lighter than the page so cards visibly lift.
    return m_mode == Light ? QColor("#FAF6EE") : QColor("#2A2826");
}

QColor Theme::bgComponent() const {
    return m_mode == Light ? QColor("#DED7C5") : QColor("#363229");
}

QColor Theme::bgHover() const {
    return m_mode == Light ? QColor("#E0DACA") : QColor("#3A362E");
}

QColor Theme::stroke() const {
    return m_mode == Light ? QColor(60, 50, 40, 22) : QColor(240, 230, 210, 18);
}

QColor Theme::textPrimary() const {
    // V4.2 #8: dark mode text bumped from F7F7F5 to fully checked against deeper bg
    return m_mode == Light ? QColor("#1D1C16") : QColor("#F7F7F5");
}

QColor Theme::textSecondary() const {
    return m_mode == Light ? QColor("#6B645A") : QColor("#C2B6B6");
}

QColor Theme::textPlaceholder() const {
    return m_mode == Light ? QColor("#A39B8E") : QColor("#92948A");
}

QColor Theme::brand() const {
    return m_mode == Light ? QColor("#C26646") : QColor("#E08A6E");
}

QColor Theme::brandLight() const {
    return m_mode == Light ? QColor("#F4DDD0") : QColor("#4A3328");
}

QColor Theme::accent() const {
    return m_mode == Light ? QColor("#6F8B7E") : QColor("#8FA59A");
}

QColor Theme::todayHighlight() const {
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
    // V4.2: cards opaque, slightly off-white in Light to live on the deeper page
    return m_mode == Light
        ? QStringLiteral("rgba(250, 246, 238, 1.0)")
        : QStringLiteral("rgba(42, 40, 38, 1.0)");
}

QString Theme::cardBgHoverRgba() const {
    return m_mode == Light
        ? QStringLiteral("rgba(245, 240, 230, 1.0)")
        : QStringLiteral("rgba(52, 48, 44, 1.0)");
}

QString Theme::sidebarBgRgba() const {
    return m_mode == Light
        ? QStringLiteral("rgba(232, 225, 207, 1.0)")
        : QStringLiteral("rgba(28, 27, 25, 1.0)");
}

QString Theme::componentBgRgba() const {
    return m_mode == Light
        ? QStringLiteral("rgba(222, 215, 197, 1.0)")
        : QStringLiteral("rgba(54, 50, 41, 1.0)");
}

QString Theme::strokeRgba() const {
    return m_mode == Light
        ? QStringLiteral("rgba(60, 50, 40, 0.09)")
        : QStringLiteral("rgba(240, 230, 210, 0.07)");
}

QString Theme::shadowRgba() const {
    return m_mode == Light
        ? QStringLiteral("rgba(60, 45, 30, 0.06)")
        : QStringLiteral("rgba(0, 0, 0, 0.24)");
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
        // V4.3 #3 — 缺失的三种颜色补齐：之前 allColors() 返 12 个但 palette() 只
        // 定义 9 个，Brown/Gray/Cyan 查表得到 default-constructed ColorPalette → text
        // 是无效 QColor，按钮渲染近黑或全透明，用户在编辑弹窗里完全看不见。
        p[EventColor::Brown]  = {QColor("#E8DCC9"), QColor("#7A5430"), QColor("#C8AE85"), "Brown"};
        p[EventColor::Gray]   = {QColor("#DCDCD6"), QColor("#5A5A55"), QColor("#A8A89E"), "Gray"};
        p[EventColor::Cyan]   = {QColor("#D3E2E7"), QColor("#3F6C7A"), QColor("#86B3BD"), "Cyan"};
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
        // V4.3 #3
        p[EventColor::Brown]  = {QColor(122, 84, 48, 38),    QColor("#D4B58A"), QColor("#5A3E22"), "Brown"};
        p[EventColor::Gray]   = {QColor(120, 120, 115, 38),  QColor("#BFBFB6"), QColor("#4A4A45"), "Gray"};
        p[EventColor::Cyan]   = {QColor(63, 108, 122, 38),   QColor("#8DB6C0"), QColor("#2E4F5A"), "Cyan"};
    }
    return p;
}

// ============== Global QSS (V4.2 typography) ==============

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

    // V4.2 §3 typography hierarchy (re-tuned upward — user said "整体偏小"):
    //   page title    26px / 700
    //   section       17px / 600
    //   subtitle      17px / 600
    //   body / base   15px / 400
    //   caption       14px / 500
    //   hint          13px / 400
    //
    // V4.2 §3 Item 3 — kill dashed focus rectangle on every button.
    // V4.2 §6 — punctuation: rely on font fallback. CJK strings naturally use
    // CJK quotation marks; English uses Latin quotes from IBM Plex Serif.
    // V4.2 §8 — descender clipping: bump line-height across the board.
    return QString(R"(
        /* ============ Base type ============ */
        QWidget {
            color: %1;
            font-family: )" + fontFamily + R"(;
            font-size: 15px;
        }
        QLabel { background: transparent; color: %1; }
        QLabel[class="title"] {
            font-size: 26px;
            font-weight: 700;
            color: %1;
            letter-spacing: -0.3px;
        }
        QLabel[class="section"] {
            font-size: 17px;
            font-weight: 600;
            color: %1;
            letter-spacing: -0.1px;
        }
        QLabel[class="subtitle"] {
            font-size: 17px;
            font-weight: 600;
            color: %1;
            letter-spacing: -0.1px;
        }
        QLabel[class="caption"] {
            color: %2;
            font-size: 14px;
            font-weight: 500;
        }
        QLabel[class="hint"] {
            color: %6;
            font-size: 13px;
        }

        /* V4.2 #3 — global kill of the dashed focus rectangle */
        QPushButton:focus, QToolButton:focus, QCheckBox:focus,
        QRadioButton:focus, QLabel:focus, QListView:focus,
        QListWidget:focus, QTreeView:focus, QFrame:focus, QWidget:focus {
            outline: 0;
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
            outline: 0;
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

        /* SpinBox up/down buttons (used by Settings AI context) */
        QSpinBox::up-button, QSpinBox::down-button {
            background: transparent;
            border: none;
            width: 16px;
        }
        QSpinBox::up-button:hover, QSpinBox::down-button:hover {
            background: %9;
        }

        /* ============ Buttons ============ */
        QPushButton {
            background-color: %4;
            color: %1;
            border: 1px solid %3;
            border-radius: 8px;
            padding: 6px 14px;
            outline: 0;
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
