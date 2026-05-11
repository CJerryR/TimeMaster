#include "DeepSeekClient.h"

#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QRegularExpression>
#include <QDebug>

namespace timemaster {

namespace {
constexpr const char *kDefaultBaseUrl = "https://api.deepseek.com/v1/chat/completions";
constexpr const char *kDefaultModel = "deepseek-chat";

const QString kSystemPromptChat = QStringLiteral(
    "你是「时间管理大师」（Time Master）—— 一位专业的时间规划顾问，"
    "用简洁、结构化、可操作的中文与用户交流。"
    "你的回复要点明确，避免啰嗦。当用户询问日程时，参考下方提供的「用户当前日历」实事求是地回答；"
    "如果某天没有安排，直接告诉用户「这天目前没有安排」，不要编造。"
);
} // namespace

DeepSeekClient::DeepSeekClient(QObject *parent)
    : QObject(parent),
      m_baseUrl(kDefaultBaseUrl),
      m_model(kDefaultModel)
{
    m_nam = new QNetworkAccessManager(this);
}

void DeepSeekClient::setApiKey(const QString &key) {
    m_apiKey = key.trimmed();
}

void DeepSeekClient::setBaseUrl(const QString &url) {
    m_baseUrl = url.isEmpty() ? QString(kDefaultBaseUrl) : url;
}

void DeepSeekClient::setModel(const QString &model) {
    m_model = model.isEmpty() ? QString(kDefaultModel) : model;
}

void DeepSeekClient::abort() {
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
}

void DeepSeekClient::sendChat(const QString &userMessage,
                              const QString &systemPrompt,
                              const QString &calendarContext) {
    if (!hasApiKey()) {
        emit chatError("未配置 DeepSeek API Key，请到「设置」填写。");
        return;
    }
    abort();
    m_isParseMode = false;
    m_fullText.clear();
    m_buffer.clear();

    QJsonArray messages;

    // 第一条 system：基础角色
    QJsonObject sys;
    sys["role"] = "system";
    sys["content"] = systemPrompt.isEmpty() ? kSystemPromptChat : systemPrompt;
    messages.append(sys);

    // 第二条 system：日历上下文（可选）
    if (!calendarContext.isEmpty()) {
        QJsonObject ctx;
        ctx["role"] = "system";
        ctx["content"] = calendarContext;
        messages.append(ctx);
    }

    QJsonObject usr;
    usr["role"] = "user";
    usr["content"] = userMessage;
    messages.append(usr);

    sendRequestStream(messages, false);
}

void DeepSeekClient::parseSchedule(const QString &text) {
    if (!hasApiKey()) {
        emit parseError("未配置 DeepSeek API Key");
        return;
    }
    abort();
    m_isParseMode = true;
    m_fullText.clear();
    m_buffer.clear();

    QDateTime now = QDateTime::currentDateTime();
    QString todayIso = now.toString("yyyy-MM-dd");
    static const char *weekdays[] = {"周日","周一","周二","周三","周四","周五","周六"};
    int wd = now.date().dayOfWeek() % 7;
    if (wd < 0) wd = 0;

    QString sys = QString(
        "你是日程解析专家。请严格输出 JSON，无任何额外解释。\n"
        "规则：\n"
        "1. 所有日期时间必须 ISO 8601 格式，如 \"%1T14:00:00\"。绝对不能用中文。\n"
        "2. \"明天\"=今天的下一天；\"后天\"=后两天；\"下周一\"=下一个周一。\n"
        "3. 没有指定日期默认 %1。没有具体时间：上午=09:00，下午=14:00，晚上=19:00。\n"
        "4. category 取值：work/study/entertainment/exercise/rest/social/personal/other。\n"
        "5. priority 取值：urgent/normal/low。\n"
        "6. color 取值：red/orange/yellow/green/teal/blue/indigo/purple/pink/brown/gray/cyan。\n"
        "7. 输出纯 JSON，不要 markdown 代码块。\n"
        "示例：{\"events\":[{\"title\":\"项目评审\",\"description\":\"\",\"startDate\":\"%1T14:00:00\","
        "\"endDate\":\"%1T15:30:00\",\"category\":\"work\",\"priority\":\"normal\","
        "\"color\":\"blue\",\"allDay\":false}]}"
    ).arg(todayIso);

    QString userMsg = QString("【今天】%1（%2）\n请解析以下日程：\n%3")
        .arg(todayIso, weekdays[wd], text);

    QJsonArray messages;
    QJsonObject m1; m1["role"] = "system"; m1["content"] = sys; messages.append(m1);
    QJsonObject m2; m2["role"] = "user";   m2["content"] = userMsg; messages.append(m2);

    sendRequestStream(messages, true);
}

void DeepSeekClient::sendRequestStream(const QJsonArray &messages, bool forParse) {
    QNetworkRequest req{QUrl(m_baseUrl)};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", QByteArray("Bearer ") + m_apiKey.toUtf8());
    req.setRawHeader("Accept", "text/event-stream");

    QJsonObject body;
    body["model"] = m_model;
    body["messages"] = messages;
    body["stream"] = true;
    body["temperature"] = forParse ? 0.2 : 0.7;
    body["max_tokens"] = 4096;

    QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);
    m_reply = m_nam->post(req, payload);

