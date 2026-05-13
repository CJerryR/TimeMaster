//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#include "MotivationWidget.h"
#include "../Theme.h"
#include "../FontLoader.h"
#include "../../core/Database.h"
#include "../../core/I18n.h"

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

// V4 § 4.1：删除所有 "—— 时间管理大师" 自署名引言，只保留真实人物的名言。
const QList<Quote> &quotePool() {
    static const QList<Quote> q = {
        {"You don't find time. You make it.",                                  "— Charles Buxton"},
        {"The bad news is time flies. The good news is you're the pilot.",     "— Michael Altshuler"},
        {"Lost time is never found again.",                                    "— Benjamin Franklin"},
        {"Do the difficult things while they are easy and do the great things while they are small.",
                                                                               "— Lao Tzu"},
        {"Time is what we want most, but what we use worst.",                  "— William Penn"},
        {"Start where you are. Use what you have. Do what you can.",           "— Arthur Ashe"},
        {"How we spend our days is, of course, how we spend our lives.",       "— Annie Dillard"},
        {"It always seems impossible until it's done.",                        "— Nelson Mandela"},
        {"The key is not to prioritize what's on your schedule, but to schedule your priorities.",
                                                                               "— Stephen Covey"},
        {"Time is the most valuable thing a man can spend.",                   "— Theophrastus"},
        {"Either you run the day, or the day runs you.",                       "— Jim Rohn"},
        {"You may delay, but time will not.",                                  "— Benjamin Franklin"},
        {"The two most powerful warriors are patience and time.",              "— Leo Tolstoy"},
        {"Yesterday is gone. Tomorrow has not yet come. We have only today.",  "— Mother Teresa"},
        {"Until we can manage time, we can manage nothing else.",              "— Peter Drucker"},
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
    // V4.2 #6 — slogan now uses Smiley Sans (得意黑) at ~36px for the
    // "mature, punchy headline" feel the user asked for.
    QString slogenFam = FontLoader::slogenFamily();
    if (!slogenFam.isEmpty()) qf.setFamily(slogenFam);
    qf.setPointSize(27);   // ~36px @ 96dpi
    qf.setWeight(QFont::Bold);
    qf.setLetterSpacing(QFont::AbsoluteSpacing, -0.5);
    m_quoteLab->setFont(qf);
    // V4.2 #2 — give descenders room
    m_quoteLab->setMinimumHeight(48);
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
    connect(&I18n::instance(),  &I18n::languageChanged, this, [this]{
        // Tell parent to refresh via its own refresh call; for safety also
        // re-style and re-pick the quote so language change reflects immediately.
        QString q = pickDailyQuote();
        QStringList parts = q.split('|');
        m_quoteLab->setText(parts.value(0));
        m_attributeLab->setText(parts.value(1));
    });
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

    if (total <= 0) return I18n::t("insight.empty");

    int streak = 0;
    for (int i = daily.size() - 1; i >= 0; --i) {
        if (daily[i].totalMinutes > 0) ++streak; else break;
    }

    QString topCatKey;
    qint64 topMin = 0;
    double topPct = 0;
    for (const auto &s : stats) {
        if (s.totalMinutes > topMin) {
            topMin = s.totalMinutes;
            switch (s.category) {
                case EventCategory::Work:          topCatKey = "cat.work"; break;
                case EventCategory::Study:         topCatKey = "cat.study"; break;
                case EventCategory::Entertainment: topCatKey = "cat.entertainment"; break;
                case EventCategory::Exercise:      topCatKey = "cat.exercise"; break;
                case EventCategory::Rest:          topCatKey = "cat.rest"; break;
                case EventCategory::Social:        topCatKey = "cat.social"; break;
                case EventCategory::Personal:      topCatKey = "cat.personal"; break;
                case EventCategory::Other:         topCatKey = "cat.other"; break;
            }
            topPct = s.percentage;
        }
    }

    QStringList candidates;
    if (streak >= 7) {
        candidates << I18n::t("insight.streak_high_fmt").arg(streak);
    } else if (streak >= 3) {
        candidates << I18n::t("insight.streak_med_fmt").arg(streak);
    }

    if (!topCatKey.isEmpty() && topPct >= 35) {
        candidates << I18n::t("insight.top_cat_fmt").arg(I18n::t(topCatKey)).arg(int(topPct));
    }

    qint64 avgPerDay = total / qMax(1, int(start.daysTo(end)));
    if (avgPerDay >= 240) {
        candidates << I18n::t("insight.avg_high_fmt").arg(avgPerDay / 60);
    } else if (avgPerDay >= 60) {
        candidates << I18n::t("insight.avg_normal_fmt").arg(avgPerDay / 60);
    }

    QDate today = QDate::currentDate();
    QDateTime tStart(today, QTime(0, 0));
    QDateTime tEnd(today, QTime(23, 59, 59));
    auto todayEvents = m_db->getEventsByRange(tStart, tEnd);
    int todayCnt = 0;
    for (const auto &e : todayEvents)
        if (e.startDate.date() == today) todayCnt++;
    if (todayCnt == 0) {
        candidates << I18n::t("insight.today_empty");
    } else if (todayCnt <= 2) {
        candidates << I18n::t("insight.today_light_fmt").arg(todayCnt);
    } else {
        candidates << I18n::t("insight.today_busy_fmt").arg(todayCnt);
    }

    if (candidates.isEmpty()) return I18n::t("insight.default");
    int idx = QDate::currentDate().dayOfYear() % candidates.size();
    return candidates[idx];
}

void MotivationWidget::refresh(const QDateTime &start, const QDateTime &end) {
    auto &t = Theme::instance();
    QString q = pickDailyQuote();
    QStringList parts = q.split('|');
    QString quoteText = parts.value(0);
    m_quoteLab->setText(quoteText);
    m_attributeLab->setText(parts.value(1));

    QString insight = buildInsight(start, end);
    m_insightLab->setText(insight);

    QString brand = t.brand().name();

    // V4.2 #6 — 根据 quote 文本是否包含中文字符选择字体：
    //   含中文 → Smiley Sans (得意黑) 粗体
    //   纯英文 → IBM Plex Serif 斜体粗体
    // 字号必须写进 stylesheet，否则会被全局 QSS cascade 重置为 15px。
    bool hasChinese = false;
    for (QChar c : quoteText) {
        // CJK Unified Ideographs
        if (c.unicode() >= 0x4E00 && c.unicode() <= 0x9FFF) { hasChinese = true; break; }
    }

    QString slogenFont;
    QString italicCss;
    if (hasChinese) {
        slogenFont = FontLoader::slogenFamily();         // Smiley Sans
        if (slogenFont.isEmpty()) slogenFont = FontLoader::cjkFamily();
        italicCss = "";
    } else {
        slogenFont = FontLoader::serifFamily();          // IBM Plex Serif
        if (slogenFont.isEmpty()) slogenFont = FontLoader::primaryFamily();
        italicCss = "font-style:italic;";
    }

    m_quoteLab->setStyleSheet(QString(
        "color:%1;background:transparent;"
        "font-family:\"%2\";"
        "font-size:36px;font-weight:700;%3"
        "letter-spacing:-0.5px;line-height:1.3;")
        .arg(t.textPrimary().name())
        .arg(slogenFont)
        .arg(italicCss));
    m_quoteLab->setMinimumHeight(56);  // 36px line + descender breathing room

    m_attributeLab->setStyleSheet(QString("color:%1;background:transparent;font-size:14px;")
                                       .arg(t.textSecondary().name()));
    m_insightLab->setStyleSheet(QString("color:%1;background:transparent;font-size:15px;font-weight:600;")
                                     .arg(brand));
}

void MotivationWidget::applyTheme() {
    // 实际颜色在 refresh 时刷新，这里仅保留背景
    setStyleSheet("background:transparent;");
}

} // namespace timemaster
