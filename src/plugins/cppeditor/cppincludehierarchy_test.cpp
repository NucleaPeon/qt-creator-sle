/****************************************************************************
**
** Copyright (C) 2014 Przemyslaw Gorszkowski <pgorszkowski@gmail.com>
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
#include "cppeditortestcase.h"
#include "cppincludehierarchymodel.h"

#include <coreplugin/editormanager/editormanager.h>
#include <cppeditor/cppeditor.h>
#include <cpptools/cppmodelmanagerinterface.h>
#include <utils/fileutils.h>

#include <QByteArray>
#include <QList>
#include <QtTest>

Q_DECLARE_METATYPE(QList<QByteArray>)

using namespace CPlusPlus;
using namespace CppEditor::Internal;
using namespace CppTools;

namespace {

QString toString(CppIncludeHierarchyModel &model, const QModelIndex &index, int indent = 0)
{
    QString result = QString(indent, QLatin1Char(' ')) + index.data().toString()
            + QLatin1Char('\n');
    const int itemsCount = model.rowCount(index);
    for (int i = 0; i < itemsCount; ++i) {
        QModelIndex child = index.child(i, 0);
        if (model.canFetchMore(child))
            model.fetchMore(child);

        indent += 2;
        result += toString(model, child, indent);
    }
    return result;
}

QString toString(CppIncludeHierarchyModel &model)
{
    return toString(model, model.index(0, 0))
            + toString(model, model.index(1, 0));
}

class IncludeHierarchyTestCase: public CppEditor::Internal::Tests::TestCase
{
public:
    IncludeHierarchyTestCase(const QList<QByteArray> &sourceList,
                             const QString &expectedHierarchy)
    {
        QVERIFY(succeededSoFar());

        QStringList filePaths;
        const int sourceListSize = sourceList.size();
        for (int i = 0; i < sourceListSize; ++i) {
            const QByteArray &source = sourceList.at(i);

            // Write source to file
            const QString fileName = QString::fromLatin1("%1/file%2.h").arg(QDir::tempPath())
                    .arg(i+1);
            QVERIFY(writeFile(fileName, source));

            filePaths << fileName;
        }

        // Update Code Model
        QVERIFY(parseFiles(filePaths));

        // Open Editor
        const QString fileName = QDir::tempPath() + QLatin1String("/file1.h");
        CPPEditor *editor;
        QVERIFY(openCppEditor(fileName, &editor));
        closeEditorAtEndOfTestCase(editor);

        // Test model
        CppIncludeHierarchyModel model(0);
        model.buildHierarchy(editor, fileName);
        const QString actualHierarchy = toString(model);
        QCOMPARE(actualHierarchy, expectedHierarchy);
    }
};

} // anonymous namespace

void CppEditorPlugin::test_includehierarchy_data()
{
    QTest::addColumn<QList<QByteArray> >("documents");
    QTest::addColumn<QString>("expectedHierarchy");

    QTest::newRow("single-includes")
            << (QList<QByteArray>()
                << QByteArray("#include \"file2.h\"\n")
                << QByteArray())
            << QString::fromLatin1(
                   "Includes\n"
                   "  file2.h\n"
                   "Included by (none)\n");

    QTest::newRow("single-includedBy")
            << (QList<QByteArray>()
                << QByteArray()
                << QByteArray("#include \"file1.h\"\n"))
            << QString::fromLatin1(
                   "Includes (none)\n"
                   "Included by\n"
                   "  file2.h\n"
                   );

    QTest::newRow("both-includes-and-includedBy")
            << (QList<QByteArray>()
                << QByteArray("#include \"file2.h\"\n")
                << QByteArray()
                << QByteArray("#include \"file1.h\"\n"))
            << QString::fromLatin1(
                   "Includes\n"
                   "  file2.h\n"
                   "Included by\n"
                   "  file3.h\n"
                   );

    QTest::newRow("simple-cyclic")
            << (QList<QByteArray>()
                << QByteArray("#include \"file2.h\"\n")
                << QByteArray("#include \"file1.h\"\n"))
            << QString::fromLatin1(
                   "Includes\n"
                   "  file2.h\n"
                   "    file1.h (cyclic)\n"
                   "Included by\n"
                   "  file2.h\n"
                   "    file1.h (cyclic)\n"
                   );

    QTest::newRow("complex-cyclic")
            << (QList<QByteArray>()
                << QByteArray("#include \"file2.h\"\n")
                << QByteArray("#include \"file3.h\"\n")
                << QByteArray("#include \"file1.h\"\n"))
            << QString::fromLatin1(
                   "Includes\n"
                   "  file2.h\n"
                   "    file3.h\n"
                   "      file1.h (cyclic)\n"
                   "Included by\n"
                   "  file3.h\n"
                   "    file2.h\n"
                   "      file1.h (cyclic)\n"
                   );
}

void CppEditorPlugin::test_includehierarchy()
{
    QFETCH(QList<QByteArray>, documents);
    QFETCH(QString, expectedHierarchy);

    IncludeHierarchyTestCase(documents, expectedHierarchy);
}
