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

#include <buteosyncfw5/LogMacros.h>
#include <QContactEmailAddress>
#include <QContactNickname>
#include <QContactOrganization>
#include <QDir>
#include <QLockFile>
#include <QStandardPaths>
#include <qtcontacts-extensions_manager_impl.h>
#include <twowaycontactsyncadapter_impl.h>

#include "knowncontactssyncer.h"

const auto KnownContactsSyncTarget = QLatin1String("knowncontacts");
const auto AccountId = QLatin1String("0");

static void setGuid(QContact *contact, const QString &id);
static void setNames(QContact *contact, const QSettings &file);
static void setPhoneNumbers(QContact *contact, const QSettings &file);
static void setEmailAddress(QContact *contact, const QSettings &file);
static void setCompanyInfo(QContact *contact, const QSettings &file);

KnownContactsSyncer::KnownContactsSyncer(QString path, QObject *parent)
    : QObject(parent)
    , QtContactsSqliteExtensions::TwoWayContactSyncAdapter(KnownContactsSyncTarget)
    , m_syncFolder(path)
{
    FUNCTION_CALL_TRACE;
}

KnownContactsSyncer::~KnownContactsSyncer()
{
    FUNCTION_CALL_TRACE;
}

bool KnownContactsSyncer::startSync()
{
    FUNCTION_CALL_TRACE;

    QDateTime remoteSince;
    if (!initSyncAdapter(AccountId, KnownContactsSyncTarget)
            || !readSyncStateData(&remoteSince, AccountId)) {
        handleSyncFailure(InitFailure);
        LOG_WARNING("Can not sync knowncontacts");
        return false;
    }

    determineRemoteChanges(remoteSince);
    return true;
}

void KnownContactsSyncer::determineRemoteChanges(const QDateTime &remoteSince)
{
    FUNCTION_CALL_TRACE;

    QDir syncDir(m_syncFolder);
    QStringList files;
    for (const auto &file : syncDir.entryList(QStringList() << "*.ini", QDir::Files)) {
        QFileInfo info(syncDir, file);
        if (info.lastModified() >= remoteSince)
            files.append(info.absoluteFilePath());
    }
    LOG_DEBUG(files.size() << "files to sync");
    if (!QMetaObject::invokeMethod(this, "asyncDeterminedRemoteChanges",
                                   Qt::QueuedConnection,
                                   Q_ARG(const QStringList, files))) {
        handleSyncFailure(CallFailure);
    }
}

void KnownContactsSyncer::asyncDeterminedRemoteChanges(const QStringList files)
{
    FUNCTION_CALL_TRACE;

    QList<QContact> contacts;
    for (const auto &path : files) {
        QSettings file(path, QSettings::IniFormat);
            contacts.append(readContacts(file));
    }
    LOG_DEBUG(contacts.size() << "contacts to store");
    if (!storeRemoteChanges(QList<QContact>(), &contacts, AccountId)) {
        handleSyncFailure(StoreChangesFailure);
        return;
    }

    QDateTime localSince;
    QList<QContact> locallyAdded, locallyModified, locallyDeleted;
    determineLocalChanges(&localSince, &locallyAdded, &locallyModified, &locallyDeleted, AccountId);
    if (!storeSyncStateData(AccountId)) {
        handleSyncFailure(StoreStateFailure);
        return;
    }

    for (const auto &path : files) {
        if (!QLockFile(path + QStringLiteral(".lock")).tryLock()) {
            LOG_DEBUG("File in use, not removing" << path);
        } else if (!QFile::remove(path)) {
            LOG_WARNING("Could not remove" << path);
        }
    }

    LOG_DEBUG("knowncontacts sync finished successfully");
    emit syncSucceeded();
}

template <typename T>
static inline T findDetail(QContact &contact, int field, const QString &value)
{
    T result;
    QList<T> details = contact.details<T>();
    for (T &detail : details) {
        if (detail.value(field) == value) {
            result = detail;
            break;
        }
    }
    return result;
}

static void setGuid(QContact *contact, const QString &id)
{
    Q_ASSERT(contact);
    auto detail = contact->detail<QContactGuid>();
    auto guid = QStringLiteral("%1:%2").arg(KnownContactsSyncTarget).arg(id);
    detail.setGuid(guid);
    contact->saveDetail(&detail);
}

static void setNames(QContact *contact, const QSettings &file)
{
    Q_ASSERT(contact);
    const auto firstName = file.value("FirstName").toString();
    const auto lastName = file.value("LastName").toString();
    if (!firstName.isEmpty() || !lastName.isEmpty()) {
        auto detail = contact->detail<QContactName>();
        if (!firstName.isEmpty())
            detail.setFirstName(firstName);
        if (!lastName.isEmpty())
            detail.setLastName(lastName);
        contact->saveDetail(&detail);
    }
}

