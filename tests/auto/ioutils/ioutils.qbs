import qbs
import "../autotest.qbs" as Autotest

Autotest {
    name: "IoUtils autotest"
    Depends { name: "Qt.core" }
    files: [
        project.ide_source_tree + "/src/shared/proparser/ioutils.cpp",
        "tst_ioutils.cpp"
    ]
    cpp.includePaths: base.concat([project.ide_source_tree + "/src/shared"])
}
