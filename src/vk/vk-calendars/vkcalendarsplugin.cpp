/****************************************************************************
 **
 ** Copyright (c) 2014 - 2021 Jolla Ltd.
 **
 ****************************************************************************/

#include "vkcalendarsplugin.h"
#include "vkcalendarsyncadaptor.h"
#include "socialnetworksyncadaptor.h"

VKCalendarsPlugin::VKCalendarsPlugin(const QString& pluginName,
                             const Buteo::SyncProfile& profile,
                             Buteo::PluginCbInterface *callbackInterface)
    : SocialdButeoPlugin(pluginName, profile, callbackInterface,
                         QStringLiteral("vk"),
                         SocialNetworkSyncAdaptor::dataTypeName(SocialNetworkSyncAdaptor::Calendars))
{
}

VKCalendarsPlugin::~VKCalendarsPlugin()
{
}

SocialNetworkSyncAdaptor *VKCalendarsPlugin::createSocialNetworkSyncAdaptor()
{
    return new VKCalendarSyncAdaptor(this);
}

Buteo::ClientPlugin* VKCalendarsPluginLoader::createClientPlugin(
        const QString& pluginName,
        const Buteo::SyncProfile& profile,
        Buteo::PluginCbInterface* cbInterface)
{
    return new VKCalendarsPlugin(pluginName, profile, cbInterface);
}

