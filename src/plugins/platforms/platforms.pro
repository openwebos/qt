TEMPLATE = subdirs

SUBDIRS += minimal
SUBDIRS += linuxfb
#SUBDIRS += palm
#SUBDIRS += webos
SUBDIRS += goodlife

contains(QT_CONFIG, wayland) {
    SUBDIRS += wayland
}

qnx {
    SUBDIRS += blackberry
}
