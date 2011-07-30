TEMPLATE = subdirs

SUBDIRS += minimal
SUBDIRS += eglfs
SUBDIRS += linuxfb
SUBDIRS += webos
contains(QT_CONFIG, wayland) {
    SUBDIRS += wayland
}

