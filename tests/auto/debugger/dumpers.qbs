import qbs
import "../autotest.qbs" as Autotest

Autotest {
    name: "Debugger dumpers autotest"
    Depends { name: "Utils" }
    Depends { name: "Qt.network" } // For QHostAddress
    Depends { name: "Qt.widgets" }
    Group {
        name: "Sources from Debugger plugin"
        prefix: project.debuggerDir
        files: [
            "debuggerprotocol.h", "debuggerprotocol.cpp",
            "simplifytype.h", "simplifytype.cpp",
            "watchdata.h", "watchdata.cpp",
            "watchutils.h", "watchutils.cpp"
        ]
    }

    Group {
        name: "Test sources"
        files: [
            "temporarydir.h",
            "tst_dumpers.cpp"
        ]
    }

    cpp.defines: base.concat([
        'CDBEXT_PATH="' + buildDirectory + '\\\\lib"',
        'DUMPERDIR="' + path + '/../../../share/qtcreator/debugger"',
        'QT_NO_CAST_FROM_ASCII',
        'QT_DISABLE_DEPRECATED_BEFORE=0x040900'
    ])
    cpp.includePaths: base.concat([project.debuggerDir])
}
