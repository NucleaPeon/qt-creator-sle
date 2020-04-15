#############################################################################
##
## Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
## Contact: http://www.qt-project.org/legal
##
## This file is part of Qt Creator.
##
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and Digia.  For licensing terms and
## conditions see http://qt.digia.com/licensing.  For further information
## use the contact form at http://qt.digia.com/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 2.1 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL included in the
## packaging of this file.  Please review the following information to
## ensure the GNU Lesser General Public License version 2.1 requirements
## will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
##
## In addition, as a special exception, Digia gives you certain additional
## rights.  These rights are described in the Digia Qt LGPL Exception
## version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
##
#############################################################################

import __builtin__
import re

def openQmakeProject(projectPath, targets=Targets.desktopTargetClasses(), fromWelcome=False):
    cleanUpUserFiles(projectPath)
    if fromWelcome:
        mouseClick(waitForObject(":OpenProject_QStyleItem"), 5, 5, 0, Qt.LeftButton)
        if not platform.system() == "Darwin":
            waitFor("waitForObject(':fileNameEdit_QLineEdit', 1000).focus == True", 3000)
    else:
        invokeMenuItem("File", "Open File or Project...")
    selectFromFileDialog(projectPath)
    try:
        # handle update generated files dialog
        waitForObject("{type='QLabel' name='qt_msgbox_label' visible='1' "
                      "text?='The following files are either outdated or have been modified*' "
                      "window={type='QMessageBox' unnamed='1' visible='1'}}", 3000)
        clickButton(waitForObject("{text='Yes' type='QPushButton' unnamed='1' visible='1'}"))
    except:
        pass
    checkedTargets = __chooseTargets__(targets)
    configureButton = waitForObject("{text='Configure Project' type='QPushButton' unnamed='1' visible='1'"
                                    "window=':Qt Creator_Core::Internal::MainWindow'}")
    clickButton(configureButton)
    return checkedTargets

def openCmakeProject(projectPath, buildDir):
    invokeMenuItem("File", "Open File or Project...")
    selectFromFileDialog(projectPath)
    replaceEditorContent("{type='Utils::FancyLineEdit' unnamed='1' visible='1'"
                         "window=':CMake Wizard_CMakeProjectManager::Internal::CMakeOpenProjectWizard'}", buildDir)
    clickButton(waitForObject(":CMake Wizard.Next_QPushButton"))
    return __handleCmakeWizardPage__()

def __handleCmakeWizardPage__():
    generatorCombo = waitForObject(":Generator:_QComboBox")
    generatorText = "Unix Generator (Desktop 480 GCC)"
    if platform.system() in ('Windows', 'Microsoft'):
        generatorText = "MinGW Generator (Desktop 480 GCC)"
    index = generatorCombo.findText(generatorText)
    if index == -1:
        test.warning("No matching CMake generator for found.")
    else:
        generatorCombo.setCurrentIndex(index)

    clickButton(waitForObject(":CMake Wizard.Run CMake_QPushButton"))
    try:
        clickButton(waitForObject(":CMake Wizard.Finish_QPushButton", 60000))
    except LookupError:
        cmakeOutput = waitForObject("{type='QPlainTextEdit' unnamed='1' visible='1' "
                                    "window=':CMake Wizard_CMakeProjectManager::Internal::CMakeOpenProjectWizard'}")
        test.warning("Error while executing cmake - see details for cmake output.",
                     str(cmakeOutput.plainText))
        clickButton(waitForObject(":CMake Wizard.Cancel_QPushButton"))
        return False
    return True

# this function returns a list of available targets - this is not 100% error proof
# because the Simulator target is added for some cases even when Simulator has not
# been set up inside Qt versions/Toolchains
# this list can be used in __chooseTargets__()
def __createProjectOrFileSelectType__(category, template, fromWelcome = False, isProject=True):
    if fromWelcome:
        mouseClick(waitForObject(":CreateProject_QStyleItem"), 5, 5, 0, Qt.LeftButton)
    else:
        invokeMenuItem("File", "New File or Project...")
    categoriesView = waitForObject("{type='QTreeView' name='templateCategoryView'}")
    if isProject:
        clickItem(categoriesView, "Projects." + category, 5, 5, 0, Qt.LeftButton)
    else:
        clickItem(categoriesView, "Files and Classes." + category, 5, 5, 0, Qt.LeftButton)
    templatesView = waitForObject("{name='templatesView' type='QListView'}")
    clickItem(templatesView, template, 5, 5, 0, Qt.LeftButton)
    text = waitForObject("{type='QTextBrowser' name='templateDescription' visible='1'}").plainText
    clickButton(waitForObject("{text='Choose...' type='QPushButton' unnamed='1' visible='1'}"))
    return __getSupportedPlatforms__(str(text), template)[0]

