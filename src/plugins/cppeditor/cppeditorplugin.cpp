/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "cppeditorplugin.h"

#include "cppclasswizard.h"
#include "cppeditor.h"
#include "cppeditorconstants.h"
#include "cppfilewizard.h"
#include "cpphoverhandler.h"
#include "cppoutline.h"
#include "cpptypehierarchy.h"
#include "cppincludehierarchy.h"
#include "cppsnippetprovider.h"
#include "cppquickfixassistant.h"
#include "cppquickfixes.h"
#include "cpphighlighterfactory.h"

#include "cppcodemodelinspectordialog.h"

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/fileiconprovider.h>
#include <coreplugin/icore.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/progressmanager/progressmanager.h>
#include <cpptools/cpptoolsconstants.h>
#include <texteditor/texteditoractionhandler.h>
#include <texteditor/texteditorsettings.h>

#include <utils/hostosinfo.h>

#include <QCoreApplication>
#include <QStringList>

using namespace Core;
using namespace CppEditor;
using namespace CppEditor::Internal;

void registerQuickFixes(ExtensionSystem::IPlugin *plugIn);

enum { QUICKFIX_INTERVAL = 20 };

//////////////////////////// CppEditorFactory /////////////////////////////

CppEditorFactory::CppEditorFactory(CppEditorPlugin *owner) :
    m_owner(owner)
{
    setId(CppEditor::Constants::CPPEDITOR_ID);
    setDisplayName(qApp->translate("OpenWith::Editors", CppEditor::Constants::CPPEDITOR_DISPLAY_NAME));
    addMimeType(CppEditor::Constants::C_SOURCE_MIMETYPE);
    addMimeType(CppEditor::Constants::C_HEADER_MIMETYPE);
    addMimeType(CppEditor::Constants::CPP_SOURCE_MIMETYPE);
    addMimeType(CppEditor::Constants::CPP_HEADER_MIMETYPE);

    new TextEditor::TextEditorActionHandler(this, CppEditor::Constants::C_CPPEDITOR,
        TextEditor::TextEditorActionHandler::Format
        | TextEditor::TextEditorActionHandler::UnCommentSelection
        | TextEditor::TextEditorActionHandler::UnCollapseAll
        | TextEditor::TextEditorActionHandler::FollowSymbolUnderCursor);

    if (!Utils::HostOsInfo::isMacHost() && !Utils::HostOsInfo::isWindowsHost()) {
        FileIconProvider::registerIconOverlayForMimeType(":/cppeditor/images/qt_cpp.png", CppEditor::Constants::CPP_SOURCE_MIMETYPE);
        FileIconProvider::registerIconOverlayForMimeType(":/cppeditor/images/qt_c.png", CppEditor::Constants::C_SOURCE_MIMETYPE);
        FileIconProvider::registerIconOverlayForMimeType(":/cppeditor/images/qt_h.png", CppEditor::Constants::CPP_HEADER_MIMETYPE);
    }
}

IEditor *CppEditorFactory::createEditor()
{
    CPPEditorWidget *editor = new CPPEditorWidget();
    editor->setRevisionsVisible(true);
    m_owner->initializeEditor(editor);
    return editor->editor();
}

///////////////////////////////// CppEditorPlugin //////////////////////////////////

CppEditorPlugin *CppEditorPlugin::m_instance = 0;

CppEditorPlugin::CppEditorPlugin() :
    m_sortedOutline(true),
    m_renameSymbolUnderCursorAction(0),
    m_findUsagesAction(0),
    m_reparseExternallyChangedFiles(0),
    m_openTypeHierarchyAction(0),
    m_openIncludeHierarchyAction(0),
    m_quickFixProvider(0)
{
    m_instance = this;
}

CppEditorPlugin::~CppEditorPlugin()
{
    m_instance = 0;
}

CppEditorPlugin *CppEditorPlugin::instance()
{
    return m_instance;
}

void CppEditorPlugin::initializeEditor(CPPEditorWidget *editor)
{
    editor->setLanguageSettingsId(CppTools::Constants::CPP_SETTINGS_ID);
    TextEditor::TextEditorSettings::initializeEditor(editor);

    // function combo box sorting
    connect(this, SIGNAL(outlineSortingChanged(bool)),
            editor, SLOT(setSortedOutline(bool)));
}

void CppEditorPlugin::setSortedOutline(bool sorted)
{
    m_sortedOutline = sorted;
    emit outlineSortingChanged(sorted);
}

bool CppEditorPlugin::sortedOutline() const
{
    return m_sortedOutline;
}

