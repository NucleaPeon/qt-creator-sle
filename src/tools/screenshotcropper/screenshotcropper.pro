SOURCES += \
    main.cpp\
    screenshotcropperwindow.cpp \
    cropimageview.cpp \
    ../../plugins/qtsupport/screenshotcropper.cpp

HEADERS += \
    screenshotcropperwindow.h \
    cropimageview.h \
    ../../plugins/qtsupport/screenshotcropper.h

INCLUDEPATH += \
    ../../plugins/qtsupport \
    ../../plugins

FORMS += \
    screenshotcropperwindow.ui

isEqual(QT_MAJOR_VERSION, 5):QT += widgets
