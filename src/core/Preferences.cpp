//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#include "Preferences.h"
#include <QDate>

namespace timemaster {

Preferences &Preferences::instance() {
    static Preferences p;
    return p;
}

Preferences::Preferences(QObject *parent) : QObject(parent) {}

bool Preferences::weekStartsMonday() const {
    return m_settings.value("week_starts_monday", true).toBool();
}

void Preferences::setWeekStartsMonday(bool v) {
    if (weekStartsMonday() == v) return;
    m_settings.setValue("week_starts_monday", v);
    emit weekStartChanged();
}

int Preferences::weekColumnOf(const QDate &d) const {
    // Qt: dayOfWeek() returns Mon=1..Sun=7
    int dow = d.dayOfWeek();
    if (weekStartsMonday()) {
        return dow - 1;          // Mon=0..Sun=6
    } else {
        return dow % 7;          // Sun=0, Mon=1, ..., Sat=6
    }
}

QDate Preferences::weekStartOf(const QDate &d) const {
    return d.addDays(-weekColumnOf(d));
}

// ---- permissions ----

bool Preferences::autoApproveAdd() const {
    return m_settings.value("chat_auto_approve_add", false).toBool();
}
void Preferences::setAutoApproveAdd(bool v) {
    if (autoApproveAdd() == v) return;
    m_settings.setValue("chat_auto_approve_add", v);
    emit permissionsChanged();
}
bool Preferences::autoApproveDelete() const {
    return m_settings.value("chat_auto_approve_delete", false).toBool();
}
void Preferences::setAutoApproveDelete(bool v) {
    if (autoApproveDelete() == v) return;
    m_settings.setValue("chat_auto_approve_delete", v);
    emit permissionsChanged();
}
bool Preferences::autoApproveUpdate() const {
    return m_settings.value("chat_auto_approve_update", false).toBool();
}
void Preferences::setAutoApproveUpdate(bool v) {
    if (autoApproveUpdate() == v) return;
    m_settings.setValue("chat_auto_approve_update", v);
    emit permissionsChanged();
}

void Preferences::resetAllAutoApprovals() {
    m_settings.setValue("chat_auto_approve_add",    false);
    m_settings.setValue("chat_auto_approve_delete", false);
    m_settings.setValue("chat_auto_approve_update", false);
    emit permissionsChanged();
}

} // namespace timemaster
