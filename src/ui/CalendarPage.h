//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include "../core/Types.h"
#include <QWidget>

class QStackedWidget;
class QLabel;
class QLineEdit;
class QPushButton;

namespace timemaster {

class Database;
class DeepSeekClient;
class MonthView;
class TimeGridView;
class EmptyState;

/**
 * V4.1 calendar page.
 *  · Row 1 header: [<] [>] Title [Today]    [Day|Week|Month]  [+ New Event]
 *  · Row 2 (always visible): compact AI parse input + [History] button
 *  · Row 3 (expandable, hidden by default): full input + [Parse] + [X] +
 *    status/hint. Triggered by clicking the compact bar OR pressing Ctrl+K.
 *    Auto-collapses on focus loss or X click.
 *  · Empty state overlay (EmptyState) when no events exist.
 */
// 日历主页面：顶栏标题/导航/视图切换/+新建，紧凑 AI 解析条，展开式浮动 AI 面板，视图栈（月/周/日），空状态覆盖
class CalendarPage : public QWidget {
    Q_OBJECT
public:
    // 构造函数：构建 UI、连接所有信号、初始刷新
    explicit CalendarPage(Database *db, DeepSeekClient *ai, QWidget *parent = nullptr);

    // 切换日/周/月视图模式
    void setView(CalendarView v);
    // 返回当前视图模式
    CalendarView currentView() const { return m_view; }

    // 跳转到今天
    void goToday();
    // 跳转到下一段（日/周/月）
    void goNext();
    // 跳转到上一段（日/周/月）
    void goPrev();
    // 打开新建日程对话框，可选预设时间
    void openCreateDialog(const QDateTime &dt = QDateTime());

protected:
    // 事件过滤器：点击紧凑输入栏展开 AI 面板，焦点丢失自动折叠
    bool eventFilter(QObject *o, QEvent *e) override;
    // 窗口大小变化时重新定位 AI 浮动面板
    void resizeEvent(QResizeEvent *e) override;
    // 鼠标点击：点击 AI 浮动面板外部自动折叠
    void mousePressEvent(QMouseEvent *e) override;

public slots:
    // 刷新所有视图：重新查询数据并更新显示
    void refresh();

private slots:
    // 点击解析按钮：发送 AI 解析请求
    void onParseClicked();
    // AI 解析完成：展示解析结果对话框并导入选中项
    void onParseFinished(const QList<ScheduleSuggestion> &items);
    // AI 解析出错：显示错误提示
    void onParseError(const QString &msg);
    // 点击事件：打开编辑对话框
    void onEventClicked(const CalendarEvent &event);
    // 点击时间槽：打开新建事件对话框，预设时间
    void onTimeSlotClicked(const QDateTime &dt);
    // 月视图点击日期：切换到日视图
    void onMonthDateClicked(const QDate &d);
    // 月视图点击"+N"溢出：切换到日视图
    void onMonthOverflowClicked(const QDate &d, const QList<CalendarEvent> &events);
    // 打开 AI 解析历史对话框
    void onHistoryClicked();
    // 刷新空状态覆盖层：有事件时隐藏，无事件时显示快捷模板
    void refreshEmptyState();

    // 应用主题样式：刷新全局 QSS 和图标颜色
    void applyTheme();
    // 应用语言设置：更新所有文本标签
    void applyLanguage();

    // 展开 AI 浮动面板
    void expandAiPanel();
    // 折叠 AI 浮动面板
    void collapseAiPanel();

private:
    // 构建界面：创建所有控件和布局
    void buildUi();
    // 更新标题栏文字（含日期范围）
    void updateHeader();
    // 切换视图栈的选中页（月/周/日）
    void switchViewWidget();
    // 插入快速模板事件集（0=晨间,1=深度工作,2=周复盘）
    void insertTemplate(int which);
    // 返回本地化的月份名称
    QString monthName(int month) const;
    // V4.2 #5: position the floating Spotlight-style popup below the compact pill.
    // 定位 AI 浮动面板到紧凑输入栏下方居中
    void positionAiPopup();

    Database *m_db;
    DeepSeekClient *m_ai;

    CalendarView m_view = CalendarView::Month;
    QDate m_currentDate;
    QList<CalendarEvent> m_events;
    QString m_pendingParseText;

    // header row 1
    QLabel      *m_titleLabel  = nullptr;
    QPushButton *m_btnDay      = nullptr;
    QPushButton *m_btnWeek     = nullptr;
    QPushButton *m_btnMonth    = nullptr;
    QPushButton *m_btnPrev     = nullptr;
    QPushButton *m_btnNext     = nullptr;
    QPushButton *m_btnToday    = nullptr;
    QPushButton *m_btnAdd      = nullptr;

    // row 2: always-visible compact AI bar
    QWidget     *m_aiBar          = nullptr;
    QLineEdit   *m_aiCompactInput = nullptr;
    QPushButton *m_historyButton  = nullptr;

    // row 3: expandable AI panel
    QWidget     *m_aiExpanded   = nullptr;
    QLabel      *m_aiPanelTitle = nullptr;
    QLineEdit   *m_parseInput   = nullptr;
    QPushButton *m_parseButton  = nullptr;
    QPushButton *m_aiCloseBtn   = nullptr;
    QLabel      *m_aiHintLabel  = nullptr;
    QLabel      *m_parseStatus  = nullptr;

    QStackedWidget *m_stack     = nullptr;
    MonthView      *m_monthView = nullptr;
    TimeGridView   *m_weekView  = nullptr;
    TimeGridView   *m_dayView   = nullptr;

    EmptyState     *m_emptyState = nullptr;
    QWidget        *m_viewCard   = nullptr;
};

} // namespace timemaster
