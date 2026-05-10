#pragma once

#include "Types.h"
#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace timeplan {

/**
 * DeepSeek API 客户端
 * - 支持 SSE 流式
 * - sendChat()：通用对话
 * - parseSchedule()：解析自然语言为日程列表
 */
class DeepSeekClient : public QObject {
    Q_OBJECT
public:
    explicit DeepSeekClient(QObject *parent = nullptr);

    void setApiKey(const QString &key);
    QString apiKey() const { return m_apiKey; }
    bool hasApiKey() const { return !m_apiKey.isEmpty(); }

    // 对话：流式增量返回
    void sendChat(const QString &userMessage, const QString &systemPrompt = QString());

    // 解析日程：完成后通过 scheduleParsed 信号一次性返回
    void parseSchedule(const QString &text);

    void abort();

signals:
    void chatChunk(const QString &delta);     // 流式增量
    void chatFinished(const QString &full);   // 完成
    void chatError(const QString &message);

    void parseFinished(const QList<ScheduleSuggestion> &suggestions);
    void parseError(const QString &message);

private slots:
    void onReadyRead();
    void onFinished();

private:
    void sendRequestStream(const QJsonArray &messages, bool forParse);
    void processBuffer();
    QList<ScheduleSuggestion> parseJsonResponse(const QString &raw);

    QNetworkAccessManager *m_nam;
    QNetworkReply *m_reply = nullptr;
    QString m_apiKey;
    QString m_buffer;       // SSE 接收 buffer
    QString m_fullText;     // 拼接的完整文本
    bool m_isParseMode = false;
};

} // namespace timeplan
