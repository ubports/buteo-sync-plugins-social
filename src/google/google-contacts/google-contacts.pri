CONFIG += link_pkgconfig
PKGCONFIG += Qt5Contacts qtcontacts-sqlite-qt5-extensions contactcache-qt5
QT += gui

SOURCES += \
    $$PWD/googletwowaycontactsyncadaptor.cpp \
    $$PWD/googlepeopleapi.cpp \
    $$PWD/googlepeoplejson.cpp \
    $$PWD/googlecontactimagedownloader.cpp

HEADERS += \
    $$PWD/googletwowaycontactsyncadaptor.h \
    $$PWD/googlepeopleapi.h \
    $$PWD/googlepeoplejson.h \
    $$PWD/googlecontactimagedownloader.h

INCLUDEPATH += $$PWD

