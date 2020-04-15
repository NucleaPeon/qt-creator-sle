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

#include "cpppreprocessor.h"
#include "cppsnapshotupdater.h"

#include <utils/qtcassert.h>

using namespace CPlusPlus;
using namespace CppTools;
using namespace CppTools::Internal;

SnapshotUpdater::SnapshotUpdater(const QString &fileInEditor)
    : m_mutex(QMutex::Recursive)
    , m_fileInEditor(fileInEditor)
    , m_editorDefinesChangedSinceLastUpdate(false)
    , m_usePrecompiledHeaders(false)
    , m_forceSnapshotInvalidation(false)
{
}

void SnapshotUpdater::update(CppModelManager::WorkingCopy workingCopy)
{
    QMutexLocker locker(&m_mutex);

    if (m_fileInEditor.isEmpty())
        return;

    bool invalidateSnapshot = false, invalidateConfig = false, editorDefinesChanged = false;

    CppModelManager *modelManager
        = dynamic_cast<CppModelManager *>(CppModelManagerInterface::instance());
    QByteArray configFile = modelManager->codeModelConfiguration();
    QStringList includePaths;
    QStringList frameworkPaths;
    QStringList precompiledHeaders;
    QString projectConfigFile;

    updateProjectPart();

    if (m_forceSnapshotInvalidation) {
        invalidateSnapshot = true;
        m_forceSnapshotInvalidation = false;
    }

    if (m_projectPart) {
        configFile += m_projectPart->toolchainDefines;
        configFile += m_projectPart->projectDefines;
        includePaths = m_projectPart->includePaths;
        frameworkPaths = m_projectPart->frameworkPaths;
        projectConfigFile = m_projectPart->projectConfigFile;
        if (m_usePrecompiledHeaders)
            precompiledHeaders = m_projectPart->precompiledHeaders;
    }

    if (configFile != m_configFile) {
        m_configFile = configFile;
        invalidateSnapshot = true;
        invalidateConfig = true;
    }

    if (m_editorDefinesChangedSinceLastUpdate) {
        invalidateSnapshot = true;
        editorDefinesChanged = true;
        m_editorDefinesChangedSinceLastUpdate = false;
    }

    if (includePaths != m_includePaths) {
        m_includePaths = includePaths;
        invalidateSnapshot = true;
    }

    if (frameworkPaths != m_frameworkPaths) {
        m_frameworkPaths = frameworkPaths;
        invalidateSnapshot = true;
    }

    if (projectConfigFile != m_projectConfigFile) {
        m_projectConfigFile = projectConfigFile;
        invalidateSnapshot = true;
    }

    if (precompiledHeaders != m_precompiledHeaders) {
        m_precompiledHeaders = precompiledHeaders;
        invalidateSnapshot = true;
    }

    unsigned rev = 0;
    if (Document::Ptr doc = document())
        rev = doc->revision();
    else
        invalidateSnapshot = true;

    Snapshot globalSnapshot = modelManager->snapshot();

    if (invalidateSnapshot) {
        m_snapshot = Snapshot();
    } else {
        // Remove changed files from the snapshot
        QSet<QString> toRemove;
        foreach (const Document::Ptr &doc, m_snapshot) {
            QString fileName = doc->fileName();
            if (workingCopy.contains(fileName)) {
                if (workingCopy.get(fileName).second != doc->editorRevision())
                    addFileAndDependencies(&toRemove, fileName);
                continue;
            }
            Document::Ptr otherDoc = globalSnapshot.document(fileName);
            if (!otherDoc.isNull() && otherDoc->revision() != doc->revision())
                addFileAndDependencies(&toRemove, fileName);
        }

        if (!toRemove.isEmpty()) {
            invalidateSnapshot = true;
            foreach (const QString &fileName, toRemove)
                m_snapshot.remove(fileName);
        }
    }

    // Update the snapshot
    if (invalidateSnapshot) {
        const QString configurationFileName = modelManager->configurationFileName();
        if (invalidateConfig)
            m_snapshot.remove(configurationFileName);
        if (!m_snapshot.contains(configurationFileName))
            workingCopy.insert(configurationFileName, m_configFile);
        m_snapshot.remove(m_fileInEditor);

        static const QString editorDefinesFileName
            = CppModelManagerInterface::editorConfigurationFileName();
        if (editorDefinesChanged) {
            m_snapshot.remove(editorDefinesFileName);
            workingCopy.insert(editorDefinesFileName, m_editorDefines);
        }

        CppPreprocessor preproc(modelManager, m_snapshot);
        Snapshot globalSnapshot = modelManager->snapshot();
        globalSnapshot.remove(fileInEditor());
        preproc.setGlobalSnapshot(globalSnapshot);
        preproc.setWorkingCopy(workingCopy);
        preproc.setIncludePaths(m_includePaths);
        preproc.setFrameworkPaths(m_frameworkPaths);
        preproc.run(configurationFileName);
        if (!m_projectConfigFile.isEmpty())
            preproc.run(m_projectConfigFile);
        if (m_usePrecompiledHeaders) {
            foreach (const QString &precompiledHeader, m_precompiledHeaders)
                preproc.run(precompiledHeader);
        }
        if (!m_editorDefines.isEmpty())
            preproc.run(editorDefinesFileName);
        preproc.run(m_fileInEditor);

        m_snapshot = preproc.snapshot();
        Snapshot newSnapshot = m_snapshot.simplified(document());
        for (Snapshot::const_iterator i = m_snapshot.begin(), ei = m_snapshot.end(); i != ei; ++i) {
            if (Client::isInjectedFile(i.key()))
                newSnapshot.insert(i.value());
        }
        m_snapshot = newSnapshot;
        m_deps.build(m_snapshot);

        foreach (Document::Ptr doc, m_snapshot) {
            QString fileName = doc->fileName();
            if (doc->revision() == 0) {
                Document::Ptr otherDoc = globalSnapshot.document(fileName);
                doc->setRevision(otherDoc.isNull() ? 0 : otherDoc->revision());
            }
            if (fileName != fileInEditor())
                doc->releaseSourceAndAST();
        }

        QTC_CHECK(document());
        if (Document::Ptr doc = document())
            doc->setRevision(rev + 1);
    }
}