def __createProjectSetNameAndPath__(path, projectName = None, checks = True, libType = None):
    directoryEdit = waitForObject("{type='Utils::FancyLineEdit' unnamed='1' visible='1' "
                                  "toolTip?='Full path: *'}")
    replaceEditorContent(directoryEdit, path)
    projectNameEdit = waitForObject("{name='nameLineEdit' visible='1' "
                                    "type='Utils::ProjectNameValidatingLineEdit'}")
    if projectName == None:
        projectName = projectNameEdit.text
    else:
        replaceEditorContent(projectNameEdit, projectName)
    if checks:
        stateLabel = findObject("{type='QLabel' name='stateLabel'}")
        labelCheck = stateLabel.text=="" and stateLabel.styleSheet == ""
        test.verify(labelCheck, "Project name and base directory without warning or error")
    # make sure this is not set as default location
    ensureChecked("{type='QCheckBox' name='projectsDirectoryCheckBox' visible='1'}", False)
    if libType != None:
        selectFromCombo(waitForObject("{leftWidget={text='Type' type='QLabel' unnamed='1' "
                                      "visible='1'} type='QComboBox' unnamed='1' visible='1'}"),
                        LibType.getStringForLib(libType))
    clickButton(waitForObject(":Next_QPushButton"))
    return str(projectName)

def __createProjectHandleQtQuickSelection__(qtQuickOrControlsVersion):
    comboBox = waitForObject("{type='QComboBox' unnamed='1' visible='1' "
                             "leftWidget={text='Qt Quick component set:' type='QLabel' unnamed='1' "
                             "visible='1'}}")
    try:
        selectFromCombo(comboBox, "Qt Quick %s" % qtQuickOrControlsVersion)
    except:
        t,v = sys.exc_info()[:2]
        test.fatal("Exception while trying to select Qt Quick version", "%s (%s)" % (str(t), str(v)))
    label = waitForObject("{type='QLabel' unnamed='1' visible='1' text?='Creates a *' }")
    requires = re.match(".*Requires Qt (\d\.\d).*", str(label.text))
    if requires:
        requires = requires.group(1)
    clickButton(waitForObject(":Next_QPushButton"))
    return requires

# Selects the Qt versions for a project
# param checks turns tests in the function on if set to True
# param available a list holding the available targets
def __selectQtVersionDesktop__(checks, available=None):
    checkedTargets = __chooseTargets__(Targets.desktopTargetClasses(), available)
    if checks:
        for target in checkedTargets:
            detailsWidget = waitForObject("{type='Utils::DetailsWidget' unnamed='1' visible='1' "
                                          "summaryText='%s'}" % Targets.getStringForTarget(target))
            detailsButton = getChildByClass(detailsWidget, "Utils::DetailsButton")
            if test.verify(detailsButton != None, "Verifying if 'Details' button could be found"):
                clickButton(detailsButton)
                cbObject = ("{type='QCheckBox' text='%s' unnamed='1' visible='1' "
                            "container=%s}")
                verifyChecked(cbObject % ("Debug", objectMap.realName(detailsWidget)))
                verifyChecked(cbObject % ("Release", objectMap.realName(detailsWidget)))
                clickButton(detailsButton)
    clickButton(waitForObject(":Next_QPushButton"))
    return checkedTargets

def __createProjectHandleLastPage__(expectedFiles = None, addToVersionControl = "<None>", addToProject = None):
    if expectedFiles != None:
        summary = waitForObject("{name='filesLabel' text?='<qt>Files to be added in<pre>*</pre>' "
                                "type='QLabel' visible='1'}").text
        verifyItemOrder(expectedFiles, summary)
    if addToProject:
        selectFromCombo(":projectComboBox_QComboBox", addToProject)
    selectFromCombo(":addToVersionControlComboBox_QComboBox", addToVersionControl)
    clickButton(waitForObject("{type='QPushButton' text~='(Finish|Done)' visible='1'}"))

def __verifyFileCreation__(path, expectedFiles):
    for filename in expectedFiles:
        if filename != path:
            filename = os.path.join(path, filename)
        test.verify(os.path.exists(filename), "Checking if '" + filename + "' was created")

