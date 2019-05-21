TEMPLATE = lib
CONFIG += static create_pc create_prl no_install_prl

TARGET = socialcache
VERSION = "1.0"

DEFINES += 'PRIVILEGED_DATA_DIR=\'QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + \"/system/privileged/\"\''

HEADERS = \
    socialcache/semaphore_p.h \
    socialcache/socialsyncinterface.h \
    socialcache/abstractimagedownloader.h \
    socialcache/abstractimagedownloader_p.h \
    socialcache/abstractsocialcachedatabase.h \
    socialcache/abstractsocialcachedatabase_p.h \
    socialcache/socialnetworksyncdatabase.h \
    socialcache/socialimagesdatabase.h \


SOURCES = \
    socialcache/semaphore_p.cpp \
    socialcache/socialsyncinterface.cpp \
    socialcache/abstractimagedownloader.cpp \
    socialcache/abstractsocialcachedatabase.cpp \
    socialcache/socialnetworksyncdatabase.cpp \
    socialcache/socialimagesdatabase.cpp \


QMAKE_PKGCONFIG_NAME = lib$$TARGET
QMAKE_PKGCONFIG_DESCRIPTION = Social cache development files
QMAKE_PKGCONFIG_LIBDIR = $$OUT_PWD
QMAKE_PKGCONFIG_INCDIR = $$PWD
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_VERSION = $$VERSION
