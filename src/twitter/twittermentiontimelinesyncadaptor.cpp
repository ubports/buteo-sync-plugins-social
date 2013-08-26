/****************************************************************************
 **
 ** Copyright (C) 2013 Jolla Ltd.
 ** Contact: Chris Adams <chris.adams@jollamobile.com>
 **
 ****************************************************************************/

#include "twittermentiontimelinesyncadaptor.h"
#include "syncservice.h"
#include "trace.h"
#include "constants_p.h"

#include <QtCore/QPair>
#include <QtCore/QUrlQuery>

#include <QtContacts/QContactManager>
#include <QtContacts/QContactFetchHint>
#include <QtContacts/QContactFetchRequest>
#include <QtContacts/QContact>
#include <QtContacts/QContactName>
#include <QtContacts/QContactDisplayLabel>
#include <QtContacts/QContactNickname>
#include <QtContacts/QContactPresence>
#include <QtContacts/QContactAvatar>

//nemo-qml-plugins/notifications
#include <notification.h>

#define SOCIALD_TWITTER_MENTIONS_ID_PREFIX QLatin1String("twitter-mentions-")
#define SOCIALD_TWITTER_MENTIONS_GROUPNAME QLatin1String("sociald-sync-twitter-mentions")
#define QTCONTACTS_SQLITE_AVATAR_METADATA QLatin1String("AvatarMetadata")

// currently, we integrate with the device notifications via nemo-qml-plugin-notification

TwitterMentionTimelineSyncAdaptor::TwitterMentionTimelineSyncAdaptor(SyncService *syncService, QObject *parent)
    : TwitterDataTypeSyncAdaptor(syncService, SyncService::Notifications, parent)
    , m_contactFetchRequest(new QContactFetchRequest(this))
{
    // can sync, enabled
    setInitialActive(true);

    // fetch all contacts.  We detect which contact a mention came from.
    // XXX TODO: we really shouldn't do this, we should do it on demand instead
    // of holding the contacts in memory.
    if (m_contactFetchRequest) {
        QContactFetchHint cfh;
        cfh.setOptimizationHints(QContactFetchHint::NoRelationships | QContactFetchHint::NoActionPreferences | QContactFetchHint::NoBinaryBlobs);
        cfh.setDetailTypesHint(QList<QContactDetail::DetailType>()
                               << QContactDetail::TypeAvatar
                               << QContactDetail::TypeName
                               << QContactDetail::TypeNickname
                               << QContactDetail::TypePresence);
        m_contactFetchRequest->setFetchHint(cfh);
        m_contactFetchRequest->setManager(&m_contactManager);
        connect(m_contactFetchRequest, SIGNAL(stateChanged(QContactAbstractRequest::State)), this, SLOT(contactFetchStateChangedHandler(QContactAbstractRequest::State)));
        m_contactFetchRequest->start();
    }
}

TwitterMentionTimelineSyncAdaptor::~TwitterMentionTimelineSyncAdaptor()
{
}

void TwitterMentionTimelineSyncAdaptor::sync(const QString &dataType)
{
    // refresh local cache of contacts.
    // we do this asynchronous request in parallel to the sync code below
    // since the network request round-trip times should far exceed the
    // local database fetch.  If not, then the current sync run will
    // still work, but the "notifications is from which contact" detection
    // will be using slightly stale data.
    if (m_contactFetchRequest &&
            (m_contactFetchRequest->state() == QContactAbstractRequest::InactiveState ||
             m_contactFetchRequest->state() == QContactAbstractRequest::FinishedState)) {
        m_contactFetchRequest->start();
    }

    // call superclass impl.
    TwitterDataTypeSyncAdaptor::sync(dataType);
}

void TwitterMentionTimelineSyncAdaptor::purgeDataForOldAccounts(const QList<int> &purgeIds)
{
    foreach (int accountIdentifier, purgeIds) {
        Notification *notification = findNotification(accountIdentifier);
        if (notification) {
            notification->close();
            notification->deleteLater();
        }
    }
}

