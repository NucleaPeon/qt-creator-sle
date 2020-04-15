CONFIG          += warn_on console use_c_linker static
CONFIG          -= qt app_bundle

include(../../../qtcreator.pri)

# Switch to statically linked CRT. Note: There will be only one
# global state of the CRT, reconsider if other DLLs are required!
# TODO: No effect, currently?

win32-msvc* {
    QMAKE_CXXFLAGS_RELEASE    -= -MD
    QMAKE_CXXFLAGS_DEBUG      -= -MDd
    QMAKE_CXXFLAGS_RELEASE    += -MT
    QMAKE_CXXFLAGS_DEBUG      += -MT
} else {
    QMAKE_CXXFLAGS            += -static
}

SOURCES = wininterrupt.c

TEMPLATE        = app
DESTDIR         = $$IDE_LIBEXEC_PATH

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

ENV_CPU=$$(CPU)
ENV_LIBPATH=$$(LIBPATH)
contains(ENV_CPU, ^AMD64$) {
    TARGET = win64interrupt
} else:isEmpty(ENV_CPU):contains(ENV_LIBPATH, ^.*amd64.*$) {
    TARGET = win64interrupt
} else {
    TARGET = win32interrupt
}

target.path  = $$QTC_PREFIX/bin # FIXME: libexec, more or less
INSTALLS    += target
