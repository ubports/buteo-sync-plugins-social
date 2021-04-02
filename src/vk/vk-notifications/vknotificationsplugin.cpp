/****************************************************************************
 **
 ** Copyright (C) 2014 Jolla Ltd.
 ** Contact: Chris Adams <chris.adams@jolla.com>
 **
 ****************************************************************************/

#include "vknotificationsplugin.h"
#include "vknotificationsyncadaptor.h"
#include "socialnetworksyncadaptor.h"

VKNotificationsPlugin::VKNotificationsPlugin(const QString& pluginName,
                             const Buteo::SyncProfile& profile,
                             Buteo::PluginCbInterface *callbackInterface)
    : SocialdButeoPlugin(pluginName, profile, callbackInterface,
                         QStringLiteral("vk"),
                         SocialNetworkSyncAdaptor::dataTypeName(SocialNetworkSyncAdaptor::Notifications))
{
}

VKNotificationsPlugin::~VKNotificationsPlugin()
{
}

SocialNetworkSyncAdaptor *VKNotificationsPlugin::createSocialNetworkSyncAdaptor()
{
    return new VKNotificationSyncAdaptor(this);
}

Buteo::ClientPlugin* VKNotificationsPluginLoader::createClientPlugin(
        const QString& pluginName,
        const Buteo::SyncProfile& profile,
        Buteo::PluginCbInterface* cbInterface)
{
    return new VKNotificationsPlugin(pluginName, profile, cbInterface);
}
