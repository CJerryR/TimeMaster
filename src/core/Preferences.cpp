//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#include "Preferences.h"
#include <QDate>

namespace timemaster {

// 获取全局单例实例
Preferences &Preferences::instance() {
    static Preferences p;
    return p;
}

// 构造函数
Preferences::Preferences(QObject *parent) : QObject(parent) {}

// 判断周起始日偏好
bool Preferences::weekStartsMonday() const {
    return m_settings.value("week_starts_monday", true).toBool();
}

// 设置周起始日
void Preferences::setWeekStartsMonday(bool v) {
    if (weekStartsMonday() == v) return;
    m_settings.setValue("week_starts_monday", v);
    emit weekStartChanged();
}

// 按周起始规则计算日期列索引
int Preferences::weekColumnOf(const QDate &d) const {
    // Qt: dayOfWeek() returns Mon=1..Sun=7
    int dow = d.dayOfWeek();
    if (weekStartsMonday()) {
        return dow - 1;          // Mon=0..Sun=6
    } else {
        return dow % 7;          // Sun=0, Mon=1, ..., Sat=6
    }
}

// 计算日期所在周的第一天
QDate Preferences::weekStartOf(const QDate &d) const {
    return d.addDays(-weekColumnOf(d));
}

// ---- permissions ----

// AI 添加日程免审批状态
bool Preferences::autoApproveAdd() const {
    return m_settings.value("chat_auto_approve_add", false).toBool();
}
// 设置添加免审批
void Preferences::setAutoApproveAdd(bool v) {
    if (autoApproveAdd() == v) return;
    m_settings.setValue("chat_auto_approve_add", v);
    emit permissionsChanged();
}
// AI 删除日程免审批状态
bool Preferences::autoApproveDelete() const {
    return m_settings.value("chat_auto_approve_delete", false).toBool();
}
// 设置删除免审批
void Preferences::setAutoApproveDelete(bool v) {
    if (autoApproveDelete() == v) return;
    m_settings.setValue("chat_auto_approve_delete", v);
    emit permissionsChanged();
}
// AI 修改日程免审批状态
bool Preferences::autoApproveUpdate() const {
    return m_settings.value("chat_auto_approve_update", false).toBool();
}
// 设置修改免审批
void Preferences::setAutoApproveUpdate(bool v) {
    if (autoApproveUpdate() == v) return;
    m_settings.setValue("chat_auto_approve_update", v);
    emit permissionsChanged();
}

// 一键关闭全部免审批
void Preferences::resetAllAutoApprovals() {
    m_settings.setValue("chat_auto_approve_add",    false);
    m_settings.setValue("chat_auto_approve_delete", false);
    m_settings.setValue("chat_auto_approve_update", false);
    emit permissionsChanged();
}

} // namespace timemaster