void SnapshotUpdater::releaseSnapshot()
{
    QMutexLocker locker(&m_mutex);
    m_snapshot = Snapshot();
    m_deps = DependencyTable();
    m_forceSnapshotInvalidation = true;
}

Document::Ptr SnapshotUpdater::document() const
{
    QMutexLocker locker(&m_mutex);
    return m_snapshot.document(m_fileInEditor);
}

Snapshot SnapshotUpdater::snapshot() const
{
    QMutexLocker locker(&m_mutex);
    return m_snapshot;
}

QStringList SnapshotUpdater::includePaths() const
{
    QMutexLocker locker(&m_mutex);
    return m_includePaths;
}

QStringList SnapshotUpdater::frameworkPaths() const
{
    QMutexLocker locker(&m_mutex);
    return m_frameworkPaths;
}

ProjectPart::Ptr SnapshotUpdater::currentProjectPart() const
{
    QMutexLocker locker(&m_mutex);
    return m_projectPart;
}

void SnapshotUpdater::setProjectPart(ProjectPart::Ptr projectPart)
{
    QMutexLocker locker(&m_mutex);
    m_manuallySetProjectPart = projectPart;
}

void SnapshotUpdater::setUsePrecompiledHeaders(bool usePrecompiledHeaders)
{
    QMutexLocker locker(&m_mutex);
    m_usePrecompiledHeaders = usePrecompiledHeaders;
}

void SnapshotUpdater::setEditorDefines(const QByteArray &editorDefines)
{
    QMutexLocker locker(&m_mutex);

    if (editorDefines != m_editorDefines) {
        m_editorDefines = editorDefines;
        m_editorDefinesChangedSinceLastUpdate = true;
    }
}

void SnapshotUpdater::updateProjectPart()
{
    if (m_manuallySetProjectPart) {
        m_projectPart = m_manuallySetProjectPart;
        return;
    }

    CppModelManager *cmm = dynamic_cast<CppModelManager *>(CppModelManagerInterface::instance());
    QList<ProjectPart::Ptr> pParts = cmm->projectPart(m_fileInEditor);
    if (pParts.isEmpty()) {
        if (m_projectPart)
            // File is not directly part of any project, but we got one before. We will re-use it,
            // because re-calculating this can be expensive when the dependency table is big.
            return;

        // Fall-back step 1: Get some parts through the dependency table:
        pParts = cmm->projectPartFromDependencies(m_fileInEditor);
        if (pParts.isEmpty())
            // Fall-back step 2: Use fall-back part from the model manager:
            m_projectPart = cmm->fallbackProjectPart();
        else
            m_projectPart = pParts.first();
    } else {
        if (!pParts.contains(m_projectPart))
            // Apparently the project file changed, so update our project part.
            m_projectPart = pParts.first();
    }
}

void SnapshotUpdater::addFileAndDependencies(QSet<QString> *toRemove, const QString &fileName) const
{
    toRemove->insert(fileName);
    if (fileName != m_fileInEditor) {
        QStringList deps = m_deps.filesDependingOn(fileName);
        toRemove->unite(QSet<QString>::fromList(deps));
    }
}

