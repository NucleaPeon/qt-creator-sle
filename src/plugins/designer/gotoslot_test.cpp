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

#include "formeditorplugin.h"

#if QT_VERSION < 0x050000
#include <QtTest>
#else
#include "formeditorw.h"

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/testdatadir.h>
#include <cpptools/cppmodelmanager.h>
#include <cpptools/cpptoolseditorsupport.h>
#include <cpptools/cpptoolstestcase.h>

#include <cplusplus/CppDocument.h>
#include <cplusplus/Overview.h>
#include <utils/fileutils.h>

#include <QDesignerFormEditorInterface>
#include <QDesignerIntegrationInterface>
#include <QStringList>
#include <QtTest>

using namespace Core;
using namespace Core::Tests;
using namespace CppTools;
using namespace CPlusPlus;
using namespace Designer;
using namespace Designer::Internal;

namespace {

QTC_DECLARE_MYTESTDATADIR("../../../tests/designer/")

class DocumentContainsFunctionDefinition: protected SymbolVisitor
{
public:
    bool operator()(Scope *scope, const QString function)
    {
        if (!scope)
            return false;

        m_referenceFunction = function;
        m_result = false;

        accept(scope);
        return m_result;
    }

protected:
    bool preVisit(Symbol *) { return !m_result; }

    bool visit(Function *symbol)
    {
        const QString function = m_overview.prettyName(symbol->name());
        if (function == m_referenceFunction)
            m_result = true;
        return false;
    }

private:
    bool m_result;
    QString m_referenceFunction;
    Overview m_overview;
};

class DocumentContainsDeclaration: protected SymbolVisitor
{
public:
    bool operator()(Scope *scope, const QString function)
    {
        if (!scope)
            return false;

        m_referenceFunction = function;
        m_result = false;

        accept(scope);
        return m_result;
    }

protected:
    bool preVisit(Symbol *) { return !m_result; }

    void postVisit(Symbol *symbol)
    {
        if (symbol->isClass())
            m_currentClass.clear();
    }

    bool visit(Class *symbol)
    {
        m_currentClass = m_overview.prettyName(symbol->name());
        return true;
    }

    bool visit(Declaration *symbol)
    {
        QString declaration = m_overview.prettyName(symbol->name());
        if (!m_currentClass.isEmpty())
            declaration = m_currentClass + QLatin1String("::") + declaration;
        if (m_referenceFunction == declaration)
            m_result = true;
        return false;
    }

private:
    bool m_result;
    QString m_referenceFunction;
    QString m_currentClass;
    Overview m_overview;
};

bool documentContainsFunctionDefinition(const Document::Ptr &document, const QString function)
{
    return DocumentContainsFunctionDefinition()(document->globalNamespace(), function);
}

bool documentContainsMemberFunctionDeclaration(const Document::Ptr &document,
                                               const QString declaration)
{
    return DocumentContainsDeclaration()(document->globalNamespace(), declaration);
}

class GoToSlotTestCase : public CppTools::Tests::TestCase
{
public:
    GoToSlotTestCase(const QStringList &files)
    {
        QVERIFY(succeededSoFar());
        QCOMPARE(files.size(), 3);

        QList<TextEditor::BaseTextEditor *> editors;
        foreach (const QString &file, files) {
            IEditor *editor = EditorManager::openEditor(file);
            TextEditor::BaseTextEditor *e = qobject_cast<TextEditor::BaseTextEditor *>(editor);
            QVERIFY(e);
            closeEditorAtEndOfTestCase(editor);
            editors << e;
        }
        TextEditor::BaseTextEditor *cppFileEditor = editors.at(0);
        TextEditor::BaseTextEditor *hFileEditor = editors.at(1);

        const QString cppFile = files.at(0);
        const QString hFile = files.at(1);

        QCOMPARE(EditorManager::documentModel()->openedDocuments().size(), files.size());
        waitForFilesInGlobalSnapshot(QStringList() << cppFile << hFile);

        // Execute "Go To Slot"
        FormEditorW *few = FormEditorW::instance();
        QDesignerIntegrationInterface *integration = few->designerEditor()->integration();
        QVERIFY(integration);
        integration->emitNavigateToSlot(QLatin1String("pushButton"), QLatin1String("clicked()"),
                                        QStringList());

        QCOMPARE(EditorManager::currentDocument()->filePath(), cppFile);
        QVERIFY(EditorManager::currentDocument()->isModified());

        // Wait for updated documents
        foreach (TextEditor::BaseTextEditor *editor, editors) {
            if (CppEditorSupport *editorSupport = m_modelManager->cppEditorSupport(editor)) {
                while (editorSupport->isUpdatingDocument())
                    QApplication::processEvents();
            }
        }

        // Compare
        const Document::Ptr cppDocument
            = m_modelManager->cppEditorSupport(cppFileEditor)->snapshotUpdater()->document();
        const Document::Ptr hDocument
            = m_modelManager->cppEditorSupport(hFileEditor)->snapshotUpdater()->document();
        QVERIFY(documentContainsFunctionDefinition(cppDocument,
            QLatin1String("Form::on_pushButton_clicked")));
        QVERIFY(documentContainsMemberFunctionDeclaration(hDocument,
            QLatin1String("Form::on_pushButton_clicked")));
    }
};

} // anonymous namespace
#endif

/// Check: Executes "Go To Slot..." on a QPushButton in a *.ui file and checks if the respective
/// header and source files are correctly updated.
void Designer::Internal::FormEditorPlugin::test_gotoslot()
{
#if QT_VERSION >= 0x050000
    QFETCH(QStringList, files);
    (GoToSlotTestCase(files));
#else
    QSKIP("Available only with >= Qt5", SkipSingle);
#endif
}

void Designer::Internal::FormEditorPlugin::test_gotoslot_data()
{
#if QT_VERSION >= 0x050000
    typedef QLatin1String _;
    QTest::addColumn<QStringList>("files");

    MyTestDataDir testDataDirWithoutProject(_("gotoslot_withoutProject"));
    QTest::newRow("withoutProject")
        << (QStringList()
            << testDataDirWithoutProject.file(_("form.cpp"))
            << testDataDirWithoutProject.file(_("form.h"))
            << testDataDirWithoutProject.file(_("form.ui")));

    // Finding the right class for inserting definitions/declarations is based on
    // finding a class with a member whose type is the class from the "ui_xxx.h" header.
    // In the following test data the header files contain an extra class referencing
    // the same class name.

    MyTestDataDir testDataDir;

    testDataDir = MyTestDataDir(_("gotoslot_insertIntoCorrectClass_pointer"));
    QTest::newRow("insertIntoCorrectClass_pointer")
        << (QStringList()
            << testDataDir.file(_("form.cpp"))
            << testDataDir.file(_("form.h"))
            << testDataDirWithoutProject.file(_("form.ui"))); // reuse

    testDataDir = MyTestDataDir(_("gotoslot_insertIntoCorrectClass_non-pointer"));
    QTest::newRow("insertIntoCorrectClass_non-pointer")
        << (QStringList()
            << testDataDir.file(_("form.cpp"))
            << testDataDir.file(_("form.h"))
            << testDataDirWithoutProject.file(_("form.ui"))); // reuse

    testDataDir = MyTestDataDir(_("gotoslot_insertIntoCorrectClass_pointer_ns_using"));
    QTest::newRow("insertIntoCorrectClass_pointer_ns_using")
        << (QStringList()
            << testDataDir.file(_("form.cpp"))
            << testDataDir.file(_("form.h"))
            << testDataDir.file(_("form.ui")));
#endif
}
