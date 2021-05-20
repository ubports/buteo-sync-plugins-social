TEMPLATE = lib

QT -= gui
QT += network dbus

CONFIG += link_pkgconfig
PKGCONFIG += \
    accounts-qt5 \
    buteosyncfw5 \
    socialcache \

TARGET = syncpluginscommon
TARGET = $$qtLibraryTarget($$TARGET)

HEADERS += \
    $$PWD/buteosyncfw_p.h \
    $$PWD/socialdbuteoplugin.h \
    $$PWD/socialnetworksyncadaptor.h \
    $$PWD/trace.h

SOURCES += \
    $$PWD/socialdbuteoplugin.cpp \
    $$PWD/socialnetworksyncadaptor.cpp

HEADERS += $$PWD/socialdnetworkaccessmanager_p.h

SOURCES += socialdnetworkaccessmanager_p.cpp

TARGETPATH = $$[QT_INSTALL_LIBS]
target.path = $$TARGETPATH

INSTALLS += target
