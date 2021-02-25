/****************************************************************************
 **
 ** Copyright (c) 2021 Jolla Ltd.
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

#ifndef GOOGLEPEOPLEJSON_H
#define GOOGLEPEOPLEJSON_H

#include <QContact>
#include <QContactDetail>
#include <QContactCollection>
#include <QContactAvatar>

#include <QJsonObject>
#include <QDate>

QTCONTACTS_USE_NAMESPACE

namespace GooglePeople
{
    class Source
    {
    public:
        QString type;
        QString id;
        QString etag;

        /* Ignored fields:
        QString updateTime;
        ProfileMetadata profileMetadata;
        */

        static Source fromJsonObject(const QJsonObject &obj);
    };

    class FieldMetadata
    {
    public:
        bool primary = false;
        bool verified = false;
        Source source;

        static FieldMetadata fromJsonObject(const QJsonObject &obj);
    };

    class Address
    {
    public:
        FieldMetadata metadata;
        QString formattedValue;
        QString type;
        QString formattedType;
        QString poBox;
        QString streetAddress;
        QString extendedAddress;
        QString city;
        QString region;
        QString postalCode;
        QString country;
        QString countryCode;

        static bool saveContactDetails(QContact *contact, const QList<Address> &values);
        static Address fromJsonObject(const QJsonObject &obj);
        static QJsonArray jsonValuesForContact(const QContact &contact, bool *hasChanges);
    };

    class Biography
    {
    public:
        FieldMetadata metadata;
        QString value;

        /* Ignored fields:
        QString contentType;
        */

        static bool saveContactDetails(QContact *contact, const QList<Biography> &values);
        static Biography fromJsonObject(const QJsonObject &obj);
        static QJsonArray jsonValuesForContact(const QContact &contact, bool *hasChanges);
    };

    class Birthday
    {
    public:
        FieldMetadata metadata;
        QDate date;

        /* Ignored fields:
        QString text;
        */

        static bool saveContactDetails(QContact *contact, const QList<Birthday> &values);
        static Birthday fromJsonObject(const QJsonObject &obj);
        static QJsonArray jsonValuesForContact(const QContact &contact, bool *hasChanges);
    };

    class EmailAddress
    {
    public:
        FieldMetadata metadata;
        QString value;
        QString type;
        QString formattedType;
        QString displayName;

        static bool saveContactDetails(QContact *contact, const QList<EmailAddress> &values);
        static EmailAddress fromJsonObject(const QJsonObject &obj);
        static QJsonArray jsonValuesForContact(const QContact &contact, bool *hasChanges);
    };

    class Event
    {
    public:
        FieldMetadata metadata;
        QDate date;
        QString type;

        /* Ignored fields:
        QString formattedType;
        */

        static bool saveContactDetails(QContact *contact, const QList<Event> &values);
        static Event fromJsonObject(const QJsonObject &obj);
        static QJsonArray jsonValuesForContact(const QContact &contact, bool *hasChanges);
    };

    class Membership
    {
    public:
        FieldMetadata metadata;
        QString contactGroupResourceName;

        /* Ignored fields:
        DomainMembership domainMembership;
        */

        bool matchesCollection(const QContactCollection &collection, int accountId) const;

        static bool saveContactDetails(QContact *contact,
                                       const QList<Membership> &values,
                                       int accountId,
                                       const QList<QContactCollection> &candidateCollections);
        static Membership fromJsonObject(const QJsonObject &obj);
        static QJsonArray jsonValuesForContact(const QContact &contact, bool *hasChanges);
    };

    class Name
    {
    public:
        FieldMetadata metadata;
        QString familyName;
        QString givenName;
        QString middleName;

        /* Ignored fields:
        QString displayName;
        QString displayNameLastFirst;
        QString unstructuredName;
        QString phoneticFullName;
        QString phoneticFamilyName;
        QString phoneticGivenName;
        QString phoneticMiddleName;
        QString honorificPrefix;
        QString honorificSuffix;
        QString phoneticHonorificPrefix;
        QString phoneticHonorificSuffix;
        */

        static bool saveContactDetails(QContact *contact, const QList<Name> &values);
        static Name fromJsonObject(const QJsonObject &obj);
        static QJsonArray jsonValuesForContact(const QContact &contact, bool *hasChanges);
    };

    class Nickname
    {
    public:
        FieldMetadata metadata;
        QString value;

        /* Ignored fields:
        QString type;
        */

        static bool saveContactDetails(QContact *contact, const QList<Nickname> &values);
        static Nickname fromJsonObject(const QJsonObject &obj);
        static QJsonArray jsonValuesForContact(const QContact &contact, bool *hasChanges);
    };

    class Organization
    {
    public:
        FieldMetadata metadata;
        QString name;
        QString title;
        QString jobDescription;
        QString department;

        /* Ignored fields:
        QString type;
        QString formattedType;
        QDate startDate;
        QDate endDate;
        QString phoneticName;
        QString symbol;
        QString domain;
        QString location;
        */

        static bool saveContactDetails(QContact *contact, const QList<Organization> &values);
        static Organization fromJsonObject(const QJsonObject &obj);
        static QJsonArray jsonValuesForContact(const QContact &contact, bool *hasChanges);
    };

    class PhoneNumber
    {
    public:
        FieldMetadata metadata;
        QString value;
        QString type;

        /* Ignored fields:
        QString canonicalForm;
        QString formattedType;
        */

        static bool saveContactDetails(QContact *contact, const QList<PhoneNumber> &values);
        static PhoneNumber fromJsonObject(const QJsonObject &obj);
        static QJsonArray jsonValuesForContact(const QContact &contact, bool *hasChanges);
    };

    class PersonMetadata
    {
    public:
        QList<Source> sources;
        QStringList previousResourceNames;
        QStringList linkedPeopleResourceNames;
        bool deleted = false;

        static QString etag(const QContact &contact);

        static bool saveContactDetails(QContact *contact, const PersonMetadata &value);
        static PersonMetadata fromJsonObject(const QJsonObject &obj);
        static QJsonObject toJsonObject(const QContact &contact);
    };

    class Photo
    {
    public:
        FieldMetadata metadata;
        QString url;
        bool default_ = false;

        static QContactAvatar getPrimaryPhoto(const QContact &contact,
                                       QString *remoteAvatarUrl = nullptr,
                                       QString *localAvatarFile = nullptr);

        static bool saveContactDetails(QContact *contact, const QList<Photo> &values);
        static Photo fromJsonObject(const QJsonObject &obj);
        static QJsonArray jsonValuesForContact(const QContact &contact, bool *hasChanges);
    };

    class Url
    {
    public:
        FieldMetadata metadata;
        QString value;
        QString type;
        QString formattedType;

        static bool saveContactDetails(QContact *contact, const QList<Url> &values);
        static Url fromJsonObject(const QJsonObject &obj);
        static QJsonArray jsonValuesForContact(const QContact &contact, bool *hasChanges);
    };

    class Person
    {
    public:
        QString resourceName;
        PersonMetadata metadata;
        QList<Address> addresses;
        QList<Biography> biographies;
        QList<Birthday> birthdays;
        QList<EmailAddress> emailAddresses;
        QList<Event> events;
        QList<Membership> memberships;
        QList<Name> names;
        QList<Nickname> nicknames;
        QList<Organization> organizations;
        QList<PhoneNumber> phoneNumbers;
        QList<Photo> photos;
        QList<Url> urls;

        /* Ignored fields:
        QString etag;
        QList<AgeRangeType> ageRanges;
        QList<CalendarUrl> calendarUrls;
        QList<ClientData> clientData;
        QList<CoverPhoto> coverPhotos;
        QList<ExternalId> externalIds;
        QList<FileAs> fileAses;
        QList<Gender> genders;
        QList<ImClient> imClients;
        QList<Interest> interests;
        QList<Locale> locales;
        QList<Location> locations;
        QList<MiscKeyword> miscKeywords;
        QList<Occupation> occupations;
        QList<Relation> relations;
        QList<SipAddress> sipAddresses;
        QList<Skill> skills;
        QList<UserDefined> userDefined;
        */

        inline bool isValid() const { return !resourceName.isEmpty(); }

        QContact toContact(int accountId,
                           const QList<QContactCollection> &candidateCollections) const;
        bool saveToContact(QContact *contact,
                           int accountId,
                           const QList<QContactCollection> &candidateCollections) const;

        static Person fromJsonObject(const QJsonObject &obj);
        static QJsonObject contactToJsonObject(const QContact &contact,
                                               QStringList *updatedFields = nullptr);

        static QString personResourceName(const QContact &contact);
        static QStringList supportedPersonFields();

    private:
        static QString guidForPerson(int accountId, const QString &resourceName);
    };

    class ContactGroupMetadata
    {
    public:
        QDateTime updateTime;
        bool deleted = false;

        static ContactGroupMetadata fromJsonObject(const QJsonObject &obj);
    };

    class ContactGroup
    {
    public:
        QString resourceName;
        QString etag;
        ContactGroupMetadata contactGroupMetadata;
        QString groupType;
        QString name;
        QString formattedName;
        QStringList memberResourceNames;
        int memberCount = 0;

        bool isMyContactsGroup() const;
        QContactCollection toCollection(int accountId) const;

        static bool isMyContactsCollection(const QContactCollection &collection, int accountId = 0);
        static ContactGroup fromJsonObject(const QJsonObject &obj);
    };
}

