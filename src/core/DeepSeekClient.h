//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

#pragma once

#include "Types.h"
#include <QObject>
#include <QString>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace timemaster {

// DeepSeek API 客户端：支持流式聊天和日程解析，通过 SSE 协议通信
/**
 * DeepSeek API 客户端
 * - sendChat()：对话（流式）
 * - parseSchedule()：解析自然语言为日程列表（一次性返回）
 *
 * 支持注入日历上下文：调用 sendChat 时传入 calendarContext，
 * 客户端会把它作为额外的 system message 注入到对话里。
 *
 * V4.3.3 #1 — 支持多轮对话历史。调用方（ChatPage）维护一份 history（user/
 * assistant 交替的 QJsonArray），每次调用 sendChat 时把它注入到 messages
 * 数组里。这样小师才会记得师傅上一句说了什么；否则每一句都像新开了对话。
 */
class DeepSeekClient : public QObject {
    Q_OBJECT
public:
    // 构造函数：初始化网络管理器
    explicit DeepSeekClient(QObject *parent = nullptr);

    // 设置 API Bearer Token
    void setApiKey(const QString &key);
    // 获取当前 API Key
    QString apiKey() const { return m_apiKey; }
    // 检查是否已配置 API Key
    bool hasApiKey() const { return !m_apiKey.isEmpty(); }

    // 自定义 API 端点（默认 api.deepseek.com）
    void setBaseUrl(const QString &url);
    // 获取当前端点地址
    QString baseUrl() const { return m_baseUrl; }
    // 设置模型名称（默认 deepseek-chat）
    void setModel(const QString &model);
    // 获取当前模型名
    QString model() const { return m_model; }

    // 发送流式聊天请求（支持多轮对话记忆）
    // 对话：systemPrompt 是基础角色定义，calendarContext 是动态注入的日历快照，
    // history 是已确认过的 user/assistant 交替消息（V4.3.3 #1 新增多轮记忆）。
    void sendChat(const QString &userMessage,
                  const QString &systemPrompt = QString(),
                  const QString &calendarContext = QString(),
                  const QJsonArray &history = QJsonArray());

    // 解析自然语言日程文本为结构化事件列表
    // 解析日程：完成后通过 parseFinished 信号一次性返回
    void parseSchedule(const QString &text);

    // 取消正在进行的网络请求
    void abort();

signals:
    // 流式响应增量信号
    void chatChunk(const QString &delta);
    // 聊天响应完成信号
    void chatFinished(const QString &full);
    // 聊天出错信号
    void chatError(const QString &message);

    // 日程解析完成信号
    void parseFinished(const QList<ScheduleSuggestion> &suggestions);
    // 日程解析出错信号
    void parseError(const QString &message);

private slots:
    // 读取网络响应数据
    void onReadyRead();
    // 网络请求完成处理
    void onFinished();

private:
    // 构建并发送流式 POST 请求
    void sendRequestStream(const QJsonArray &messages, bool forParse);
    // 解析 SSE 事件流缓冲区
    void processBuffer();
    // 从 AI 返回文本中提取 JSON 并解析为 ScheduleSuggestion 列表
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
