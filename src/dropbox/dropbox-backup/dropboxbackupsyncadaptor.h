/****************************************************************************
 **
 ** Copyright (C) 2015-2019 Jolla Ltd.
 ** Copyright (C) 2019 Open Mobile Platform LLC
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

#ifndef DROPBOXBACKUPSYNCADAPTOR_H
#define DROPBOXBACKUPSYNCADAPTOR_H

#include "dropboxdatatypesyncadaptor.h"
#include "backuprestoreoptions_p.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QVariantMap>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QSet>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslError>
#include <QtSql/QSqlDatabase>

namespace Buteo {
    class ProfileManager;
}

class DropboxBackupSyncAdaptor : public DropboxDataTypeSyncAdaptor
{
    Q_OBJECT

public:
    DropboxBackupSyncAdaptor(const QString &profileName, QObject *parent);
    ~DropboxBackupSyncAdaptor();

    QString syncServiceName() const;
    void sync(const QString &dataTypeString, int accountId);

protected: // implementing DropboxDataTypeSyncAdaptor interface
    void purgeDataForOldAccount(int oldId, SocialNetworkSyncAdaptor::PurgeMode mode);
    void beginSync(int accountId, const QString &accessToken);
    void finalize(int accountId);
    void finalCleanup();

private:
    void requestList(int accountId,
                     const QString &accessToken,
                     int operationType,
                     const QString &remotePath,
                     const QString &continuationCursor,
                     const QVariantMap &extraProperties);
    void requestData(int accountId, const QString &accessToken,
                     const QString &localPath, const QString &remotePath,
                     const QString &remoteFile = QString());
    void uploadData(int accountId, const QString &accessToken,
                    const QString &localPath, const QString &remotePath,
                    const QString &localFile = QString());
    void purgeAccount(int accountId);

    void beginListOperation(int accountId, const QString &accessToken, const BackupRestoreOptions &options);
    void beginSyncOperation(int accountId, const QString &accessToken, const BackupRestoreOptions &options);

private Q_SLOTS:
    void remotePathFinishedHandler();
    void remoteFileFinishedHandler();
    void createRemotePathFinishedHandler();
    void createRemoteFileFinishedHandler();
    void downloadProgressHandler(qint64 bytesReceived, qint64 bytesTotal);
    void uploadProgressHandler(qint64 bytesSent, qint64 bytesTotal);

private:
    Buteo::ProfileManager *m_profileManager;

    QSet<QString> m_backupFiles;
    QString m_profileName;
};

#endif // DROPBOXBACKUPSYNCADAPTOR_H
