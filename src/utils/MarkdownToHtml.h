//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include <QString>

namespace timemaster {

/**
 * 轻量 Markdown → HTML 转换器（仅支持聊天气泡常用语法）
 *  · **bold**  *italic*  `code`  ```code block```
 *  · # / ## / ### 标题
 *  · - / * / 1. 列表
 *  · 段落换行（双换行）
 *  · 自动转义 < > & 避免 HTML 注入
 * 设计目标：流式增量也能渲染（即使是半段也能输出合理的 HTML）。
 */
class MarkdownToHtml {
public:
    static QString convert(const QString &md);
};

} // namespace timemaster
