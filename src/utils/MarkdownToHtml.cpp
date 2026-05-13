//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#include "MarkdownToHtml.h"

#include <QRegularExpression>
#include <QStringList>

namespace timemaster {

namespace {

// HTML 实体转义：& < > 防止注入
QString escapeHtml(const QString &s) {
    QString r = s;
    r.replace('&', "&amp;");
    r.replace('<', "&lt;");
    r.replace('>', "&gt;");
    return r;
}

// 处理一行内的行内语法（粗体、斜体、行内代码、删除线）
// 处理行内语法：行内代码 -> 粗体 -> 斜体 -> 删除线
QString applyInline(QString line) {
    // 1) 行内代码 `...` —— 最先处理，里面不再做替换
    static const QRegularExpression reCode(R"(`([^`]+)`)");
    QString out;
    int last = 0;
    auto it = reCode.globalMatch(line);
    while (it.hasNext()) {
        auto m = it.next();
        out += line.mid(last, m.capturedStart() - last);
        out += "<code style=\"background:rgba(120,110,90,0.15);padding:1px 5px;"
               "border-radius:4px;font-family:Consolas,monospace;\">"
            + escapeHtml(m.captured(1)) + "</code>";
        last = m.capturedEnd();
    }
    out += line.mid(last);
    line = out;

    // 2) **粗体**
    line.replace(QRegularExpression(R"(\*\*([^*]+)\*\*)"), "<b>\\1</b>");
    // 3) *斜体* （不与 ** 冲突，因为 ** 已经被替换掉了）
    line.replace(QRegularExpression(R"(\*([^*]+)\*)"), "<i>\\1</i>");
    // 4) ~~删除线~~
    line.replace(QRegularExpression(R"(~~([^~]+)~~)"), "<s>\\1</s>");

    return line;
}

} // namespace

// 主转换入口：逐行解析 Markdown，输出带内联样式的 HTML
QString MarkdownToHtml::convert(const QString &md) {
    if (md.isEmpty()) return {};

    QStringList lines = md.split('\n');
    QString html;
    bool inCodeBlock = false;
    QString codeBuf;
    QString listType; // "ul" / "ol" / ""

    auto closeList = [&]() {
        if (!listType.isEmpty()) {
            html += "</" + listType + ">";
            listType.clear();
        }
    };

    for (int i = 0; i < lines.size(); ++i) {
        QString raw = lines[i];

        // 代码块围栏 ```
        if (raw.startsWith("```")) {
            if (inCodeBlock) {
                html += "<pre style=\"background:rgba(120,110,90,0.12);padding:8px 10px;"
                        "border-radius:8px;font-family:Consolas,monospace;font-size:12px;"
                        "white-space:pre-wrap;margin:6px 0;\">"
                     + escapeHtml(codeBuf) + "</pre>";
                codeBuf.clear();
                inCodeBlock = false;
            } else {
                closeList();
                inCodeBlock = true;
                codeBuf.clear();
            }
            continue;
        }
        if (inCodeBlock) {
            if (!codeBuf.isEmpty()) codeBuf += "\n";
            codeBuf += raw;
            continue;
        }

        QString trimmed = raw.trimmed();

        // 空行 → 段落分隔
        if (trimmed.isEmpty()) {
            closeList();
            continue;
        }

        // 标题
        if (trimmed.startsWith("### ")) {
            closeList();
            html += "<div style=\"font-size:14px;font-weight:600;margin:6px 0 2px;\">"
                 + applyInline(escapeHtml(trimmed.mid(4))) + "</div>";
            continue;
        }
        if (trimmed.startsWith("## ")) {
            closeList();
            html += "<div style=\"font-size:15px;font-weight:700;margin:8px 0 3px;\">"
                 + applyInline(escapeHtml(trimmed.mid(3))) + "</div>";
            continue;
        }
        if (trimmed.startsWith("# ")) {
            closeList();
            html += "<div style=\"font-size:16px;font-weight:700;margin:8px 0 4px;\">"
                 + applyInline(escapeHtml(trimmed.mid(2))) + "</div>";
            continue;
        }

        // 无序列表 - 或 *
        static const QRegularExpression reUl(R"(^[\-\*\u2022]\s+(.+)$)");
        // 有序列表 1.
        static const QRegularExpression reOl(R"(^(\d+)\.\s+(.+)$)");
        auto mUl = reUl.match(trimmed);
        auto mOl = reOl.match(trimmed);
        if (mUl.hasMatch()) {
            if (listType != "ul") { closeList(); html += "<ul style=\"margin:4px 0;padding-left:18px;\">"; listType = "ul"; }
            html += "<li>" + applyInline(escapeHtml(mUl.captured(1))) + "</li>";
            continue;
        }
        if (mOl.hasMatch()) {
            if (listType != "ol") { closeList(); html += "<ol style=\"margin:4px 0;padding-left:22px;\">"; listType = "ol"; }
            html += "<li>" + applyInline(escapeHtml(mOl.captured(2))) + "</li>";
            continue;
        }

        // 引用块
        if (trimmed.startsWith("> ")) {
            closeList();
            html += "<div style=\"border-left:3px solid rgba(217,119,87,0.5);"
                    "padding:2px 10px;margin:4px 0;color:#9a8b78;\">"
                 + applyInline(escapeHtml(trimmed.mid(2))) + "</div>";
            continue;
        }

        // 普通段落（首行外用 div，便于流式裸文本也能渲染）
        closeList();
        html += "<div>" + applyInline(escapeHtml(trimmed)) + "</div>";
    }

    // 收尾：未闭合的代码块也输出
    if (inCodeBlock && !codeBuf.isEmpty()) {
        html += "<pre style=\"background:rgba(120,110,90,0.12);padding:8px 10px;"
                "border-radius:8px;font-family:Consolas,monospace;font-size:12px;"
                "white-space:pre-wrap;margin:6px 0;\">"
             + escapeHtml(codeBuf) + "</pre>";
    }
    closeList();

    return html;
}

} // namespace timemaster
