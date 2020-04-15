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

#include "giteditor.h"

#include "annotationhighlighter.h"
#include "gitplugin.h"
#include "gitclient.h"
#include "gitsettings.h"
#include "gitsubmiteditorwidget.h"
#include "gitconstants.h"
#include "githighlighters.h"

#include <coreplugin/icore.h>
#include <utils/qtcassert.h>
#include <vcsbase/vcsbaseoutputwindow.h>
#include <texteditor/basetextdocument.h>

#include <QFileInfo>
#include <QRegExp>
#include <QSet>
#include <QTemporaryFile>
#include <QTextCodec>
#include <QDir>

#include <QTextCursor>
#include <QTextBlock>
#include <QMessageBox>

#define CHANGE_PATTERN "[a-f0-9]{7,40}"

namespace Git {
namespace Internal {

// ------------ GitEditor
GitEditor::GitEditor(const VcsBase::VcsBaseEditorParameters *type,
                     QWidget *parent)  :
    VcsBase::VcsBaseEditorWidget(type, parent),
    m_changeNumberPattern(QLatin1String(CHANGE_PATTERN))
{
    QTC_ASSERT(m_changeNumberPattern.isValid(), return);
    /* Diff format:
        diff --git a/src/plugins/git/giteditor.cpp b/src/plugins/git/giteditor.cpp
        index 40997ff..4e49337 100644
        --- a/src/plugins/git/giteditor.cpp
        +++ b/src/plugins/git/giteditor.cpp
    */
    setDiffFilePattern(QRegExp(QLatin1String("^(?:diff --git a/|index |[+-]{3} (?:/dev/null|[ab]/(.+$)))")));
    setLogEntryPattern(QRegExp(QLatin1String("^commit ([0-9a-f]{8})[0-9a-f]{32}")));
    setAnnotateRevisionTextFormat(tr("Blame %1"));
    setAnnotatePreviousRevisionTextFormat(tr("Blame Parent Revision %1"));
}

QSet<QString> GitEditor::annotationChanges() const
{
    QSet<QString> changes;
    const QString txt = toPlainText();
    if (txt.isEmpty())
        return changes;
    // Hunt for first change number in annotation: "<change>:"
    QRegExp r(QLatin1String("^(" CHANGE_PATTERN ") "));
    QTC_ASSERT(r.isValid(), return changes);
    if (r.indexIn(txt) != -1) {
        changes.insert(r.cap(1));
        r.setPattern(QLatin1String("\n(" CHANGE_PATTERN ") "));
        QTC_ASSERT(r.isValid(), return changes);
        int pos = 0;
        while ((pos = r.indexIn(txt, pos)) != -1) {
            pos += r.matchedLength();
            changes.insert(r.cap(1));
        }
    }
    return changes;
}

QString GitEditor::changeUnderCursor(const QTextCursor &c) const
{
    QTextCursor cursor = c;
    // Any number is regarded as change number.
    cursor.select(QTextCursor::WordUnderCursor);
    if (!cursor.hasSelection())
        return QString();
    const QString change = cursor.selectedText();
    if (m_changeNumberPattern.exactMatch(change))
        return change;
    return QString();
}

VcsBase::BaseAnnotationHighlighter *GitEditor::createAnnotationHighlighter(const QSet<QString> &changes) const
{
    return new GitAnnotationHighlighter(changes);
}

/* Remove the date specification from annotation, which is tabular:
\code
8ca887aa (author               YYYY-MM-DD HH:MM:SS <offset>  <line>)<content>
\endcode */

static QString removeAnnotationDate(const QString &b)
{
    if (b.isEmpty())
        return b;

    const QChar space(QLatin1Char(' '));
    const int parenPos = b.indexOf(QLatin1Char(')'));
    if (parenPos == -1)
        return b;
    int datePos = parenPos;

    int i = parenPos;
    while (i >= 0 && b.at(i) != space)
        --i;
    while (i >= 0 && b.at(i) == space)
        --i;
    int spaceCount = 0;
    // i is now on timezone. Go back 3 spaces: That is where the date starts.
    while (i >= 0) {
        if (b.at(i) == space)
            ++spaceCount;
        if (spaceCount == 3) {
            datePos = i;
            break;
        }
        --i;
    }
    if (datePos == 0)
        return b;

    // Copy over the parts that have not changed into a new byte array
    QString result;
    QTC_ASSERT(b.size() >= parenPos, return result);
    int prevPos = 0;
    int pos = b.indexOf(QLatin1Char('\n'), 0) + 1;
    forever {
        QTC_CHECK(prevPos < pos);
        int afterParen = prevPos + parenPos;
        result.append(b.mid(prevPos, datePos));
        result.append(b.mid(afterParen, pos - afterParen));
        prevPos = pos;
        QTC_CHECK(prevPos != 0);
        if (pos == b.size())
            break;

        pos = b.indexOf(QLatin1Char('\n'), pos) + 1;
        if (pos == 0) // indexOf returned -1
            pos = b.size();
    }
    return result;
}

void GitEditor::setPlainTextFiltered(const QString &text)
{
    QString modText = text;
    GitPlugin *plugin = GitPlugin::instance();
    // If desired, filter out the date from annotation
    switch (contentType())
    {
    case VcsBase::AnnotateOutput: {
        const bool omitAnnotationDate = plugin->settings().boolValue(GitSettings::omitAnnotationDateKey);
        if (omitAnnotationDate)
            modText = removeAnnotationDate(text);
        break;
    }
    case VcsBase::DiffOutput: {
        if (modText.isEmpty()) {
            modText = QLatin1String("No difference to HEAD");
        } else {
            const QFileInfo fi(source());
            const QString workingDirectory = fi.isDir() ? fi.absoluteFilePath() : fi.absolutePath();
            modText = plugin->gitClient()->extendedShowDescription(workingDirectory, modText);
        }
        break;
    }
    default:
        break;
    }

    baseTextDocument()->setPlainText(modText);
}

void GitEditor::commandFinishedGotoLine(bool ok, int exitCode, const QVariant &v)
{
    reportCommandFinished(ok, exitCode, v);
    if (ok && v.type() == QVariant::Int) {
        const int line = v.toInt();
        if (line >= 0)
            gotoLine(line);
    }
}

void GitEditor::checkoutChange()
{
    const QFileInfo fi(source());
    const QString workingDirectory = fi.isDir() ? fi.absoluteFilePath() : fi.absolutePath();
    GitPlugin::instance()->gitClient()->stashAndCheckout(workingDirectory, m_currentChange);
}

void GitEditor::resetChange()
{
    const QFileInfo fi(source());
    const QString workingDir = fi.isDir() ? fi.absoluteFilePath() : fi.absolutePath();

    GitClient *client = GitPlugin::instance()->gitClient();
    if (client->gitStatus(workingDir, StatusMode(NoUntracked | NoSubmodules))
            != GitClient::StatusUnchanged) {
        if (QMessageBox::question(
                    Core::ICore::mainWindow(), tr("Reset"),
                    tr("All changes in working directory will be discarded. Are you sure?"),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No) == QMessageBox::No) {
            return;
        }
    }
    client->reset(workingDir, QLatin1String("--hard"), m_currentChange);
}

void GitEditor::cherryPickChange()
{
    const QFileInfo fi(source());
    const QString workingDirectory = fi.isDir() ? fi.absoluteFilePath() : fi.absolutePath();
    GitPlugin::instance()->gitClient()->synchronousCherryPick(workingDirectory, m_currentChange);
}

void GitEditor::revertChange()
{
    const QFileInfo fi(source());
    const QString workingDirectory = fi.isDir() ? fi.absoluteFilePath() : fi.absolutePath();
    GitPlugin::instance()->gitClient()->synchronousRevert(workingDirectory, m_currentChange);
}

void GitEditor::stageDiffChunk()
{
    const QAction *a = qobject_cast<QAction *>(sender());
    QTC_ASSERT(a, return);
    const VcsBase::DiffChunk chunk = qvariant_cast<VcsBase::DiffChunk>(a->data());
    return applyDiffChunk(chunk, false);
}

void GitEditor::unstageDiffChunk()
{
    const QAction *a = qobject_cast<QAction *>(sender());
    QTC_ASSERT(a, return);
    const VcsBase::DiffChunk chunk = qvariant_cast<VcsBase::DiffChunk>(a->data());
    return applyDiffChunk(chunk, true);
}

void GitEditor::applyDiffChunk(const VcsBase::DiffChunk& chunk, bool revert)
{
    VcsBase::VcsBaseOutputWindow *outwin = VcsBase::VcsBaseOutputWindow::instance();
    QTemporaryFile patchFile;
    if (!patchFile.open())
        return;

    const QString baseDir = workingDirectory();
    patchFile.write(chunk.header);
    patchFile.write(chunk.chunk);
    patchFile.close();

    GitClient *client = GitPlugin::instance()->gitClient();
    QStringList args = QStringList() << QLatin1String("--cached");
    if (revert)
        args << QLatin1String("--reverse");
    QString errorMessage;
    if (client->synchronousApplyPatch(baseDir, patchFile.fileName(), &errorMessage, args)) {
        if (errorMessage.isEmpty())
            outwin->append(tr("Chunk successfully staged"));
        else
            outwin->append(errorMessage);
        if (revert)
            emit diffChunkReverted(chunk);
        else
            emit diffChunkApplied(chunk);
    } else {
        outwin->appendError(errorMessage);
    }
}

void GitEditor::init()
{
    VcsBase::VcsBaseEditorWidget::init();
    Core::Id editorId = editor()->id();
    if (editorId == Git::Constants::GIT_COMMIT_TEXT_EDITOR_ID)
        new GitSubmitHighlighter(baseTextDocument());
    else if (editorId == Git::Constants::GIT_REBASE_EDITOR_ID)
        new GitRebaseHighlighter(baseTextDocument());
}

void GitEditor::addDiffActions(QMenu *menu, const VcsBase::DiffChunk &chunk)
{
    menu->addSeparator();

    QAction *stageAction = menu->addAction(tr("Stage Chunk..."));
    stageAction->setData(qVariantFromValue(chunk));
    connect(stageAction, SIGNAL(triggered()), this, SLOT(stageDiffChunk()));

    QAction *unstageAction = menu->addAction(tr("Unstage Chunk..."));
    unstageAction->setData(qVariantFromValue(chunk));
    connect(unstageAction, SIGNAL(triggered()), this, SLOT(unstageDiffChunk()));
}

bool GitEditor::open(QString *errorString, const QString &fileName, const QString &realFileName)
{
    Core::Id editorId = editor()->id();
    if (editorId == Git::Constants::GIT_COMMIT_TEXT_EDITOR_ID
            || editorId == Git::Constants::GIT_REBASE_EDITOR_ID) {
        QFileInfo fi(fileName);
        const QString gitPath = fi.absolutePath();
        setSource(gitPath);
        baseTextDocument()->setCodec(
                    GitPlugin::instance()->gitClient()->encoding(gitPath, "i18n.commitEncoding"));
    }
    return VcsBaseEditorWidget::open(errorString, fileName, realFileName);
}

QString GitEditor::decorateVersion(const QString &revision) const
{
    const QFileInfo fi(source());
    const QString workingDirectory = fi.absolutePath();

    // Format verbose, SHA1 being first token
    return GitPlugin::instance()->gitClient()->synchronousShortDescription(workingDirectory, revision);
}

QStringList GitEditor::annotationPreviousVersions(const QString &revision) const
{
    QStringList revisions;
    QString errorMessage;
    GitClient *client = GitPlugin::instance()->gitClient();
    const QFileInfo fi(source());
    const QString workingDirectory = fi.absolutePath();
    // Get the SHA1's of the file.
    if (!client->synchronousParentRevisions(workingDirectory, QStringList(fi.fileName()),
                                            revision, &revisions, &errorMessage)) {
        VcsBase::VcsBaseOutputWindow::instance()->appendSilently(errorMessage);
        return QStringList();
    }
    return revisions;
}

bool GitEditor::isValidRevision(const QString &revision) const
{
    return GitPlugin::instance()->gitClient()->isValidRevision(revision);
}

void GitEditor::addChangeActions(QMenu *menu, const QString &change)
{
    m_currentChange = change;
    if (contentType() != VcsBase::OtherContent) {
        menu->addAction(tr("Cherry-Pick Change %1").arg(change), this, SLOT(cherryPickChange()));
        menu->addAction(tr("Revert Change %1").arg(change), this, SLOT(revertChange()));
        menu->addAction(tr("Checkout Change %1").arg(change), this, SLOT(checkoutChange()));
        menu->addAction(tr("Hard Reset to Change %1").arg(change), this, SLOT(resetChange()));
    }
}

QString GitEditor::revisionSubject(const QTextBlock &inBlock) const
{
    for (QTextBlock block = inBlock.next(); block.isValid(); block = block.next()) {
        const QString line = block.text().trimmed();
        if (line.isEmpty()) {
            block = block.next();
            return block.text().trimmed();
        }
    }
    return QString();
}

bool GitEditor::supportChangeLinks() const
{
    return VcsBaseEditorWidget::supportChangeLinks()
            || (editor()->id() == Git::Constants::GIT_COMMIT_TEXT_EDITOR_ID)
            || (editor()->id() == Git::Constants::GIT_REBASE_EDITOR_ID);
}

QString GitEditor::fileNameForLine(int line) const
{
    // 7971b6e7 share/qtcreator/dumper/dumper.py   (hjk
    QTextBlock block = document()->findBlockByLineNumber(line - 1);
    QTC_ASSERT(block.isValid(), return source());
    static QRegExp renameExp(QLatin1String("^" CHANGE_PATTERN "\\s+([^(]+)"));
    if (renameExp.indexIn(block.text()) != -1) {
        const QString fileName = renameExp.cap(1).trimmed();
        if (!fileName.isEmpty())
            return fileName;
    }
    return source();
}

} // namespace Internal
} // namespace Git
