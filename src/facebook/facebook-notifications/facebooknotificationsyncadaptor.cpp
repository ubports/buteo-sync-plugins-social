/****************************************************************************
 **
 ** Copyright (C) 2013 Jolla Ltd.
 ** Contact: Chris Adams <chris.adams@jollamobile.com>
 **
 ****************************************************************************/

#include "facebooknotificationsyncadaptor.h"
#include "trace.h"

#include <QUrlQuery>

//nemo-qml-plugins/notifications
#include <notification.h>

FacebookNotificationSyncAdaptor::FacebookNotificationSyncAdaptor(QObject *parent)
    : FacebookDataTypeSyncAdaptor(SocialNetworkSyncAdaptor::Notifications, parent)
{
    setInitialActive(true);
}

FacebookNotificationSyncAdaptor::~FacebookNotificationSyncAdaptor()
{
}

QString FacebookNotificationSyncAdaptor::syncServiceName() const
{
    return QStringLiteral("facebook-notifications");
}

void FacebookNotificationSyncAdaptor::purgeDataForOldAccounts(const QList<int> &purgeIds)
{
    foreach (int accountId, purgeIds) {
        // Search for the notification and close it
        Notification *notification = 0;
        QList<QObject *> notifications = Notification::notifications();
        foreach (QObject *object, notifications) {
            Notification *castedNotification = static_cast<Notification *>(object);
            if (castedNotification->category() == "x-nemo.social.facebook.notification"
                && castedNotification->hintValue("x-nemo.sociald.account-id").toInt() == accountId) {
                notification = castedNotification;
                break;
            }
        }

        if (notification) {
            notification->close();
        }

        qDeleteAll(notifications);
    }
}

void FacebookNotificationSyncAdaptor::beginSync(int accountId, const QString &accessToken)
{
    requestNotifications(accountId, accessToken);
}

void FacebookNotificationSyncAdaptor::requestNotifications(int accountId, const QString &accessToken, const QString &until, const QString &pagingToken)
{
    // TODO: continuation requests need these two.  if exists, also set limit = 5000.
    // if not set, set "since" to the timestamp value.
    Q_UNUSED(until);
    Q_UNUSED(pagingToken);

    QList<QPair<QString, QString> > queryItems;
    queryItems.append(QPair<QString, QString>(QString(QLatin1String("include_read")), QString(QLatin1String("false"))));
    queryItems.append(QPair<QString, QString>(QString(QLatin1String("access_token")), accessToken));
    QUrl url(QLatin1String("https://graph.facebook.com/me/notifications"));
    QUrlQuery query(url);
    query.setQueryItems(queryItems);
    url.setQuery(query);
    QNetworkReply *reply = networkAccessManager->get(QNetworkRequest(url));

    if (reply) {
        reply->setProperty("accountId", accountId);
        reply->setProperty("accessToken", accessToken);
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(errorHandler(QNetworkReply::NetworkError)));
        connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrorsHandler(QList<QSslError>)));
        connect(reply, SIGNAL(finished()), this, SLOT(finishedHandler()));

        // we're requesting data.  Increment the semaphore so that we know we're still busy.
        incrementSemaphore(accountId);
        setupReplyTimeout(accountId, reply);
    } else {
        TRACE(SOCIALD_ERROR,
                QString(QLatin1String("error: unable to request notifications from Facebook account with id %1"))
                .arg(accountId));
    }
}

void FacebookNotificationSyncAdaptor::finishedHandler()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    bool isError = reply->property("isError").toBool();
    int accountId = reply->property("accountId").toInt();
    QByteArray replyData = reply->readAll();
    disconnect(reply);
    reply->deleteLater();
    removeReplyTimeout(accountId, reply);

    bool ok = false;

    QDateTime lastSync = lastSyncTimestamp(serviceName(), SocialNetworkSyncAdaptor::dataTypeName(dataType),
                                           accountId).toUTC();

    QJsonObject parsed = parseJsonObjectReplyData(replyData, &ok);
    if (!isError && ok && parsed.contains(QLatin1String("summary"))) {
        QJsonArray data = parsed.value(QLatin1String("data")).toArray();

        int notificationCount = 0;
        bool haveNewNotifs = false;
        foreach (const QJsonValue &entry, data) {
            QString updatedTimeStr
                    = entry.toObject().value(QLatin1String("updated_time")).toString();
            QDateTime updatedTime = QDateTime::fromString(updatedTimeStr, Qt::ISODate);
            updatedTime.setTimeSpec(Qt::UTC);

            if (updatedTime > lastSync) {
                haveNewNotifs = true;
                notificationCount++;
            }
        }

        Notification *notification = existingNemoNotification(accountId);
        if (notification != 0) {
            notificationCount += notification->itemCount();
        }

        // Only publish a notification if one doesn't exist or we have new notifications to publish.
        if (haveNewNotifs) {
            if (notification == 0) {
                notification = new Notification;
                notification->setCategory("x-nemo.social.facebook.notification");
                notification->setHintValue("x-nemo.sociald.account-id", accountId);
            }

            // When clicked, take the user to their notifications list page
            QStringList openUrlArgs(QLatin1String("https://touch.facebook.com/notifications"));
            notification->setRemoteDBusCallServiceName("org.sailfishos.browser");
            notification->setRemoteDBusCallObjectPath("/");
            notification->setRemoteDBusCallInterface("org.sailfishos.browser");
            notification->setRemoteDBusCallMethodName("openUrl");
            notification->setRemoteDBusCallArguments(QVariantList() << openUrlArgs);

            //: The summary text of the Facebook Notifications device notification
            //% "You have %n new notification(s)!"
            QString summary = qtTrId("sociald_facebook_notifications-notification_body", notificationCount);
            //: The body text of the Facebook Notifications device notification, describing that it came from Facebook.
            //% "Facebook"
            QString body = qtTrId("sociald_facebook_notifications-notification_summary");
            notification->setSummary(summary);
            notification->setBody(body);
            notification->setPreviewSummary(summary);
            notification->setPreviewBody(body);
            notification->setItemCount(notificationCount);
            notification->setTimestamp(QDateTime::currentDateTime());
            notification->publish();
        } else if (notificationCount == 0 && notification != 0) {
            // Destroy any existing notification if there should be no notifications
            notification->close();
        }
        delete notification;
    } else {
        // error occurred during request.
        TRACE(SOCIALD_ERROR,
                QString(QLatin1String("error: unable to parse notification data from request with account %1; got: %2"))
                .arg(accountId).arg(QString::fromLatin1(replyData.constData())));
    }

    // we're finished this request.  Decrement our busy semaphore.
    decrementSemaphore(accountId);
}

Notification *FacebookNotificationSyncAdaptor::existingNemoNotification(int accountId)
{
    foreach (QObject *object, Notification::notifications()) {
        Notification *notification = static_cast<Notification *>(object);
        if (notification->category() == "x-nemo.social.facebook.notification" && notification->hintValue("x-nemo.sociald.account-id").toInt() == accountId) {
            return notification;
        }
    }
    return 0;
}