def __modifyAvailableTargets__(available, requiredQt, asStrings=False):
    threeDigits = re.compile("\d{3}")
    requiredQtVersion = requiredQt.replace(".", "") + "0"
    tmp = list(available) # we need a deep copy
    for currentItem in tmp:
        if asStrings:
            item = currentItem
        else:
            item = Targets.getStringForTarget(currentItem)
        found = threeDigits.search(item)
        if found:
            if found.group(0) < requiredQtVersion:
                # Quick 1.1 supports 4.7.4 only for running, debugging is unsupported
                # so the least required version is 4.8, but 4.7.4 will be still listed
                if not (requiredQtVersion == "480" and found.group(0) == "474"):
                    available.remove(currentItem)
            if requiredQtVersion > "480":
                toBeRemoved = [Targets.EMBEDDED_LINUX, Targets.SIMULATOR]
                if asStrings:
                    toBeRemoved = Targets.getTargetsAsStrings(toBeRemoved)
                for t in toBeRemoved:
                    if t in available:
                        available.remove(t)

# Creates a Qt GUI project
# param path specifies where to create the project
# param projectName is the name for the new project
# param checks turns tests in the function on if set to True
def createProject_Qt_GUI(path, projectName, checks = True, addToVersionControl = "<None>"):
    template = "Qt Widgets Application"
    available = __createProjectOrFileSelectType__("  Applications", template)
    __createProjectSetNameAndPath__(path, projectName, checks)
    checkedTargets = __selectQtVersionDesktop__(checks, available)

    if checks:
        exp_filename = "mainwindow"
        h_file = exp_filename + ".h"
        cpp_file = exp_filename + ".cpp"
        ui_file = exp_filename + ".ui"
        pro_file = projectName + ".pro"

        waitFor("object.exists(':headerFileLineEdit_Utils::FileNameValidatingLineEdit')", 20000)
        waitFor("object.exists(':sourceFileLineEdit_Utils::FileNameValidatingLineEdit')", 20000)
        waitFor("object.exists(':formFileLineEdit_Utils::FileNameValidatingLineEdit')", 20000)

        test.compare(findObject(":headerFileLineEdit_Utils::FileNameValidatingLineEdit").text, h_file)
        test.compare(findObject(":sourceFileLineEdit_Utils::FileNameValidatingLineEdit").text, cpp_file)
        test.compare(findObject(":formFileLineEdit_Utils::FileNameValidatingLineEdit").text, ui_file)

    clickButton(waitForObject(":Next_QPushButton"))

    expectedFiles = None
    if checks:
        if platform.system() in ('Windows', 'Microsoft'):
            path = os.path.abspath(path)
        path = os.path.join(path, projectName)
        expectedFiles = [path]
        expectedFiles.extend(__sortFilenamesOSDependent__(["main.cpp", cpp_file, h_file, ui_file, pro_file]))
    __createProjectHandleLastPage__(expectedFiles, addToVersionControl)

    progressBarWait(20000)
    if checks:
        __verifyFileCreation__(path, expectedFiles)
    return checkedTargets

# Creates a Qt Console project
# param path specifies where to create the project
# param projectName is the name for the new project
# param checks turns tests in the function on if set to True
def createProject_Qt_Console(path, projectName, checks = True):
    available = __createProjectOrFileSelectType__("  Applications", "Qt Console Application")
    __createProjectSetNameAndPath__(path, projectName, checks)
    checkedTargets = __selectQtVersionDesktop__(checks, available)

    expectedFiles = None
    if checks:
        if platform.system() in ('Windows', 'Microsoft'):
            path = os.path.abspath(path)
        path = os.path.join(path, projectName)
        cpp_file = "main.cpp"
        pro_file = projectName + ".pro"
        expectedFiles = [path]
        expectedFiles.extend(__sortFilenamesOSDependent__([cpp_file, pro_file]))
    __createProjectHandleLastPage__(expectedFiles)

    progressBarWait(10000)
    if checks:
        __verifyFileCreation__(path, expectedFiles)
    return checkedTargets

def createNewQtQuickApplication(workingDir, projectName = None,
                                targets=Targets.desktopTargetClasses(), qtQuickVersion="1.1",
                                fromWelcome=False):
    available = __createProjectOrFileSelectType__("  Applications", "Qt Quick Application", fromWelcome)
    projectName = __createProjectSetNameAndPath__(workingDir, projectName)
    requiredQt = __createProjectHandleQtQuickSelection__(qtQuickVersion)
    __modifyAvailableTargets__(available, requiredQt)
    checkedTargets = __chooseTargets__(targets, available)
    snooze(1)
    clickButton(waitForObject(":Next_QPushButton"))
    __createProjectHandleLastPage__()

    progressBarWait(10000)
    return checkedTargets, projectName

