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

#include "qmljseditor.h"

#include "qmljseditoreditable.h"
#include "qmljseditorconstants.h"
#include "qmljseditordocument.h"
#include "qmljseditorplugin.h"
#include "qmloutlinemodel.h"
#include "qmljsfindreferences.h"
#include "qmljsautocompleter.h"
#include "qmljscompletionassist.h"
#include "qmljsquickfixassist.h"

#include <qmljs/qmljsbind.h>
#include <qmljs/qmljsevaluate.h>
#include <qmljs/qmljsicontextpane.h>
#include <qmljs/qmljsmodelmanagerinterface.h>
#include <qmljs/qmljsutils.h>

#include <qmljstools/qmljstoolsconstants.h>

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/id.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/mimedatabase.h>
#include <extensionsystem/pluginmanager.h>
#include <texteditor/basetextdocument.h>
#include <texteditor/fontsettings.h>
#include <texteditor/tabsettings.h>
#include <texteditor/texteditorconstants.h>
#include <texteditor/texteditorsettings.h>
#include <texteditor/syntaxhighlighter.h>
#include <texteditor/refactoroverlay.h>
#include <texteditor/codeassist/genericproposal.h>
#include <texteditor/codeassist/basicproposalitemlistmodel.h>
#include <qmldesigner/qmldesignerconstants.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <utils/changeset.h>
#include <utils/uncommentselection.h>
#include <utils/qtcassert.h>
#include <utils/annotateditemdelegate.h>

#include <QFileInfo>
#include <QSignalMapper>
#include <QTimer>
#include <QPointer>
#include <QScopedPointer>
#include <QTextCodec>

#include <QMenu>
#include <QComboBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QToolBar>
#include <QTreeView>

enum {
    UPDATE_USES_DEFAULT_INTERVAL = 150,
    UPDATE_OUTLINE_INTERVAL = 500 // msecs after new semantic info has been arrived / cursor has moved
};

using namespace Core;
using namespace QmlJS;
using namespace QmlJS::AST;
using namespace QmlJSTools;

