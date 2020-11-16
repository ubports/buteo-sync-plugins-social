TEMPLATE = subdirs

SUBDIRS =

CONFIG(google): SUBDIRS += tst_google
CONFIG(twitter): SUBDIRS += tst_twitter

QMAKE_EXTRA_TARGETS += check
