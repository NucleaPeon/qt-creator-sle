import qbs 1.0

import QtcPlugin

QtcPlugin {
    name: "DiffEditor"

    Depends { name: "Qt.widgets" }
    Depends { name: "Aggregation" }
    Depends { name: "Utils" }

    Depends { name: "Core" }
    Depends { name: "TextEditor" }

    files: [
        "diffeditor.cpp",
        "diffeditor.h",
        "diffeditor_global.h",
        "diffeditorconstants.h",
        "diffeditorcontroller.cpp",
        "diffeditorcontroller.h",
        "diffeditordocument.cpp",
        "diffeditordocument.h",
        "diffeditorfactory.cpp",
        "diffeditorfactory.h",
        "diffeditorguicontroller.cpp",
        "diffeditorguicontroller.h",
        "diffeditormanager.cpp",
        "diffeditormanager.h",
        "diffeditorplugin.cpp",
        "diffeditorplugin.h",
        "differ.cpp",
        "differ.h",
        "diffutils.cpp",
        "diffutils.h",
        "sidebysidediffeditorwidget.cpp",
        "sidebysidediffeditorwidget.h",
    ]
}