def createNewQtQuickUI(workingDir, qtQuickVersion="1.1"):
    __createProjectOrFileSelectType__("  Applications", "Qt Quick UI")
    if workingDir == None:
        workingDir = tempDir()
    projectName = __createProjectSetNameAndPath__(workingDir)
    __createProjectHandleQtQuickSelection__(qtQuickVersion)
    __createProjectHandleLastPage__()
    return projectName

def createNewQmlExtension(workingDir, targets=Targets.DESKTOP_474_GCC, qtQuickVersion=1):
    available = __createProjectOrFileSelectType__("  Libraries", "Qt Quick %d Extension Plugin"
                                                  % qtQuickVersion)
    if workingDir == None:
        workingDir = tempDir()
    __createProjectSetNameAndPath__(workingDir)
    checkedTargets = __chooseTargets__(targets, available)
    nextButton = waitForObject(":Next_QPushButton")
    clickButton(nextButton)
    nameLineEd = waitForObject("{buddy={type='QLabel' text='Object class-name:' unnamed='1' visible='1'} "
                               "type='QLineEdit' unnamed='1' visible='1'}")
    replaceEditorContent(nameLineEd, "TestItem")
    uriLineEd = waitForObject("{buddy={type='QLabel' text='URI:' unnamed='1' visible='1'} "
                              "type='QLineEdit' unnamed='1' visible='1'}")
    replaceEditorContent(uriLineEd, "org.qt-project.test.qmlcomponents")
    clickButton(nextButton)
    __createProjectHandleLastPage__()
    return checkedTargets

def createEmptyQtProject(workingDir=None, projectName=None, targets=Targets.desktopTargetClasses()):
    __createProjectOrFileSelectType__("  Other Project", "Empty Qt Project")
    if workingDir == None:
        workingDir = tempDir()
    projectName = __createProjectSetNameAndPath__(workingDir, projectName)
    checkedTargets = __chooseTargets__(targets)
    snooze(1)
    clickButton(waitForObject(":Next_QPushButton"))
    __createProjectHandleLastPage__()
    return projectName, checkedTargets

def createNewNonQtProject(workingDir=None, projectName=None, target=Targets.DESKTOP_474_GCC,
                          plainC=False, cmake=False):
    if plainC:
        template = "Plain C Project"
    else:
        template = "Plain C++ Project"
    if cmake:
        template += " (CMake Build)"
    available = __createProjectOrFileSelectType__("  Non-Qt Project", template)
    if workingDir == None:
        workingDir = tempDir()
    projectName = __createProjectSetNameAndPath__(workingDir, projectName)
    if cmake:
        __createProjectHandleLastPage__()
        clickButton(waitForObject(":Next_QPushButton"))
        if not __handleCmakeWizardPage__():
            return None
    else:
        __chooseTargets__(target, availableTargets=available)
        clickButton(waitForObject(":Next_QPushButton"))
        __createProjectHandleLastPage__()
    return projectName

def createNewCPPLib(projectDir = None, projectName = None, className = None, fromWelcome = False,
                    target = Targets.DESKTOP_474_GCC, isStatic = False, modules = ["QtCore"]):
    available = __createProjectOrFileSelectType__("  Libraries", "C++ Library", fromWelcome, True)
    if isStatic:
        libType = LibType.STATIC
    else:
        libType = LibType.SHARED
    if projectDir == None:
        projectDir = tempDir()
    projectName = __createProjectSetNameAndPath__(projectDir, projectName, False, libType)
    checkedTargets = __chooseTargets__(target, available)
    snooze(1)
    clickButton(waitForObject(":Next_QPushButton"))
    __createProjectHandleModuleSelection__(modules)
    className = __createProjectHandleClassInformation__(className)
    __createProjectHandleLastPage__()
    return checkedTargets, projectName, className

def createNewQtPlugin(projectDir=None, projectName=None, className=None, fromWelcome=False,
                      target=Targets.DESKTOP_474_GCC, baseClass="QGenericPlugin"):
    available = __createProjectOrFileSelectType__("  Libraries", "C++ Library", fromWelcome, True)
    if projectDir == None:
        projectDir = tempDir()
    projectName = __createProjectSetNameAndPath__(projectDir, projectName, False, LibType.QT_PLUGIN)
    checkedTargets = __chooseTargets__(target, available)
    snooze(1)
    clickButton(waitForObject(":Next_QPushButton"))
    className = __createProjectHandleClassInformation__(className, baseClass)
    __createProjectHandleLastPage__()
    return checkedTargets, projectName, className

