/****************************************************************************
 **
 ** Copyright (C) 2014-15 Jolla Ltd.
 ** Contact: Bea Lam <bea.lam@jollamobile.com>
 **
 ****************************************************************************/

#ifndef VKPOSTSYNCADAPTOR_H
#define VKPOSTSYNCADAPTOR_H

#include "vkdatatypesyncadaptor.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtCore/QJsonObject>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslError>

#include <socialcache/vkpostsdatabase.h>
#include <socialcache/socialimagesdatabase.h>

class VKPostSyncAdaptor : public VKDataTypeSyncAdaptor
{
    Q_OBJECT

public:
    VKPostSyncAdaptor(QObject *parent);
    ~VKPostSyncAdaptor();

    QString syncServiceName() const;
    void sync(const QString &dataTypeString, int accountId = 0);

protected: // implementing VKDataTypeSyncAdaptor interface
    void purgeDataForOldAccount(int oldId, SocialNetworkSyncAdaptor::PurgeMode mode);
    void beginSync(int accountId, const QString &accessToken);
    void finalize(int accountId);
    void retryThrottledRequest(const QString &request, const QVariantList &args, bool retryLimitReached);

private:
    void requestPosts(int accountId, const QString &accessToken);
    void determineOptimalImageSize();
    QDateTime lastSuccessfulSyncTime(int accountId);
    void setLastSuccessfulSyncTime(int accountId);

private Q_SLOTS:
    void finishedPostsHandler();

private:
    void saveVKPostFromObject(int accountId, const QJsonObject &post, const QList<UserProfile> &userProfiles, const QList<GroupProfile> &groupProfiles);
    void saveVKPhotoPostFromObject(int accountId, const QJsonObject &post, const QList<UserProfile> &userProfiles, const QList<GroupProfile> &groupProfiles);
    struct PostData {
        PostData() : accountId(0) {}
        PostData(int accountId, const QJsonObject &object,
                 const QList<UserProfile> &userProfiles, const QList<GroupProfile> &groupProfiles)
            : accountId(accountId), post(object)
            , userProfiles(userProfiles), groupProfiles(groupProfiles) {}
        int accountId;
        QJsonObject post;
        QList<UserProfile> userProfiles;
        QList<GroupProfile> groupProfiles;
    };
    QList<PostData> m_postsToAdd;
    QList<PostData> m_photoPostsToAdd;
    VKPostsDatabase m_db;
    QString m_optimalImageSize;
    SocialImagesDatabase m_imageCacheDb;
};

#endif // VKPOSTSYNCADAPTOR_H