void TwitterMentionTimelineSyncAdaptor::beginSync(int accountId, const QString &oauthToken, const QString &oauthTokenSecret)
{
    requestNotifications(accountId, oauthToken, oauthTokenSecret);
}

void TwitterMentionTimelineSyncAdaptor::requestNotifications(int accountId, const QString &oauthToken, const QString &oauthTokenSecret, const QString &sinceTweetId)
{
    QList<QPair<QString, QString> > queryItems;
    queryItems.append(QPair<QString, QString>(QString(QLatin1String("count")), QString(QLatin1String("50"))));
    if (!sinceTweetId.isEmpty()) {
        queryItems.append(QPair<QString, QString>(QString(QLatin1String("since_id")), sinceTweetId));
    }
    QString baseUrl = QLatin1String("https://api.twitter.com/1.1/statuses/mentions_timeline.json");
    QUrl url(baseUrl);
    QUrlQuery query(url);
    query.setQueryItems(queryItems);
    url.setQuery(query);

    QNetworkRequest nreq(url);
    nreq.setRawHeader("Authorization", authorizationHeader(
            accountId, oauthToken, oauthTokenSecret,
            QLatin1String("GET"), baseUrl, queryItems).toLatin1());

    QNetworkReply *reply = networkAccessManager->get(nreq);
    
    if (reply) {
        reply->setProperty("accountId", accountId);
        reply->setProperty("oauthToken", oauthToken);
        reply->setProperty("oauthTokenSecret", oauthTokenSecret);
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(errorHandler(QNetworkReply::NetworkError)));
        connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrorsHandler(QList<QSslError>)));
        connect(reply, SIGNAL(finished()), this, SLOT(finishedHandler()));

        // we're requesting data.  Increment the semaphore so that we know we're still busy.
        incrementSemaphore(accountId);
    } else {
        TRACE(SOCIALD_ERROR,
                QString(QLatin1String("error: unable to request mention timeline notifications from Twitter account with id %1"))
                .arg(accountId));
    }
}

