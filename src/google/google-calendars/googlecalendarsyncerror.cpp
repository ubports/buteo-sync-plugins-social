#include <QJsonDocument>
#include <QJsonObject>

#include "socialnetworksyncadaptor.h"
#include "trace.h"

#include "googlecalendarsyncerror.h"

GoogleCalendarSyncError::GoogleCalendarSyncError()
    : m_valid(false)
    , m_code(0)
{
}

GoogleCalendarSyncError::GoogleCalendarSyncError(QByteArray const &replyData)
    : m_valid(false)
    , m_code(0)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(replyData);
    if (!jsonDocument.isEmpty()
            && jsonDocument.isObject()
            && jsonDocument.object().contains(QStringLiteral("error"))) {
        QJsonObject error = jsonDocument.object().value(QStringLiteral("error")).toObject();

        if (error.contains(QStringLiteral("code"))
                && error.value(QStringLiteral("code")).isDouble()
                && error.contains(QStringLiteral("errors"))
                && error.value(QStringLiteral("errors")).isArray()) {
            m_valid = true;
            m_code = error.value(QStringLiteral("code")).toInt();
            m_message = error.value(QStringLiteral("message")).toString();
            QJsonArray errors = error.value(QStringLiteral("errors")).toArray();

            for (QJsonValue errorItem : errors) {
                if (errorItem.isObject()) {
                    QJsonObject item = errorItem.toObject();
                    GoogleCalendarSyncErrorInfo details;
                    details.m_domain = item.value(QStringLiteral("domain")).toString();
                    details.m_reason = item.value(QStringLiteral("reason")).toString();
                    details.m_message = item.value(QStringLiteral("message")).toString();
                    details.m_locationType = item.value(QStringLiteral("locationType")).toString();
                    details.m_location = item.value(QStringLiteral("location")).toString();
                    m_error.append(details);
                }
            }

            if (m_error.isEmpty()) {
                SOCIALD_LOG_ERROR("no error block when parsing returned json");
            }
        } else {
            SOCIALD_LOG_ERROR("essential components missing when parsing error json");
        }
    } else {
        SOCIALD_LOG_ERROR("error parsing returned json");
    }
}

bool GoogleCalendarSyncError::isValid() const
{
    return m_valid;
}

int GoogleCalendarSyncError::code() const
{
    return m_code;
}

QString GoogleCalendarSyncError::message() const
{
    return m_message;
}

QString GoogleCalendarSyncError::firstReason() const
{
    return m_error.empty() ? QString() : m_error[0].m_reason;
}