namespace QmlJSEditor {
namespace Internal {

QmlJSTextEditorWidget::QmlJSTextEditorWidget(QWidget *parent) :
    TextEditor::BaseTextEditorWidget(new QmlJSEditorDocument, parent)
{
    ctor();
}

QmlJSTextEditorWidget::QmlJSTextEditorWidget(QmlJSTextEditorWidget *other)
    : TextEditor::BaseTextEditorWidget(other)
{
    ctor();
}

void QmlJSTextEditorWidget::ctor()
{
    m_qmlJsEditorDocument = static_cast<QmlJSEditorDocument *>(baseTextDocument());
    m_outlineCombo = 0;
    m_contextPane = 0;
    m_findReferences = new FindReferences(this);

    setParenthesesMatchingEnabled(true);
    setMarksVisible(true);
    setCodeFoldingSupported(true);
    setAutoCompleter(new AutoCompleter);
    setLanguageSettingsId(QmlJSTools::Constants::QML_JS_SETTINGS_ID);

    m_updateUsesTimer = new QTimer(this);
    m_updateUsesTimer->setInterval(UPDATE_USES_DEFAULT_INTERVAL);
    m_updateUsesTimer->setSingleShot(true);
    connect(m_updateUsesTimer, SIGNAL(timeout()), this, SLOT(updateUses()));
    connect(this, SIGNAL(cursorPositionChanged()), m_updateUsesTimer, SLOT(start()));

    m_updateOutlineIndexTimer = new QTimer(this);
    m_updateOutlineIndexTimer->setInterval(UPDATE_OUTLINE_INTERVAL);
    m_updateOutlineIndexTimer->setSingleShot(true);
    connect(m_updateOutlineIndexTimer, SIGNAL(timeout()), this, SLOT(updateOutlineIndexNow()));

    baseTextDocument()->setCodec(QTextCodec::codecForName("UTF-8")); // qml files are defined to be utf-8

    m_modelManager = QmlJS::ModelManagerInterface::instance();
    m_contextPane = ExtensionSystem::PluginManager::getObject<QmlJS::IContextPane>();

    m_modelManager->activateScan();

    m_contextPaneTimer  = new QTimer(this);
    m_contextPaneTimer->setInterval(UPDATE_OUTLINE_INTERVAL);
    m_contextPaneTimer->setSingleShot(true);
    connect(m_contextPaneTimer, SIGNAL(timeout()), this, SLOT(updateContextPane()));
    if (m_contextPane) {
        connect(this, SIGNAL(cursorPositionChanged()), m_contextPaneTimer, SLOT(start()));
        connect(m_contextPane, SIGNAL(closed()), this, SLOT(showTextMarker()));
    }
    m_oldCursorPosition = -1;

    connect(this->document(), SIGNAL(modificationChanged(bool)), this, SLOT(modificationChanged(bool)));

    connect(m_qmlJsEditorDocument, SIGNAL(updateCodeWarnings(QmlJS::Document::Ptr)),
            this, SLOT(updateCodeWarnings(QmlJS::Document::Ptr)));
    connect(m_qmlJsEditorDocument, SIGNAL(semanticInfoUpdated(QmlJSTools::SemanticInfo)),
            this, SLOT(semanticInfoUpdated(QmlJSTools::SemanticInfo)));

    connect(this, SIGNAL(refactorMarkerClicked(TextEditor::RefactorMarker)),
            SLOT(onRefactorMarkerClicked(TextEditor::RefactorMarker)));

    setRequestMarkEnabled(true);
}

QmlJSTextEditorWidget::~QmlJSTextEditorWidget()
{
}

QModelIndex QmlJSTextEditorWidget::outlineModelIndex()
{
    if (!m_outlineModelIndex.isValid()) {
        m_outlineModelIndex = indexForPosition(position());
        emit outlineModelIndexChanged(m_outlineModelIndex);
    }
    return m_outlineModelIndex;
}

IEditor *QmlJSEditor::duplicate()
{
    QmlJSTextEditorWidget *newEditor = new QmlJSTextEditorWidget(
                qobject_cast<QmlJSTextEditorWidget *>(editorWidget()));
    TextEditor::TextEditorSettings::initializeEditor(newEditor);
    return newEditor->editor();
}

bool QmlJSEditor::open(QString *errorString, const QString &fileName, const QString &realFileName)
{
    bool b = TextEditor::BaseTextEditor::open(errorString, fileName, realFileName);
    baseTextDocument()->setMimeType(MimeDatabase::findByFile(QFileInfo(fileName)).type());
    return b;
}

static void appendExtraSelectionsForMessages(
        QList<QTextEdit::ExtraSelection> *selections,
        const QList<DiagnosticMessage> &messages,
        const QTextDocument *document)
{
    foreach (const DiagnosticMessage &d, messages) {
        const int line = d.loc.startLine;
        const int column = qMax(1U, d.loc.startColumn);

        QTextEdit::ExtraSelection sel;
        QTextCursor c(document->findBlockByNumber(line - 1));
        sel.cursor = c;

        sel.cursor.setPosition(c.position() + column - 1);

        if (d.loc.length == 0) {
            if (sel.cursor.atBlockEnd())
                sel.cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
            else
                sel.cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        } else {
            sel.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, d.loc.length);
        }

        if (d.isWarning())
            sel.format.setUnderlineColor(Qt::darkYellow);
        else
            sel.format.setUnderlineColor(Qt::red);

        sel.format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
        sel.format.setToolTip(d.message);

        selections->append(sel);
    }
}

void QmlJSTextEditorWidget::updateCodeWarnings(QmlJS::Document::Ptr doc)
{
    if (doc->ast()) {
        setExtraSelections(CodeWarningsSelection, QList<QTextEdit::ExtraSelection>());
    } else if (Document::isFullySupportedLanguage(doc->language())) {
        // show parsing errors
        QList<QTextEdit::ExtraSelection> selections;
        appendExtraSelectionsForMessages(&selections, doc->diagnosticMessages(), document());
        setExtraSelections(CodeWarningsSelection, selections);
    } else {
        setExtraSelections(CodeWarningsSelection, QList<QTextEdit::ExtraSelection>());
    }
}

void QmlJSTextEditorWidget::modificationChanged(bool changed)
{
    if (!changed && m_modelManager)
        m_modelManager->fileChangedOnDisk(baseTextDocument()->filePath());
}

void QmlJSTextEditorWidget::jumpToOutlineElement(int /*index*/)
{
    QModelIndex index = m_outlineCombo->view()->currentIndex();
    AST::SourceLocation location = m_qmlJsEditorDocument->outlineModel()->sourceLocation(index);

    if (!location.isValid())
        return;

    EditorManager::cutForwardNavigationHistory();
    EditorManager::addCurrentPositionToNavigationHistory();

    QTextCursor cursor = textCursor();
    cursor.setPosition(location.offset);
    setTextCursor(cursor);

    setFocus();
}

void QmlJSTextEditorWidget::updateOutlineIndexNow()
{
    if (!m_qmlJsEditorDocument->outlineModel()->document())
        return;

    if (m_qmlJsEditorDocument->outlineModel()->document()->editorRevision() != document()->revision()) {
        m_updateOutlineIndexTimer->start();
        return;
    }

    m_outlineModelIndex = QModelIndex(); // invalidate
    QModelIndex comboIndex = outlineModelIndex();

    if (comboIndex.isValid()) {
        bool blocked = m_outlineCombo->blockSignals(true);

        // There is no direct way to select a non-root item
        m_outlineCombo->setRootModelIndex(comboIndex.parent());
        m_outlineCombo->setCurrentIndex(comboIndex.row());
        m_outlineCombo->setRootModelIndex(QModelIndex());

        m_outlineCombo->blockSignals(blocked);
    }
}
} // namespace Internal
} // namespace QmlJSEditor

