#include "InsightsWidget.h"
#include "../Theme.h"
#include "../../core/Database.h"

#include <QHBoxLayout>
#include <QFrame>
#include <algorithm>

namespace timemaster {

InsightsWidget::InsightsWidget(Database *db, QWidget *parent)
    : QWidget(parent), m_db(db) {
    setAttribute(Qt::WA_StyledBackground, false);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(4);

    m_title = new QLabel("💡  智能洞察");
    QFont tf;
    tf.setPointSize(11);
    tf.setWeight(QFont::DemiBold);
    m_title->setFont(tf);
    lay->addWidget(m_title);

    m_subtitle = new QLabel("从你的日程里发现的规律");
    QFont sf;
    sf.setPointSize(9);
    m_subtitle->setFont(sf);
    lay->addWidget(m_subtitle);

    auto *itemsContainer = new QWidget();
    itemsContainer->setStyleSheet("background:transparent;");
    m_itemsLayout = new QVBoxLayout(itemsContainer);
    m_itemsLayout->setContentsMargins(0, 8, 0, 0);
    m_itemsLayout->setSpacing(8);
    lay->addWidget(itemsContainer);
    lay->addStretch(1);

    applyTheme();
    connect(&Theme::instance(), &Theme::changed, this, [this]() { applyTheme(); });
}

void InsightsWidget::clearItems() {
    for (auto *w : m_itemWidgets) {
        m_itemsLayout->removeWidget(w);
        w->deleteLater();
    }
    m_itemWidgets.clear();
}

QList<InsightsWidget::InsightItem> InsightsWidget::compute(const QDateTime &start, const QDateTime &end) {
    QList<InsightItem> out;
    if (!m_db) return out;

    auto stats = m_db->getCategoryStats(start, end);
    auto daily = m_db->getDailySummaries(start, end);
    auto hourly = m_db->getHourlyDistribution(start, end);

    qint64 total = 0;
    for (auto &s : stats) total += s.totalMinutes;

    if (total <= 0) {
        out.append(InsightItem{"📭", "这段时间还没有记录任何事件,先添加几条试试吧。"});
        return out;
    }

    // 1) Top 类别
    std::sort(stats.begin(), stats.end(), [](const CategoryStat &a, const CategoryStat &b) {
        return a.totalMinutes > b.totalMinutes;
    });
    if (!stats.isEmpty() && stats.first().totalMinutes > 0) {
        double pct = double(stats.first().totalMinutes) / double(total) * 100.0;
        out.append(InsightItem{"🏆",
            QString("「%1」占用了你最多的时间,约 %2 小时(%3%)")
                .arg(categoryLabel(stats.first().category))
                .arg(QString::number(stats.first().totalMinutes / 60.0, 'f', 1))
                .arg(QString::number(pct, 'f', 0))});
    }

    // 2) 工作 vs 学习
    qint64 work = 0, study = 0;
    for (auto &s : stats) {
        if (s.category == EventCategory::Work) work = s.totalMinutes;
        if (s.category == EventCategory::Study) study = s.totalMinutes;
    }
    if (work > 0 && study > 0) {
        if (work > study * 2) {
            out.append(InsightItem{"💼", QString("工作时长是学习的 %1 倍,记得给自己留点充电时间。")
                .arg(QString::number(double(work) / double(study), 'f', 1))});
        } else if (study > work * 2) {
            out.append(InsightItem{"📚", QString("学习时长是工作的 %1 倍,这是个充电季!")
                .arg(QString::number(double(study) / double(work), 'f', 1))});
        } else {
            out.append(InsightItem{"⚖️", "工作与学习时长比较均衡,节奏不错。"});
        }
    } else if (work > 0 && study == 0) {
        out.append(InsightItem{"📚", "这段时间没有学习类事件,要不要安排一次?"});
    }

    // 3) 高产日 / 最忙日(按星期几汇总)
    if (!daily.isEmpty()) {
        QHash<int, qint64> byDow;
        QHash<int, int> dowCount;
        qint64 dayMax = 0;
        QDate peakDate;
        for (const auto &d : daily) {
            byDow[d.date.dayOfWeek()] += d.totalMinutes;
            dowCount[d.date.dayOfWeek()] += 1;
            if (d.totalMinutes > dayMax) {
                dayMax = d.totalMinutes;
                peakDate = d.date;
            }
        }
        int bestDow = 1;
        qint64 bestAvg = 0;
        for (auto it = byDow.constBegin(); it != byDow.constEnd(); ++it) {
            int cnt = dowCount.value(it.key(), 1);
            qint64 avg = cnt > 0 ? it.value() / cnt : 0;
            if (avg > bestAvg) { bestAvg = avg; bestDow = it.key(); }
        }
        const QStringList wk = {"周一","周二","周三","周四","周五","周六","周日"};
        int idx = bestDow - 1;
        if (idx >= 0 && idx < wk.size() && bestAvg > 0) {
            out.append(InsightItem{"🚀", QString("%1 通常是你最高产的一天,平均 %2 小时事件。")
                .arg(wk.at(idx))
                .arg(QString::number(bestAvg / 60.0, 'f', 1))});
        }
        if (peakDate.isValid() && dayMax > 0) {
            out.append(InsightItem{"🔥", QString("最忙的一天是 %1,共 %2 小时。")
                .arg(peakDate.toString("M月d日"))
                .arg(QString::number(dayMax / 60.0, 'f', 1))});
        }
    }

    // 4) 节奏:晨型 / 夜型
    if (!hourly.isEmpty()) {
        qint64 morning = 0, afternoon = 0, evening = 0, night = 0;
        for (const auto &b : hourly) {
            if (b.hour < 6)      night     += b.totalMinutes;
            else if (b.hour < 12) morning   += b.totalMinutes;
            else if (b.hour < 18) afternoon += b.totalMinutes;
            else                  evening   += b.totalMinutes;
        }
        QString type;
        if (morning > afternoon && morning > evening && morning > night) type = "晨型选手 🌅";
        else if (evening > morning && evening > afternoon) type = "夜型选手 🌙";
        else if (afternoon > morning && afternoon > evening) type = "午后型 ☀️";
        else type = "节奏均匀 ⚖️";
        out.append(InsightItem{"⏰", QString("你这段时间偏向 %1。").arg(type)});
    }

    // 5) 来源分布(轻量)
    int manualC  = m_db->eventCountBySource(EventSource::Manual,  start, end);
    int aiC      = m_db->eventCountBySource(EventSource::AiParse, start, end);
    int chatC    = m_db->eventCountBySource(EventSource::Chat,    start, end);
    int sum = manualC + aiC + chatC;
    if (sum > 0) {
        int aiTotal = aiC + chatC;
        if (aiTotal == 0) {
            out.append(InsightItem{"✋", "全部事件都是手动添加的,试试用 AI 解析快速导入吧。"});
        } else if (aiTotal > manualC) {
            out.append(InsightItem{"🤖", QString("AI 帮你导入了 %1 个事件,占比 %2%。")
                .arg(aiTotal)
                .arg(QString::number(double(aiTotal) / double(sum) * 100.0, 'f', 0))});
        }
    }

    return out;
}

void InsightsWidget::refresh(const QDateTime &start, const QDateTime &end) {
    auto items = compute(start, end);
    rebuild(items);
}

void InsightsWidget::rebuild(const QList<InsightItem> &items) {
    clearItems();
    auto &t = Theme::instance();

    if (items.isEmpty()) {
        auto *empty = new QLabel("暂无可洞察的数据");
        empty->setStyleSheet(QString("color:%1;background:transparent;").arg(t.textPlaceholder().name()));
        m_itemsLayout->addWidget(empty);
        m_itemWidgets.append(empty);
        return;
    }

    for (const auto &it : items) {
        auto *row = new QFrame();
        row->setObjectName("insightRow");
        row->setFrameShape(QFrame::NoFrame);
        auto *rl = new QHBoxLayout(row);
        rl->setContentsMargins(12, 10, 12, 10);
        rl->setSpacing(10);

        auto *iconLabel = new QLabel(it.icon);
        QFont icf;
        icf.setPointSize(14);
        iconLabel->setFont(icf);
        iconLabel->setFixedWidth(24);
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setStyleSheet("background:transparent;");
        rl->addWidget(iconLabel);

        auto *txt = new QLabel(it.text);
        txt->setWordWrap(true);
        QFont tf;
        tf.setPointSize(10);
        txt->setFont(tf);
        txt->setStyleSheet(QString("color:%1;background:transparent;").arg(t.textPrimary().name()));
        rl->addWidget(txt, 1);

        row->setStyleSheet(QString(
            "QFrame#insightRow{background:%1;border:1px solid %2;border-radius:10px;}"
        ).arg(t.cardBgHoverRgba()).arg(t.strokeRgba()));

        m_itemsLayout->addWidget(row);
        m_itemWidgets.append(row);
    }
}

void InsightsWidget::applyTheme() {
    auto &t = Theme::instance();
    m_title->setStyleSheet(QString("color:%1;background:transparent;").arg(t.textPrimary().name()));
    m_subtitle->setStyleSheet(QString("color:%1;background:transparent;").arg(t.textSecondary().name()));
    // 已有的 item 也需重新刷新颜色
    for (auto *w : m_itemWidgets) {
        if (auto *f = qobject_cast<QFrame *>(w)) {
            if (f->objectName() == "insightRow") {
                f->setStyleSheet(QString(
                    "QFrame#insightRow{background:%1;border:1px solid %2;border-radius:10px;}"
                ).arg(t.cardBgHoverRgba()).arg(t.strokeRgba()));
            }
        }
        // 文字颜色:刷新所有子 label
        for (auto *lbl : w->findChildren<QLabel *>()) {
            // 跳过 emoji icon(它没设主颜色,保持原色),只刷文字
            QString s = lbl->styleSheet();
            if (s.contains("color:")) {
                lbl->setStyleSheet(QString("color:%1;background:transparent;").arg(t.textPrimary().name()));
            }
        }
    }
}

} // namespace timemaster
