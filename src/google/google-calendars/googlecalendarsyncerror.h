#ifndef GOOGLECALENDARSYNCERROR_H
#define GOOGLECALENDARSYNCERROR_H

#include <QByteArray>
#include <QString>
#include <QList>

class GoogleCalendarSyncErrorInfo {
public:
    QString m_domain;
    QString m_reason;
    QString m_message;
    QString m_locationType;
    QString m_location;
};

// GoogleCalendarSyncError captures the json data
// representing a request error. See:
// https://developers.google.com/calendar/v3/errors
class GoogleCalendarSyncError
{
public:
    explicit GoogleCalendarSyncError();
    GoogleCalendarSyncError(QByteArray const &replyData);

    bool isValid() const;
    int code() const;
    QString message() const;
    QString firstReason() const;

public:
    bool m_valid;
    int m_code;
    QString m_message;
    QList<GoogleCalendarSyncErrorInfo> m_error;
};

#endif // GOOGLECALENDARSYNCERROR_H
