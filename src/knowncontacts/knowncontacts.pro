TARGET = knowncontacts-client

DEFINES += "CLASSNAME=KnownContactsPlugin"
DEFINES += CLASSNAME_H=\\\"knowncontactsplugin.h\\\"
knowncontacts_sync_profile.path = /etc/buteo/profiles/sync
knowncontacts_sync_profile.files = knowncontacts.Contacts.xml
knowncontacts_client_plugin_xml.path = /etc/buteo/profiles/client
knowncontacts_client_plugin_xml.files = knowncontacts.xml

HEADERS += \
    knowncontactsplugin.h \
    knowncontactssyncer.h

SOURCES += \
    knowncontactsplugin.cpp \
    knowncontactssyncer.cpp

OTHER_FILES = \
    knowncontacts.Contacts.xml
    knowncontacts.xml

QT -= gui
QT += dbus

CONFIG += link_pkgconfig c++14
PKGCONFIG += buteosyncfw5 Qt5Contacts qtcontacts-sqlite-qt5-extensions

QMAKE_CXXFLAGS = -Wall \
    -g \
    -Wno-cast-align \
    -O2 -finline-functions

TEMPLATE = app
target.path = $$[QT_INSTALL_LIBS]/buteo-plugins-qt5/oopp
DEFINES += CLIENT_PLUGIN
INCLUDE_DIR = $$system(pkg-config --cflags buteosyncfw5|cut -f2 -d'I')

HEADERS += $$INCLUDE_DIR/ButeoPluginIfaceAdaptor.h   \
           $$INCLUDE_DIR/PluginCbImpl.h              \
           $$INCLUDE_DIR/PluginServiceObj.h

SOURCES += $$INCLUDE_DIR/ButeoPluginIfaceAdaptor.cpp \
           $$INCLUDE_DIR/PluginCbImpl.cpp            \
           $$INCLUDE_DIR/PluginServiceObj.cpp        \
           $$INCLUDE_DIR/plugin_main.cpp

INSTALLS += target \
    knowncontacts_sync_profile \
    knowncontacts_client_plugin_xml
