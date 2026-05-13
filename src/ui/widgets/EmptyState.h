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
class EmptyState : public QWidget {
    Q_OBJECT
public:
    explicit EmptyState(QWidget *parent = nullptr);

    void setTitle(const QString &t);
    void setSubtitle(const QString &t);
    void setProgress(const QString &t);     // 留空则隐藏
    void clearActions();
    void addAction(const QString &label, const std::function<void()> &cb);

private slots:
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
