QMAKE_CXXFLAGS += -Werror
CONFIG += link_pkgconfig
PKGCONFIG += \
    libsignon-qt5 \
    accounts-qt5 \
    buteosyncfw5 \
    socialcache
LIBS += -lssu

packagesExist(libsailfishkeyprovider) {
    PKGCONFIG += libsailfishkeyprovider
    DEFINES += USE_SAILFISHKEYPROVIDER
}

QT += \
    network \
    dbus \
    sql

QT -= \
    gui

QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF

# the unit tests need to provide a custom QNAM and uses a different database directory
HEADERS += $$PWD/common/socialdnetworkaccessmanager_p.h
!contains(DEFINES, 'SOCIALD_TEST_DEFINE') {
    SOURCES += $$PWD/common/socialdnetworkaccessmanager_p.cpp
}

DEFINES += 'SYNC_DATABASE_DIR=\'\"Sync\"\''
DEFINES += 'SOCIALD_SYNC_DATABASE_NAME=\'\"sociald.db\"\''

INCLUDEPATH += . $$PWD/common/

HEADERS += \
    $$PWD/common/buteosyncfw_p.h \
    $$PWD/common/socialdbuteoplugin.h \
    $$PWD/common/socialnetworksyncadaptor.h \
    $$PWD/common/trace.h

SOURCES += \
    $$PWD/common/socialdbuteoplugin.cpp \
    $$PWD/common/socialnetworksyncadaptor.cpp

contains(DEFINES, 'SOCIALD_USE_QTPIM') {
    DEFINES *= USE_CONTACTS_NAMESPACE=QTCONTACTS_USE_NAMESPACE
    PKGCONFIG += Qt5Contacts qtcontacts-sqlite-qt5-extensions
    HEADERS += $$PWD/common/constants_p.h

    # We need the moc output for ContactManagerEngine from sqlite-extensions
    extensionsIncludePath = $$system(pkg-config --cflags-only-I qtcontacts-sqlite-qt5-extensions)
    VPATH += $$replace(extensionsIncludePath, -I, )
    HEADERS += contactmanagerengine.h
}

# don't pull in buteo plugin framework for unit test builds
!contains (DEFINES, 'SOCIALD_TEST_DEFINE') {
    !contains (DEFINES, OUT_OF_PROCESS_PLUGIN) {
        TEMPLATE = lib
        CONFIG += plugin
        target.path = $$[QT_INSTALL_LIBS]/buteo-plugins-qt5
        message("building" $$TARGET "as in-process plugin")
    }
    contains (DEFINES, OUT_OF_PROCESS_PLUGIN) {
        TEMPLATE = app
        target.path = $$[QT_INSTALL_LIBS]/buteo-plugins-qt5/oopp
        message("building" $$TARGET "as out-of-process plugin")

        DEFINES += CLIENT_PLUGIN
        BUTEO_OOPP_INCLUDE_DIR = $$system(pkg-config --cflags buteosyncfw5|cut -f2 -d'I')
        INCLUDEPATH += $$BUTEO_OOPP_INCLUDE_DIR

        HEADERS += $$BUTEO_OOPP_INCLUDE_DIR/ButeoPluginIfaceAdaptor.h   \
                   $$BUTEO_OOPP_INCLUDE_DIR/PluginCbImpl.h              \
                   $$BUTEO_OOPP_INCLUDE_DIR/PluginServiceObj.h

        SOURCES += $$BUTEO_OOPP_INCLUDE_DIR/ButeoPluginIfaceAdaptor.cpp \
                   $$BUTEO_OOPP_INCLUDE_DIR/PluginCbImpl.cpp            \
                   $$BUTEO_OOPP_INCLUDE_DIR/PluginServiceObj.cpp        \
                   $$BUTEO_OOPP_INCLUDE_DIR/plugin_main.cpp
    }
}

#NOTE: This causes issues with the unit tests ?
#MOC_DIR = $$PWD/../.moc
#OBJECTS_DIR = $$PWD/../.obj