void TwitterMentionTimelineSyncAdaptor::finishedHandler()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    int accountId = reply->property("accountId").toInt();
    QString oauthToken = reply->property("oauthToken").toString();
    QString oauthTokenSecret = reply->property("oauthTokenSecret").toString();
    QDateTime lastSync = lastSyncTimestamp(QLatin1String("twitter"),
                                           SyncService::dataType(SyncService::Notifications),
                                           QString::number(accountId));
    TRACE(SOCIALD_DEBUG,
            QString(QLatin1String("Last sync:")) << lastSync);

    QByteArray replyData = reply->readAll();
    disconnect(reply);
    reply->deleteLater();

    bool ok = false;
    QVariant parsed = TwitterDataTypeSyncAdaptor::parseReplyData(replyData, &ok);
    if (ok && parsed.type() == QVariant::List) {
        QVariantList data = parsed.toList();
        if (!data.size()) {
            TRACE(SOCIALD_DEBUG,
                    QString(QLatin1String("no notifications received for account %1"))
                    .arg(accountId));
            decrementSemaphore(accountId);
            return;
        }

        bool needMorePages = true;
        bool postedNew = false;
        int mentionsCount = 0;
        QString body;
        QString summary;
        QDateTime timestamp;
        QString link;
        for (int i = 0; i < data.size(); ++i) {
            QVariantMap currData = data.at(i).toMap();
            QDateTime createdTime = parseTwitterDateTime(currData.value(QLatin1String("created_at")).toString());
            QString mention_id = currData.value(QLatin1String("id_str")).toString();
            QString text = currData.value(QLatin1String("text")).toString();
            QVariantMap user = currData.value(QLatin1String("user")).toMap();
            QString user_id = user.value(QLatin1String("id_str")).toString();
            QString user_name = user.value(QLatin1String("name")).toString();
            QString user_screen_name = user.value(QLatin1String("screen_name")).toString();
            link = QLatin1String("https://twitter.com/") + user_screen_name + QLatin1String("/status/") + mention_id;

            // check to see if we need to post it to the notifications feed
            if (lastSync.isValid() && createdTime < lastSync) {
                TRACE(SOCIALD_DEBUG,
                        QString(QLatin1String("notification for account %1 came after last sync:"))
                        .arg(accountId) << "    " << createdTime << ":" << text);
                needMorePages = false; // don't fetch more pages of results.
                break;                 // all subsequent notifications will be even older.
            } else if (createdTime.daysTo(QDateTime::currentDateTimeUtc()) > 7) {
                TRACE(SOCIALD_DEBUG,
                        QString(QLatin1String("notification for account %1 is more than a week old:\n"))
                        .arg(accountId) << "    " << createdTime << ":" << text);
                needMorePages = false; // don't fetch more pages of results.
                break;                 // all subsequent notifications will be even older.
            } else {
                // XXX TODO: use twitter user id to look up the contact directly, instead of heuristic detection.
                QString nameString = user_name;
                QString avatar = QLatin1String("icon-s-service-twitter"); // default.
                QContact matchingContact = findMatchingContact(nameString);
                if (matchingContact != QContact()) {
                    QContactDisplayLabel displayLabel = matchingContact.detail<QContactDisplayLabel>();
                    QContactName contactName = matchingContact.detail<QContactName>();
                    QString originalNameString = nameString;
                    if (!displayLabel.label().isEmpty()) {
                        nameString = displayLabel.label();
                    } else if (!contactName.value<QString>(QContactName__FieldCustomLabel).isEmpty()) {
                        nameString = contactName.value<QString>(QContactName__FieldCustomLabel);
                    }

                    QList<QContactAvatar> allAvatars = matchingContact.details<QContactAvatar>();
                    bool foundTwitterProfileImage = false;
                    foreach (const QContactAvatar &avat, allAvatars) {
                        // TODO: avat.value(QTCONTACTS_SQLITE_AVATAR_METADATA) == QLatin1String("profile")
                        if (!avat.imageUrl().toString().isEmpty()) {
                            // found avatar synced from Twitter sociald sync adaptor
                            avatar = avat.imageUrl().toString();
                            foundTwitterProfileImage = true;
                            break;
                        }
                    }
                    if (!foundTwitterProfileImage && !matchingContact.detail<QContactAvatar>().imageUrl().toString().isEmpty()) {
                        // fallback.
                        avatar = matchingContact.detail<QContactAvatar>().imageUrl().toString();
                    }

                    TRACE(SOCIALD_DEBUG,
                            QString(QLatin1String("heuristically matched %1 as %2 with avatar %3"))
                            .arg(originalNameString).arg(nameString).arg(avatar));
                }

                body = nameString;
                summary = text;
                timestamp = createdTime;
                mentionsCount ++;
            }
        }

        if (mentionsCount > 0) {
            // Search if we already have a notification
            Notification *notification = createNotification(accountId);

            // Set properties of the notification
            notification->setItemCount(notification->itemCount() + mentionsCount);
            notification->setRemoteDBusCallServiceName("org.sailfishos.browser");
            notification->setRemoteDBusCallObjectPath("/");
            notification->setRemoteDBusCallInterface("org.sailfishos.browser");
            notification->setRemoteDBusCallMethodName("openUrl");
            QStringList openUrlArgs;


            if (notification->itemCount() == 1) {
                notification->setTimestamp(timestamp);
                notification->setSummary(summary);
                notification->setBody(body);
                openUrlArgs << link;
            } else {
                notification->setTimestamp(QDateTime::currentDateTimeUtc());
                // TODO: maybe we should display the name of the account
                //% "Twitter"
                notification->setBody(qtTrId("qtn_social_notifications_twitter"));
                //% "You received %n mentions"
                notification->setSummary(qtTrId("qtn_social_notifications_n_mentions", notification->itemCount()));
                openUrlArgs << QLatin1String("https://twitter.com/i/connect");
            }
            notification->setRemoteDBusCallArguments(QVariantList() << openUrlArgs);
            notification->publish();

            qlonglong localId = (0 + notification->replacesId());
            if (localId == 0) {
                // failed.
                TRACE(SOCIALD_ERROR,
                        QString(QLatin1String("error: failed to publish notification: %1"))
                        .arg(body));
            }
        }

        if (postedNew && needMorePages) {
            // XXX TODO: paging?
        }
    } else {
        // error occurred during request.
        TRACE(SOCIALD_ERROR,
                QString(QLatin1String("error: unable to parse notification data from request with account %1; got: %2"))
                .arg(accountId).arg(QString::fromLatin1(replyData.constData())));
    }

    // we're finished this request.  Decrement our busy semaphore.
    decrementSemaphore(accountId);
}

