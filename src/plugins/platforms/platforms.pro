TEMPLATE = subdirs

SUBDIRS += minimal
SUBDIRS += eglfs linuxfb
contains(QT_CONFIG, wayland) {
    SUBDIRS += wayland
}

