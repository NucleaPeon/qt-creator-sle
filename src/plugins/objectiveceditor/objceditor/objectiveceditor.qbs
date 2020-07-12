import qbs 1.0

import QtcPlugin

QtcPlugin {
    name: "ObjectiveCEditor"

    Depends { name: "Qt.widgets" }
    Depends { name: "Utils" }

    Depends { name: "Core" }
    Depends { name: "TextEditor" }
    Depends { name: "QtSupport" }
    Depends { name: "ProjectExplorer" }

    Group {
        name: "General"
        files: [
            "objectiveceditor.cpp", "objectiveceditor.h",
            "objectiveceditorconstants.h",
            "objectiveceditorfactory.cpp", "objectiveceditorfactory.h",
            "objectiveceditorplugin.cpp", "objectiveceditorplugin.h",
            "objectiveceditorplugin.qrc",
            "objectiveceditorwidget.cpp", "objectiveceditorwidget.h",
        ]
    }

    Group {
        name: "Tools"
        prefix: "tools/"
        files: [
            "lexical/objectivecformattoken.h",
            "lexical/objectivecscanner.h", "lexical/objectivecscanner.cpp",
            "lexical/sourcecodestream.h",
            "objectivechighlighter.h", "objectivechighlighter.cpp",
            "objectivechighlighterfactory.h", "objectivechighlighterfactory.cpp",
            "objectivecindenter.cpp", "objectivecindenter.h"
        ]
    }

    Group {
        name: "Wizard"
        prefix: "wizard/"
        files: [
            "objectivecclassnamepage.cpp", "objectivecclassnamepage.h",
            "objectivecclasswizard.h", "objectivecclasswizard.cpp",
            "objectivecclasswizarddialog.h", "objectivecclasswizarddialog.cpp",
            "objectivecfilewizard.h", "objectivecfilewizard.cpp",
            "objectivecsourcegenerator.h", "objectivecsourcegenerator.cpp"
        ]
    }
}