# parameter target can be an OR'd value of Targets
# parameter availableTargets should be the result of __createProjectOrFileSelectType__()
#           or use None as a fallback
def __chooseTargets__(targets=Targets.DESKTOP_474_GCC, availableTargets=None):
    if availableTargets != None:
        available = availableTargets
    else:
        # following targets depend on the build environment - added for further/later tests
        available = [Targets.DESKTOP_474_GCC, Targets.DESKTOP_480_GCC, Targets.DESKTOP_501_DEFAULT,
                     Targets.DESKTOP_521_DEFAULT, Targets.MAEMO5, Targets.EMBEDDED_LINUX,
                     Targets.SIMULATOR, Targets.HARMATTAN]
        if platform.system() in ('Windows', 'Microsoft'):
            available.remove(Targets.EMBEDDED_LINUX)
            available.append(Targets.DESKTOP_480_MSVC2010)
    for target in filter(lambda x: x in available,
                         (Targets.MAEMO5, Targets.HARMATTAN)):
        available.remove(target)
    checkedTargets = []
    for current in available:
        mustCheck = targets & current == current
        try:
            ensureChecked("{type='QCheckBox' text='%s' visible='1'}" % Targets.getStringForTarget(current),
                          mustCheck, 3000)
            if (mustCheck):
                checkedTargets.append(current)
        except LookupError:
            if mustCheck:
                test.fail("Failed to check target '%s'." % Targets.getStringForTarget(current))
            else:
                # Simulator has been added without knowing whether configured or not - so skip warning here?
                if current != Targets.SIMULATOR:
                    test.warning("Target '%s' is not set up correctly." % Targets.getStringForTarget(current))
    return checkedTargets

def __createProjectHandleModuleSelection__(modules):
    modulesPage = waitForObject("{type='QmakeProjectManager::Internal::ModulesPage' unnamed='1' "
                                "visible='1'}")
    chckBoxes = filter(lambda x: className(x) == 'QCheckBox', object.children(modulesPage))
    chckBoxLabels = set([str(cb.text) for cb in chckBoxes])
    if not set(modules).issubset(chckBoxLabels):
        test.fatal("You want to check module(s) not available at 'Module Selection' page.",
                   "Not available: %s" % str(set(modules).difference(chckBoxLabels)))
    for checkBox in chckBoxes:
        test.log("(Un)Checking module checkbox '%s'" % str(checkBox.text))
        ensureChecked(checkBox, str(checkBox.text) in modules, 3000)
    clickButton(waitForObject(":Next_QPushButton"))

def __createProjectHandleClassInformation__(className, baseClass=None):
    if baseClass:
        selectFromCombo("{name='baseClassComboBox' type='QComboBox' visible='1'}", baseClass)
    classLineEd = waitForObject("{name='classLineEdit' type='Utils::ClassNameValidatingLineEdit' "
                                "visible='1'}")
    result = str(classLineEd.text)
    if className:
        replaceEditorContent(classLineEd, className)
    try:
        waitForObject("{text='The class name contains invalid characters.' type='QLabel' "
                     "unnamed='1' visible='1'}", 1000)
        test.fatal("Class name contains invalid characters - using default.")
        replaceEditorContent(classLineEd, result)
    except:
        result = className
    clickButton(waitForObject(":Next_QPushButton"))
    return result

def waitForProcessRunning(running=True):
    outputButton = waitForObject(":Qt Creator_AppOutput_Core::Internal::OutputPaneToggleButton")
    if not waitFor("outputButton.checked", 10000):
        ensureChecked(outputButton)
    waitFor("object.exists(':Qt Creator.ReRun_QToolButton')", 20000)
    reRunButton = findObject(":Qt Creator.ReRun_QToolButton")
    waitFor("object.exists(':Qt Creator.Stop_QToolButton')", 20000)
    stopButton = findObject(":Qt Creator.Stop_QToolButton")
    return waitFor("(reRunButton.enabled != running) and (stopButton.enabled == running)", 10000)