// Using QVariant as optional (aka 'maybe') type
static inline void addPhoneNumberDetail(QContact *contact, const QString &value,
                                        const QVariant subType, const QVariant context)
{
    Q_ASSERT(contact);
    if (!value.isEmpty()) {
        auto detail = findDetail<QContactPhoneNumber>(*contact, QContactPhoneNumber::FieldNumber, value);
        detail.setValue(QContactPhoneNumber::FieldNumber, value);
        if (subType.isValid())
            detail.setSubTypes({subType.value<int>()});
        if (context.isValid())
            detail.setContexts({context.value<int>()});
        contact->saveDetail(&detail);
    }
}

static void setPhoneNumbers(QContact *contact, const QSettings &file)
{
    Q_ASSERT(contact);
    addPhoneNumberDetail(contact, file.value("Phone").toString(),
                         QContactPhoneNumber::SubTypeLandline, QVariant());
    addPhoneNumberDetail(contact, file.value("HomePhone").toString(),
                         QContactPhoneNumber::SubTypeLandline, QContactDetail::ContextHome);
    addPhoneNumberDetail(contact, file.value("MobilePhone").toString(),
                         QContactPhoneNumber::SubTypeMobile, QVariant());
}

static void setEmailAddress(QContact *contact, const QSettings &file)
{
    Q_ASSERT(contact);
    const auto emailAddress = file.value("EmailAddress").toString();
    if (!emailAddress.isEmpty()) {
        auto detail = findDetail<QContactEmailAddress>(
                *contact, QContactEmailAddress::FieldEmailAddress, emailAddress);
        detail.setValue(QContactEmailAddress::FieldEmailAddress, emailAddress);
        contact->saveDetail(&detail);
    }
}

static void setCompanyInfo(QContact *contact, const QSettings &file)
{
    Q_ASSERT(contact);
    const auto company = file.value("Company").toString();
    const auto title = file.value("Title").toString();
    const auto office = file.value("Office").toString();
    if (!title.isEmpty() || !office.isEmpty()) {
        auto detail = contact->detail<QContactOrganization>();
        if (!company.isEmpty())
            detail.setName(company);
        if (!title.isEmpty())
            detail.setTitle(title);
        if (!office.isEmpty())
            detail.setLocation(office);
        contact->saveDetail(&detail);
    }
}

QContact KnownContactsSyncer::getContact(const QString &id)
{
    QContactDetailFilter syncTargetFilter;
    syncTargetFilter.setDetailType(QContactDetail::TypeSyncTarget, QContactSyncTarget::FieldSyncTarget);
    syncTargetFilter.setValue(KnownContactsSyncTarget);
    QContactDetailFilter guidFilter;
    guidFilter.setDetailType(QContactDetail::TypeGuid, QContactGuid::FieldGuid);
    guidFilter.setValue(QStringLiteral("%1:%2").arg(KnownContactsSyncTarget).arg(id));
    guidFilter.setMatchFlags(QContactDetailFilter::MatchExactly);
    QList<QContactId> candidates = contactManager().contactIds(syncTargetFilter & guidFilter);
    if (candidates.size()) {
        if (candidates.size() > 1)
            LOG_WARNING("More than one plausible contact found");
        return contactManager().contact(candidates.at(0));
    }
    return QContact();
}

QList<QContact> KnownContactsSyncer::readContacts(QSettings &file)
{
    FUNCTION_CALL_TRACE;

    /*
     * This was implemented to support certain subset of contact fields
     * but can be extended to support more as long as the fields are
     * kept optional.
     */
    QList<QContact> contacts;
    for (const auto &id : file.childGroups()) {
        file.beginGroup(id);
        auto contact = getContact(id);
        setGuid(&contact, id);
        setNames(&contact, file);
        setPhoneNumbers(&contact, file);
        setEmailAddress(&contact, file);
        setCompanyInfo(&contact, file);

        contacts.append(contact);
        file.endGroup();
    }
    return contacts;
}

void KnownContactsSyncer::handleSyncFailure(Failure error)
{
    LOG_WARNING("knowncontacts sync failed, emitting error code" << error);
    purgeSyncStateData(AccountId);
    emit syncFailed(error);
}

bool KnownContactsSyncer::purgeData()
{
    // Remove stale contacts
    bool removed = removeAllContacts();
    // Remove OOB data
    removed &= purgeSyncStateData(AccountId, true);
    // Return true if all data got removed
    return removed;
}
