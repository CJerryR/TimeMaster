//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include <QObject>
#include <QString>
#include <QSettings>

namespace timemaster {

// 国际化单例：管理中英文翻译表，支持运行时切换，发射 languageChanged 信号驱动 UI 刷新

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
    // 语言类型
    enum Language { English = 0, Chinese = 1 };

    // 获取全局单例实例
    static I18n &instance();

    // 返回当前语言
    Language language() const { return m_lang; }
    // 切换语言并持久化，发射 languageChanged 信号
    void setLanguage(Language l);
    // 中英文来回切换
    void toggle();
    // 当前是否为英文
    bool isEnglish() const { return m_lang == English; }

    // 核心翻译函数：按 key 查找当前语言的对应文本
    // 主翻译入口：用稳定的英文 key 查表
    static QString t(const QString &key);

signals:
    // 语言切换后发射，驱动全部 UI 重设文字
    void languageChanged();

private:
    // 构造函数：从 QSettings 读取持久化语言设置
    explicit I18n(QObject *parent = nullptr);
    Language m_lang = English;
    QSettings m_settings;
};

// 便捷宏 / 函数
inline QString tr_(const QString &key) { return I18n::t(key); }

} // namespace timemaster
