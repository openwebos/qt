TEMPLATE = subdirs

SUBDIRS += minimal
SUBDIRS += linuxfb
#SUBDIRS += palm
SUBDIRS += webos

contains(DEFINES, GOODLIFE) {
    SUBDIRS += goodlife
}

contains(QT_CONFIG, wayland) {
    SUBDIRS += wayland
}

qnx {
    SUBDIRS += blackberry
}