CppQuickFixAssistProvider *CppEditorPlugin::quickFixProvider() const
{
    return m_quickFixProvider;
}

bool CppEditorPlugin::initialize(const QStringList & /*arguments*/, QString *errorMessage)
{
    if (!Core::MimeDatabase::addMimeTypes(QLatin1String(":/cppeditor/CppEditor.mimetypes.xml"), errorMessage))
        return false;

    addAutoReleasedObject(new CppEditorFactory(this));
    addAutoReleasedObject(new CppHoverHandler);
    addAutoReleasedObject(new CppOutlineWidgetFactory);
    addAutoReleasedObject(new CppTypeHierarchyFactory);
    addAutoReleasedObject(new CppIncludeHierarchyFactory);
    addAutoReleasedObject(new CppSnippetProvider);
    addAutoReleasedObject(new CppHighlighterFactory);

    m_quickFixProvider = new CppQuickFixAssistProvider;
    addAutoReleasedObject(m_quickFixProvider);
    CppEditor::Internal::registerQuickFixes(this);

    QString trCat = QCoreApplication::translate(Constants::WIZARD_CATEGORY, Constants::WIZARD_TR_CATEGORY);

    IWizard *wizard = new CppClassWizard;
    wizard->setWizardKind(IWizard::ClassWizard);
    wizard->setCategory(QLatin1String(Constants::WIZARD_CATEGORY));
    wizard->setDisplayCategory(trCat);
    wizard->setDisplayName(tr("C++ Class"));
    wizard->setId(QLatin1String("A.Class"));
    wizard->setDescription(tr("Creates a C++ header and a source file for a new class that you can add to a C++ project."));
    addAutoReleasedObject(wizard);

    wizard = new CppFileWizard(Source);
    wizard->setWizardKind(IWizard::FileWizard);
    wizard->setCategory(QLatin1String(Constants::WIZARD_CATEGORY));
    wizard->setDisplayCategory(trCat);
    wizard->setDisplayName(tr("C++ Class"));
    wizard->setDescription(tr("Creates a C++ source file that you can add to a C++ project."));
    wizard->setDisplayName(tr("C++ Source File"));
    wizard->setId(QLatin1String("B.Source"));
    addAutoReleasedObject(wizard);

    wizard = new CppFileWizard(Header);
    wizard->setWizardKind(IWizard::FileWizard);
    wizard->setCategory(QLatin1String(Constants::WIZARD_CATEGORY));
    wizard->setDisplayCategory(trCat);
    wizard->setDescription(tr("Creates a C++ header file that you can add to a C++ project."));
    wizard->setDisplayName(tr("C++ Header File"));
    wizard->setId(QLatin1String("C.Header"));
    addAutoReleasedObject(wizard);

    Context context(CppEditor::Constants::C_CPPEDITOR);

    ActionContainer *contextMenu = ActionManager::createMenu(CppEditor::Constants::M_CONTEXT);

    Command *cmd;
    ActionContainer *cppToolsMenu = ActionManager::actionContainer(CppTools::Constants::M_TOOLS_CPP);

    cmd = ActionManager::command(CppTools::Constants::SWITCH_HEADER_SOURCE);
    contextMenu->addAction(cmd);

    cmd = ActionManager::command(TextEditor::Constants::FOLLOW_SYMBOL_UNDER_CURSOR);
    contextMenu->addAction(cmd);
    cppToolsMenu->addAction(cmd);

    QAction *openPreprocessorDialog = new QAction(tr("Additional Preprocessor Directives..."), this);
    cmd = ActionManager::registerAction(openPreprocessorDialog,
                                        Constants::OPEN_PREPROCESSOR_DIALOG, context);
    cmd->setDefaultKeySequence(QKeySequence());
    connect(openPreprocessorDialog, SIGNAL(triggered()), this, SLOT(showPreProcessorDialog()));
    cppToolsMenu->addAction(cmd);

    QAction *switchDeclarationDefinition = new QAction(tr("Switch Between Function Declaration/Definition"), this);
    cmd = ActionManager::registerAction(switchDeclarationDefinition,
        Constants::SWITCH_DECLARATION_DEFINITION, context, true);
    cmd->setDefaultKeySequence(QKeySequence(tr("Shift+F2")));
    connect(switchDeclarationDefinition, SIGNAL(triggered()),
            this, SLOT(switchDeclarationDefinition()));
    contextMenu->addAction(cmd);
    cppToolsMenu->addAction(cmd);

    cmd = ActionManager::command(TextEditor::Constants::FOLLOW_SYMBOL_UNDER_CURSOR_IN_NEXT_SPLIT);
    cppToolsMenu->addAction(cmd);

    QAction *openDeclarationDefinitionInNextSplit =
            new QAction(tr("Open Function Declaration/Definition in Next Split"), this);
    cmd = ActionManager::registerAction(openDeclarationDefinitionInNextSplit,
        Constants::OPEN_DECLARATION_DEFINITION_IN_NEXT_SPLIT, context, true);
    cmd->setDefaultKeySequence(QKeySequence(Utils::HostOsInfo::isMacHost()
                                            ? tr("Meta+E, Shift+F2")
                                            : tr("Ctrl+E, Shift+F2")));
    connect(openDeclarationDefinitionInNextSplit, SIGNAL(triggered()),
            this, SLOT(openDeclarationDefinitionInNextSplit()));
    cppToolsMenu->addAction(cmd);

    m_findUsagesAction = new QAction(tr("Find Usages"), this);
    cmd = ActionManager::registerAction(m_findUsagesAction, Constants::FIND_USAGES, context);
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+U")));
    connect(m_findUsagesAction, SIGNAL(triggered()), this, SLOT(findUsages()));
    contextMenu->addAction(cmd);
    cppToolsMenu->addAction(cmd);

    m_openTypeHierarchyAction = new QAction(tr("Open Type Hierarchy"), this);
    cmd = ActionManager::registerAction(m_openTypeHierarchyAction, Constants::OPEN_TYPE_HIERARCHY, context);
    cmd->setDefaultKeySequence(QKeySequence(UseMacShortcuts ? tr("Meta+Shift+T") : tr("Ctrl+Shift+T")));
    connect(m_openTypeHierarchyAction, SIGNAL(triggered()), this, SLOT(openTypeHierarchy()));
    contextMenu->addAction(cmd);
    cppToolsMenu->addAction(cmd);

    m_openIncludeHierarchyAction = new QAction(tr("Open Include Hierarchy"), this);
    cmd = Core::ActionManager::registerAction(m_openIncludeHierarchyAction, Constants::OPEN_INCLUDE_HIERARCHY, context);
    cmd->setDefaultKeySequence(QKeySequence(Core::UseMacShortcuts ? tr("Meta+Shift+I") : tr("Ctrl+Shift+I")));
    connect(m_openIncludeHierarchyAction, SIGNAL(triggered()), this, SLOT(openIncludeHierarchy()));
    contextMenu->addAction(cmd);
    cppToolsMenu->addAction(cmd);

    // Refactoring sub-menu
    Context globalContext(Core::Constants::C_GLOBAL);
    Command *sep = contextMenu->addSeparator(globalContext);
    sep->action()->setObjectName(QLatin1String(Constants::M_REFACTORING_MENU_INSERTION_POINT));
    contextMenu->addSeparator(globalContext);

    m_renameSymbolUnderCursorAction = new QAction(tr("Rename Symbol Under Cursor"),
                                                  this);
    cmd = ActionManager::registerAction(m_renameSymbolUnderCursorAction,
                             Constants::RENAME_SYMBOL_UNDER_CURSOR,
                             context);
    cmd->setDefaultKeySequence(QKeySequence(tr("CTRL+SHIFT+R")));
    connect(m_renameSymbolUnderCursorAction, SIGNAL(triggered()),
            this, SLOT(renameSymbolUnderCursor()));
    cppToolsMenu->addAction(cmd);

    // Update context in global context
    cppToolsMenu->addSeparator(globalContext);
    m_reparseExternallyChangedFiles = new QAction(tr("Reparse Externally Changed Files"), this);
    cmd = ActionManager::registerAction(m_reparseExternallyChangedFiles, Constants::UPDATE_CODEMODEL, globalContext);
    CppTools::CppModelManagerInterface *cppModelManager = CppTools::CppModelManagerInterface::instance();
    connect(m_reparseExternallyChangedFiles, SIGNAL(triggered()), cppModelManager, SLOT(updateModifiedSourceFiles()));
    cppToolsMenu->addAction(cmd);

    cppToolsMenu->addSeparator(globalContext);
    QAction *inspectCppCodeModel = new QAction(tr("Inspect C++ Code Model..."), this);
    cmd = ActionManager::registerAction(inspectCppCodeModel, Constants::INSPECT_CPP_CODEMODEL, globalContext);
    cmd->setDefaultKeySequence(QKeySequence(Core::UseMacShortcuts ? tr("Meta+Shift+F12") : tr("Ctrl+Shift+F12")));
    connect(inspectCppCodeModel, SIGNAL(triggered()), this, SLOT(inspectCppCodeModel()));
    cppToolsMenu->addAction(cmd);

    contextMenu->addSeparator(context);

    cmd = ActionManager::command(TextEditor::Constants::AUTO_INDENT_SELECTION);
    contextMenu->addAction(cmd);

    cmd = ActionManager::command(TextEditor::Constants::UN_COMMENT_SELECTION);
    contextMenu->addAction(cmd);

    connect(ProgressManager::instance(), SIGNAL(taskStarted(Core::Id)),
            this, SLOT(onTaskStarted(Core::Id)));
    connect(ProgressManager::instance(), SIGNAL(allTasksFinished(Core::Id)),
            this, SLOT(onAllTasksFinished(Core::Id)));

    readSettings();
    return true;
}

