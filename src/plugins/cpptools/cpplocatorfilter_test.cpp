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

#include "cpptoolsplugin.h"

#include "cppclassesfilter.h"
#include "cppcurrentdocumentfilter.h"
#include "cppfunctionsfilter.h"
#include "cpplocatorfilter.h"
#include "cppmodelmanager.h"
#include "cpptoolstestcase.h"

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/testdatadir.h>
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/locator/locatorfiltertest.h>
#include <utils/fileutils.h>

#include <QDebug>
#include <QFileInfo>
#include <QtTest>

using namespace Core;
using namespace Core::Tests;
using namespace CppTools::Internal;
using namespace ExtensionSystem;
using namespace Utils;

Q_DECLARE_METATYPE(ILocatorFilter *)

namespace {

QTC_DECLARE_MYTESTDATADIR("../../../tests/cpplocators/")

inline QString _(const QByteArray &ba) { return QString::fromLatin1(ba, ba.size()); }

class CppLocatorFilterTestCase
    : public BasicLocatorFilterTest
    , public CppTools::Tests::TestCase
{
public:
    CppLocatorFilterTestCase(ILocatorFilter *filter,
                             const QString &fileName,
                             const QString &searchText,
                             const ResultDataList &expectedResults)
        : BasicLocatorFilterTest(filter)
        , m_fileName(fileName)
    {
        QVERIFY(succeededSoFar());
        QVERIFY(!m_fileName.isEmpty());
        QVERIFY(garbageCollectGlobalSnapshot());

        ResultDataList results = ResultData::fromFilterEntryList(matchesFor(searchText));
//        ResultData::printFilterEntries(results);
        QVERIFY(!results.isEmpty());
        QCOMPARE(results, expectedResults);
    }

private:
    void doBeforeLocatorRun() { QVERIFY(parseFiles(m_fileName)); }
    void doAfterLocatorRun() { QVERIFY(garbageCollectGlobalSnapshot()); }

private:
    const QString m_fileName;
};

class CppCurrentDocumentFilterTestCase
    : public BasicLocatorFilterTest
    , public CppTools::Tests::TestCase
{
public:
    CppCurrentDocumentFilterTestCase(const QString &fileName,
                                     const ResultDataList &expectedResults)
        : BasicLocatorFilterTest(PluginManager::getObject<CppCurrentDocumentFilter>())
        , m_editor(0)
        , m_fileName(fileName)
    {
        QVERIFY(succeededSoFar());
        QVERIFY(!m_fileName.isEmpty());

        ResultDataList results = ResultData::fromFilterEntryList(matchesFor());
//        ResultData::printFilterEntries(results);
        QVERIFY(!results.isEmpty());
        QCOMPARE(results, expectedResults);
    }

private:
    void doBeforeLocatorRun()
    {
        QVERIFY(EditorManager::documentModel()->openedDocuments().isEmpty());
        QVERIFY(garbageCollectGlobalSnapshot());

        m_editor = EditorManager::openEditor(m_fileName);
        QVERIFY(m_editor);

        waitForFileInGlobalSnapshot(m_fileName);
    }

    void doAfterLocatorRun()
    {
        QVERIFY(closeEditorWithoutGarbageCollectorInvocation(m_editor));
        QCoreApplication::processEvents();
        QVERIFY(EditorManager::documentModel()->openedDocuments().isEmpty());
        QVERIFY(garbageCollectGlobalSnapshot());
    }

private:
    IEditor *m_editor;
    const QString m_fileName;
};

} // anonymous namespace

void CppToolsPlugin::test_cpplocatorfilters_CppLocatorFilter()
{
    QFETCH(QString, testFile);
    QFETCH(ILocatorFilter *, filter);
    QFETCH(QString, searchText);
    QFETCH(ResultDataList, expectedResults);

    CppLocatorFilterTestCase(filter, testFile, searchText, expectedResults);
}

