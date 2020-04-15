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

#include "qmljsbind.h"
#include "qmljsconstants.h"
#include "qmljsfindexportedcpptypes.h"
#include "qmljsinterpreter.h"
#include "qmljsmodelmanagerinterface.h"
#include "qmljsplugindumper.h"
#include "qmljstypedescriptionreader.h"

#include <cplusplus/cppmodelmanagerbase.h>
#include <utils/function.h>
#include <utils/hostosinfo.h>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaObject>
#include <QRegExp>
#include <QTextDocument>
#include <QTextStream>
#include <QTimer>
#include <QtAlgorithms>
#include <utils/runextensions.h>

#include <stdio.h>

namespace QmlJS {

/*!
    \class QmlJS::ModelManagerInterface
    \brief The ModelManagerInterface class acts as an interface to the
    global state of the QmlJS code model.
    \sa QmlJS::Document QmlJS::Snapshot QmlJSTools::Internal::ModelManager

    The ModelManagerInterface is an interface for global state and actions in
    the QmlJS code model. It is implemented by \l{QmlJSTools::Internal::ModelManager}
    and the instance can be accessed through ModelManagerInterface::instance().

    One of its primary concerns is to keep the Snapshots it
    maintains up to date by parsing documents and finding QML modules.

    It has a Snapshot that contains only valid Documents,
    accessible through ModelManagerInterface::snapshot() and a Snapshot with
    potentially more recent, but invalid documents that is exposed through
    ModelManagerInterface::newestSnapshot().
*/

static ModelManagerInterface *g_instance = 0;

static QStringList environmentImportPaths()
{
    QStringList paths;

    QByteArray envImportPath = qgetenv("QML_IMPORT_PATH");

    foreach (const QString &path, QString::fromLatin1(envImportPath)
             .split(Utils::HostOsInfo::pathListSeparator(), QString::SkipEmptyParts)) {
        QString canonicalPath = QDir(path).canonicalPath();
        if (!canonicalPath.isEmpty() && !paths.contains(canonicalPath))
            paths.append(canonicalPath);
    }

    return paths;
}

ModelManagerInterface::ModelManagerInterface(QObject *parent)
    : QObject(parent),
      m_shouldScanImports(false),
      m_pluginDumper(new PluginDumper(this))
{
    m_synchronizer.setCancelOnWait(true);

    m_updateCppQmlTypesTimer = new QTimer(this);
    m_updateCppQmlTypesTimer->setInterval(1000);
    m_updateCppQmlTypesTimer->setSingleShot(true);
    connect(m_updateCppQmlTypesTimer, SIGNAL(timeout()), SLOT(startCppQmlTypeUpdate()));

    m_asyncResetTimer = new QTimer(this);
    m_asyncResetTimer->setInterval(15000);
    m_asyncResetTimer->setSingleShot(true);
    connect(m_asyncResetTimer, SIGNAL(timeout()), SLOT(resetCodeModel()));

    qRegisterMetaType<QmlJS::Document::Ptr>("QmlJS::Document::Ptr");
    qRegisterMetaType<QmlJS::LibraryInfo>("QmlJS::LibraryInfo");

    m_defaultImportPaths << environmentImportPaths();
    updateImportPaths();

    Q_ASSERT(! g_instance);
    g_instance = this;
}

ModelManagerInterface::~ModelManagerInterface()
{
    m_cppQmlTypesUpdater.cancel();
    m_cppQmlTypesUpdater.waitForFinished();
    Q_ASSERT(g_instance == this);
    g_instance = 0;
}

static QHash<QString, Language::Enum> defaultLanguageMapping()
{
    QHash<QString, Language::Enum> res;
    res[QLatin1String("js")] = Language::JavaScript;
    res[QLatin1String("qml")] = Language::Qml;
    res[QLatin1String("qmltypes")] = Language::QmlTypeInfo;
    res[QLatin1String("qmlproject")] = Language::QmlProject;
    res[QLatin1String("json")] = Language::Json;
    res[QLatin1String("qbs")] = Language::QmlQbs;
    return res;
}

Language::Enum ModelManagerInterface::guessLanguageOfFile(const QString &fileName)
{
    QHash<QString, Language::Enum> lMapping;
    if (instance())
        lMapping = instance()->languageForSuffix();
    else
        lMapping = defaultLanguageMapping();
    const QFileInfo info(fileName);
    const QString fileSuffix = info.suffix();
    return lMapping.value(fileSuffix, Language::Unknown);
}

QStringList ModelManagerInterface::globPatternsForLanguages(const QList<Language::Enum> languages)
{
    QHash<QString, Language::Enum> lMapping;
    if (instance())
        lMapping = instance()->languageForSuffix();
    else
        lMapping = defaultLanguageMapping();
    QStringList patterns;
    QHashIterator<QString,Language::Enum> i(lMapping);
    while (i.hasNext()) {
        i.next();
        if (languages.contains(i.value()))
            patterns << QLatin1String("*.") + i.key();
    }
    return patterns;
}

ModelManagerInterface *ModelManagerInterface::instance()
{
    return g_instance;
}

void ModelManagerInterface::writeWarning(const QString &msg)
{
    if (ModelManagerInterface *i = instance())
        i->writeMessageInternal(msg);
    else
        qDebug() << msg;
}

ModelManagerInterface::WorkingCopy ModelManagerInterface::workingCopy()
{
    if (ModelManagerInterface *i = instance())
        return i->workingCopyInternal();
    return WorkingCopy();
}

void ModelManagerInterface::activateScan()
{
    if (!m_shouldScanImports) {
        m_shouldScanImports = true;
        updateImportPaths();
    }
}

QHash<QString, Language::Enum> ModelManagerInterface::languageForSuffix() const
{
    return defaultLanguageMapping();
}

void ModelManagerInterface::writeMessageInternal(const QString &msg) const
{
    qDebug() << msg;
}

ModelManagerInterface::WorkingCopy ModelManagerInterface::workingCopyInternal() const
{
    ModelManagerInterface::WorkingCopy res;
    return res;
}

void ModelManagerInterface::addTaskInternal(QFuture<void> result, const QString &msg,
                                            const char *taskId) const
{
    Q_UNUSED(result);
    qDebug() << "started " << taskId << " " << msg;
}

void ModelManagerInterface::loadQmlTypeDescriptionsInternal(const QString &resourcePath)
{
    const QDir typeFileDir(resourcePath + QLatin1String("/qml-type-descriptions"));
    const QStringList qmlTypesExtensions = QStringList() << QLatin1String("*.qmltypes");
    QFileInfoList qmlTypesFiles = typeFileDir.entryInfoList(
                qmlTypesExtensions,
                QDir::Files,
                QDir::Name);

    QStringList errors;
    QStringList warnings;

    // filter out the actual Qt builtins
    for (int i = 0; i < qmlTypesFiles.size(); ++i) {
        if (qmlTypesFiles.at(i).baseName() == QLatin1String("builtins")) {
            QFileInfoList list;
            list.append(qmlTypesFiles.at(i));
            CppQmlTypesLoader::defaultQtObjects =
                    CppQmlTypesLoader::loadQmlTypes(list, &errors, &warnings);
            qmlTypesFiles.removeAt(i);
            break;
        }
    }

    // load the fallbacks for libraries
    CppQmlTypesLoader::defaultLibraryObjects.unite(
                CppQmlTypesLoader::loadQmlTypes(qmlTypesFiles, &errors, &warnings));

    foreach (const QString &error, errors)
        writeMessageInternal(error);
    foreach (const QString &warning, warnings)
        writeMessageInternal(warning);
}



Snapshot ModelManagerInterface::snapshot() const
{
    QMutexLocker locker(&m_mutex);
    return m_validSnapshot;
}

Snapshot ModelManagerInterface::newestSnapshot() const
{
    QMutexLocker locker(&m_mutex);
    return m_newestSnapshot;
}

void ModelManagerInterface::updateSourceFiles(const QStringList &files,
                                     bool emitDocumentOnDiskChanged)
{
    refreshSourceFiles(files, emitDocumentOnDiskChanged);
}

QFuture<void> ModelManagerInterface::refreshSourceFiles(const QStringList &sourceFiles,
                                               bool emitDocumentOnDiskChanged)
{
    if (sourceFiles.isEmpty())
        return QFuture<void>();

    QFuture<void> result = QtConcurrent::run(&ModelManagerInterface::parse,
                                              workingCopyInternal(), sourceFiles,
                                              this, Language::Qml,
                                              emitDocumentOnDiskChanged);

    if (m_synchronizer.futures().size() > 10) {
        QList<QFuture<void> > futures = m_synchronizer.futures();

        m_synchronizer.clearFutures();

        foreach (const QFuture<void> &future, futures) {
            if (! (future.isFinished() || future.isCanceled()))
                m_synchronizer.addFuture(future);
        }
    }

    m_synchronizer.addFuture(result);

    if (sourceFiles.count() > 1)
         addTaskInternal(result, tr("Indexing"), Constants::TASK_INDEX);

    if (sourceFiles.count() > 1 && !m_shouldScanImports) {
        bool scan = false;
        {
            QMutexLocker l(&m_mutex);
            if (!m_shouldScanImports) {
                m_shouldScanImports = true;
                scan = true;
            }
        }
        if (scan)
        updateImportPaths();
    }

    return result;
}


void ModelManagerInterface::fileChangedOnDisk(const QString &path)
{
    QtConcurrent::run(&ModelManagerInterface::parse,
                      workingCopyInternal(), QStringList() << path,
                      this, Language::Unknown, true);
}

void ModelManagerInterface::removeFiles(const QStringList &files)
{
    emit aboutToRemoveFiles(files);

    QMutexLocker locker(&m_mutex);

    foreach (const QString &file, files) {
        m_validSnapshot.remove(file);
        m_newestSnapshot.remove(file);
    }
}

namespace {
bool pInfoLessThanActive(const ModelManagerInterface::ProjectInfo &p1, const ModelManagerInterface::ProjectInfo &p2)
{
    QStringList s1 = p1.activeResourceFiles;
    QStringList s2 = p2.activeResourceFiles;
    if (s1.size() < s2.size())
        return true;
    if (s1.size() > s2.size())
        return false;
    for (int i = 0; i < s1.size(); ++i) {
        if (s1.at(i) < s2.at(i))
            return true;
        else if (s1.at(i) > s2.at(i))
            return false;
    }
    return false;
}

bool pInfoLessThanAll(const ModelManagerInterface::ProjectInfo &p1, const ModelManagerInterface::ProjectInfo &p2)
{
    QStringList s1 = p1.allResourceFiles;
    QStringList s2 = p2.allResourceFiles;
    if (s1.size() < s2.size())
        return true;
    if (s1.size() > s2.size())
        return false;
    for (int i = 0; i < s1.size(); ++i) {
        if (s1.at(i) < s2.at(i))
            return true;
        else if (s1.at(i) > s2.at(i))
            return false;
    }
    return false;
}
}

QStringList ModelManagerInterface::filesAtQrcPath(const QString &path, const QLocale *locale,
                                         ProjectExplorer::Project *project,
                                         QrcResourceSelector resources)
{
    QString normPath = QrcParser::normalizedQrcFilePath(path);
    QList<ProjectInfo> pInfos;
    if (project)
        pInfos.append(projectInfo(project));
    else
        pInfos = projectInfos();

    QStringList res;
    QSet<QString> pathsChecked;
    foreach (const ModelManagerInterface::ProjectInfo &pInfo, pInfos) {
        QStringList qrcFilePaths;
        if (resources == ActiveQrcResources)
            qrcFilePaths = pInfo.activeResourceFiles;
        else
            qrcFilePaths = pInfo.allResourceFiles;
        foreach (const QString &qrcFilePath, qrcFilePaths) {
            if (pathsChecked.contains(qrcFilePath))
                continue;
            pathsChecked.insert(qrcFilePath);
            QrcParser::ConstPtr qrcFile = m_qrcCache.parsedPath(qrcFilePath);
            if (qrcFile.isNull())
                continue;
            qrcFile->collectFilesAtPath(normPath, &res, locale);
        }
    }
    res.sort(); // make the result predictable
    return res;
}

QMap<QString, QStringList> ModelManagerInterface::filesInQrcPath(const QString &path,
                                                        const QLocale *locale,
                                                        ProjectExplorer::Project *project,
                                                        bool addDirs,
                                                        QrcResourceSelector resources)
{
    QString normPath = QrcParser::normalizedQrcDirectoryPath(path);
    QList<ProjectInfo> pInfos;
    if (project) {
        pInfos.append(projectInfo(project));
    } else {
        pInfos = projectInfos();
        if (resources == ActiveQrcResources) // make the result predictable
            qSort(pInfos.begin(), pInfos.end(), &pInfoLessThanActive);
        else
            qSort(pInfos.begin(), pInfos.end(), &pInfoLessThanAll);
    }
    QMap<QString, QStringList> res;
    QSet<QString> pathsChecked;
    foreach (const ModelManagerInterface::ProjectInfo &pInfo, pInfos) {
        QStringList qrcFilePaths;
        if (resources == ActiveQrcResources)
            qrcFilePaths = pInfo.activeResourceFiles;
        else
            qrcFilePaths = pInfo.allResourceFiles;
        foreach (const QString &qrcFilePath, qrcFilePaths) {
            if (pathsChecked.contains(qrcFilePath))
                continue;
            pathsChecked.insert(qrcFilePath);
            QrcParser::ConstPtr qrcFile = m_qrcCache.parsedPath(qrcFilePath);

            if (qrcFile.isNull())
                continue;
            qrcFile->collectFilesInPath(normPath, &res, addDirs, locale);
        }
    }
    return res;
}

QList<ModelManagerInterface::ProjectInfo> ModelManagerInterface::projectInfos() const
{
    QMutexLocker locker(&m_mutex);

    return m_projects.values();
}

ModelManagerInterface::ProjectInfo ModelManagerInterface::projectInfo(ProjectExplorer::Project *project) const
{
    QMutexLocker locker(&m_mutex);

    return m_projects.value(project, ProjectInfo());
}

void ModelManagerInterface::updateProjectInfo(const ProjectInfo &pinfo, ProjectExplorer::Project *p)
{
    if (! pinfo.isValid() || !p)
        return;

    Snapshot snapshot;
    ProjectInfo oldInfo;
    {
        QMutexLocker locker(&m_mutex);
        oldInfo = m_projects.value(p);
        m_projects.insert(p, pinfo);
        snapshot = m_validSnapshot;
    }

    if (oldInfo.qmlDumpPath != pinfo.qmlDumpPath
            || oldInfo.qmlDumpEnvironment != pinfo.qmlDumpEnvironment) {
        m_pluginDumper->scheduleRedumpPlugins();
        m_pluginDumper->scheduleMaybeRedumpBuiltins(pinfo);
    }


    updateImportPaths();

    // remove files that are no longer in the project and have been deleted
    QStringList deletedFiles;
    foreach (const QString &oldFile, oldInfo.sourceFiles) {
        if (snapshot.document(oldFile)
                && !pinfo.sourceFiles.contains(oldFile)
                && !QFile::exists(oldFile)) {
            deletedFiles += oldFile;
        }
    }
    removeFiles(deletedFiles);
    foreach (const QString &oldFile, deletedFiles)
        m_fileToProject.remove(oldFile, p);

    // parse any files not yet in the snapshot
    QStringList newFiles;
    foreach (const QString &file, pinfo.sourceFiles) {
        if (!snapshot.document(file))
            newFiles += file;
    }
    updateSourceFiles(newFiles, false);
    foreach (const QString &newFile, deletedFiles)
        m_fileToProject.insert(newFile, p);

    // update qrc cache
    foreach (const QString &newQrc, pinfo.allResourceFiles)
        m_qrcCache.addPath(newQrc);
    foreach (const QString &oldQrc, oldInfo.allResourceFiles)
        m_qrcCache.removePath(oldQrc);

    int majorVersion, minorVersion, patchVersion;
    // dump builtin types if the shipped definitions are probably outdated and the
    // Qt version ships qmlplugindump
    if (::sscanf(pinfo.qtVersionString.toLatin1().constData(), "%d.%d.%d",
           &majorVersion, &minorVersion, &patchVersion) != 3)
        majorVersion = minorVersion = patchVersion = -1;

    if (majorVersion > 4 || (majorVersion == 4 && (minorVersion > 8 || (majorVersion == 8
                                                                       && patchVersion >= 5)))) {
        m_pluginDumper->loadBuiltinTypes(pinfo);
    }

    emit projectInfoUpdated(pinfo);
}


void ModelManagerInterface::removeProjectInfo(ProjectExplorer::Project *project)
{
    ProjectInfo info;
    info.sourceFiles.clear();
    // update with an empty project info to clear data
    updateProjectInfo(info, project);

    {
        QMutexLocker locker(&m_mutex);
        m_projects.remove(project);
    }
}

ModelManagerInterface::ProjectInfo ModelManagerInterface::projectInfoForPath(QString path)
{
    QMutexLocker locker(&m_mutex);

    foreach (const ProjectInfo &p, m_projects)
        if (p.sourceFiles.contains(path))
            return p;
    return ProjectInfo();
}

void ModelManagerInterface::emitDocumentChangedOnDisk(Document::Ptr doc)
{ emit documentChangedOnDisk(doc); }

void ModelManagerInterface::updateQrcFile(const QString &path)
{
    m_qrcCache.updatePath(path);
}

void ModelManagerInterface::updateDocument(Document::Ptr doc)
{
    {
        QMutexLocker locker(&m_mutex);
        m_validSnapshot.insert(doc);
        m_newestSnapshot.insert(doc, true);
    }
    emit documentUpdated(doc);
}

void ModelManagerInterface::updateLibraryInfo(const QString &path, const LibraryInfo &info)
{
    if (!info.pluginTypeInfoError().isEmpty())
        qDebug() << "Dumping errors for " << path << ":" << info.pluginTypeInfoError();

    {
        QMutexLocker locker(&m_mutex);
        m_validSnapshot.insertLibraryInfo(path, info);
        m_newestSnapshot.insertLibraryInfo(path, info);
    }
    // only emit if we got new useful information
    if (info.isValid())
        emit libraryInfoUpdated(path, info);
}

static QStringList filesInDirectoryForLanguages(const QString &path, QList<Language::Enum> languages)
{
    const QStringList pattern = ModelManagerInterface::globPatternsForLanguages(languages);
    QStringList files;

    const QDir dir(path);
    foreach (const QFileInfo &fi, dir.entryInfoList(pattern, QDir::Files))
        files += fi.absoluteFilePath();

    return files;
}

static void findNewImplicitImports(const Document::Ptr &doc, const Snapshot &snapshot,
                            QStringList *importedFiles, QSet<QString> *scannedPaths)
{
    // scan files that could be implicitly imported
    // it's important we also do this for JS files, otherwise the isEmpty check will fail
    if (snapshot.documentsInDirectory(doc->path()).isEmpty()) {
        if (! scannedPaths->contains(doc->path())) {
            *importedFiles += filesInDirectoryForLanguages(doc->path(),
                                                  Document::companionLanguages(doc->language()));
            scannedPaths->insert(doc->path());
        }
    }
}

static void findNewFileImports(const Document::Ptr &doc, const Snapshot &snapshot,
                        QStringList *importedFiles, QSet<QString> *scannedPaths)
{
    // scan files and directories that are explicitly imported
    foreach (const ImportInfo &import, doc->bind()->imports()) {
        const QString &importName = import.path();
        if (import.type() == ImportType::File) {
            if (! snapshot.document(importName))
                *importedFiles += importName;
        } else if (import.type() == ImportType::Directory) {
            if (snapshot.documentsInDirectory(importName).isEmpty()) {
                if (! scannedPaths->contains(importName)) {
                    *importedFiles += filesInDirectoryForLanguages(importName,
                                                          Document::companionLanguages(doc->language()));
                    scannedPaths->insert(importName);
                }
            }
        } else if (import.type() == ImportType::QrcFile) {
            QStringList importPaths = ModelManagerInterface::instance()->filesAtQrcPath(importName);
            foreach (const QString &importPath, importPaths) {
                if (! snapshot.document(importPath))
                    *importedFiles += importPath;
            }
        } else if (import.type() == ImportType::QrcDirectory) {
            QMapIterator<QString,QStringList> dirContents(ModelManagerInterface::instance()->filesInQrcPath(importName));
            while (dirContents.hasNext()) {
                dirContents.next();
                if (Document::isQmlLikeOrJsLanguage(ModelManagerInterface::guessLanguageOfFile(dirContents.key()))) {
                    foreach (const QString &filePath, dirContents.value()) {
                        if (! snapshot.document(filePath))
                            *importedFiles += filePath;
                    }
                }
            }
        }
    }
}

static bool findNewQmlLibraryInPath(const QString &path,
                                    const Snapshot &snapshot,
                                    ModelManagerInterface *modelManager,
                                    QStringList *importedFiles,
                                    QSet<QString> *scannedPaths,
                                    QSet<QString> *newLibraries,
                                    bool ignoreMissing)
{
    // if we know there is a library, done
    const LibraryInfo &existingInfo = snapshot.libraryInfo(path);
    if (existingInfo.isValid())
        return true;
    if (newLibraries->contains(path))
        return true;
    // if we looked at the path before, done
    if (existingInfo.wasScanned())
        return false;

    const QDir dir(path);
    QFile qmldirFile(dir.filePath(QLatin1String("qmldir")));
    if (!qmldirFile.exists()) {
        if (!ignoreMissing) {
            LibraryInfo libraryInfo(LibraryInfo::NotFound);
            modelManager->updateLibraryInfo(path, libraryInfo);
        }
        return false;
    }

    if (Utils::HostOsInfo::isWindowsHost()) {
        // QTCREATORBUG-3402 - be case sensitive even here?
    }

    // found a new library!
    qmldirFile.open(QFile::ReadOnly);
    QString qmldirData = QString::fromUtf8(qmldirFile.readAll());

    QmlDirParser qmldirParser;
    qmldirParser.parse(qmldirData);

    const QString libraryPath = QFileInfo(qmldirFile).absolutePath();
    newLibraries->insert(libraryPath);
    modelManager->updateLibraryInfo(libraryPath, LibraryInfo(qmldirParser));

    // scan the qml files in the library
    foreach (const QmlDirParser::Component &component, qmldirParser.components()) {
        if (! component.fileName.isEmpty()) {
            const QFileInfo componentFileInfo(dir.filePath(component.fileName));
            const QString path = QDir::cleanPath(componentFileInfo.absolutePath());
            if (! scannedPaths->contains(path)) {
                *importedFiles += filesInDirectoryForLanguages(path,
                                                      Document::companionLanguages(Language::Unknown));
                scannedPaths->insert(path);
            }
        }
    }

    return true;
}

static void findNewQmlLibrary(
    const QString &path,
    const LanguageUtils::ComponentVersion &version,
    const Snapshot &snapshot,
    ModelManagerInterface *modelManager,
    QStringList *importedFiles,
    QSet<QString> *scannedPaths,
    QSet<QString> *newLibraries)
{
    QString libraryPath = QString::fromLatin1("%1.%2.%3").arg(
                path,
                QString::number(version.majorVersion()),
                QString::number(version.minorVersion()));
    findNewQmlLibraryInPath(
                libraryPath, snapshot, modelManager,
                importedFiles, scannedPaths, newLibraries, false);

    libraryPath = QString::fromLatin1("%1.%2").arg(
                path,
                QString::number(version.majorVersion()));
    findNewQmlLibraryInPath(
                libraryPath, snapshot, modelManager,
                importedFiles, scannedPaths, newLibraries, false);

    findNewQmlLibraryInPath(
                path, snapshot, modelManager,
                importedFiles, scannedPaths, newLibraries, false);
}

static void findNewLibraryImports(const Document::Ptr &doc, const Snapshot &snapshot,
                           ModelManagerInterface *modelManager,
                           QStringList *importedFiles, QSet<QString> *scannedPaths, QSet<QString> *newLibraries)
{
    // scan current dir
    findNewQmlLibraryInPath(doc->path(), snapshot, modelManager,
                            importedFiles, scannedPaths, newLibraries, false);

    // scan dir and lib imports
    const QStringList importPaths = modelManager->importPaths();
    foreach (const ImportInfo &import, doc->bind()->imports()) {
        if (import.type() == ImportType::Directory) {
            const QString targetPath = import.path();
            findNewQmlLibraryInPath(targetPath, snapshot, modelManager,
                                    importedFiles, scannedPaths, newLibraries, false);
        }

        if (import.type() == ImportType::Library) {
            if (!import.version().isValid())
                continue;
            foreach (const QString &importPath, importPaths) {
                const QString targetPath = QDir(importPath).filePath(import.path());
                findNewQmlLibrary(targetPath, import.version(), snapshot, modelManager,
                                  importedFiles, scannedPaths, newLibraries);
            }
        }
    }
}

void ModelManagerInterface::parseLoop(QSet<QString> &scannedPaths,
                             QSet<QString> &newLibraries,
                             WorkingCopy workingCopy,
                             QStringList files,
                             ModelManagerInterface *modelManager,
                             Language::Enum mainLanguage,
                             bool emitDocChangedOnDisk,
                             Utils::function<bool(qreal)> reportProgress)
{
    for (int i = 0; i < files.size(); ++i) {
        if (!reportProgress(qreal(i) / files.size()))
            return;

        const QString fileName = files.at(i);

        Language::Enum language = guessLanguageOfFile(fileName);
        if (language == Language::Unknown) {
            if (fileName.endsWith(QLatin1String(".qrc")))
                modelManager->updateQrcFile(fileName);
            continue;
        }
        if (language == Language::Qml
                && (mainLanguage == Language::QmlQtQuick1 || mainLanguage == Language::QmlQtQuick2))
            language = mainLanguage;
        QString contents;
        int documentRevision = 0;

        if (workingCopy.contains(fileName)) {
            QPair<QString, int> entry = workingCopy.get(fileName);
            contents = entry.first;
            documentRevision = entry.second;
        } else {
            QFile inFile(fileName);

            if (inFile.open(QIODevice::ReadOnly)) {
                QTextStream ins(&inFile);
                contents = ins.readAll();
                inFile.close();
            }
        }

        Document::MutablePtr doc = Document::create(fileName, language);
        doc->setEditorRevision(documentRevision);
        doc->setSource(contents);
        doc->parse();

        // update snapshot. requires synchronization, but significantly reduces amount of file
        // system queries for library imports because queries are cached in libraryInfo
        const Snapshot snapshot = modelManager->snapshot();

        // get list of referenced files not yet in snapshot or in directories already scanned
        QStringList importedFiles;
        findNewImplicitImports(doc, snapshot, &importedFiles, &scannedPaths);
        findNewFileImports(doc, snapshot, &importedFiles, &scannedPaths);
        findNewLibraryImports(doc, snapshot, modelManager, &importedFiles, &scannedPaths, &newLibraries);

        // add new files to parse list
        foreach (const QString &file, importedFiles) {
            if (! files.contains(file))
                files.append(file);
        }

        modelManager->updateDocument(doc);
        if (emitDocChangedOnDisk)
            modelManager->emitDocumentChangedOnDisk(doc);
    }
}

class FutureReporter
{
public:
    FutureReporter(QFutureInterface<void> &future, int multiplier = 100, int base = 0)
        :future(future), multiplier(multiplier), base(base)
    { }
    bool operator()(qreal val)
    {
        if (future.isCanceled())
            return false;
        future.setProgressValue(int(base + multiplier * val));
        return true;
    }
private:
    QFutureInterface<void> &future;
    int multiplier;
    int base;
};

void ModelManagerInterface::parse(QFutureInterface<void> &future,
                         WorkingCopy workingCopy,
                         QStringList files,
                         ModelManagerInterface *modelManager,
                         Language::Enum mainLanguage,
                         bool emitDocChangedOnDisk)
{
    FutureReporter reporter(future);
    future.setProgressRange(0, 100);

    // paths we have scanned for files and added to the files list
    QSet<QString> scannedPaths;
    // libraries we've found while scanning imports
    QSet<QString> newLibraries;
    parseLoop(scannedPaths, newLibraries, workingCopy, files, modelManager, mainLanguage,
              emitDocChangedOnDisk, reporter);
    future.setProgressValue(100);
}

struct ScanItem {
    QString path;
    int depth;
    ScanItem(QString path = QString(), int depth = 0)
        : path(path), depth(depth)
    { }
};

void ModelManagerInterface::importScan(QFutureInterface<void> &future,
                              ModelManagerInterface::WorkingCopy workingCopy,
                              QStringList paths, ModelManagerInterface *modelManager,
                              Language::Enum language,
                              bool emitDocChangedOnDisk)
{
    // paths we have scanned for files and added to the files list
    QSet<QString> scannedPaths = modelManager->m_scannedPaths;
    // libraries we've found while scanning imports
    QSet<QString> newLibraries;

    QVector<ScanItem> pathsToScan;
    pathsToScan.reserve(paths.size());
    {
        QMutexLocker l(&modelManager->m_mutex);
        foreach (const QString &path, paths) {
            QString cPath = QDir::cleanPath(path);
            if (modelManager->m_scannedPaths.contains(cPath))
                continue;
            pathsToScan.append(ScanItem(cPath));
            modelManager->m_scannedPaths.insert(cPath);
        }
    }
    const int maxScanDepth = 5;
    int progressRange = pathsToScan.size() * (1 << (2 + maxScanDepth));
    int totalWork(progressRange), workDone(0);
    future.setProgressRange(0, progressRange); // update max length while iterating?
    const bool libOnly = true; // FIXME remove when tested more
    const Snapshot snapshot = modelManager->snapshot();
    while (!pathsToScan.isEmpty() && !future.isCanceled()) {
        ScanItem toScan = pathsToScan.last();
        pathsToScan.pop_back();
        int pathBudget = (maxScanDepth + 2 - toScan.depth);
        if (!scannedPaths.contains(toScan.path)) {
            QStringList importedFiles;
            if (!findNewQmlLibraryInPath(toScan.path, snapshot, modelManager, &importedFiles,
                                         &scannedPaths, &newLibraries, true)
                    && !libOnly && snapshot.documentsInDirectory(toScan.path).isEmpty())
                importedFiles += filesInDirectoryForLanguages(toScan.path,
                                                     Document::companionLanguages(language));
            workDone += 1;
            future.setProgressValue(progressRange * workDone / totalWork);
            if (!importedFiles.isEmpty()) {
                FutureReporter reporter(future, progressRange * pathBudget / (4 * totalWork),
                                        progressRange * workDone / totalWork);
                parseLoop(scannedPaths, newLibraries, workingCopy, importedFiles, modelManager,
                          language, emitDocChangedOnDisk, reporter); // run in parallel??
                importedFiles.clear();
            }
            workDone += pathBudget / 4 - 1;
            future.setProgressValue(progressRange * workDone / totalWork);
        } else {
            workDone += pathBudget / 4;
        }
        // always descend tree, as we might have just scanned with a smaller depth
        if (toScan.depth < maxScanDepth) {
            QDir dir(toScan.path);
            QStringList subDirs(dir.entryList(QDir::Dirs));
            workDone += 1;
            totalWork += pathBudget / 2 * subDirs.size() - pathBudget * 3 / 4 + 1;
            foreach (const QString path, subDirs)
                pathsToScan.append(ScanItem(dir.absoluteFilePath(path), toScan.depth + 1));
        } else {
            workDone += pathBudget *3 / 4;
        }
        future.setProgressValue(progressRange * workDone / totalWork);
    }
    future.setProgressValue(progressRange);
    if (future.isCanceled()) {
        // assume no work has been done
        QMutexLocker l(&modelManager->m_mutex);
        foreach (const QString &path, paths)
            modelManager->m_scannedPaths.remove(path);
    }
}

QStringList ModelManagerInterface::importPaths() const
{
    QMutexLocker l(&m_mutex);
    return m_allImportPaths;
}

QmlLanguageBundles ModelManagerInterface::activeBundles() const
{
    QMutexLocker l(&m_mutex);
    return m_activeBundles;
}

QmlLanguageBundles ModelManagerInterface::extendedBundles() const
{
    QMutexLocker l(&m_mutex);
    return m_extendedBundles;
}

void ModelManagerInterface::maybeScan(const QStringList &importPaths,
                                      Language::Enum defaultLanguage)
{
    QStringList pathToScan;
    {
        QMutexLocker l(&m_mutex);
        foreach (QString importPath, importPaths)
            if (!m_scannedPaths.contains(importPath)) {
                pathToScan.append(importPath);
            }
    }

    if (pathToScan.count() > 1) {
        QFuture<void> result = QtConcurrent::run(&ModelManagerInterface::importScan,
                                                  workingCopyInternal(), pathToScan,
                                                  this, defaultLanguage,
                                                  true);

        if (m_synchronizer.futures().size() > 10) {
            QList<QFuture<void> > futures = m_synchronizer.futures();

            m_synchronizer.clearFutures();

            foreach (const QFuture<void> &future, futures) {
                if (! (future.isFinished() || future.isCanceled()))
                    m_synchronizer.addFuture(future);
            }
        }

        m_synchronizer.addFuture(result);

        addTaskInternal(result, tr("QML import scan"), Constants::TASK_IMPORT_SCAN);
    }
}

void ModelManagerInterface::updateImportPaths()
{
    QStringList allImportPaths;
    QmlLanguageBundles activeBundles;
    QmlLanguageBundles extendedBundles;
    QMapIterator<ProjectExplorer::Project *, ProjectInfo> it(m_projects);
    while (it.hasNext()) {
        it.next();
        foreach (const QString &path, it.value().importPaths) {
            const QString canonicalPath = QFileInfo(path).canonicalFilePath();
            if (!canonicalPath.isEmpty())
                allImportPaths += canonicalPath;
        }
    }
    it.toFront();
    while (it.hasNext()) {
        it.next();
        activeBundles.mergeLanguageBundles(it.value().activeBundle);
        foreach (Language::Enum l, it.value().activeBundle.languages()) {
            foreach (const QString &path, it.value().activeBundle.bundleForLanguage(l)
                 .searchPaths().stringList()) {
                const QString canonicalPath = QFileInfo(path).canonicalFilePath();
                if (!canonicalPath.isEmpty())
                    allImportPaths += canonicalPath;
            }
        }
    }
    it.toFront();
    while (it.hasNext()) {
        it.next();
        extendedBundles.mergeLanguageBundles(it.value().extendedBundle);
        foreach (Language::Enum l, it.value().extendedBundle.languages()) {
            foreach (const QString &path, it.value().extendedBundle.bundleForLanguage(l)
                     .searchPaths().stringList()) {
                const QString canonicalPath = QFileInfo(path).canonicalFilePath();
                if (!canonicalPath.isEmpty())
                    allImportPaths += canonicalPath;
            }
        }
    }
    allImportPaths += m_defaultImportPaths;
    allImportPaths.removeDuplicates();

    {
        QMutexLocker l(&m_mutex);
        m_allImportPaths = allImportPaths;
        m_activeBundles = activeBundles;
        m_extendedBundles = extendedBundles;
    }


    // check if any file in the snapshot imports something new in the new paths
    Snapshot snapshot = m_validSnapshot;
    QStringList importedFiles;
    QSet<QString> scannedPaths;
    QSet<QString> newLibraries;
    foreach (const Document::Ptr &doc, snapshot)
        findNewLibraryImports(doc, snapshot, this, &importedFiles, &scannedPaths, &newLibraries);

    updateSourceFiles(importedFiles, true);

    if (!m_shouldScanImports)
        return;
    maybeScan(allImportPaths, Language::Qml);
}

ModelManagerInterface::ProjectInfo ModelManagerInterface::defaultProjectInfo() const
{
    if (m_projects.isEmpty())
        return ProjectInfo();
    return m_projects.begin().value();
}

void ModelManagerInterface::loadPluginTypes(const QString &libraryPath, const QString &importPath,
                                   const QString &importUri, const QString &importVersion)
{
    m_pluginDumper->loadPluginTypes(libraryPath, importPath, importUri, importVersion);
}

// is called *inside a c++ parsing thread*, to allow hanging on to source and ast
void ModelManagerInterface::maybeQueueCppQmlTypeUpdate(const CPlusPlus::Document::Ptr &doc)
{
    // avoid scanning documents without source code available
    doc->keepSourceAndAST();
    if (doc->utf8Source().isEmpty()) {
        doc->releaseSourceAndAST();
        return;
    }

    // keep source and AST alive if we want to scan for register calls
    const bool scan = FindExportedCppTypes::maybeExportsTypes(doc);
    if (!scan)
        doc->releaseSourceAndAST();

    // delegate actual queuing to the gui thread
    QMetaObject::invokeMethod(this, "queueCppQmlTypeUpdate",
                              Q_ARG(CPlusPlus::Document::Ptr, doc), Q_ARG(bool, scan));
}

void ModelManagerInterface::queueCppQmlTypeUpdate(const CPlusPlus::Document::Ptr &doc, bool scan)
{
    QPair<CPlusPlus::Document::Ptr, bool> prev = m_queuedCppDocuments.value(doc->fileName());
    if (prev.first && prev.second)
        prev.first->releaseSourceAndAST();
    m_queuedCppDocuments.insert(doc->fileName(), qMakePair(doc, scan));
    m_updateCppQmlTypesTimer->start();
}

void ModelManagerInterface::startCppQmlTypeUpdate()
{
    // if a future is still running, delay
    if (m_cppQmlTypesUpdater.isRunning()) {
        m_updateCppQmlTypesTimer->start();
        return;
    }

    CPlusPlus::CppModelManagerBase *cppModelManager =
            CPlusPlus::CppModelManagerBase::instance();
    if (!cppModelManager)
        return;

    m_cppQmlTypesUpdater = QtConcurrent::run(
                &ModelManagerInterface::updateCppQmlTypes,
                this, cppModelManager->snapshot(), m_queuedCppDocuments);
    m_queuedCppDocuments.clear();
}

void ModelManagerInterface::asyncReset()
{
    m_asyncResetTimer->start();
}

void ModelManagerInterface::updateCppQmlTypes(QFutureInterface<void> &interface,
                                     ModelManagerInterface *qmlModelManager,
                                     CPlusPlus::Snapshot snapshot,
                                     QHash<QString, QPair<CPlusPlus::Document::Ptr, bool> > documents)
{
    CppDataHash newData = qmlModelManager->cppData();

    FindExportedCppTypes finder(snapshot);

    bool hasNewInfo = false;
    typedef QPair<CPlusPlus::Document::Ptr, bool> DocScanPair;
    foreach (const DocScanPair &pair, documents) {
        if (interface.isCanceled())
            return;

        CPlusPlus::Document::Ptr doc = pair.first;
        const bool scan = pair.second;
        const QString fileName = doc->fileName();
        if (!scan) {
            hasNewInfo = hasNewInfo || newData.remove(fileName) > 0;
            continue;
        }

        finder(doc);

        QList<LanguageUtils::FakeMetaObject::ConstPtr> exported = finder.exportedTypes();
        QHash<QString, QString> contextProperties = finder.contextProperties();
        if (exported.isEmpty() && contextProperties.isEmpty()) {
            hasNewInfo = hasNewInfo || newData.remove(fileName) > 0;
        } else {
            CppData &data = newData[fileName];
            // currently we have no simple way to compare, so we assume the worse
            hasNewInfo = true;
            data.exportedTypes = exported;
            data.contextProperties = contextProperties;
        }

        doc->releaseSourceAndAST();
    }

    QMutexLocker locker(&qmlModelManager->m_cppDataMutex);
    qmlModelManager->m_cppDataHash = newData;
    if (hasNewInfo)
        // one could get away with re-linking the cpp types...
        QMetaObject::invokeMethod(qmlModelManager, "asyncReset");
}

ModelManagerInterface::CppDataHash ModelManagerInterface::cppData() const
{
    QMutexLocker locker(&m_cppDataMutex);
    return m_cppDataHash;
}

LibraryInfo ModelManagerInterface::builtins(const Document::Ptr &doc) const
{
    QList<ProjectExplorer::Project *> projects = m_fileToProject.values(doc->fileName());

    ProjectExplorer::Project *project = 0;
    foreach (ProjectExplorer::Project *p, projects) {
        if (p) {
            project = p;
            break;
        }
    }

    if (!project)
        return LibraryInfo();

    QMutexLocker locker(&m_mutex);
    ProjectInfo info = m_projects.value(project);
    if (!info.isValid())
        return LibraryInfo();

    return m_validSnapshot.libraryInfo(info.qtImportsPath);
}

ViewerContext ModelManagerInterface::completeVContext(const ViewerContext &vCtx,
                                             const Document::Ptr &doc) const
{
    Q_UNUSED(doc);
    ViewerContext res = vCtx;
    switch (res.flags) {
    case ViewerContext::Complete:
        break;
    case ViewerContext::AddQtPath:
    case ViewerContext::AddAllPaths:
        res.paths << importPaths();
    }
    res.flags = ViewerContext::Complete;
    return res;
}

ViewerContext ModelManagerInterface::defaultVContext(bool autoComplete, const Document::Ptr &doc) const
{
    if (autoComplete)
        return completeVContext(m_vContext, doc);
    else
        return m_vContext;
}

void ModelManagerInterface::setDefaultVContext(const ViewerContext &vContext)
{
    m_vContext = vContext;
}

void ModelManagerInterface::joinAllThreads()
{
    foreach (QFuture<void> future, m_synchronizer.futures())
        future.waitForFinished();
}

void ModelManagerInterface::resetCodeModel()
{
    QStringList documents;

    {
        QMutexLocker locker(&m_mutex);

        // find all documents currently in the code model
        foreach (Document::Ptr doc, m_validSnapshot)
            documents.append(doc->fileName());

        // reset the snapshot
        m_validSnapshot = Snapshot();
        m_newestSnapshot = Snapshot();
    }

    // start a reparse thread
    updateSourceFiles(documents, false);
}

} // namespace QmlJS