void CppEditorPlugin::readSettings()
{
    m_sortedOutline = ICore::settings()->value(QLatin1String("CppTools/SortedMethodOverview"), true).toBool();
}

void CppEditorPlugin::writeSettings()
{
    ICore::settings()->setValue(QLatin1String("CppTools/SortedMethodOverview"), m_sortedOutline);
}

void CppEditorPlugin::extensionsInitialized()
{
}

ExtensionSystem::IPlugin::ShutdownFlag CppEditorPlugin::aboutToShutdown()
{
    writeSettings();
    return SynchronousShutdown;
}

static CPPEditorWidget *currentCppEditorWidget()
{
    return qobject_cast<CPPEditorWidget*>(EditorManager::currentEditor()->widget());
}

void CppEditorPlugin::switchDeclarationDefinition()
{
    if (CPPEditorWidget *editorWidget = currentCppEditorWidget())
        editorWidget->switchDeclarationDefinition(/*inNextSplit*/ false);
}

void CppEditorPlugin::openDeclarationDefinitionInNextSplit()
{
    if (CPPEditorWidget *editorWidget = currentCppEditorWidget())
        editorWidget->switchDeclarationDefinition(/*inNextSplit*/ true);
}

void CppEditorPlugin::renameSymbolUnderCursor()
{
    if (CPPEditorWidget *editorWidget = currentCppEditorWidget())
        editorWidget->renameSymbolUnderCursor();
}

