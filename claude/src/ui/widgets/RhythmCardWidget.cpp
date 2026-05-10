#include "RhythmCardWidget.h"
#include "../Theme.h"

#include <QPainter>
#include <QPaintEvent>

namespace cadence {

RhythmCardWidget::RhythmCardWidget(const QString& title, QWidget* parent)
    : QWidget(parent), m_title(title) {
    setMinimumHeight(80);
}

QSize RhythmCardWidget::sizeHint() const {
    return QSize(280, 80);
}

void RhythmCardWidget::setValue(const QString& v) {
    m_value = v.isEmpty() ? "—" : v;
    update();
}

void RhythmCardWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const auto& th = Theme::instance();

    p.setBrush(th.bgContainer());
    p.setPen(QPen(th.borderColor(), 1));
    p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 12, 12);

    QFont titleFont = font(); titleFont.setPixelSize(13);
    p.setFont(titleFont);
    p.setPen(th.textPlaceholder());
    p.drawText(QRect(20, 16, width() - 40, 18),
               Qt::AlignLeft | Qt::AlignVCenter, m_title);

    QFont valFont = font(); valFont.setPixelSize(20); valFont.setWeight(QFont::Bold);
    p.setFont(valFont);
    p.setPen(th.textPrimary());
    p.drawText(QRect(20, 38, width() - 40, 28),
               Qt::AlignLeft | Qt::AlignVCenter, m_value);
}

} // namespace cadence
