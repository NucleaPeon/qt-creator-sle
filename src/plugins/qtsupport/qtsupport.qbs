import qbs 1.0

import QtcFunctions
import QtcPlugin

QtcPlugin {
    name: "QtSupport"

    Depends { name: "Qt"; submodules: ["widgets"]; }
    Depends { name: "Qt.quick"; condition: QtcFunctions.versionIsAtLeast(Qt.core.version, "5.1"); }
    Depends { name: "Aggregation" }
    Depends { name: "QmlJS" }
    Depends { name: "Utils" }

    Depends { name: "Core" }
    Depends { name: "ProjectExplorer" }
    Depends { name: "CppTools" }

    cpp.includePaths: base.concat([
        "../../shared",
    ])

    cpp.defines: base.concat([
        "QMAKE_AS_LIBRARY",
        "QMAKE_LIBRARY",
        "PROPARSER_THREAD_SAFE",
        "PROEVALUATOR_THREAD_SAFE",
        "PROEVALUATOR_CUMULATIVE",
        "QMAKE_BUILTIN_PRFS",
        "PROEVALUATOR_SETENV"
    ])

    Group {
        name: "Shared"
        prefix: "../../shared/proparser/"
        files: [
            "ioutils.cpp",
            "ioutils.h",
            "profileevaluator.cpp",
            "profileevaluator.h",
            "proitems.cpp",
            "proitems.h",
            "proparser.qrc",
            "prowriter.cpp",
            "prowriter.h",
            "qmake_global.h",
            "qmakebuiltins.cpp",
            "qmakeevaluator.cpp",
            "qmakeevaluator.h",
            "qmakeevaluator_p.h",
            "qmakeglobals.cpp",
            "qmakeglobals.h",
            "qmakeparser.cpp",
            "qmakeparser.h",
            "qmakevfs.cpp",
            "qmakevfs.h",
        ]
    }

    files: [
        "baseqtversion.cpp",
        "baseqtversion.h",
        "qtconfigwidget.cpp",
        "qtconfigwidget.h",
        "qtsupport.qrc",
        "customexecutableconfigurationwidget.cpp",
        "customexecutableconfigurationwidget.h",
        "customexecutablerunconfiguration.cpp",
        "customexecutablerunconfiguration.h",
        "debugginghelper.ui",
        "debugginghelperbuildtask.cpp",
        "debugginghelperbuildtask.h",
        "exampleslistmodel.cpp",
        "exampleslistmodel.h",
        "profilereader.cpp",
        "profilereader.h",
        "qmldumptool.cpp",
        "qmldumptool.h",
        "qtkitconfigwidget.cpp",
        "qtkitconfigwidget.h",
        "qtkitinformation.cpp",
        "qtkitinformation.h",
        "qtoptionspage.cpp",
        "qtoptionspage.h",
        "qtoutputformatter.cpp",
        "qtoutputformatter.h",
        "qtparser.cpp",
        "qtparser.h",
        "qtsupport_global.h",
        "qtsupportconstants.h",
        "qtsupportplugin.cpp",
        "qtsupportplugin.h",
        "qtversionfactory.cpp",
        "qtversionfactory.h",
        "qtversioninfo.ui",
        "qtversionmanager.cpp",
        "qtversionmanager.h",
        "qtversionmanager.ui",
        "qtfeatureprovider.h",
        "screenshotcropper.cpp",
        "screenshotcropper.h",
        "showbuildlog.ui",
        "uicodemodelsupport.cpp",
        "uicodemodelsupport.h",
        "images/forms.png",
        "images/qml.png",
        "images/qt_project.png",
        "images/qt_qrc.png",
    ]

    Group {
        name: "QtVersion"
        files: [
            "desktopqtversion.cpp", "desktopqtversion.h",
            "desktopqtversionfactory.cpp", "desktopqtversionfactory.h",
            "simulatorqtversion.cpp", "simulatorqtversion.h",
            "simulatorqtversionfactory.cpp", "simulatorqtversionfactory.h",
            "winceqtversion.cpp", "winceqtversion.h",
            "winceqtversionfactory.cpp", "winceqtversionfactory.h",
        ]
    }

    Group {
        name: "Getting Started Welcome Page"
        condition: QtcFunctions.versionIsAtLeast(Qt.core.version, "5.1")
        files: [
            "gettingstartedwelcomepage.cpp",
            "gettingstartedwelcomepage.h"
        ]
    }


    Export {
        cpp.includePaths: "../../shared"
        cpp.defines: [
            "QMAKE_AS_LIBRARY",
            "PROPARSER_THREAD_SAFE",
            "PROEVALUATOR_CUMULATIVE",
            "PROEVALUATOR_THREAD_SAFE",
            "QMAKE_BUILTIN_PRFS",
            "PROEVALUATOR_SETENV"
        ]
    }
}
