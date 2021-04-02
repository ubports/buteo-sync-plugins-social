/****************************************************************************
 **
 ** Copyright (C) 2014 Jolla Ltd.
 ** Contact: Chris Adams <chris.adams@jolla.com>
 **
 ****************************************************************************/

#include "constants_p.h"
#include <qtcontacts-extensions_impl.h>
#include <qcontactoriginmetadata_impl.h>

#include "vkcontactsplugin.h"
#include "vkcontactsyncadaptor.h"
#include "socialnetworksyncadaptor.h"

VKContactsPlugin::VKContactsPlugin(const QString& pluginName,
                             const Buteo::SyncProfile& profile,
                             Buteo::PluginCbInterface *callbackInterface)
    : SocialdButeoPlugin(pluginName, profile, callbackInterface,
                         QStringLiteral("vk"),
                         SocialNetworkSyncAdaptor::dataTypeName(SocialNetworkSyncAdaptor::Contacts))
{
}

VKContactsPlugin::~VKContactsPlugin()
{
}

SocialNetworkSyncAdaptor *VKContactsPlugin::createSocialNetworkSyncAdaptor()
{
    return new VKContactSyncAdaptor(this);
}

Buteo::ClientPlugin* VKContactsPluginLoader::createClientPlugin(
        const QString& pluginName,
        const Buteo::SyncProfile& profile,
        Buteo::PluginCbInterface* cbInterface)
{
    return new VKContactsPlugin(pluginName, profile, cbInterface);
}