class QtQuickToolbarMarker {};
Q_DECLARE_METATYPE(QtQuickToolbarMarker)

namespace QmlJSEditor {
namespace Internal {

template <class T>
static QList<TextEditor::RefactorMarker> removeMarkersOfType(const QList<TextEditor::RefactorMarker> &markers)
{
    QList<TextEditor::RefactorMarker> result;
    foreach (const TextEditor::RefactorMarker &marker, markers) {
        if (!marker.data.canConvert<T>())
            result += marker;
    }
    return result;
}

void QmlJSTextEditorWidget::updateContextPane()
{
    const SemanticInfo info = m_qmlJsEditorDocument->semanticInfo();
    if (m_contextPane && document() && info.isValid()
            && document()->revision() == info.document->editorRevision())
    {
        Node *oldNode = info.declaringMemberNoProperties(m_oldCursorPosition);
        Node *newNode = info.declaringMemberNoProperties(position());
        if (oldNode != newNode && m_oldCursorPosition != -1)
            m_contextPane->apply(editor(), info.document, 0, newNode, false);

        if (m_contextPane->isAvailable(editor(), info.document, newNode) &&
            !m_contextPane->widget()->isVisible()) {
            QList<TextEditor::RefactorMarker> markers = removeMarkersOfType<QtQuickToolbarMarker>(refactorMarkers());
            if (UiObjectMember *m = newNode->uiObjectMemberCast()) {
                const int start = qualifiedTypeNameId(m)->identifierToken.begin();
                for (UiQualifiedId *q = qualifiedTypeNameId(m); q; q = q->next) {
                    if (! q->next) {
                        const int end = q->identifierToken.end();
                        if (position() >= start && position() <= end) {
                            TextEditor::RefactorMarker marker;
                            QTextCursor tc(document());
                            tc.setPosition(end);
                            marker.cursor = tc;
                            marker.tooltip = tr("Show Qt Quick ToolBar");
                            marker.data = QVariant::fromValue(QtQuickToolbarMarker());
                            markers.append(marker);
                        }
                    }
                }
            }
            setRefactorMarkers(markers);
        } else if (oldNode != newNode) {
            setRefactorMarkers(removeMarkersOfType<QtQuickToolbarMarker>(refactorMarkers()));
        }
        m_oldCursorPosition = position();

        setSelectedElements();
    }
}

void QmlJSTextEditorWidget::showTextMarker()
{
    m_oldCursorPosition = -1;
    updateContextPane();
}

void QmlJSTextEditorWidget::updateUses()
{
    if (m_qmlJsEditorDocument->isSemanticInfoOutdated()) // will be updated when info is updated
        return;

    QList<QTextEdit::ExtraSelection> selections;
    foreach (const AST::SourceLocation &loc,
             m_qmlJsEditorDocument->semanticInfo().idLocations.value(wordUnderCursor())) {
        if (! loc.isValid())
            continue;

        QTextEdit::ExtraSelection sel;
        sel.format = baseTextDocument()->fontSettings().toTextCharFormat(TextEditor::C_OCCURRENCES);
        sel.cursor = textCursor();
        sel.cursor.setPosition(loc.begin());
        sel.cursor.setPosition(loc.end(), QTextCursor::KeepAnchor);
        selections.append(sel);
    }

    setExtraSelections(CodeSemanticsSelection, selections);
}

class SelectedElement: protected Visitor
{
    unsigned m_cursorPositionStart;
    unsigned m_cursorPositionEnd;
    QList<UiObjectMember *> m_selectedMembers;

public:
    SelectedElement()
        : m_cursorPositionStart(0), m_cursorPositionEnd(0) {}

