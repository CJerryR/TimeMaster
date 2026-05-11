#include "MotivationWidget.h"
#include "../Theme.h"
#include "../../core/Database.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QFont>
#include <QDate>
#include <QDateTime>

namespace timemaster {

namespace {
struct Quote {
    QString text;
    QString attribute;
};

// 严选 30 句 · 中英混排 · 时间 / 节奏 / 行动主题
const QList<Quote> &quotePool() {
    static const QList<Quote> q = {
        {"今天的事，今天做完，明天还有明天的山。",                "—— 时间管理大师"},
        {"You don't find time. You make it.",                  "—— Charles Buxton"},
        {"专注是稀缺品，把它留给真正重要的事。",                   "—— 时间管理大师"},
        {"The bad news is time flies. The good news is you're the pilot.", "—— Michael Altshuler"},
        {"持续的小动作，胜过偶尔的大爆发。",                       "—— James Clear"},
        {"Lost time is never found again.",                     "—— Benjamin Franklin"},
        {"你不必赢在每一分钟，只需赢在每一段时间。",                "—— 时间管理大师"},
        {"Do the difficult things while they are easy and do the great things while they are small.", "—— Lao Tzu"},
        {"今日事，今日毕；今日难，今日记。",                        "—— 时间管理大师"},
        {"Time is what we want most, but what we use worst.",   "—— William Penn"},
        {"复盘一天，等于多过一天。",                                "—— 时间管理大师"},
        {"Start where you are. Use what you have. Do what you can.", "—— Arthur Ashe"},
        {"日历不会骗人，留白处藏着你真正的选择。",                  "—— 时间管理大师"},
        {"How we spend our days is, of course, how we spend our lives.", "—— Annie Dillard"},
        {"先做最难的，剩下都是顺水推舟。",                          "—— 时间管理大师"},
        {"It always seems impossible until it's done.",         "—— Nelson Mandela"},
        {"少即是多。日程表的留白本身就是生产力。",                  "—— 时间管理大师"},
        {"The key is not to prioritize what's on your schedule, but to schedule your priorities.", "—— Stephen Covey"},
        {"今天不必完美，能完成就好。",                              "—— 时间管理大师"},
        {"Time is the most valuable thing a man can spend.",     "—— Theophrastus"},
        {"凡是真正属于你的事情，时间会替你记得。",                  "—— 时间管理大师"},
        {"Either you run the day, or the day runs you.",        "—— Jim Rohn"},
        {"今天迈出一小步，明天的你会感谢。",                        "—— 时间管理大师"},
        {"You may delay, but time will not.",                   "—— Benjamin Franklin"},
        {"忙不是借口，慌才是。",                                    "—— 时间管理大师"},
        {"Time = Life. Therefore, waste your time and waste of your life.", "—— Alan Lakein"},
        {"把好的时间花给好的事情，是这世上最公平的事。",            "—— 时间管理大师"},
        {"The two most powerful warriors are patience and time.","—— Leo Tolstoy"},
        {"目标先于日历，日历先于忙碌。",                            "—— 时间管理大师"},
        {"Yesterday is gone. Tomorrow has not yet come. We have only today.", "—— Mother Teresa"},
    };
    return q;
}
} // namespace

MotivationWidget::MotivationWidget(Database *db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(8);

    m_quoteLab = new QLabel;
    m_quoteLab->setObjectName("QuoteLab");
    m_quoteLab->setWordWrap(true);
    QFont qf;
    qf.setPointSize(15);
    qf.setWeight(QFont::DemiBold);
    qf.setItalic(true);
    m_quoteLab->setFont(qf);
    lay->addWidget(m_quoteLab);

    m_attributeLab = new QLabel;
    m_attributeLab->setObjectName("QuoteAttrLab");
    lay->addWidget(m_attributeLab);

    m_insightLab = new QLabel;
    m_insightLab->setObjectName("InsightLab");
    m_insightLab->setWordWrap(true);
    m_insightLab->setContentsMargins(0, 4, 0, 0);
    lay->addWidget(m_insightLab);

    connect(&Theme::instance(), &Theme::changed, this, &MotivationWidget::applyTheme);
    applyTheme();
}

QString MotivationWidget::pickDailyQuote() const {
    // 按 day-of-year 稳定选取，同一天看到同一句
    int dy = QDate::currentDate().dayOfYear();
    const auto &pool = quotePool();
    int idx = dy % pool.size();
    return QString("\u201C%1\u201D|%2").arg(pool[idx].text, pool[idx].attribute);
}

QString MotivationWidget::buildInsight(const QDateTime &start, const QDateTime &end) const {
    if (!m_db) return {};

    auto stats = m_db->getCategoryStats(start, end);
    auto daily = m_db->getDailySummaries(start, end);

    qint64 total = 0;
    for (const auto &s : stats) total += s.totalMinutes;

    if (total <= 0) {
        return "✦  今天打开第一页 —— 添加第一个日程，新的节奏从这里开始。";
    }

    // 连续天数
    int streak = 0;
    for (int i = daily.size() - 1; i >= 0; --i) {
        if (daily[i].totalMinutes > 0) ++streak; else break;
    }

    // 找类别占比第一
    QString topCat;
    qint64 topMin = 0;
    double topPct = 0;
    for (const auto &s : stats) {
        if (s.totalMinutes > topMin) {
            topMin = s.totalMinutes;
            topCat = categoryLabel(s.category);
            topPct = s.percentage;
        }
    }

    QStringList candidates;
    if (streak >= 7) {
        candidates << QString("✦  连续 %1 天稳定记录，已养成节奏。").arg(streak);
    } else if (streak >= 3) {
        candidates << QString("✦  已连续记录 %1 天，再坚持就要破纪录了。").arg(streak);
    }

    if (!topCat.isEmpty() && topPct >= 35) {
        candidates << QString("✦  你这段时间最大的投入是「%1」，占比 %2%。").arg(topCat).arg(int(topPct));
    }

    qint64 avgPerDay = total / qMax(1, int(start.daysTo(end)));
    if (avgPerDay >= 240) {
        candidates << QString("✦  日均投入 %1 小时，能量充沛，记得给身体留一段休息。")
                            .arg(avgPerDay / 60);
    } else if (avgPerDay >= 60) {
        candidates << QString("✦  日均投入 %1 小时，节奏在线。").arg(avgPerDay / 60);
    }

    // 今天还能再做点什么
    QDate today = QDate::currentDate();
    QDateTime tStart(today, QTime(0, 0));
    QDateTime tEnd(today, QTime(23, 59, 59));
    auto todayEvents = m_db->getEventsByRange(tStart, tEnd);
    int todayCnt = 0;
    for (const auto &e : todayEvents)
        if (e.startDate.date() == today) todayCnt++;
    if (todayCnt == 0) {
        candidates << "✦  今天还是一张白纸，添加点什么再开始？";
    } else if (todayCnt <= 2) {
        candidates << QString("✦  今天只安排了 %1 件事，留给灵感的空间挺大。").arg(todayCnt);
    } else {
        candidates << QString("✦  今天 %1 件事在排队，按优先级开始就好。").arg(todayCnt);
    }

    if (candidates.isEmpty()) return "✦  保持当前节奏，下一周也会很稳。";
    int idx = QDate::currentDate().dayOfYear() % candidates.size();
    return candidates[idx];
}

void MotivationWidget::refresh(const QDateTime &start, const QDateTime &end) {
    auto &t = Theme::instance();
    QString q = pickDailyQuote();
    QStringList parts = q.split('|');
    m_quoteLab->setText(parts.value(0));
    m_attributeLab->setText(parts.value(1));

    QString insight = buildInsight(start, end);
    m_insightLab->setText(insight);

    QString brand = t.brand().name();
    m_quoteLab->setStyleSheet(QString("color:%1;background:transparent;").arg(t.textPrimary().name()));
    m_attributeLab->setStyleSheet(QString("color:%1;font-size:12px;font-style:italic;").arg(t.textSecondary().name()));
    m_insightLab->setStyleSheet(QString("color:%1;font-size:13px;font-weight:600;").arg(brand));
}

void MotivationWidget::applyTheme() {
    // 实际颜色在 refresh 时刷新，这里仅保留背景
    setStyleSheet("background:transparent;");
}

} // namespace timemaster
