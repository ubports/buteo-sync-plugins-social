/*
 * Buteo sync plugin that stores locally created contacts
 * Copyright (C) 2020 Open Mobile Platform LLC.
 * Copyright (c) 2019 - 2021 Jolla Ltd.
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

#ifndef KNOWNCONTACTSPLUGIN_H
#define KNOWNCONTACTSPLUGIN_H

#include <ClientPlugin.h>
#include <SyncResults.h>
#include <buteosyncfw5/SyncPluginLoader.h>

class KnownContactsSyncer;

/*! \brief Implementation for client plugin
 *
 */
class KnownContactsPlugin : public Buteo::ClientPlugin
{
    Q_OBJECT;

public:
    /*! \brief Constructor
     *
     * @param pluginName Name of this client plugin
     * @param profile Sync profile
     * @param cbInterface Pointer to the callback interface
     */
    KnownContactsPlugin(const QString& pluginName,
                        const Buteo::SyncProfile& profile,
                        Buteo::PluginCbInterface *cbInterface);

    /*! \brief Destructor
     *
     * Call uninit before destroying the object.
     */
    virtual ~KnownContactsPlugin();

    //! @see SyncPluginBase::init
    virtual bool init();

    //! @see SyncPluginBase::uninit
    virtual bool uninit();

    //! @see ClientPlugin::startSync
    virtual bool startSync();

    //! @see SyncPluginBase::abortSync
    virtual void abortSync(Sync::SyncStatus status = Sync::SYNC_ABORTED);

    //! @see SyncPluginBase::getSyncResults
    virtual Buteo::SyncResults getSyncResults() const;

    //! @see SyncPluginBase::cleanUp
    virtual bool cleanUp();

public slots:
    //! @see SyncPluginBase::connectivityStateChanged
    virtual void connectivityStateChanged(Sync::ConnectivityType type,
                                          bool state);

protected slots:
    void syncSucceeded();

    void syncFailed();

private:
    Buteo::SyncResults m_results;
    KnownContactsSyncer *m_syncer;
};


class KnownContactsPluginLoader : public Buteo::SyncPluginLoader
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.sailfishos.plugins.sync.KnownContactsPluginLoader")
    Q_INTERFACES(Buteo::SyncPluginLoader)

public:
    Buteo::ClientPlugin* createClientPlugin(const QString& pluginName,
                                            const Buteo::SyncProfile& profile,
                                            Buteo::PluginCbInterface* cbInterface) override;
};
#endif  //  KNOWNCONTACTSPLUGIN_H
