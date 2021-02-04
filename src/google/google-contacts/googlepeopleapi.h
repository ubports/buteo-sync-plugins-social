/****************************************************************************
 **
 ** Copyright (c) 2021 Jolla Ltd.
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

#ifndef GOOGLEPEOPLEAPI_H
#define GOOGLEPEOPLEAPI_H

#include "googlepeoplejson.h"

#include <QContact>
#include <QContactCollection>

QTCONTACTS_USE_NAMESPACE

namespace GooglePeopleApi
{
    enum OperationType {
        UnsupportedOperation,
        CreateContact,
        UpdateContact,
        DeleteContact,
        AddContactPhoto,
        UpdateContactPhoto,
        DeleteContactPhoto
    };
}

class GooglePeopleApiRequest
{
public:
    GooglePeopleApiRequest(const QString &accessToken);
    ~GooglePeopleApiRequest();

    static QByteArray writeMultiPartRequest(const QMap<GooglePeopleApi::OperationType, QList<QContact> > &batch);


private:
    QString m_accessToken;
};


class GooglePeopleApiResponse
{
public:
    class PeopleConnectionsListResponse
    {
    public:
        QList<GooglePeople::Person> connections;
        QString nextPageToken;
        QString nextSyncToken;
        int totalPeople = 0;
        int totalItems = 0;

        void getContacts(int accountId,
                         const QList<QContactCollection> &candidateCollections,
                         QList<QContact> *addedOrModified,
                         QList<QContact> *deleted) const;
    };

    class ContactGroupsResponse
    {
    public:
        // Note: for this response, memberResourceNames of each group are not populated
        QList<GooglePeople::ContactGroup> contactGroups;
        int totalItems = 0;
        QString nextPageToken;
        QString nextSyncToken;
    };

    class BatchResponsePart
    {
    public:
        struct Error {
            int code;
            QString message;
            QString status;
        };

        QString contentType;
        QString contentId;
        QString bodyStatusLine;
        QString bodyContentType;
        QByteArray body;

        void reset();

        void parse(GooglePeopleApi::OperationType *operationType,
                   QString *contactId,
                   GooglePeople::Person *person,
                   Error *error) const;
    };

    static bool readResponse(const QByteArray &data, ContactGroupsResponse *response);
    static bool readResponse(const QByteArray &data, PeopleConnectionsListResponse *response);
    static bool readMultiPartResponse(const QByteArray &data, QList<BatchResponsePart> *responseParts);
};

#endif // GOOGLEPEOPLEAPI_H
