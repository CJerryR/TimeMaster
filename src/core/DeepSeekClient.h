//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include "Types.h"
#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace timemaster {

/**
 * DeepSeek API 客户端
 * - sendChat()：对话（流式）
 * - parseSchedule()：解析自然语言为日程列表（一次性返回）
 *
 * 支持注入日历上下文：调用 sendChat 时传入 calendarContext，
 * 客户端会把它作为额外的 system message 注入到对话里。
 */
class DeepSeekClient : public QObject {
    Q_OBJECT
public:
    explicit DeepSeekClient(QObject *parent = nullptr);

    void setApiKey(const QString &key);
    QString apiKey() const { return m_apiKey; }
    bool hasApiKey() const { return !m_apiKey.isEmpty(); }

    void setBaseUrl(const QString &url);
    QString baseUrl() const { return m_baseUrl; }
    void setModel(const QString &model);
    QString model() const { return m_model; }

    // 对话：systemPrompt 是基础角色定义，calendarContext 是动态注入的日历快照
    void sendChat(const QString &userMessage,
                  const QString &systemPrompt = QString(),
                  const QString &calendarContext = QString());

    // 解析日程：完成后通过 parseFinished 信号一次性返回
    void parseSchedule(const QString &text);

    void abort();

signals:
    void chatChunk(const QString &delta);
    void chatFinished(const QString &full);
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
    QString m_baseUrl;
    QString m_model;
    QString m_buffer;
    QString m_fullText;
    bool m_isParseMode = false;
};

} // namespace timemaster
