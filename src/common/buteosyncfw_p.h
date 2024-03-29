/****************************************************************************
 **
 ** Copyright (C) 2014 Jolla Ltd.
 ** Contact: Chris Adams <chris.adams@jollamobile.com>
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

#ifndef SOCIALD_BUTEOSYNCFW_P_H
#define SOCIALD_BUTEOSYNCFW_P_H

#include <SyncCommonDefs.h>
#include <SyncPluginBase.h>
#include <ProfileManager.h>
#include <ClientPlugin.h>
#include <SyncResults.h>
#include <ProfileEngineDefs.h>
#include <SyncProfile.h>
#include <Profile.h>
#include <PluginCbInterface.h>
#include <LogMacros.h>

#ifndef SOCIALD_TEST_DEFINE
#define PRIVILEGED_DATA_DIR QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QLatin1String("/.local/share/system/privileged")
#endif

#endif // SOCIALD_BUTEOSYNCFW_P_H
