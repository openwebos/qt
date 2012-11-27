TEMPLATE = subdirs
CONFIG += $$(WEBOS_CONFIG)

SUBDIRS += minimal
SUBDIRS += linuxfb

webos {
    desktop {
        SUBDIRS += xcb
        DEFINES += TARGET_DESKTOP
    } else:qemu* {
        SUBDIRS += palm
    } else {
        SUBDIRS += palm
        SUBDIRS += webos
        DEFINES += TARGET_DEVICE
    }
}

contains(QT_CONFIG, wayland) {
    SUBDIRS += wayland
}

qnx {
    SUBDIRS += blackberry
}