    QList<UiObjectMember *> operator()(const Document::Ptr &doc, unsigned startPosition, unsigned endPosition)
    {
        m_cursorPositionStart = startPosition;
        m_cursorPositionEnd = endPosition;
        m_selectedMembers.clear();
        Node::accept(doc->qmlProgram(), this);
        return m_selectedMembers;
    }

protected:

    bool isSelectable(UiObjectMember *member) const
    {
        UiQualifiedId *id = qualifiedTypeNameId(member);
        if (id) {
            const QStringRef &name = id->name;
            if (!name.isEmpty() && name.at(0).isUpper())
                return true;
        }

        return false;
    }

    inline bool isIdBinding(UiObjectMember *member) const
    {
        if (UiScriptBinding *script = cast<UiScriptBinding *>(member)) {
            if (! script->qualifiedId)
                return false;
            else if (script->qualifiedId->name.isEmpty())
                return false;
            else if (script->qualifiedId->next)
                return false;

            const QStringRef &propertyName = script->qualifiedId->name;

            if (propertyName == QLatin1String("id"))
                return true;
        }

        return false;
    }

    inline bool containsCursor(unsigned begin, unsigned end)
    {
        return m_cursorPositionStart >= begin && m_cursorPositionEnd <= end;
    }

    inline bool intersectsCursor(unsigned begin, unsigned end)
    {
        return (m_cursorPositionEnd >= begin && m_cursorPositionStart <= end);
    }

    inline bool isRangeSelected() const
    {
        return (m_cursorPositionStart != m_cursorPositionEnd);
    }

    void postVisit(Node *ast)
    {
        if (!isRangeSelected() && !m_selectedMembers.isEmpty())
            return; // nothing to do, we already have the results.

        if (UiObjectMember *member = ast->uiObjectMemberCast()) {
            unsigned begin = member->firstSourceLocation().begin();
            unsigned end = member->lastSourceLocation().end();

            if ((isRangeSelected() && intersectsCursor(begin, end))
            || (!isRangeSelected() && containsCursor(begin, end)))
            {
                if (initializerOfObject(member) && isSelectable(member)) {
                    m_selectedMembers << member;
                    // move start towards end; this facilitates multiselection so that root is usually ignored.
                    m_cursorPositionStart = qMin(end, m_cursorPositionEnd);
                }
            }
        }
    }
};

void QmlJSTextEditorWidget::setSelectedElements()
{
    if (!receivers(SIGNAL(selectedElementsChanged(QList<QmlJS::AST::UiObjectMember*>,QString))))
        return;

    QTextCursor tc = textCursor();
    QString wordAtCursor;
    QList<UiObjectMember *> offsets;

    unsigned startPos;
    unsigned endPos;

    if (tc.hasSelection()) {
        startPos = tc.selectionStart();
        endPos = tc.selectionEnd();
    } else {
        tc.movePosition(QTextCursor::StartOfWord);
        tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);

        startPos = textCursor().position();
        endPos = textCursor().position();
    }