# run and close an application
# withHookInto - if set to True the function tries to attach to the sub-process instead of simply pressing Stop inside Creator
# executable - must be defined when using hook-into
# port - must be defined when using hook-into
# function - can be a string holding a function name or a reference to the function itself - this function will be called on
# the sub-process when hooking-into has been successful - if its missing simply closing the Qt Quick app will be done
# sType the SubprocessType - is nearly mandatory - except when using the function parameter
# userDefinedType - if you set sType to SubprocessType.USER_DEFINED you must(!) specify the WindowType for hooking into
# by yourself (or use the function parameter)
# ATTENTION! Make sure this function won't fail and the sub-process will end when the function returns
# returns None if the build failed, False if the subprocess did not start, and True otherwise
def runAndCloseApp(withHookInto=False, executable=None, port=None, function=None, sType=None, userDefinedType=None, quickVersion="1.1"):
    runButton = waitForObject(":*Qt Creator.Run_Core::Internal::FancyToolButton")
    clickButton(runButton)
    if sType != SubprocessType.QT_QUICK_UI:
        waitForCompile(300000)
        buildSucceeded = checkLastBuild()
        ensureChecked(waitForObject(":Qt Creator_AppOutput_Core::Internal::OutputPaneToggleButton"))
        if not buildSucceeded:
            test.fatal("Build inside run wasn't successful - leaving test")
            return None
    if not waitForProcessRunning():
        test.fatal("Couldn't start application - leaving test")
        return False
    if sType == SubprocessType.QT_QUICK_UI and os.getenv("SYSTEST_QMLVIEWER_NO_HOOK_INTO", "0") == "1":
        withHookInto = False
    if withHookInto and not validType(sType, userDefinedType, quickVersion):
        if function != None:
            test.warning("You did not provide a valid value for the SubprocessType value - sType, but you have "
                         "provided a function to execute on the subprocess. Please ensure that your function "
                         "closes the subprocess before exiting, or this test will not complete.")
        else:
            test.warning("You did not provide a valid value for the SubprocessType value - sType, nor a "
                         "function to execute on the subprocess. Falling back to pushing the STOP button "
                         "inside creator to terminate execution of the subprocess.")
            withHookInto = False
    if withHookInto and not executable in ("", None):
        __closeSubprocessByHookingInto__(executable, port, function, sType, userDefinedType, quickVersion)
    else:
        __closeSubprocessByPushingStop__(sType)
    return True

def validType(sType, userDef, quickVersion):
    if sType == None:
        return False
    ty = SubprocessType.getWindowType(sType, quickVersion)
    return ty != None and not (ty == "user-defined" and (userDef == None or userDef.strip() == ""))

def __closeSubprocessByPushingStop__(sType):
    ensureChecked(":Qt Creator_AppOutput_Core::Internal::OutputPaneToggleButton")
    try:
        waitForObject(":Qt Creator.Stop_QToolButton", 5000)
    except:
        pass
    playButton = verifyEnabled(":Qt Creator.ReRun_QToolButton", False)
    stopButton = verifyEnabled(":Qt Creator.Stop_QToolButton")
    if stopButton.enabled:
        clickButton(stopButton)
        test.verify(waitFor("playButton.enabled", 5000), "Play button should be enabled")
        test.compare(stopButton.enabled, False, "Stop button should be disabled")
        if sType == SubprocessType.QT_QUICK_UI and platform.system() == "Darwin":
            waitFor("stopButton.enabled==False")
            snooze(2)
            nativeType("<Escape>")
    else:
        test.fatal("Subprocess does not seem to have been started.")

