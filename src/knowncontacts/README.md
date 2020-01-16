Knowncontacts Sync Plugin
=========================
This is a sync plugin for Buteo framework. It stores locally created contacts,
such as email recipients that are not to be synced elsewhere and only ever
syncs to the device side.

In this case the "server" means QSettings files containing contacts. On sync
this plugin reads the files and creates local contacts from the information.

Contact file format
-------------------
A contact in QSettings file must have an id as group name and contact
information as key value pairs. There are no requirements for the id but it
must be consistent between syncs to avoid duplicating contact information. All
keys are optional. A file may have as many contacts as needed. The file must
end with file extension `ini` and they are stored in
`~/.local/share/system/privileged/Contacts/knowncontacts`.

```ini
[john.doe.example]
FirstName=John
LastName=Doe
EmailAddress=john@example.com
```

Supported keys:
- FirstName, LastName
- EmailAddress
- Phone, HomePhone, MobilePhone
- Company, Title, Office
