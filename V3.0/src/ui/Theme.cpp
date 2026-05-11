#include "Theme.h"

namespace timemaster {

Theme &Theme::instance() {
    static Theme inst;
    return inst;
}

Theme::Theme(QObject *parent)
    : QObject(parent),
      m_settings("TimeMaster", "TimeMaster")
{
    QString saved = m_settings.value("theme", "light").toString();
    m_mode = (saved == "dark") ? Dark : Light;
}

void Theme::setMode(Mode m) {
    if (m == m_mode) return;
    m_mode = m;
    m_settings.setValue("theme", m == Dark ? "dark" : "light");
    emit changed();
}

void Theme::toggle() {
    setMode(m_mode == Light ? Dark : Light);
}

// ---- 不透明色：用于 QPainter 绘制 ----

QColor Theme::bgPage() const {
    return m_mode == Light ? QColor("#f4f5fa") : QColor("#0f1014");
}

QColor Theme::bgPageTop() const {
    return m_mode == Light ? QColor("#f8f9fd") : QColor("#13141a");
}

QColor Theme::bgPageBottom() const {
    return m_mode == Light ? QColor("#eceef5") : QColor("#0a0b0f");
}

QColor Theme::bgContainer() const {
    return m_mode == Light ? QColor("#ffffff") : QColor("#1c1d23");
}

QColor Theme::bgComponent() const {
    return m_mode == Light ? QColor("#f5f5f7") : QColor("#26272d");
}

QColor Theme::bgHover() const {
    return m_mode == Light ? QColor("#eeeff3") : QColor("#2d2e35");
}

QColor Theme::stroke() const {
    return m_mode == Light ? QColor("#e4e5ea") : QColor("#33343c");
}

QColor Theme::textPrimary() const {
    return m_mode == Light ? QColor("#18181b") : QColor("#f4f4f5");
}

QColor Theme::textSecondary() const {
    return m_mode == Light ? QColor("#52525b") : QColor("#a1a1aa");
}

QColor Theme::textPlaceholder() const {
    return m_mode == Light ? QColor("#a1a1aa") : QColor("#71717a");
}

QColor Theme::brand() const {
    return QColor("#ef4444");
}

QColor Theme::brandLight() const {
    QColor c = brand();
    c.setAlphaF(0.12);
    return c;
}

QColor Theme::accent() const {
    return m_mode == Light ? QColor("#4f46e5") : QColor("#818cf8");
}

QColor Theme::todayHighlight() const {
    return m_mode == Light ? QColor("#fff1f1") : QColor("#3a1818");
}

QColor Theme::nowLine() const {
    return QColor("#ef4444");
}

QColor Theme::success() const {
    return QColor("#16a34a");
}

QColor Theme::danger() const {
    return QColor("#dc2626");
}

// ---- 半透明色：rgba 字符串供 QSS ----

QString Theme::cardBgRgba() const {
    // 亮色：白色 75% alpha; 暗色：深灰 65% alpha (让渐变透出来)
    return m_mode == Light
        ? "rgba(255,255,255,0.72)"
        : "rgba(32,33,40,0.62)";
}

QString Theme::cardBgHoverRgba() const {
    return m_mode == Light
        ? "rgba(255,255,255,0.88)"
        : "rgba(46,47,55,0.75)";
}

QString Theme::sidebarBgRgba() const {
    return m_mode == Light
        ? "rgba(255,255,255,0.55)"
        : "rgba(22,23,28,0.65)";
}

QString Theme::componentBgRgba() const {
    return m_mode == Light
        ? "rgba(245,246,250,0.85)"
        : "rgba(38,39,46,0.7)";
}

QString Theme::strokeRgba() const {
    return m_mode == Light
        ? "rgba(0,0,0,0.06)"
        : "rgba(255,255,255,0.08)";
}

QString Theme::shadowRgba() const {
    return m_mode == Light
        ? "rgba(0,0,0,0.04)"
        : "rgba(0,0,0,0.35)";
}

QString Theme::pageGradient() const {
    QString top = bgPageTop().name();
    QString bot = bgPageBottom().name();
    return QString("qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 %1, stop:1 %2)")
        .arg(top, bot);
}

QHash<EventColor, ColorPalette> Theme::palette() const {
    return m_mode == Light ? eventColorsLight() : eventColorsDark();
}

QString Theme::globalStylesheet() const {
    QString text = textPrimary().name();
    QString text2 = textSecondary().name();
    QString placeholder = textPlaceholder().name();
    QString brandC = brand().name();
    QString accentC = accent().name();
    QString strokeC = stroke().name();
    QString strokeRgbaC = strokeRgba();
    QString componentRgba = componentBgRgba();
    QString cardRgba = cardBgRgba();
    QString hoverRgba = cardBgHoverRgba();
    QString container = bgContainer().name();
    QString page = bgPage().name();

    // 注意：Qt QSS 中要让 rgba 生效，背景色必须给 widget；
    // QWidget 默认透明传递，所以我们对 QFrame/QDialog 单独指定。
    return QString(R"(
        /* === 全局字体与基础文字 === */
        QWidget {
            color: %1;
            font-family: "Segoe UI", -apple-system, "SF Pro Display", "PingFang SC",
                         "Microsoft YaHei UI", "Microsoft YaHei", system-ui, sans-serif;
            font-size: 13px;
        }

        /* === 卡片容器（半透明，让页面渐变透出） === */
        QFrame#cardFrame,
        QFrame#dialogContent {
            background-color: %4;
            border: 1px solid %6;
            border-radius: 14px;
        }

        /* === 普通按钮 === */
        QPushButton {
            background-color: %5;
            border: 1px solid %6;
            border-radius: 9px;
            padding: 7px 16px;
            color: %1;
        }
        QPushButton:hover {
            background-color: %7;
            border-color: %8;
        }
        QPushButton:pressed {
            background-color: %8;
        }
        QPushButton:disabled {
            color: %3;
            background-color: %5;
        }

        /* === 主按钮 === */
        QPushButton[class="primary"] {
            background-color: %9;
            color: white;
            border: none;
            font-weight: 600;
            padding: 7px 18px;
        }
        QPushButton[class="primary"]:hover {
            background-color: #dc2626;
        }
        QPushButton[class="primary"]:disabled {
            background-color: %5;
            color: %3;
        }

        /* === ghost 按钮 === */
        QPushButton[class="ghost"] {
            background-color: transparent;
            border: none;
            color: %2;
            padding: 6px 12px;
        }
        QPushButton[class="ghost"]:hover {
            background-color: %7;
            color: %1;
        }

        /* === 输入框 === */
        QLineEdit, QTextEdit, QPlainTextEdit, QComboBox, QDateTimeEdit, QSpinBox {
            background-color: %5;
            border: 1px solid %6;
            border-radius: 9px;
            padding: 6px 11px;
            color: %1;
            selection-background-color: %9;
            selection-color: white;
        }
        QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus, QComboBox:focus,
        QDateTimeEdit:focus, QSpinBox:focus {
            border: 1px solid %9;
        }
        QComboBox::drop-down {
            border: none;
            width: 24px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 5px solid %2;
            margin-right: 8px;
        }
        QComboBox QAbstractItemView {
            background-color: %10;
            border: 1px solid %6;
            border-radius: 8px;
            padding: 4px;
            color: %1;
            selection-background-color: rgba(239,68,68,0.12);
            selection-color: %9;
            outline: 0;
        }

        /* === 滚动条 === */
        QScrollBar:vertical {
            background-color: transparent;
            width: 10px;
            margin: 4px 2px 4px 0;
        }
        QScrollBar::handle:vertical {
            background-color: %6;
            border-radius: 4px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: %3;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
        QScrollBar:horizontal {
            background-color: transparent;
            height: 10px;
            margin: 0 4px 2px 4px;
        }
        QScrollBar::handle:horizontal {
            background-color: %6;
            border-radius: 4px;
            min-width: 30px;
        }
        QScrollBar::handle:horizontal:hover { background-color: %3; }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }

        /* === Tooltip === */
        QToolTip {
            background-color: %10;
            color: %1;
            border: 1px solid %6;
            border-radius: 6px;
            padding: 5px 9px;
        }

        /* === 标题文本 === */
        QLabel[class="title"] {
            font-size: 19px;
            font-weight: 700;
            color: %1;
            background-color: transparent;
        }
        QLabel[class="subtitle"] {
            font-size: 14px;
            font-weight: 600;
            color: %1;
            background-color: transparent;
        }
        QLabel[class="caption"] {
            font-size: 11px;
            color: %2;
            background-color: transparent;
            margin-bottom: 4px;
        }
        QLabel { background-color: transparent; }

        /* === 对话框 === */
        QDialog {
            background-color: %10;
        }

        QCheckBox, QRadioButton {
            color: %1;
            background-color: transparent;
            spacing: 8px;
        }
        QCheckBox::indicator, QRadioButton::indicator {
            width: 16px; height: 16px;
        }
        QCheckBox::indicator:unchecked {
            border: 1.5px solid %6;
            border-radius: 4px;
            background-color: %5;
        }
        QCheckBox::indicator:checked {
            border: 1.5px solid %9;
            border-radius: 4px;
            background-color: %9;
        }
    )")
    /*1*/.arg(text)
    /*2*/.arg(text2)
    /*3*/.arg(placeholder)
    /*4*/.arg(cardRgba)
    /*5*/.arg(componentRgba)
    /*6*/.arg(strokeRgbaC)
    /*7*/.arg(hoverRgba)
    /*8*/.arg(strokeC)
    /*9*/.arg(brandC)
    /*10*/.arg(container);
}

} // namespace timemaster
