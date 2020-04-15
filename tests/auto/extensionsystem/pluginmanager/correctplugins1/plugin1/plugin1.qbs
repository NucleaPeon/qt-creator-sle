import qbs
import "../../../plugin.qbs" as Plugin

Plugin {
    name: "correct_plugin1"
    Depends { name: "correct_plugin2" }
    Depends { name: "correct_plugin3" }
    filesToCopy: "plugin.spec"
    additionalRPaths: [
        destinationDirectory + "/../plugin2",
        destinationDirectory + "/../plugin3"
    ]
    files: ["plugin1.h", "plugin1.cpp"].concat(filesToCopy)
    cpp.defines: base.concat(["PLUGIN1_LIBRARY"])
}