void CppToolsPlugin::test_cpplocatorfilters_CppLocatorFilter_data()
{
    QTest::addColumn<QString>("testFile");
    QTest::addColumn<ILocatorFilter *>("filter");
    QTest::addColumn<QString>("searchText");
    QTest::addColumn<ResultDataList>("expectedResults");

    ILocatorFilter *cppFunctionsFilter = PluginManager::getObject<CppFunctionsFilter>();
    ILocatorFilter *cppClassesFilter = PluginManager::getObject<CppClassesFilter>();
    ILocatorFilter *cppLocatorFilter = PluginManager::getObject<CppLocatorFilter>();

    MyTestDataDir testDirectory(QLatin1String("testdata_basic"));
    const QString testFile = testDirectory.file(QLatin1String("file1.cpp"));
    const QString testFileShort = FileUtils::shortNativePath(FileName::fromString(testFile));

    QTest::newRow("CppFunctionsFilter")
        << testFile
        << cppFunctionsFilter
        << _("function")
        << (QList<ResultData>()
            << ResultData(_("functionDefinedInClass(bool, int)"), _("MyClass"))
            << ResultData(_("functionDefinedInClass(bool, int)"), _("MyNamespace::MyClass"))
            << ResultData(_("functionDefinedInClass(bool, int)"),
                          _("<anonymous namespace>::MyClass"))
            << ResultData(_("functionDefinedOutSideClass(char)"), _("MyClass"))
            << ResultData(_("functionDefinedOutSideClass(char)"), _("MyNamespace::MyClass"))
            << ResultData(_("functionDefinedOutSideClass(char)"),
                          _("<anonymous namespace>::MyClass"))
            << ResultData(_("functionDefinedOutSideClassAndNamespace(float)"),
                          _("MyNamespace::MyClass"))
            << ResultData(_("myFunction(bool, int)"), testFileShort)
            << ResultData(_("myFunction(bool, int)"), _("MyNamespace"))
            << ResultData(_("myFunction(bool, int)"), _("<anonymous namespace>"))
           );

    QTest::newRow("CppFunctionsFilter-WithNamespacePrefix")
        << testFile
        << cppFunctionsFilter
        << _("mynamespace::")
        << (QList<ResultData>()
            << ResultData(_("MyClass()"), _("MyNamespace::MyClass"))
            << ResultData(_("functionDefinedInClass(bool, int)"), _("MyNamespace::MyClass"))
            << ResultData(_("functionDefinedOutSideClass(char)"), _("MyNamespace::MyClass"))
            << ResultData(_("functionDefinedOutSideClassAndNamespace(float)"),
                          _("MyNamespace::MyClass"))
            << ResultData(_("myFunction(bool, int)"), _("MyNamespace"))
           );

    QTest::newRow("CppClassesFilter")
        << testFile
        << cppClassesFilter
        << _("myclass")
        << (QList<ResultData>()
            << ResultData(_("MyClass"), testFileShort)
            << ResultData(_("MyClass"), _("MyNamespace"))
            << ResultData(_("MyClass"), _("<anonymous namespace>"))
           );

    QTest::newRow("CppClassesFilter-WithNamespacePrefix")
        << testFile
        << cppClassesFilter
        << _("mynamespace::")
        << (QList<ResultData>()
            << ResultData(_("MyClass"), _("MyNamespace"))
           );

    // all symbols in the left column are expected to be fully qualified.
    QTest::newRow("CppLocatorFilter-filtered")
        << testFile
        << cppLocatorFilter
        << _("my")
        << (QList<ResultData>()
            << ResultData(_("<anonymous namespace>::MyClass"), testFileShort)
            << ResultData(_("<anonymous namespace>::MyClass::MyClass"), _("()"))
            << ResultData(_("<anonymous namespace>::MyClass::functionDefinedOutSideClass"),
                          _("(char)"))
            << ResultData(_("<anonymous namespace>::MyEnum"), testFileShort)
            << ResultData(_("<anonymous namespace>::myFunction"), _("(bool, int)"))
            << ResultData(_("MyClass"), testFileShort)
            << ResultData(_("MyClass::MyClass"), _("()"))
            << ResultData(_("MyClass::functionDefinedOutSideClass"), _("(char)"))
            << ResultData(_("MyEnum"), testFileShort)
            << ResultData(_("MyNamespace::MyClass"), testFileShort)
            << ResultData(_("MyNamespace::MyClass::MyClass"), _("()"))
            << ResultData(_("MyNamespace::MyClass::functionDefinedOutSideClass"),
                          _("(char)"))
            << ResultData(_("MyNamespace::MyClass::functionDefinedOutSideClassAndNamespace"),
                          _("(float)"))
            << ResultData(_("MyNamespace::MyEnum"), testFileShort)
            << ResultData(_("MyNamespace::myFunction"), _("(bool, int)"))
            << ResultData(_("myFunction"), _("(bool, int)"))
            );
}

