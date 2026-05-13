//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QBoxLayout>
#include <QVBoxLayout>
#include "../../core/Types.h"

namespace timemaster {

// KPI 统计卡片行：总时长/事件数/日均/高峰日 四个指标卡片
class StatsCardsWidget : public QWidget {
    Q_OBJECT
public:
    // 构造函数：初始化四个 KPI 卡片并连接主题/语言信号
    explicit StatsCardsWidget(QWidget *parent = nullptr);

    // 设置总时长（分钟），更新卡片显示值
    void setTotal(qint64 minutes);
    // 设置事件总数，更新卡片显示值
    void setCount(int count);
    // 设置日均时长，附带低/正常/高三种状态文案
    void setDailyAvg(qint64 minutes);
    // 设置高峰日，显示月/日+星期，品牌色强调
    void setPeakDay(const QDate &date);

private slots:
    // 主题变更时重新应用所有卡片的样式
    void applyTheme();

private:
    struct Card {
        QFrame *frame = nullptr;
        QLabel *caption = nullptr;
        QLabel *value = nullptr;
        QLabel *sub = nullptr;
    };
    Card m_totalCard;
    Card m_countCard;
    Card m_avgCard;
    Card m_peakCard;

    // 记下最近一次设置的内容，主题切换后重新应用样式
    qint64 m_lastTotal = 0;
    int    m_lastCount = 0;
    qint64 m_lastAvg = 0;
    QDate  m_lastPeak;

    // 创建单个 KPI 卡片：Frame + 标题 + 数值 + 副标题
    void setupCard(Card &card, const QString &caption, QBoxLayout *row);
    // 对单个卡片应用背景/边框/圆角/阴影样式
    void applyCardStyle(Card &card);
    // 将分钟格式化为可读字符串（如 2h 30m）
    QString formatMinutes(qint64 min) const;
};

} // namespace timemaster
