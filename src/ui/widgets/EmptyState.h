//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include <QWidget>
#include <QVector>

class QLabel;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;

namespace timemaster {

/**
 * 通用空状态卡片 (V4 § 5)
 *  · 中央居中
 *  · 标题 (18px / 600) + 副文案 (14px / 400 / textSecondary)
 *  · 可选：3 个模板/动作按钮
 *  · 可选：进度提示 ("Added 1 / 3")
 *
 * 用法：
 *   auto *es = new EmptyState;
 *   es->setTitle(I18n::t("empty.cal.title"));
 *   es->setSubtitle(I18n::t("empty.cal.subtitle"));
 *   es->addAction(I18n::t("empty.cal.tmpl.morning"), [this]{ insertMorningTemplate(); });
 *   ...
 */
// 空状态占位卡片：居中标题+副标题+进度提示+操作按钮组，用于日历/分析/对话空数据时展示
class EmptyState : public QWidget {
    Q_OBJECT
public:
    // 构造函数：创建居中卡片布局（标题+副标题+进度+操作按钮）
    explicit EmptyState(QWidget *parent = nullptr);
    // 设置标题文本
    void setTitle(const QString &t);
    // 设置副标题文本
    void setSubtitle(const QString &t);
    // 设置进度提示文本（空字符串隐藏）
    void setProgress(const QString &t);     // 留空则隐藏
    // 清除所有操作按钮
    void clearActions();
    // 添加操作按钮：标签 + 点击回调
    void addAction(const QString &label, const std::function<void()> &cb);

private slots:
    // 主题变更时重新应用卡片/按钮/文字样式
    void applyTheme();

private:
    QLabel *m_title;
    QLabel *m_subtitle;
    QLabel *m_progress;
    QWidget *m_actionsRow;
    QHBoxLayout *m_actionsLayout;
    QVector<QPushButton*> m_actionButtons;
};

} // namespace timemaster
