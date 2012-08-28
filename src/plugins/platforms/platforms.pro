TEMPLATE = subdirs
CONFIG += $$(WEBOS_CONFIG)

SUBDIRS += minimal
SUBDIRS += linuxfb

webos {
    desktop {
        SUBDIRS += xcb
        DEFINES += TARGET_DESKTOP
    } else {
        SUBDIRS += palm
        SUBDIRS += webos
	}
} else {
    # This branch is here as a default until bitbake recipies can configure
    # the build in this manner, once thats done this else branch can be removed
    SUBDIRS += palm
    SUBDIRS += webos
}

contains(DEFINES, TASKONE) {
    SUBDIRS += taskone
}

contains(QT_CONFIG, wayland) {
    SUBDIRS += wayland
}

qnx {
    SUBDIRS += blackberry
}
