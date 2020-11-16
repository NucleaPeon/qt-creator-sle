include(../../qtcreatorplugin.pri)

DEFINES += \
    OBJECTIVECEDITOR_LIBRARY

RESOURCES += \
    objectiveceditorplugin.qrc

HEADERS += \
    objectiveceditorplugin.h \
    objectiveceditorfactory.h \
    objectiveceditor.h \
    objectiveceditorwidget.h \
    objectiveceditorconstants.h \
    wizard/objectivecclasswizard.h \
    wizard/objectivecclassnamepage.h \
    wizard/objectivecclasswizarddialog.h \
    wizard/objectivecsourcegenerator.h \
    tools/objectivechighlighter.h \
    tools/objectivechighlighterfactory.h \
    tools/objectivecindenter.h \
    tools/lexical/objectivecformattoken.h \
    tools/lexical/objectivecscanner.h \
    tools/lexical/sourcecodestream.h \
    wizard/objectivecsourcewizard.h \
    wizard/objectivecheaderwizard.h

SOURCES += \
    objectiveceditorplugin.cpp \
    objectiveceditorfactory.cpp \
    objectiveceditor.cpp \
    objectiveceditorwidget.cpp \
    wizard/objectivecclasswizarddialog.cpp \
    wizard/objectivecclasswizard.cpp \
    wizard/objectivecclassnamepage.cpp \
    wizard/objectivecsourcegenerator.cpp \
    tools/objectivechighlighter.cpp \
    tools/objectivechighlighterfactory.cpp \
    tools/objectivecindenter.cpp \
    tools/lexical/objectivecscanner.cpp \
    wizard/objectivecheaderwizard.cpp \
    wizard/objectivecsourcewizard.cpp
