TEMPLATE = subdirs

SUBDIRS += minimal
SUBDIRS += linuxfb
SUBDIRS += palm
SUBDIRS += webos

contains(DEFINES, TASKONE) {
    SUBDIRS += taskone
}

contains(QT_CONFIG, wayland) {
    SUBDIRS += wayland
}

qnx {
    SUBDIRS += blackberry
}
