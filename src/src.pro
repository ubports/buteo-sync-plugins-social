TEMPLATE = subdirs
SUBDIRS = \
    sociald

CONFIG(google): SUBDIRS += google
CONFIG(facebook): SUBDIRS += facebook
CONFIG(twitter): SUBDIRS += twitter
CONFIG(onedrive): SUBDIRS += onedrive
CONFIG(dropbox): SUBDIRS += dropbox
CONFIG(vk): SUBDIRS += vk
CONFIG(knowncontacts): SUBDIRS += knowncontacts