def __closeSubprocessByHookingInto__(executable, port, function, sType, userDefType, quickVersion):
    ensureChecked(":Qt Creator_AppOutput_Core::Internal::OutputPaneToggleButton")
    output = waitForObject("{type='Core::OutputWindow' visible='1' windowTitle='Application Output Window'}")
    if port == None:
        test.warning("I need a port number or attaching might fail.")
    else:
        waitFor("'Listening on port %d for incoming connectionsdone' in str(output.plainText)" % port, 5000)
    try:
        attachToApplication(executable)
    except:
        resetApplicationContextToCreator()
        if ("Loading Qt Wrapper failed" in str(output.plainText)
            or "Failed to assign process to job object" in str(output.plainText)):
            test.warning("Loading of Qt Wrapper failed - probably different Qt versions.",
                         "Resetting hook-into settings to continue.")
            # assuming we're still on the build settings of the current project (TODO)
            switchViewTo(ViewConstants.PROJECTS)
            if sType == SubprocessType.QT_QUICK_UI:
                if "qmlscene" in executable:
                    selectConfig = "QML Scene"
                else:
                    selectConfig = "QML Viewer"
            else:
                selectConfig = executable
            selectFromCombo(waitForObject("{buddy={text='Run configuration:' type='QLabel' "
                                          "unnamed='1' visible='1'} type='QComboBox' unnamed='1' "
                                          "visible='1'}"), selectConfig)
            switchViewTo(ViewConstants.EDIT)
            runButton = waitForObject(":*Qt Creator.Run_Core::Internal::FancyToolButton")
            clickButton(runButton)
            if not waitForProcessRunning():
                test.fatal("Something seems to be really wrong.", "Application output:"
                           % str(output.plainText))
                return False
            else:
                test.log("Application seems to be started without hooking-into.")
        else:
            test.warning("Could not attach to '%s' - using fallback of pushing STOP inside Creator." % executable)
        __closeSubprocessByPushingStop__(sType)
        return False
    if function == None:
        if sType==SubprocessType.USER_DEFINED:
            sendEvent("QCloseEvent", "{type='%s' unnamed='1' visible='1'}" % userDefType)
        else:
            sendEvent("QCloseEvent", "{type='%s' unnamed='1' visible='1'}" % SubprocessType.getWindowType(sType, quickVersion))
        resetApplicationContextToCreator()
    else:
        try:
            if isinstance(function, (str, unicode)):
                globals()[function]()
            else:
                function()
        except:
            test.fatal("Function to execute on sub-process could not be found.",
                       "Using fallback of pushing STOP inside Creator.")
            resetApplicationContextToCreator()
            __closeSubprocessByPushingStop__(sType)
    resetApplicationContextToCreator()
    if not (waitForProcessRunning(False) and waitFor("'exited with code' in str(output.plainText)", 10000)):
        test.warning("Sub-process seems not to have closed properly.")
        try:
            __closeSubprocessByPushingStop__(sType)
        except:
            pass
        if (platform.system() in ('Microsoft', 'Windows') and
            'Listening on port %d for incoming connectionsdone' % port not in str(output.plainText)):
            checkForStillRunningQmlExecutable([executable + ".exe"])
    return True

# this helper tries to reset the current application context back
# to creator - this strange work-around is needed _sometimes_ on MacOS
def resetApplicationContextToCreator():
    appCtxt = applicationContext("qtcreator")
    if appCtxt.name == "":
        appCtxt = applicationContext("Qt Creator")
    setApplicationContext(appCtxt)

# helper that examines the text (coming from the create project wizard)
# to figure out which available targets we have
# Simulator must be handled in a special way, because this depends on the
# configured Qt versions and Toolchains and cannot be looked up the same way
# if you set getAsStrings to True this function returns a list of strings instead
# of the constants defined in Targets
def __getSupportedPlatforms__(text, templateName, getAsStrings=False):
    reqPattern = re.compile("requires qt (?P<version>\d+\.\d+(\.\d+)?)", re.IGNORECASE)
    res = reqPattern.search(text)
    if res:
        version = res.group("version")
    else:
        version = None
    if 'Supported Platforms' in text:
        supports = text[text.find('Supported Platforms'):].split(":")[1].strip().split(" ")
        result = []
        if 'Desktop' in supports:
            if version == None or version < "5.0":
                result.append(Targets.DESKTOP_474_GCC)
                result.append(Targets.DESKTOP_480_GCC)
                if platform.system() in ("Linux", "Darwin"):
                    result.append(Targets.EMBEDDED_LINUX)
                elif platform.system() in ('Windows', 'Microsoft'):
                    result.append(Targets.DESKTOP_480_MSVC2010)
            result.extend([Targets.DESKTOP_501_DEFAULT, Targets.DESKTOP_521_DEFAULT])
        if 'MeeGo/Harmattan' in supports:
            result.append(Targets.HARMATTAN)
        if 'Maemo/Fremantle' in supports:
            result.append(Targets.MAEMO5)
        if not ("BlackBerry" in templateName or re.search("custom Qt Creator plugin", text)) and (version == None or version < "5.0"):
            result.append(Targets.SIMULATOR)
    elif 'Platform independent' in text:
        # MAEMO5 and HARMATTAN could be wrong here - depends on having Madde plugin enabled or not
        result = [Targets.DESKTOP_474_GCC, Targets.DESKTOP_480_GCC, Targets.DESKTOP_501_DEFAULT,
                  Targets.DESKTOP_521_DEFAULT, Targets.MAEMO5, Targets.SIMULATOR, Targets.HARMATTAN]
        if platform.system() in ('Windows', 'Microsoft'):
            result.append(Targets.DESKTOP_480_MSVC2010)
    else:
        test.warning("Returning None (__getSupportedPlatforms__())",
                     "Parsed text: '%s'" % text)
        return None, None
    if getAsStrings:
        result = Targets.getTargetsAsStrings(result)
    return result, version

