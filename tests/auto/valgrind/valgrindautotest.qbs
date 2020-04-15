import qbs
import "../autotest.qbs" as Autotest

Autotest {
    Depends { name: "QtcSsh" }
    Depends { name: "Utils" }
    Depends { name: "Qt.widgets" } // TODO: Remove when qbs bug is fixed
    property path pluginDir: project.ide_source_tree + "/src/plugins/valgrind"

    Group {
        name: "XML protocol files from plugin"
        prefix: product.pluginDir + "/xmlprotocol/"
        files: ["*.h", "*.cpp"]
    }
    Group {
        name: "Callgrind files from plugin"
        prefix: product.pluginDir + "/callgrind/"
        files: ["*.h", "*.cpp"]
    }
    Group {
        name: "Memcheck runner files from plugin"
        prefix: product.pluginDir + "/memcheck/"
        files: ["*.h", "*.cpp"]
    }
    Group {
        name: "Other files from plugin"
        prefix: product.pluginDir + "/"
        files: [
            "valgrindprocess.h", "valgrindprocess.cpp",
            "valgrindrunner.h", "valgrindrunner.cpp",
        ]
    }

    cpp.defines: base.concat([
        'QT_DISABLE_DEPRECATED_BEFORE=0x040900',
    ])
    cpp.includePaths: base.concat([project.ide_source_tree + "/src/plugins"])
}