    if (m_qmlJsEditorDocument->semanticInfo().isValid()) {
        SelectedElement selectedMembers;
        QList<UiObjectMember *> members
                = selectedMembers(m_qmlJsEditorDocument->semanticInfo().document, startPos, endPos);
        if (!members.isEmpty()) {
            foreach (UiObjectMember *m, members) {
                offsets << m;
            }
        }
    }
    wordAtCursor = tc.selectedText();

    emit selectedElementsChanged(offsets, wordAtCursor);
}

void QmlJSTextEditorWidget::applyFontSettings()
{
    TextEditor::BaseTextEditorWidget::applyFontSettings();
    if (!m_qmlJsEditorDocument->isSemanticInfoOutdated())
        updateUses();
}


QString QmlJSTextEditorWidget::wordUnderCursor() const
{
    QTextCursor tc = textCursor();
    const QChar ch = document()->characterAt(tc.position() - 1);
    // make sure that we're not at the start of the next word.
    if (ch.isLetterOrNumber() || ch == QLatin1Char('_'))
        tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::StartOfWord);
    tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    const QString word = tc.selectedText();
    return word;
}

bool QmlJSTextEditorWidget::isClosingBrace(const QList<Token> &tokens) const
{

    if (tokens.size() == 1) {
        const Token firstToken = tokens.first();

        return firstToken.is(Token::RightBrace) || firstToken.is(Token::RightBracket);
    }

    return false;
}

TextEditor::BaseTextEditor *QmlJSTextEditorWidget::createEditor()
{
    QmlJSEditor *editable = new QmlJSEditor(this);
    createToolBar(editable);
    return editable;
}

void QmlJSTextEditorWidget::createToolBar(QmlJSEditor *editor)
{
    m_outlineCombo = new QComboBox;
    m_outlineCombo->setMinimumContentsLength(22);
    m_outlineCombo->setModel(m_qmlJsEditorDocument->outlineModel());

    QTreeView *treeView = new QTreeView;

    Utils::AnnotatedItemDelegate *itemDelegate = new Utils::AnnotatedItemDelegate(this);
    itemDelegate->setDelimiter(QLatin1String(" "));
    itemDelegate->setAnnotationRole(QmlOutlineModel::AnnotationRole);
    treeView->setItemDelegateForColumn(0, itemDelegate);

    treeView->header()->hide();
    treeView->setItemsExpandable(false);
    treeView->setRootIsDecorated(false);
    m_outlineCombo->setView(treeView);
    treeView->expandAll();

    //m_outlineCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    // Make the combo box prefer to expand
    QSizePolicy policy = m_outlineCombo->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Expanding);
    m_outlineCombo->setSizePolicy(policy);

    connect(m_outlineCombo, SIGNAL(activated(int)), this, SLOT(jumpToOutlineElement(int)));
    connect(m_qmlJsEditorDocument->outlineModel(), SIGNAL(updated()),
            m_outlineCombo->view()/*QTreeView*/, SLOT(expandAll()));
    connect(m_qmlJsEditorDocument->outlineModel(), SIGNAL(updated()),
            this, SLOT(updateOutlineIndexNow()));

    connect(this, SIGNAL(cursorPositionChanged()), m_updateOutlineIndexTimer, SLOT(start()));

    editor->insertExtraToolBarWidget(TextEditor::BaseTextEditor::Left, m_outlineCombo);
}

