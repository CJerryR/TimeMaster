//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#include "InsightsWidget.h"
#include "../Theme.h"
#include "../../core/Database.h"
#include "../../core/I18n.h"

#include <QVBoxLayout>
#include <QLabel>

namespace timemaster {

InsightsWidget::InsightsWidget(Database *db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(8);

    m_title = new QLabel(I18n::t("widget.insights"));
    m_title->setProperty("class", "subtitle");
    lay->addWidget(m_title);

    m_text = new QLabel();
    m_text->setWordWrap(true);
    m_text->setMinimumHeight(60);
    lay->addWidget(m_text);
    lay->addStretch();

    connect(&I18n::instance(), &I18n::languageChanged, this, [this]{
        if (m_title) m_title->setText(I18n::t("widget.insights"));
        if (m_lastStart.isValid() && m_lastEnd.isValid()) {
            refresh(m_lastStart, m_lastEnd);
        }
    });
}

void InsightsWidget::refresh(const QDateTime &start, const QDateTime &end) {
    m_lastStart = start;
    m_lastEnd = end;
    if (!m_db) return;

    auto stats = m_db->getCategoryStats(start, end);
    auto daily = m_db->getDailySummaries(start, end);

    auto &theme = Theme::instance();
    QStringList insights;
    QString color = theme.textPrimary().name();
    QString sub   = theme.textSecondary().name();
    QString br    = theme.brand().name();

    qint64 total = 0;
    int count = 0;
    for (const auto &s : stats) { total += s.totalMinutes; count += s.count; }
    int days = qMax(1, int(start.daysTo(end)));

    if (total <= 0) {
        insights << QString("<span style='color:%1'>📭 %2</span>")
                        .arg(sub).arg(I18n::t("widget.insights.no_records"));
    } else {
        qint64 avgDaily = total / days;
        qint64 h = avgDaily / 60;
        qint64 m = avgDaily % 60;
        if (avgDaily > 0) {
            QString core = I18n::t("widget.insights.avg_fmt")
                              .arg(h).arg(m);
            insights << QString("<span style='color:%1'>⏱ </span>"
                                "<span style='color:%2;font-weight:bold;'>%3</span>")
                            .arg(color).arg(br).arg(core);
        }

        qint64 workMin = 0, studyMin = 0;
        for (const auto &s : stats) {
            if (s.category == EventCategory::Work)  workMin  = s.totalMinutes;
            if (s.category == EventCategory::Study) studyMin = s.totalMinutes;
        }
        if (workMin + studyMin > 0) {
            double pct = double(workMin + studyMin) / total * 100.0;
            QString core = I18n::t("widget.insights.work_pct_fmt").arg(int(pct));
            insights << QString("<span style='color:%1'>📊 </span>"
                                "<span style='color:%2;font-weight:bold;'>%3</span>")
                            .arg(color).arg(br).arg(core);
        }

        if (daily.size() >= 3) {
            int streak = 0;
            for (int i = daily.size() - 1; i >= 0; --i) {
                if (daily[i].totalMinutes > 0) ++streak; else break;
            }
            if (streak >= 3) {
                QString core = I18n::t("widget.insights.streak_fmt").arg(streak);
                insights << QString("<span style='color:%1'>🔥 </span>"
                                    "<span style='color:%2;font-weight:bold;'>%3</span>")
                                .arg(color).arg(br).arg(core);
            }
        }
    }

    QString html = QString("<div style='line-height:1.7;font-size:15px;'>%1</div>")
                        .arg(insights.join("&nbsp;&nbsp;|&nbsp;&nbsp;"));
    m_text->setText(html);
    m_text->setTextFormat(Qt::RichText);
    // V4.2: 在 widget 自身上注入 font-size 防止被全局 QSS 覆盖
    m_text->setStyleSheet("background:transparent;font-size:15px;");
}

} // namespace timemaster
