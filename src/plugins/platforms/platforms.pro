TEMPLATE = subdirs

SUBDIRS += minimal
SUBDIRS += linuxfb

contains(DEFINES, GOODLIFE) {
    SUBDIRS += goodlife
} else {
    SUBDIRS += palm
    SUBDIRS += webos
}

contains(QT_CONFIG, wayland) {
    SUBDIRS += wayland
}

qnx {
    SUBDIRS += blackberry
}
