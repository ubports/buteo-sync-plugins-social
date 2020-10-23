/****************************************************************************
 **
 ** Copyright (C) 2013-2014 Jolla Ltd.
 ** Contact: Chris Adams <chris.adams@jollamobile.com>
 **
 ** This program/library is free software; you can redistribute it and/or
 ** modify it under the terms of the GNU Lesser General Public License
 ** version 2.1 as published by the Free Software Foundation.
 **
 ** This program/library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 ** Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this program/library; if not, write to the Free
 ** Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 ** 02110-1301 USA
 **
 ****************************************************************************/

#ifndef GOOGLECALENDARSYNCADAPTOR_H
#define GOOGLECALENDARSYNCADAPTOR_H

#include "googledatatypesyncadaptor.h"

#include <QtCore/QString>
#include <QtCore/QMultiMap>
#include <QtCore/QPair>
#include <QtCore/QJsonObject>

#include <extendedcalendar.h>
#include <extendedstorage.h>
#include <icalformat.h>
#include <kdatetime.h>

class GoogleCalendarSyncAdaptor : public GoogleDataTypeSyncAdaptor
{
    Q_OBJECT

public:
    GoogleCalendarSyncAdaptor(QObject *parent);
    ~GoogleCalendarSyncAdaptor();

    QString syncServiceName() const;
    void sync(const QString &dataTypeString, int accountId);

protected: // implementing GoogleDataTypeSyncAdaptor interface
    void purgeDataForOldAccount(int oldId, SocialNetworkSyncAdaptor::PurgeMode mode);
    void beginSync(int accountId, const QString &accessToken);
    void finalCleanup();

private:
    enum ChangeType {
        NoChange = 0,
        Insert = 1,
        Modify = 2,
        Delete = 3,
        DeleteOccurrence = 4, // used to identify downsynced status->CANCELLED changes only
        CleanSync = 5 // delete followed by insert.
    };

    enum AccessRole {
        NoAccess = 0,
        FreeBusyReader = 1,
        Reader = 2,
        Writer = 3,
        Owner = 4
    };

    struct UpsyncChange {
        UpsyncChange() : upsyncType(NoChange) {}
        QString accessToken;
        ChangeType upsyncType;
        QString kcalEventId;
        KDateTime recurrenceId;
        QString calendarId;
        QString eventId;
        QByteArray eventData;
    };

    struct CalendarInfo {
        CalendarInfo() : change(NoChange), access(NoAccess) {}
        QString summary;
        QString description;
        QString color;
        ChangeType change;
        AccessRole access;
    };

    void requestCalendars(const QString &accessToken,
                          bool needCleanSync, const QString &pageToken = QString());
    void requestEvents(const QString &accessToken,
                       const QString &calendarId, const QString &syncToken,
                       const QString &pageToken = QString());
    void updateLocalCalendarNotebooks(const QString &accessToken, bool needCleanSync);
    QList<UpsyncChange> determineSyncDelta(const QString &accessToken,
                                           const QString &calendarId, const QDateTime &since);
    void upsyncChanges(const QString &accessToken,
                       GoogleCalendarSyncAdaptor::ChangeType upsyncType,
                       const QString &kcalEventId, const KDateTime &recurrenceId, const QString &calendarId,
                       const QString &eventId,const QByteArray &eventData);

    void applyRemoteChangesLocally();
    void updateLocalCalendarNotebookEvents(const QString &calendarId);

    mKCal::Notebook::Ptr notebookForCalendarId(const QString &calendarId) const;
    void finishedRequestingRemoteEvents(const QString &accessToken,
                                        const QString &calendarId, const QString &syncToken,
                                        const QString &nextSyncToken, const QDateTime &since);

    static void setCalendarProperties(mKCal::Notebook::Ptr notebook,
                                      const CalendarInfo &calendarInfo,
                                      const QString &serverCalendarId,
                                      int accountId,
                                      const QString &syncProfile,
                                      const QString &ownerEmail);

private Q_SLOTS:
    void calendarsFinishedHandler();
    void eventsFinishedHandler();
    void upsyncFinishedHandler();

private:
    QMap<QString, CalendarInfo> m_serverCalendarIdToCalendarInfo;
    QMap<QString, int> m_serverCalendarIdToDefaultReminderTimes;
    QMultiMap<QString, QJsonObject> m_calendarIdToEventObjects;
    QMap<QString, QString> m_recurringEventIdToKCalUid;
    bool m_syncSucceeded;
    int m_accountId;

    QStringList m_calendarsBeingRequested;               // calendarIds
    QStringList m_calendarsFinishedRequested;            // calendarId to updated timestamp string
    QMap<QString, QString> m_calendarsThisSyncTokens;    // calendarId to sync token used during this sync cycle
    QMap<QString, QString> m_calendarsNextSyncTokens;    // calendarId to sync token to use during next sync cycle
    QMap<QString, QDateTime> m_calendarsSyncDate;        // calendarId to since date to use when determining delta
    QMultiMap<QString, QPair<GoogleCalendarSyncAdaptor::ChangeType, QJsonObject> > m_changesFromDownsync; // calendarId to change
    QMultiMap<QString, QPair<KCalCore::Event::Ptr, QJsonObject> > m_changesFromUpsync; // calendarId to event+upsyncResponse
    QSet<QString> m_syncTokenFailure; // calendarIds suffering from 410 error due to invalid sync token
    QSet<QString> m_timeMinFailure;   // calendarIds suffering from 410 error due to invalid timeMin value

    mKCal::ExtendedCalendar::Ptr m_calendar;
    mKCal::ExtendedStorage::Ptr m_storage;
    mutable KCalCore::ICalFormat m_icalFormat;
    bool m_storageNeedsSave;
    QDateTime m_originalLastSyncTimestamp;
};

#endif // GOOGLECALENDARSYNCADAPTOR_H
