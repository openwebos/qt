TEMPLATE = subdirs

SUBDIRS += minimal
SUBDIRS += linuxfb
SUBDIRS += palm
SUBDIRS += webos
contains(QT_CONFIG, wayland) {
    SUBDIRS += wayland
}

