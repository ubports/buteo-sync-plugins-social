TEMPLATE = subdirs
SUBDIRS = \
    common \
    sociald

sociald.depends = common

CONFIG(google): {
    SUBDIRS += google
    google.depends = common
}

CONFIG(facebook): {
    SUBDIRS += facebook
    facebook.depends = common
}

CONFIG(twitter): {
    SUBDIRS += twitter
    twitter.depends = common
}

CONFIG(onedrive): {
    SUBDIRS += onedrive
    onedrive.depends = common
}

CONFIG(dropbox): {
    SUBDIRS += dropbox
    dropbox.depends = common
}

CONFIG(vk): {
    SUBDIRS += vk
    vk.depends = common
}

CONFIG(knowncontacts): {
    SUBDIRS += knowncontacts
    knowncontacts.depends = common
}