void CppToolsPlugin::test_cpplocatorfilters_CppCurrentDocumentFilter()
{
    MyTestDataDir testDirectory(QLatin1String("testdata_basic"));
    const QString testFile = testDirectory.file(QLatin1String("file1.cpp"));

    QList<ResultData> expectedResults = QList<ResultData>()
        << ResultData(_("int myVariable"), _(""))
        << ResultData(_("myFunction(bool, int)"), _(""))
        << ResultData(_("MyEnum"), _(""))
        << ResultData(_("int V1"), _("MyEnum"))
        << ResultData(_("int V2"), _("MyEnum"))
        << ResultData(_("MyClass"), _(""))
        << ResultData(_("MyClass()"), _("MyClass"))
        << ResultData(_("functionDeclaredOnly()"), _("MyClass"))
        << ResultData(_("functionDefinedInClass(bool, int)"), _("MyClass"))
        << ResultData(_("functionDefinedOutSideClass(char)"), _("MyClass"))
        << ResultData(_("functionDefinedOutSideClass(char)"), _("MyClass"))
        << ResultData(_("int myVariable"), _("MyNamespace"))
        << ResultData(_("myFunction(bool, int)"), _("MyNamespace"))
        << ResultData(_("MyEnum"), _("MyNamespace"))
        << ResultData(_("int V1"), _("MyNamespace::MyEnum"))
        << ResultData(_("int V2"), _("MyNamespace::MyEnum"))
        << ResultData(_("MyClass"), _("MyNamespace"))
        << ResultData(_("MyClass()"), _("MyNamespace::MyClass"))
        << ResultData(_("functionDeclaredOnly()"), _("MyNamespace::MyClass"))
        << ResultData(_("functionDefinedInClass(bool, int)"), _("MyNamespace::MyClass"))
        << ResultData(_("functionDefinedOutSideClass(char)"), _("MyNamespace::MyClass"))
        << ResultData(_("functionDefinedOutSideClassAndNamespace(float)"),
                      _("MyNamespace::MyClass"))
        << ResultData(_("functionDefinedOutSideClass(char)"), _("MyNamespace::MyClass"))
        << ResultData(_("functionDefinedOutSideClassAndNamespace(float)"),
                      _("MyNamespace::MyClass"))
        << ResultData(_("int myVariable"), _("<anonymous namespace>"))
        << ResultData(_("myFunction(bool, int)"), _("<anonymous namespace>"))
        << ResultData(_("MyEnum"), _("<anonymous namespace>"))
        << ResultData(_("int V1"), _("<anonymous namespace>::MyEnum"))
        << ResultData(_("int V2"), _("<anonymous namespace>::MyEnum"))
        << ResultData(_("MyClass"), _("<anonymous namespace>"))
        << ResultData(_("MyClass()"), _("<anonymous namespace>::MyClass"))
        << ResultData(_("functionDeclaredOnly()"), _("<anonymous namespace>::MyClass"))
        << ResultData(_("functionDefinedInClass(bool, int)"), _("<anonymous namespace>::MyClass"))
        << ResultData(_("functionDefinedOutSideClass(char)"), _("<anonymous namespace>::MyClass"))
        << ResultData(_("functionDefinedOutSideClass(char)"), _("<anonymous namespace>::MyClass"))
        << ResultData(_("main()"), _(""))
        ;

    CppCurrentDocumentFilterTestCase(testFile, expectedResults);
}