TextEditor::BaseTextEditorWidget::Link QmlJSTextEditorWidget::findLinkAt(const QTextCursor &cursor,
                                                                         bool /*resolveTarget*/,
                                                                         bool /*inNextSplit*/)
{
    const SemanticInfo semanticInfo = m_qmlJsEditorDocument->semanticInfo();
    if (! semanticInfo.isValid())
        return Link();

    const unsigned cursorPosition = cursor.position();

    AST::Node *node = semanticInfo.astNodeAt(cursorPosition);
    QTC_ASSERT(node, return Link());

    if (AST::UiImport *importAst = cast<AST::UiImport *>(node)) {
        // if it's a file import, link to the file
        foreach (const ImportInfo &import, semanticInfo.document->bind()->imports()) {
            if (import.ast() == importAst && import.type() == ImportType::File) {
                BaseTextEditorWidget::Link link(import.path());
                link.linkTextStart = importAst->firstSourceLocation().begin();
                link.linkTextEnd = importAst->lastSourceLocation().end();
                return link;
            }
        }
        return Link();
    }

    // string literals that could refer to a file link to them
    if (StringLiteral *literal = cast<StringLiteral *>(node)) {
        const QString &text = literal->value.toString();
        BaseTextEditorWidget::Link link;
        link.linkTextStart = literal->literalToken.begin();
        link.linkTextEnd = literal->literalToken.end();
        if (semanticInfo.snapshot.document(text)) {
            link.targetFileName = text;
            return link;
        }
        const QString relative = QString::fromLatin1("%1/%2").arg(
                    semanticInfo.document->path(),
                    text);
        if (semanticInfo.snapshot.document(relative)) {
            link.targetFileName = relative;
            return link;
        }
    }

    const ScopeChain scopeChain = semanticInfo.scopeChain(semanticInfo.rangePath(cursorPosition));
    Evaluate evaluator(&scopeChain);
    const Value *value = evaluator.reference(node);

    QString fileName;
    int line = 0, column = 0;

    if (! (value && value->getSourceLocation(&fileName, &line, &column)))
        return Link();

    BaseTextEditorWidget::Link link;
    link.targetFileName = fileName;
    link.targetLine = line;
    link.targetColumn = column - 1; // adjust the column

    if (AST::UiQualifiedId *q = AST::cast<AST::UiQualifiedId *>(node)) {
        for (AST::UiQualifiedId *tail = q; tail; tail = tail->next) {
            if (! tail->next && cursorPosition <= tail->identifierToken.end()) {
                link.linkTextStart = tail->identifierToken.begin();
                link.linkTextEnd = tail->identifierToken.end();
                return link;
            }
        }

    } else if (AST::IdentifierExpression *id = AST::cast<AST::IdentifierExpression *>(node)) {
        link.linkTextStart = id->firstSourceLocation().begin();
        link.linkTextEnd = id->lastSourceLocation().end();
        return link;

    } else if (AST::FieldMemberExpression *mem = AST::cast<AST::FieldMemberExpression *>(node)) {
        link.linkTextStart = mem->lastSourceLocation().begin();
        link.linkTextEnd = mem->lastSourceLocation().end();
        return link;
    }

    return Link();
}

void QmlJSTextEditorWidget::findUsages()
{
    m_findReferences->findUsages(baseTextDocument()->filePath(), textCursor().position());
}

void QmlJSTextEditorWidget::renameUsages()
{
    m_findReferences->renameUsages(baseTextDocument()->filePath(), textCursor().position());
}

void QmlJSTextEditorWidget::showContextPane()
{
    const SemanticInfo info = m_qmlJsEditorDocument->semanticInfo();
    if (m_contextPane && info.isValid()) {
        Node *newNode = info.declaringMemberNoProperties(position());
        ScopeChain scopeChain = info.scopeChain(info.rangePath(position()));
        m_contextPane->apply(editor(), info.document,
                             &scopeChain,
                             newNode, false, true);
        m_oldCursorPosition = position();
        setRefactorMarkers(removeMarkersOfType<QtQuickToolbarMarker>(refactorMarkers()));
    }
}

