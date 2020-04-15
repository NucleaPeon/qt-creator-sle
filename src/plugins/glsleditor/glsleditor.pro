include(../../qtcreatorplugin.pri)

DEFINES += \
    GLSLEDITOR_LIBRARY

HEADERS += \
glsleditor.h \
glsleditorconstants.h \
glsleditoreditable.h \
glsleditorfactory.h \
glsleditorplugin.h \
glslfilewizard.h \
glslhighlighter.h \
glslhighlighterfactory.h \
glslautocompleter.h \
glslindenter.h \
glslhoverhandler.h \
    glslcompletionassist.h \
    reuse.h

SOURCES += \
glsleditor.cpp \
glsleditoreditable.cpp \
glsleditorfactory.cpp \
glsleditorplugin.cpp \
glslfilewizard.cpp \
glslhighlighter.cpp \
glslhighlighterfactory.cpp \
glslautocompleter.cpp \
glslindenter.cpp \
glslhoverhandler.cpp \
    glslcompletionassist.cpp \
    reuse.cpp

RESOURCES += glsleditor.qrc
