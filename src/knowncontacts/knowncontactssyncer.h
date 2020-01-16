/*
 * Buteo sync plugin that stores locally created contacts
 * Copyright (C) 2020 Open Mobile Platform LLC.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPL included in the packaging
 * of this file.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */

#ifndef KNOWNCONTACTS_SYNCER_H
#define KNOWNCONTACTS_SYNCER_H

#include <QContact>
#include <QDateTime>
#include <QObject>
#include <QList>
#include <QSettings>
#include <QString>
#include <SyncProfile.h>
#include <twowaycontactsyncadapter.h>

QTCONTACTS_USE_NAMESPACE

class KnownContactsSyncer : public QObject, public QtContactsSqliteExtensions::TwoWayContactSyncAdapter
{
    Q_OBJECT

public:
    KnownContactsSyncer(QString path, QObject *parent = nullptr);

    ~KnownContactsSyncer();

    bool startSync();

    bool purgeData();

    enum Failure : int {
        NoFailure = 0, // Reserved as success value
        InitFailure,
        CallFailure,
        StoreChangesFailure,
        StoreStateFailure
    };
    Q_ENUM(Failure)

signals:
    void syncSucceeded();
    void syncFailed(Failure errorCode);

protected:
    void determineRemoteChanges(const QDateTime &remoteSince);

private slots:
    void asyncDeterminedRemoteChanges(const QStringList files);

private:
    QContact getContact(const QString &id);
    void handleSyncFailure(Failure error);
    QList<QContact> readContacts(QSettings &file);

    QString m_syncFolder;
};

#endif // KNOWNCONTACTS_SYNCER_H
