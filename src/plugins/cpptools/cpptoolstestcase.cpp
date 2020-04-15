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

#include "cpptoolstestcase.h"

#include <coreplugin/editormanager/editormanager.h>
#include <texteditor/basetexteditor.h>

#include <cplusplus/CppDocument.h>
#include <utils/fileutils.h>

#include <QtTest>

static bool closeEditorsWithoutGarbageCollectorInvocation(const QList<Core::IEditor *> &editors)
{
    CppTools::CppModelManagerInterface::instance()->enableGarbageCollector(false);
    const bool closeEditorsSucceeded = Core::EditorManager::closeEditors(editors, false);
    CppTools::CppModelManagerInterface::instance()->enableGarbageCollector(true);
    return closeEditorsSucceeded;
}

static bool snapshotContains(const CPlusPlus::Snapshot &snapshot, const QStringList &filePaths)
{
    foreach (const QString &filePath, filePaths) {
        if (!snapshot.contains(filePath)) {
            const QString warning = QLatin1String("Missing file in snapshot: ") + filePath;
            QWARN(qPrintable(warning));
            return false;
        }
    }
    return true;
}

namespace CppTools {
namespace Tests {

TestDocument::TestDocument(const QByteArray &fileName, const QByteArray &source, char cursorMarker)
    : m_fileName(fileName), m_source(source), m_cursorMarker(cursorMarker)
{}

QString TestDocument::filePath() const
{
    const QString fileNameAsString = QString::fromUtf8(m_fileName);
    if (!QFileInfo(fileNameAsString).isAbsolute())
        return QDir::tempPath() + QLatin1Char('/') + fileNameAsString;
    return fileNameAsString;
}

bool TestDocument::writeToDisk() const
{
    return TestCase::writeFile(filePath(), m_source);
}

TestCase::TestCase(bool runGarbageCollector)
    : m_modelManager(CppModelManagerInterface::instance())
    , m_succeededSoFar(false)
    , m_runGarbageCollector(runGarbageCollector)
{
    if (m_runGarbageCollector)
        QVERIFY(garbageCollectGlobalSnapshot());
    m_succeededSoFar = true;
}

TestCase::~TestCase()
{
    QVERIFY(closeEditorsWithoutGarbageCollectorInvocation(m_editorsToClose));
    QCoreApplication::processEvents();

    if (m_runGarbageCollector)
        QVERIFY(garbageCollectGlobalSnapshot());
}

bool TestCase::succeededSoFar() const
{
    return m_succeededSoFar;
}

bool TestCase::openBaseTextEditor(const QString &fileName, TextEditor::BaseTextEditor **editor)
{
    typedef TextEditor::BaseTextEditor BTEditor;
    if (BTEditor *e = qobject_cast<BTEditor *>(Core::EditorManager::openEditor(fileName))) {
        if (editor) {
            *editor = e;
            return true;
        }
    }
    return false;
}

CPlusPlus::Snapshot TestCase::globalSnapshot()
{
    return CppModelManagerInterface::instance()->snapshot();
}

bool TestCase::garbageCollectGlobalSnapshot()
{
    CppModelManagerInterface::instance()->GC();
    return globalSnapshot().isEmpty();
}

bool TestCase::parseFiles(const QStringList &filePaths)
{
    CppModelManagerInterface::instance()->updateSourceFiles(filePaths).waitForFinished();
    QCoreApplication::processEvents();
    const CPlusPlus::Snapshot snapshot = globalSnapshot();
    if (snapshot.isEmpty()) {
        QWARN("After parsing: snapshot is empty.");
        return false;
    }
    if (!snapshotContains(snapshot, filePaths)) {
        QWARN("After parsing: snapshot does not contain all expected files.");
        return false;
    }
    return true;
}

bool TestCase::parseFiles(const QString &filePath)
{
    return parseFiles(QStringList(filePath));
}

void TestCase::closeEditorAtEndOfTestCase(Core::IEditor *editor)
{
    if (editor && !m_editorsToClose.contains(editor))
        m_editorsToClose.append(editor);
}

bool TestCase::closeEditorWithoutGarbageCollectorInvocation(Core::IEditor *editor)
{
    return closeEditorsWithoutGarbageCollectorInvocation(QList<Core::IEditor *>() << editor);
}

CPlusPlus::Document::Ptr TestCase::waitForFileInGlobalSnapshot(const QString &filePath)
{
    return waitForFilesInGlobalSnapshot(QStringList(filePath)).first();
}

QList<CPlusPlus::Document::Ptr> TestCase::waitForFilesInGlobalSnapshot(
        const QStringList &filePaths)
{
    QList<CPlusPlus::Document::Ptr> result;
    foreach (const QString &filePath, filePaths) {
        forever {
            if (CPlusPlus::Document::Ptr document = globalSnapshot().document(filePath)) {
                result.append(document);
                break;
            }
            QCoreApplication::processEvents();
        }
    }
    return result;
}

bool TestCase::writeFile(const QString &filePath, const QByteArray &contents)
{
    Utils::FileSaver saver(filePath);
    if (!saver.write(contents) || !saver.finalize()) {
        const QString warning = QLatin1String("Failed to write file to disk: ") + filePath;
        QWARN(qPrintable(warning));
        return false;
    }
    return true;
}

} // namespace Tests
} // namespace CppTools