QDebug operator<<(QDebug debug, const GooglePeople::Source &value);
QDebug operator<<(QDebug debug, const GooglePeople::FieldMetadata &value);
QDebug operator<<(QDebug debug, const GooglePeople::Address &value);
QDebug operator<<(QDebug debug, const GooglePeople::Biography &value);
QDebug operator<<(QDebug debug, const GooglePeople::Birthday &value);
QDebug operator<<(QDebug debug, const GooglePeople::EmailAddress &value);
QDebug operator<<(QDebug debug, const GooglePeople::Event &value);
QDebug operator<<(QDebug debug, const GooglePeople::Membership &value);
QDebug operator<<(QDebug debug, const GooglePeople::Name &value);
QDebug operator<<(QDebug debug, const GooglePeople::Nickname &value);
QDebug operator<<(QDebug debug, const GooglePeople::Organization &value);
QDebug operator<<(QDebug debug, const GooglePeople::PhoneNumber &value);
QDebug operator<<(QDebug debug, const GooglePeople::PersonMetadata &value);
QDebug operator<<(QDebug debug, const GooglePeople::Photo &value);
QDebug operator<<(QDebug debug, const GooglePeople::Url &value);
QDebug operator<<(QDebug debug, const GooglePeople::Person &value);
QDebug operator<<(QDebug debug, const GooglePeople::ContactGroupMetadata &value);
QDebug operator<<(QDebug debug, const GooglePeople::ContactGroup &value);

#endif // GOOGLEPEOPLEJSON_H