    connect(m_reply, &QNetworkReply::readyRead, this, &DeepSeekClient::onReadyRead);
    connect(m_reply, &QNetworkReply::finished, this, &DeepSeekClient::onFinished);
}

void DeepSeekClient::onReadyRead() {
    if (!m_reply) return;
    m_buffer += QString::fromUtf8(m_reply->readAll());
    processBuffer();
}

void DeepSeekClient::processBuffer() {
    int idx;
    while ((idx = m_buffer.indexOf('\n')) != -1) {
        QString line = m_buffer.left(idx).trimmed();
        m_buffer.remove(0, idx + 1);
        if (line.isEmpty() || !line.startsWith("data:")) continue;

        QString jsonStr = line.mid(5).trimmed();
        if (jsonStr == "[DONE]") return;

        QJsonParseError err;
        auto doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &err);
        if (err.error != QJsonParseError::NoError) continue;

        auto choices = doc.object().value("choices").toArray();
        if (choices.isEmpty()) continue;
        auto delta = choices[0].toObject().value("delta").toObject();
        QString content = delta.value("content").toString();
        if (content.isEmpty()) continue;

        m_fullText += content;
        if (!m_isParseMode) {
            emit chatChunk(content);
        }
    }
}

void DeepSeekClient::onFinished() {
    if (!m_reply) return;

    bool aborted = (m_reply->error() == QNetworkReply::OperationCanceledError);
    QString errStr;
    if (m_reply->error() != QNetworkReply::NoError && !aborted) {
        errStr = m_reply->errorString();
        QByteArray body = m_reply->readAll();
        if (!body.isEmpty()) errStr += "\n" + QString::fromUtf8(body.left(500));
    }

    m_reply->deleteLater();
    m_reply = nullptr;

    if (aborted) return;

    if (!errStr.isEmpty()) {
        if (m_isParseMode) emit parseError(errStr);
        else emit chatError(errStr);
        return;
    }

    if (m_isParseMode) {
        auto list = parseJsonResponse(m_fullText);
        if (list.isEmpty()) {
            emit parseError("AI 未返回可解析的日程，请尝试更明确的描述。\n原始返回:\n" + m_fullText.left(300));
        } else {
            emit parseFinished(list);
        }
    } else {
        emit chatFinished(m_fullText);
    }
}

QList<ScheduleSuggestion> DeepSeekClient::parseJsonResponse(const QString &raw) {
    QList<ScheduleSuggestion> list;
    if (raw.isEmpty()) return list;

    QString jsonStr = raw.trimmed();
    static QRegularExpression fence(R"(^```(?:json)?\s*|\s*```$)");
    jsonStr.remove(fence);

    int braceStart = jsonStr.indexOf('{');
    int braceEnd = jsonStr.lastIndexOf('}');
    if (braceStart >= 0 && braceEnd > braceStart) {
        jsonStr = jsonStr.mid(braceStart, braceEnd - braceStart + 1);
    }

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << err.errorString();
        return list;
    }

    auto events = doc.object().value("events").toArray();
    for (const auto &v : events) {
        auto o = v.toObject();
        ScheduleSuggestion s;
        s.title = o.value("title").toString();
        s.category = stringToCategory(o.value("category").toString("other"));
        s.description = o.value("description").toString();
        s.startDate = QDateTime::fromString(o.value("startDate").toString(), Qt::ISODate);
        if (!s.startDate.isValid()) {
            s.startDate = QDateTime::currentDateTime();
        }
        s.endDate = QDateTime::fromString(o.value("endDate").toString(), Qt::ISODate);
        if (!s.endDate.isValid()) {
            s.endDate = s.startDate.addSecs(3600);
        }
        s.durationMinutes = static_cast<int>(s.startDate.secsTo(s.endDate) / 60);
        if (s.title.isEmpty()) {
            s.title = QString("未命名 · %1 · %2")
                .arg(categoryLabel(s.category))
                .arg(s.startDate.toString("M/d"));
        }
        s.priority = stringToPriority(o.value("priority").toString("normal"));
        QString colorStr = o.value("color").toString();
        s.color = colorStr.isEmpty()
            ? categoryDefaultColor(s.category)
            : stringToColor(colorStr);
        s.allDay = o.value("allDay").toBool(false);
        list.append(s);
    }
    return list;
}

} // namespace timemaster
