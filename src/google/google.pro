TEMPLATE = subdirs
SUBDIRS = \
    $$PWD/google-contacts \
    $$PWD/google-signon

CONFIG(calendar): SUBDIRS += $$PWD/google-calendars
