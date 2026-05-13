//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include <QObject>
#include <QString>
#include <QSettings>

namespace timemaster {

// 用户偏好单例：管理周起始日、AI 自动审批权限等持久化设置

/**
 * V4.3 — 集中读取/写入用户偏好。之前各 view 自行 hardcode "周一为每周起点"，
 * 当用户切换偏好时无处统一调整，这里收一下。
 *
 * 也作为审批模式开关的存放点。
 */
class Preferences : public QObject {
    Q_OBJECT
public:
    static Preferences &instance();

    // ---- 日历 ----
    // 周起始日：true = 周一开始（默认，符合 ISO 8601 + 国内习惯）
    //          false = 周日开始（北美习惯）
    // 周一是否为每周第一天（否则周日为第一天）
    bool weekStartsMonday() const;
    // 设置周起始日偏好
    void setWeekStartsMonday(bool v);

    // 给定 QDate，返回它的 "本周第几列"（0..6），起点由 weekStartsMonday() 决定。
    //   weekStartsMonday=true:  Mon=0, Tue=1, ..., Sun=6
    //   weekStartsMonday=false: Sun=0, Mon=1, ..., Sat=6
    // 计算某日期在当前周起始规则下的列索引（0-6）
    int weekColumnOf(const class QDate &d) const;

    // 给定 QDate，返回它所在周的第一天 (Mon or Sun，按设置)
    // 计算某日期所在周的第一天
    class QDate weekStartOf(const class QDate &d) const;

    // ---- AI 对话权限（V4.3 #7） ----
    // 三个独立开关：增 / 删 / 改 都可以单独 "总是允许"。第一次默认 false（每次询问）。
    // AI 添加日程是否免审批
    bool autoApproveAdd() const;
    // 设置添加免审批
    void setAutoApproveAdd(bool v);
    // AI 删除日程是否免审批
    bool autoApproveDelete() const;
    // 设置删除免审批
    void setAutoApproveDelete(bool v);
    // AI 修改日程是否免审批
    bool autoApproveUpdate() const;
    // 设置修改免审批
    void setAutoApproveUpdate(bool v);
    // 一键重置所有自动批准（用户后悔了）
    // 一键关闭全部免审批
    void resetAllAutoApprovals();

signals:
    // 周起始日变更信号
    // 周起始日变化时通知所有 view 重绘
    void weekStartChanged();
    // AI 权限变更信号
    void permissionsChanged();

private:
    explicit Preferences(QObject *parent = nullptr);
    QSettings m_settings;
};

} // namespace timemaster
