/****************************************************************************
 **
 ** Copyright (c) 2014 - 2019 Jolla Ltd.
 ** Copyright (c) 2020 Open Mobile Platform LLC.
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

#ifndef GOOGLETWOWAYCONTACTSYNCADAPTOR_H
#define GOOGLETWOWAYCONTACTSYNCADAPTOR_H

#include "googledatatypesyncadaptor.h"
#include "googlepeopleapi.h"

#include <twowaycontactsyncadaptor.h>

#include <QContactExtendedDetail>
#include <QContactManager>
#include <QContact>
#include <QDateTime>
#include <QList>
#include <QPair>

QTCONTACTS_USE_NAMESPACE

class GoogleContactImageDownloader;
class GoogleTwoWayContactSyncAdaptor;

class GoogleContactSqliteSyncAdaptor : public QObject, public QtContactsSqliteExtensions::TwoWayContactSyncAdaptor
{
    Q_OBJECT
public:
    GoogleContactSqliteSyncAdaptor(int accountId, GoogleTwoWayContactSyncAdaptor *parent);
   ~GoogleContactSqliteSyncAdaptor();

    bool isLocallyDeletedGuid(const QString &guid) const;

    virtual bool determineRemoteCollections() override;
    virtual bool deleteRemoteCollection(const QContactCollection &collection) override;
    virtual bool determineRemoteContacts(const QContactCollection &collection) override;

    virtual bool determineRemoteContactChanges(const QContactCollection &collection,
                                               const QList<QContact> &localAddedContacts,
                                               const QList<QContact> &localModifiedContacts,
                                               const QList<QContact> &localDeletedContacts,
                                               const QList<QContact> &localUnmodifiedContacts,
                                               QContactManager::Error *error) override;

    virtual bool storeLocalChangesRemotely(const QContactCollection &collection,
                                           const QList<QContact> &addedContacts,
                                           const QList<QContact> &modifiedContacts,
                                           const QList<QContact> &deletedContacts) override;
    virtual void storeRemoteChangesLocally(const QContactCollection &collection,
                                           const QList<QContact> &addedContacts,
                                           const QList<QContact> &modifiedContacts,
                                           const QList<QContact> &deletedContacts) override;

    virtual void syncFinishedSuccessfully() override;
    virtual void syncFinishedWithError() override;

private:
    GoogleTwoWayContactSyncAdaptor *q;
};

class GoogleTwoWayContactSyncAdaptor : public GoogleDataTypeSyncAdaptor
{
    Q_OBJECT
public:
    enum DataRequestType {
        ContactRequest,
        ContactGroupRequest
    };

    enum ContactChangeNotifier {
        NoContactChangeNotifier,
        DetermineRemoteContacts,
        DetermineRemoteContactChanges
    };
    Q_ENUM(ContactChangeNotifier)

    GoogleTwoWayContactSyncAdaptor(QObject *parent);
   ~GoogleTwoWayContactSyncAdaptor();

    virtual QString syncServiceName() const override;
    virtual void sync(const QString &dataTypeString, int accountId) override;

    void requestData(DataRequestType requestType,
                     ContactChangeNotifier contactChangeNotifier = NoContactChangeNotifier,
                     const QString &pageToken = QString());
    void upsyncLocalChanges(const QList<QContact> &locallyAdded,
                            const QList<QContact> &locallyModified,
                            const QList<QContact> &locallyDeleted);

    void syncFinished();

protected:
    // implementing GoogleDataTypeSyncAdaptor interface
    void purgeDataForOldAccount(int oldId, SocialNetworkSyncAdaptor::PurgeMode mode) override;
    void beginSync(int accountId, const QString &accessToken) override;
    void finalize(int accountId) override;
    void finalCleanup() override;

private:
    friend class GoogleContactSqliteSyncAdaptor;

    class BatchedUpdate
    {
    public:
        QMap<GooglePeopleApi::OperationType, QList<QContact> > batch;
        int batchCount = 0;
    };

    void groupsFinishedHandler();
    void contactsFinishedHandler();
    void continueSync(GoogleTwoWayContactSyncAdaptor::ContactChangeNotifier contactChangeNotifier);
    void upsyncLocalChangesList();
    bool batchRemoteChanges(BatchedUpdate *batchedUpdate,
                            QList<QContact> *contacts,
                            GooglePeopleApi::OperationType updateType);
    void storeToRemote(const QByteArray &encodedContactUpdates);
    void queueOutstandingAvatars();
    bool queueAvatarForDownload(const QString &contactGuid, const QString &imageUrl);
    bool addAvatarToDownload(QContact *contact);
    void imageDownloaded(const QString &url, const QString &path, const QVariantMap &metadata);
    void loadCollection(const QContactCollection &collection);

    void purgeAccount(int pid);
    void postFinishedHandler();
    void postErrorHandler();

    QList<QContact> m_remoteAdds;
    QList<QContact> m_remoteMods;
    QList<QContact> m_remoteDels;
    QList<QContact> m_localAdds;
    QList<QContact> m_localMods;
    QList<QContact> m_localDels;
    QList<QContact> m_localAvatarAdds;
    QList<QContact> m_localAvatarMods;
    QList<QContact> m_localAvatarDels;

    QHash<QString, QString> m_contactEtags; // contact guid -> contact etag
    QHash<QString, QString> m_contactIds; // contact guid -> contact id
    QHash<QString, QString> m_contactAvatars; // contact guid -> remote avatar path
    QHash<QString, QPair<QString,QString> > m_previousAvatarUrls;
    QHash<GooglePeopleApi::OperationType, int> m_batchUpdateIndexes;
    QHash<QString, QString> m_queuedAvatarsForDownload; // contact guid -> remote avatar path

    QContactManager *m_contactManager = nullptr;
    GoogleContactSqliteSyncAdaptor *m_sqliteSync = nullptr;
    GoogleContactImageDownloader *m_workerObject = nullptr;

    QContactCollection m_collection;
    QString m_accessToken;

    struct PeopleConnectionsListParameters {
        bool requestSyncToken;
        QString syncToken;
        QString personFields;
    } m_connectionsListParams;

    int m_accountId = 0;
    int m_apiRequestsRemaining = 0;
    bool m_retriedConnectionsList = false;
    bool m_allowFinalCleanup = false;
};

#endif // GOOGLETWOWAYCONTACTSYNCADAPTOR_H