void TwitterMentionTimelineSyncAdaptor::contactFetchStateChangedHandler(QContactAbstractRequest::State newState)
{
    // update our local cache of contacts.
    if (m_contactFetchRequest && newState == QContactAbstractRequest::FinishedState) {
        m_contacts = m_contactFetchRequest->contacts();
        TRACE(SOCIALD_DEBUG,
                QString(QLatin1String("finished refreshing local cache of contacts, have %1"))
                .arg(m_contacts.size()));
    }
}

QContact TwitterMentionTimelineSyncAdaptor::findMatchingContact(const QString &nameString) const
{
    // TODO: This heuristic detection could definitely be improved.
    // EG: instead of scraping the name string from the title, we
    // could get the actual twitter id from the mention and then
    // look the contact up directly.  But we currently don't have
    // Twitter contact syncing done properly, so...
    if (nameString.isEmpty()) {
        return QContact();
    }

    QStringList firstAndLast = nameString.split(' '); // TODO: better detection of FN/LN

    foreach (const QContact &c, m_contacts) {
        QList<QContactName> names = c.details<QContactName>();
        foreach (const QContactName &n, names) {
            if (n.value<QString>(QContactName__FieldCustomLabel) == nameString ||
                    (firstAndLast.size() >= 2 &&
                     n.firstName() == firstAndLast.at(0) &&
                     n.lastName() == firstAndLast.at(firstAndLast.size()-1))) {
                return c;
            }
        }

        QList<QContactNickname> nicknames = c.details<QContactNickname>();
        foreach (const QContactNickname &n, nicknames) {
            if (n.nickname() == nameString) {
                return c;
            }
        }

        QList<QContactPresence> presences = c.details<QContactPresence>();
        foreach (const QContactPresence &p, presences) {
            if (p.nickname() == nameString) {
                return c;
            }
        }
    }

    // this isn't a "hard error" since we can still post the notification
    // but it is a "soft error" since we _should_ have the contact in our db.
    TRACE(SOCIALD_INFORMATION,
            QString(QLatin1String("unable to find matching contact with name: %1"))
            .arg(nameString));

    return QContact();
}

Notification *TwitterMentionTimelineSyncAdaptor::createNotification(int accountId)
{
    Notification *notification = findNotification(accountId);
    if (notification) {
        return notification;
    }

    notification = new Notification(this);
    notification->setCategory(QLatin1String("x-nemo.social.twitter.mention"));
    notification->setHintValue("x-nemo.sociald.account-id", accountId);
    return notification;
}

Notification * TwitterMentionTimelineSyncAdaptor::findNotification(int accountId)
{
    Notification *notification = 0;
    QList<QObject *> notifications = Notification::notifications();
    foreach (QObject *object, notifications) {
        Notification *castedNotification = static_cast<Notification *>(object);
        if (castedNotification->category() == "x-nemo.social.twitter.mention"
            && castedNotification->hintValue("x-nemo.sociald.account-id").toInt() == accountId) {
            notification = castedNotification;
            break;
        }
    }

    if (notification) {
        notifications.removeAll(notification);
    }

    qDeleteAll(notifications);

    return notification;
}