void QmlJSTextEditorWidget::performQuickFix(int index)
{
    TextEditor::QuickFixOperation::Ptr op = m_quickFixes.at(index);
    op->perform();
}

void QmlJSTextEditorWidget::contextMenuEvent(QContextMenuEvent *e)
{
    QPointer<QMenu> menu(new QMenu(this));

    QMenu *refactoringMenu = new QMenu(tr("Refactoring"), menu);

    QSignalMapper mapper;
    connect(&mapper, SIGNAL(mapped(int)), this, SLOT(performQuickFix(int)));
    if (!m_qmlJsEditorDocument->isSemanticInfoOutdated()) {
        TextEditor::IAssistInterface *interface =
                createAssistInterface(TextEditor::QuickFix, TextEditor::ExplicitlyInvoked);
        if (interface) {
            QScopedPointer<TextEditor::IAssistProcessor> processor(
                        QmlJSEditorPlugin::instance()->quickFixAssistProvider()->createProcessor());
            QScopedPointer<TextEditor::IAssistProposal> proposal(processor->perform(interface));
            if (!proposal.isNull()) {
                TextEditor::BasicProposalItemListModel *model =
                        static_cast<TextEditor::BasicProposalItemListModel *>(proposal->model());
                for (int index = 0; index < model->size(); ++index) {
                    TextEditor::BasicProposalItem *item =
                            static_cast<TextEditor::BasicProposalItem *>(model->proposalItem(index));
                    TextEditor::QuickFixOperation::Ptr op =
                            item->data().value<TextEditor::QuickFixOperation::Ptr>();
                    m_quickFixes.append(op);
                    QAction *action = refactoringMenu->addAction(op->description());
                    mapper.setMapping(action, index);
                    connect(action, SIGNAL(triggered()), &mapper, SLOT(map()));
                }
                delete model;
            }
        }
    }

    refactoringMenu->setEnabled(!refactoringMenu->isEmpty());

    if (ActionContainer *mcontext = ActionManager::actionContainer(Constants::M_CONTEXT)) {
        QMenu *contextMenu = mcontext->menu();
        foreach (QAction *action, contextMenu->actions()) {
            menu->addAction(action);
            if (action->objectName() == QLatin1String(Constants::M_REFACTORING_MENU_INSERTION_POINT))
                menu->addMenu(refactoringMenu);
            if (action->objectName() == QLatin1String(Constants::SHOW_QT_QUICK_HELPER)) {
                bool enabled = m_contextPane->isAvailable(
                            editor(), m_qmlJsEditorDocument->semanticInfo().document,
                            m_qmlJsEditorDocument->semanticInfo().declaringMemberNoProperties(position()));
                action->setEnabled(enabled);
            }
        }
    }

    appendStandardContextMenuActions(menu);

    menu->exec(e->globalPos());
    if (!menu)
        return;
    m_quickFixes.clear();
    delete menu;
}

bool QmlJSTextEditorWidget::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::ShortcutOverride:
        if (static_cast<QKeyEvent*>(e)->key() == Qt::Key_Escape && m_contextPane) {
            if (hideContextPane()) {
                e->accept();
                return true;
            }
        }
        break;
    default:
        break;
    }

    return BaseTextEditorWidget::event(e);
}


void QmlJSTextEditorWidget::wheelEvent(QWheelEvent *event)
{
    bool visible = false;
    if (m_contextPane && m_contextPane->widget()->isVisible())
        visible = true;

    BaseTextEditorWidget::wheelEvent(event);

    if (visible)
        m_contextPane->apply(editor(), m_qmlJsEditorDocument->semanticInfo().document, 0,
                             m_qmlJsEditorDocument->semanticInfo().declaringMemberNoProperties(m_oldCursorPosition),
                             false, true);
}