# copy example project (sourceExample is path to project) to temporary directory inside repository
def prepareTemplate(sourceExample):
    templateDir = os.path.abspath(tempDir() + "/template")
    try:
        shutil.copytree(sourceExample, templateDir)
    except:
        test.fatal("Error while copying '%s' to '%s'" % (sourceExample, templateDir))
        return None
    return templateDir

# check and copy files of given dataset to an existing templateDir
def checkAndCopyFiles(dataSet, fieldName, templateDir):
    files = map(lambda record:
                os.path.normpath(os.path.join(srcPath, testData.field(record, fieldName))),
                dataSet)
    for currentFile in files:
        if not neededFilePresent(currentFile):
            return []
    return copyFilesToDir(files, templateDir)

# copy a list of files to an existing targetDir
def copyFilesToDir(files, targetDir):
    result = []
    for filepath in files:
        dst = os.path.join(targetDir, os.path.basename(filepath))
        shutil.copyfile(filepath, dst)
        result.append(dst)
    return result

def __sortFilenamesOSDependent__(filenames):
    if platform.system() in ('Windows', 'Microsoft'):
        filenames.sort(key=str.lower)
    else:
        filenames.sort()
    return filenames

def __iterateChildren__(model, parent, nestingLevel=0):
    children = []
    for currentIndex in dumpIndices(model, parent):
        children.append([str(currentIndex.text), nestingLevel])
        if model.hasChildren(currentIndex):
            children.extend(__iterateChildren__(model, currentIndex, nestingLevel + 1))
    return children

# This will write the data to a file which can then be used for comparing
def __writeProjectTreeFile__(projectTree, filename):
    f = open(filename, "w+")
    f.write('"text"\t"nestinglevel"\n')
    for elem in projectTree:
        f.write('"%s"\t"%s"\n' % (elem[0], elem[1]))
    f.close()

def __getTestData__(record):
    return [testData.field(record, "text"),
            __builtin__.int(testData.field(record, "nestinglevel"))]

def compareProjectTree(rootObject, dataset):
    root = waitForObject(rootObject)
    tree = __iterateChildren__(root.model(), root)

    # __writeProjectTreeFile__(tree, dataset)

    for i, current in enumerate(map(__getTestData__, testData.dataset(dataset))):
        try:
            # Just removing everything up to the found item
            # Writing a pass would result in truly massive logs
            tree = tree[tree.index(current) + 1:]
        except ValueError:
            test.fail('Could not find "%s" with nesting level %s' % tuple(current),
                      'Line %s in dataset' % str(i + 1))
            return
    test.passes("No errors found in project tree")

def addCPlusPlusFileToCurrentProject(name, template, forceOverwrite=False, addToVCS = "<None>"):
    if name == None:
        test.fatal("File must have a name - got None.")
        return
    __createProjectOrFileSelectType__("  C++", template, isProject=False)
    window = "{type='Utils::FileWizardDialog' unnamed='1' visible='1'}"
    basePath = str(waitForObject("{type='Utils::FancyLineEdit' unnamed='1' visible='1' "
                                 "window=%s}" % window).text)
    lineEdit = waitForObject("{name='nameLineEdit' type='Utils::FileNameValidatingLineEdit' "
                             "visible='1' window=%s}" % window)
    replaceEditorContent(lineEdit, name)
    clickButton(waitForObject(":Next_QPushButton"))
    fileExistedBefore = os.path.exists(os.path.join(basePath, name))
    __createProjectHandleLastPage__(addToVersionControl = addToVCS)
    if (fileExistedBefore):
        overwriteDialog = "{type='Core::Internal::PromptOverwriteDialog' unnamed='1' visible='1'}"
        waitForObject(overwriteDialog)
        if forceOverwrite:
            buttonToClick = 'OK'
        else:
            buttonToClick = 'Cancel'
        clickButton("{text='%s' type='QPushButton' unnamed='1' visible='1' window=%s}"
                    % (buttonToClick, overwriteDialog))

def qt5SDKPath():
    if platform.system() in ('Microsoft', 'Windows'):
        return os.path.abspath("C:/Qt/Qt5.0.1/5.0.1/msvc2010")
    elif platform.system() == 'Linux':
        if __is64BitOS__():
            return os.path.expanduser("~/Qt5.0.1/5.0.1/gcc_64")
        return os.path.expanduser("~/Qt5.0.1/5.0.1/gcc")
    else:
        return os.path.expanduser("~/Qt5.0.1/5.0.1/clang_64")
