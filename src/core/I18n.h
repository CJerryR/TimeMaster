#pragma once

#include <QObject>
#include <QString>
#include <QSettings>

namespace timemaster {

/**
 * 轻量级国际化：英文 / 简体中文 切换。
 * 通过单例存储语言，UI 在 languageChanged() 信号触发时 rebuild 文案。
 *
 * 使用：
 *   I18n::t("calendar.today")  -> "Today" 或 "今天"
 */
class I18n : public QObject {
    Q_OBJECT
public:
    enum Language { English = 0, Chinese = 1 };

    static I18n &instance();

    Language language() const { return m_lang; }
    void setLanguage(Language l);
    void toggle();
    bool isEnglish() const { return m_lang == English; }

    // 主翻译入口：用稳定的英文 key 查表
    static QString t(const QString &key);

signals:
    void languageChanged();

private:
    explicit I18n(QObject *parent = nullptr);
    Language m_lang = English;
    QSettings m_settings;
};

// 便捷宏 / 函数
inline QString tr_(const QString &key) { return I18n::t(key); }

} // namespace timemaster