void QmlJSTextEditorWidget::resizeEvent(QResizeEvent *event)
{
    BaseTextEditorWidget::resizeEvent(event);
    hideContextPane();
}

 void QmlJSTextEditorWidget::scrollContentsBy(int dx, int dy)
 {
     BaseTextEditorWidget::scrollContentsBy(dx, dy);
     hideContextPane();
 }

void QmlJSTextEditorWidget::unCommentSelection()
{
    Utils::unCommentSelection(this);
}

QmlJSEditorDocument *QmlJSTextEditorWidget::qmlJsEditorDocument() const
{
    return m_qmlJsEditorDocument;
}

void QmlJSTextEditorWidget::semanticInfoUpdated(const SemanticInfo &semanticInfo)
{
    if (isVisible()) {
         // trigger semantic highlighting and model update if necessary
        baseTextDocument()->triggerPendingUpdates();
    }

    if (m_contextPane) {
        Node *newNode = semanticInfo.declaringMemberNoProperties(position());
        if (newNode) {
            m_contextPane->apply(editor(), semanticInfo.document, 0, newNode, true);
            m_contextPaneTimer->start(); //update text marker
        }
    }

    updateUses();
}

void QmlJSTextEditorWidget::onRefactorMarkerClicked(const TextEditor::RefactorMarker &marker)
{
    if (marker.data.canConvert<QtQuickToolbarMarker>())
        showContextPane();
}

QModelIndex QmlJSTextEditorWidget::indexForPosition(unsigned cursorPosition, const QModelIndex &rootIndex) const
{
    QModelIndex lastIndex = rootIndex;

    QmlOutlineModel *model = m_qmlJsEditorDocument->outlineModel();
    const int rowCount = model->rowCount(rootIndex);
    for (int i = 0; i < rowCount; ++i) {
        QModelIndex childIndex = model->index(i, 0, rootIndex);
        AST::SourceLocation location = model->sourceLocation(childIndex);

        if ((cursorPosition >= location.offset)
              && (cursorPosition <= location.offset + location.length)) {
            lastIndex = childIndex;
            break;
        }
    }

    if (lastIndex != rootIndex) {
        // recurse
        lastIndex = indexForPosition(cursorPosition, lastIndex);
    }
    return lastIndex;
}

bool QmlJSTextEditorWidget::hideContextPane()
{
    bool b = (m_contextPane) && m_contextPane->widget()->isVisible();
    if (b)
        m_contextPane->apply(editor(), m_qmlJsEditorDocument->semanticInfo().document, 0, 0, false);
    return b;
}

TextEditor::IAssistInterface *QmlJSTextEditorWidget::createAssistInterface(
    TextEditor::AssistKind assistKind,
    TextEditor::AssistReason reason) const
{
    if (assistKind == TextEditor::Completion) {
        return new QmlJSCompletionAssistInterface(document(),
                                                  position(),
                                                  editor()->document()->filePath(),
                                                  reason,
                                                  m_qmlJsEditorDocument->semanticInfo());
    } else if (assistKind == TextEditor::QuickFix) {
        return new QmlJSQuickFixAssistInterface(const_cast<QmlJSTextEditorWidget *>(this), reason);
    }
    return 0;
}

QString QmlJSTextEditorWidget::foldReplacementText(const QTextBlock &block) const
{
    const int curlyIndex = block.text().indexOf(QLatin1Char('{'));

    if (curlyIndex != -1 && m_qmlJsEditorDocument->semanticInfo().isValid()) {
        const int pos = block.position() + curlyIndex;
        Node *node = m_qmlJsEditorDocument->semanticInfo().rangeAt(pos);

        const QString objectId = idOfObject(node);
        if (!objectId.isEmpty())
            return QLatin1String("id: ") + objectId + QLatin1String("...");
    }

    return TextEditor::BaseTextEditorWidget::foldReplacementText(block);
}

} // namespace Internal
} // namespace QmlJSEditor
