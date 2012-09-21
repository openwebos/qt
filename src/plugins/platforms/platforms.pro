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
    } else:taskone {
        SUBDIRS += taskone
        SUBDIRS += palm
    } else {
        SUBDIRS += palm
        SUBDIRS += webos
        DEFINES += TARGET_DEVICE
    }
}

contains(DEFINES, TASKONE) {
    # This should be removed once taskone is built through
    # oe-core, see the above config
    SUBDIRS += taskone
}

contains(QT_CONFIG, wayland) {
    SUBDIRS += wayland
}

qnx {
    SUBDIRS += blackberry
}
