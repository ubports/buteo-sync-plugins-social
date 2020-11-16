TEMPLATE = subdirs
SUBDIRS = src

CONFIG(build-tests) {
    SUBDIRS += tests
    tests.depends = src
}

OTHER_FILES += rpm/sociald.spec
