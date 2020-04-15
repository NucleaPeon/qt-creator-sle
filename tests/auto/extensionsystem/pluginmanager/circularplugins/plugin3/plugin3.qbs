import qbs
import "../../../plugin.qbs" as Plugin

Plugin {
    name: "circular_plugin3"
    filesToCopy: "plugin.xml"
    files: ["plugin3.h", "plugin3.cpp"].concat(filesToCopy)
    cpp.defines: base.concat(["PLUGIN3_LIBRARY"])
}
