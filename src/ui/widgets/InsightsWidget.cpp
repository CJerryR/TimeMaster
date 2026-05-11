#include "InsightsWidget.h"
#include "../Theme.h"
#include "../../core/Database.h"

#include <QVBoxLayout>
#include <QLabel>

namespace timemaster {

InsightsWidget::InsightsWidget(Database *db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(8);

    m_title = new QLabel("智能洞察");
    m_title->setProperty("class", "subtitle");
    lay->addWidget(m_title);

    m_text = new QLabel();
    m_text->setWordWrap(true);
    m_text->setMinimumHeight(60);
    lay->addWidget(m_text);
    lay->addStretch();
}

void InsightsWidget::refresh(const QDateTime &start, const QDateTime &end) {
    auto stats = m_db->getCategoryStats(start, end);
    auto daily = m_db->getDailySummaries(start, end);

    auto &theme = Theme::instance();
    QStringList insights;
    QString color = theme.textPrimary().name();
    QString sub = theme.textSecondary().name();
    QString br = theme.brand().name();

    qint64 total = 0;
    int count = 0;
    for (const auto &s : stats) { total += s.totalMinutes; count += s.count; }
    int days = qMax(1, int(start.daysTo(end)));

    if (total <= 0) {
        insights << QString("<span style='color:%1'>📭 该时间段暂无记录</span>").arg(sub);
    } else {
        qint64 avgDaily = total / days;
        qint64 h = avgDaily / 60;
        qint64 m = avgDaily % 60;
        if (avgDaily > 0) {
            insights << QString("<span style='color:%1'>⏱ 日均追踪 </span>"
                               "<span style='color:%2;font-weight:bold;'>%3h %4m</span>")
                           .arg(color).arg(br).arg(h).arg(m);
        }

        qint64 workMin = 0, studyMin = 0;
        for (const auto &s : stats) {
            if (s.category == EventCategory::Work) workMin = s.totalMinutes;
            if (s.category == EventCategory::Study) studyMin = s.totalMinutes;
        }
        if (workMin + studyMin > 0) {
            double pct = double(workMin + studyMin) / total * 100.0;
            insights << QString("<span style='color:%1'>📊 工作学习占比 </span>"
                               "<span style='color:%2;font-weight:bold;'>%3%</span>")
                           .arg(color).arg(br).arg(pct, 0, 'f', 0);
        }

        if (daily.size() >= 3) {
            int streak = 0;
            for (int i = daily.size() - 1; i >= 0; --i) {
                if (daily[i].totalMinutes > 0) ++streak; else break;
            }
            if (streak >= 3) {
                insights << QString("<span style='color:%1'>🔥 连续 </span>"
                                   "<span style='color:%2;font-weight:bold;'>%3 天</span>"
                                   "<span style='color:%1'> 保持记录</span>")
                               .arg(color).arg(br).arg(streak);
            }
        }
    }

    QString html = QString("<div style='line-height:1.7;'>%1</div>")
                       .arg(insights.join("&nbsp;&nbsp;|&nbsp;&nbsp;"));
    m_text->setText(html);
    m_text->setTextFormat(Qt::RichText);
}

} // namespace timemaster