void CppEditorPlugin::findUsages()
{
    if (CPPEditorWidget *editorWidget = currentCppEditorWidget())
        editorWidget->findUsages();
}

void CppEditorPlugin::showPreProcessorDialog()
{
    if (CPPEditorWidget *editorWidget = currentCppEditorWidget())
        editorWidget->showPreProcessorWidget();
}

void CppEditorPlugin::onTaskStarted(Core::Id type)
{
    if (type == CppTools::Constants::TASK_INDEX) {
        m_renameSymbolUnderCursorAction->setEnabled(false);
        m_findUsagesAction->setEnabled(false);
        m_reparseExternallyChangedFiles->setEnabled(false);
        m_openTypeHierarchyAction->setEnabled(false);
        m_openIncludeHierarchyAction->setEnabled(false);
    }
}

void CppEditorPlugin::onAllTasksFinished(Core::Id type)
{
    if (type == CppTools::Constants::TASK_INDEX) {
        m_renameSymbolUnderCursorAction->setEnabled(true);
        m_findUsagesAction->setEnabled(true);
        m_reparseExternallyChangedFiles->setEnabled(true);
        m_openTypeHierarchyAction->setEnabled(true);
        m_openIncludeHierarchyAction->setEnabled(true);
    }
}

void CppEditorPlugin::inspectCppCodeModel()
{
    if (m_cppCodeModelInspectorDialog) {
        ICore::raiseWindow(m_cppCodeModelInspectorDialog);
    } else {
        m_cppCodeModelInspectorDialog = new CppCodeModelInspectorDialog(ICore::mainWindow());
        m_cppCodeModelInspectorDialog->show();
    }
}

void CppEditorPlugin::openTypeHierarchy()
{
    if (currentCppEditorWidget()) {
        NavigationWidget *navigation = NavigationWidget::instance();
        navigation->activateSubWidget(Constants::TYPE_HIERARCHY_ID);
        emit typeHierarchyRequested();
    }
}

void CppEditorPlugin::openIncludeHierarchy()
{
    if (currentCppEditorWidget()) {
        Core::NavigationWidget *navigation = Core::NavigationWidget::instance();
        navigation->activateSubWidget(Core::Id(Constants::INCLUDE_HIERARCHY_ID));
        emit includeHierarchyRequested();
    }
}

Q_EXPORT_PLUGIN(CppEditorPlugin)
