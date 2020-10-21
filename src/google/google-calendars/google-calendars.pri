CONFIG += link_pkgconfig
PKGCONFIG += libmkcal-qt5 libkcalcoren-qt5
SOURCES += \
    $$PWD/googlecalendarsyncadaptor.cpp \
    $$PWD/googlecalendarsyncerror.cpp
HEADERS += \
    $$PWD/googlecalendarsyncadaptor.h \
    $$PWD/googlecalendarincidencecomparator.h \
    $$PWD/googlecalendarsyncerror.h
INCLUDEPATH += $$PWD

