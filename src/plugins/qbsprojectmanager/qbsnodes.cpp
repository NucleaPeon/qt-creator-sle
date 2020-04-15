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

#include "qbsnodes.h"

#include "qbsproject.h"
#include "qbsprojectmanagerconstants.h"
#include "qbsrunconfiguration.h"

#include <coreplugin/fileiconprovider.h>
#include <coreplugin/idocument.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/target.h>
#include <qtsupport/qtsupportconstants.h>
#include <utils/hostosinfo.h>
#include <utils/qtcassert.h>

#include <qbs.h>

#include <QDir>
#include <QStyle>

// ----------------------------------------------------------------------
// Helpers:
// ----------------------------------------------------------------------

static QString displayNameFromPath(const QString &path, const QString &base)
{
    QString dir = base;
    if (!base.endsWith(QLatin1Char('/')))
        dir.append(QLatin1Char('/'));

    QString name = path;
    if (name.startsWith(dir)) {
        name = name.mid(dir.count());
    } else {
        QFileInfo fi = QFileInfo(path);
        name = QCoreApplication::translate("Qbs::QbsProjectNode", "%1 in %2")
                .arg(fi.fileName(), fi.absolutePath());
    }

    return name;
}

static QIcon generateIcon(const QString &overlay)
{
    const QSize desiredSize = QSize(16, 16);
    const QIcon overlayIcon(overlay);
    const QPixmap pixmap
            = Core::FileIconProvider::overlayIcon(QStyle::SP_DirIcon, overlayIcon, desiredSize);

    QIcon result;
    result.addPixmap(pixmap);

    return result;
}

namespace QbsProjectManager {
namespace Internal {

QIcon QbsGroupNode::m_groupIcon;
QIcon QbsProjectNode::m_projectIcon;
QIcon QbsProductNode::m_productIcon;

class FileTreeNode {
public:
    explicit FileTreeNode(const QString &n = QString(), FileTreeNode *p = 0, bool f = false) :
        parent(p), name(n), m_isFile(f)
    {
        if (p)
            p->children.append(this);
    }

    ~FileTreeNode()
    {
        qDeleteAll(children);
    }

    FileTreeNode *addPart(const QString &n, bool isFile)
    {
        foreach (FileTreeNode *c, children) {
            if (c->name == n)
                return c;
        }
        return new FileTreeNode(n, this, isFile);
    }

    bool isFile() { return m_isFile; }

    static FileTreeNode *moveChildrenUp(FileTreeNode *node)
    {
        QTC_ASSERT(node, return 0);

        FileTreeNode *newParent = node->parent;
        if (!newParent)
            return 0;

        // disconnect node and parent:
        node->parent = 0;
        newParent->children.removeOne(node);

        foreach (FileTreeNode *c, node->children) {
            // update path, make sure there will be no / before "C:" on windows:
            if (Utils::HostOsInfo::isWindowsHost() && node->name.isEmpty())
                c->name = node->name;
            else
                c->name = node->name + QLatin1Char('/') + c->name;

            newParent->children.append(c);
            c->parent = newParent;
        }

        // Delete node
        node->children.clear();
        delete node;
        return newParent;
    }

    // Moves the children of the node pointing to basedir to the root of the tree.
    static void reorder(FileTreeNode *node, const QString &basedir)
    {
        QTC_CHECK(!basedir.isEmpty());
        QString prefix = basedir;
        if (basedir.startsWith(QLatin1Char('/')))
            prefix = basedir.mid(1);
        prefix.append(QLatin1Char('/'));

        if (node->path() == basedir) {
            // Find root node:
            FileTreeNode *root = node;
            while (root->parent)
                root = root->parent;

            foreach (FileTreeNode *c, node->children) {
                // Update children names by prepending basedir
                c->name = prefix + c->name;
                // Update parent information:
                c->parent = root;

                root->children.append(c);
            }

            // Clean up node:
            node->children.clear();
            node->parent->children.removeOne(node);
            node->parent = 0;
            delete node;

            return;
        }

        foreach (FileTreeNode *n, node->children)
            reorder(n, basedir);
    }

    static void simplify(FileTreeNode *node)
    {
        foreach (FileTreeNode *c, node->children)
            simplify(c);

        if (!node->parent)
            return;

        if (node->children.isEmpty() && !node->isFile()) {
            // Clean up empty folder nodes:
            node->parent->children.removeOne(node);
            node->parent = 0;
            delete node;
        } else if (node->children.count() == 1 && !node->children.at(0)->isFile()) {
            // Compact folder nodes with one child only:
            moveChildrenUp(node);
        }
    }

    QString path()
    {
        QString p = name;
        FileTreeNode *node = parent;
        while (node) {
            if (!Utils::HostOsInfo::isWindowsHost() || !node->name.isEmpty())
                p = node->name + QLatin1Char('/') + p;
            node = node->parent;
        }
        return p;
    }

    QList<FileTreeNode *> children;
    FileTreeNode *parent;
    QString name;
    bool m_isFile;
};

// ----------------------------------------------------------------------
// QbsFileNode:
// ----------------------------------------------------------------------

QbsFileNode::QbsFileNode(const QString &filePath, const ProjectExplorer::FileType fileType,
                         bool generated, int line) :
    ProjectExplorer::FileNode(filePath, fileType, generated, line)
{ }

QString QbsFileNode::displayName() const
{
    int l = line();
    if (l < 0)
        return ProjectExplorer::FileNode::displayName();
    return ProjectExplorer::FileNode::displayName() + QLatin1Char(':') + QString::number(l);
}

bool QbsFileNode::update(const qbs::CodeLocation &loc)
{
    const QString oldPath = path();
    const int oldLine = line();

    setPathAndLine(loc.fileName(), loc.line());
    return (line() != oldLine || path() != oldPath);
}

// ---------------------------------------------------------------------------
// QbsBaseProjectNode:
// ---------------------------------------------------------------------------

QbsBaseProjectNode::QbsBaseProjectNode(const QString &path) :
    ProjectExplorer::ProjectNode(path)
{ }

bool QbsBaseProjectNode::showInSimpleTree() const
{
    return false;
}

QList<ProjectExplorer::ProjectAction> QbsBaseProjectNode::supportedActions(ProjectExplorer::Node *node) const
{
    Q_UNUSED(node);
    return QList<ProjectExplorer::ProjectAction>();
}

bool QbsBaseProjectNode::canAddSubProject(const QString &proFilePath) const
{
    Q_UNUSED(proFilePath);
    return false;
}

bool QbsBaseProjectNode::addSubProjects(const QStringList &proFilePaths)
{
    Q_UNUSED(proFilePaths);
    return false;
}

bool QbsBaseProjectNode::removeSubProjects(const QStringList &proFilePaths)
{
    Q_UNUSED(proFilePaths);
    return false;
}

bool QbsBaseProjectNode::addFiles(const QStringList &filePaths, QStringList *notAdded)
{
    Q_UNUSED(filePaths);
    Q_UNUSED(notAdded);
    return false;
}

bool QbsBaseProjectNode::removeFiles(const QStringList &filePaths, QStringList *notRemoved)
{
    Q_UNUSED(filePaths);
    Q_UNUSED(notRemoved);
    return false;
}

bool QbsBaseProjectNode::deleteFiles(const QStringList &filePaths)
{
    Q_UNUSED(filePaths);
    return false;
}

bool QbsBaseProjectNode::renameFile(const QString &filePath, const QString &newFilePath)
{
    Q_UNUSED(filePath);
    Q_UNUSED(newFilePath);
    return false;
}

QList<ProjectExplorer::RunConfiguration *> QbsBaseProjectNode::runConfigurationsFor(ProjectExplorer::Node *node)
{
    Q_UNUSED(node);
    return QList<ProjectExplorer::RunConfiguration *>();
}

// --------------------------------------------------------------------
// QbsGroupNode:
// --------------------------------------------------------------------

QbsGroupNode::QbsGroupNode(const qbs::GroupData *grp, const QString &productPath) :
    QbsBaseProjectNode(QString()),
    m_qbsGroupData(0)
{
    if (m_groupIcon.isNull())
        m_groupIcon = QIcon(QString::fromLatin1(Constants::QBS_GROUP_ICON));

    setIcon(m_groupIcon);

    QbsFileNode *idx = new QbsFileNode(grp->location().fileName(),
                                       ProjectExplorer::ProjectFileType, false,
                                       grp->location().line());
    addFileNodes(QList<ProjectExplorer::FileNode *>() << idx);

    updateQbsGroupData(grp, productPath, true, true);
}

bool QbsGroupNode::isEnabled() const
{
    if (!parentFolderNode() || !m_qbsGroupData)
        return false;
    return static_cast<QbsProductNode *>(parentFolderNode())->isEnabled()
            && m_qbsGroupData->isEnabled();
}

void QbsGroupNode::updateQbsGroupData(const qbs::GroupData *grp, const QString &productPath,
                                      bool productWasEnabled, bool productIsEnabled)
{
    Q_ASSERT(grp);

    if (grp == m_qbsGroupData && productPath == m_productPath)
        return;

    bool groupWasEnabled = productWasEnabled && m_qbsGroupData && m_qbsGroupData->isEnabled();
    bool groupIsEnabled = productIsEnabled && grp->isEnabled();
    bool updateExisting = groupWasEnabled != groupIsEnabled;

    m_productPath = productPath;
    m_qbsGroupData = grp;

    setPath(grp->location().fileName());
    setDisplayName(grp->name());

    QbsFileNode *idx = 0;
    foreach (ProjectExplorer::FileNode *fn, fileNodes()) {
        idx = qobject_cast<QbsFileNode *>(fn);
        if (idx)
            break;
    }
    if (idx->update(grp->location()) || updateExisting)
        idx->emitNodeUpdated();

    setupFiles(this, grp->allFilePaths(), productPath, updateExisting);

    if (updateExisting)
        emitNodeUpdated();
}

void QbsGroupNode::setupFiles(QbsBaseProjectNode *root, const QStringList &files,
                              const QString &productPath, bool updateExisting)
{
    // Build up a tree of nodes:
    FileTreeNode tree;

    foreach (const QString &path, files) {
        QStringList pathSegments = path.split(QLatin1Char('/'), QString::SkipEmptyParts);

        FileTreeNode *root = &tree;
        while (!pathSegments.isEmpty()) {
            bool isFile = pathSegments.count() == 1;
            root = root->addPart(pathSegments.takeFirst(), isFile);
        }
    }

    FileTreeNode::reorder(&tree, productPath);
    FileTreeNode::simplify(&tree);

    setupFolder(root, &tree, productPath, updateExisting);
}

void QbsGroupNode::setupFolder(ProjectExplorer::FolderNode *root,
                               const FileTreeNode *fileTree, const QString &baseDir,
                               bool updateExisting)
{
    // We only need to care about FileNodes and FolderNodes here. Everything else is
    // handled elsewhere.
    // QbsGroupNodes are managed by the QbsProductNode.
    // The buildsystem file is either managed by QbsProductNode or by updateQbsGroupData(...).

    QList<ProjectExplorer::FileNode *> filesToRemove;
    foreach (ProjectExplorer::FileNode *fn, root->fileNodes()) {
        if (!qobject_cast<QbsFileNode *>(fn))
            filesToRemove << fn;
    }
    QList<ProjectExplorer::FileNode *> filesToAdd;

    QList<ProjectExplorer::FolderNode *> foldersToRemove;
    foreach (ProjectExplorer::FolderNode *fn, root->subFolderNodes()) {
        if (fn->nodeType() == ProjectExplorer::ProjectNodeType)
            continue; // Skip ProjectNodes mixed into the folders...
        foldersToRemove.append(fn);
    }

    foreach (FileTreeNode *c, fileTree->children) {
        QString path = c->path();

        // Handle files:
        if (c->isFile()) {
            ProjectExplorer::FileNode *fn = 0;
            foreach (ProjectExplorer::FileNode *f, root->fileNodes()) {
                // There can be one match only here!
                if (f->path() != path)
                    continue;
                fn = f;
                break;
            }
            if (fn) {
                filesToRemove.removeOne(fn);
                if (updateExisting)
                    fn->emitNodeUpdated();
            } else {
                fn = new ProjectExplorer::FileNode(path, ProjectExplorer::UnknownFileType, false);
                filesToAdd.append(fn);
            }
            continue;
        } else {
            ProjectExplorer::FolderNode *fn = 0;
            foreach (ProjectExplorer::FolderNode *f, root->subFolderNodes()) {
                // There can be one match only here!
                if (f->path() != path)
                    continue;
                fn = f;
                break;
            }
            if (!fn) {
                fn = new FolderNode(c->path());
                root->addFolderNodes(QList<FolderNode *>() << fn);
            } else {
                foldersToRemove.removeOne(fn);
                if (updateExisting)
                    fn->emitNodeUpdated();
            }
            fn->setDisplayName(displayNameFromPath(c->path(), baseDir));

            setupFolder(fn, c, c->path(), updateExisting);
        }
    }
    root->removeFileNodes(filesToRemove);
    root->removeFolderNodes(foldersToRemove);
    root->addFileNodes(filesToAdd);
}

// --------------------------------------------------------------------
// QbsProductNode:
// --------------------------------------------------------------------

QbsProductNode::QbsProductNode(const qbs::ProductData &prd) :
    QbsBaseProjectNode(prd.location().fileName())
{
    if (m_productIcon.isNull())
        m_productIcon = generateIcon(QString::fromLatin1(Constants::QBS_PRODUCT_OVERLAY_ICON));

    setIcon(m_productIcon);

    ProjectExplorer::FileNode *idx = new QbsFileNode(prd.location().fileName(),
                                                     ProjectExplorer::ProjectFileType, false,
                                                     prd.location().line());
    addFileNodes(QList<ProjectExplorer::FileNode *>() << idx);

    setQbsProductData(prd);
}

bool QbsProductNode::isEnabled() const
{
    return m_qbsProductData.isEnabled();
}

bool QbsProductNode::showInSimpleTree() const
{
    return true;
}

void QbsProductNode::setQbsProductData(const qbs::ProductData prd)
{
    if (m_qbsProductData == prd)
        return;

    bool productWasEnabled = m_qbsProductData.isValid() && m_qbsProductData.isEnabled();
    bool productIsEnabled = prd.isEnabled();
    bool updateExisting = productWasEnabled != productIsEnabled;

    setDisplayName(prd.name());
    setPath(prd.location().fileName());
    const QString &productPath = QFileInfo(prd.location().fileName()).absolutePath();

    // Find the QbsFileNode we added earlier:
    QbsFileNode *idx = 0;
    foreach (ProjectExplorer::FileNode *fn, fileNodes()) {
        idx = qobject_cast<QbsFileNode *>(fn);
        if (idx)
            break;
    }
    if (idx->update(prd.location()) || updateExisting)
        idx->emitNodeUpdated();

    QList<ProjectExplorer::ProjectNode *> toAdd;
    QList<ProjectExplorer::ProjectNode *> toRemove = subProjectNodes();

    foreach (const qbs::GroupData &grp, prd.groups()) {
        if (grp.name() == prd.name() && grp.location() == prd.location()) {
            // Set implicit product group right onto this node:
            QbsGroupNode::setupFiles(this, grp.allFilePaths(), productPath, updateExisting);
            continue;
        }
        QbsGroupNode *gn = findGroupNode(grp.name());
        if (gn) {
            toRemove.removeOne(gn);
            gn->updateQbsGroupData(&grp, productPath, productWasEnabled, productIsEnabled);
        } else {
            gn = new QbsGroupNode(&grp, productPath);
            toAdd.append(gn);
        }
    }

    addProjectNodes(toAdd);
    removeProjectNodes(toRemove);

    m_qbsProductData = prd;
    if (updateExisting)
        emitNodeUpdated();
}

QList<ProjectExplorer::RunConfiguration *> QbsProductNode::runConfigurationsFor(ProjectExplorer::Node *node)
{
    Q_UNUSED(node);
    QList<ProjectExplorer::RunConfiguration *> result;
    QbsProjectNode *pn = qobject_cast<QbsProjectNode *>(projectNode());
    if (!isEnabled() || !pn || !pn->qbsProject().isValid()
            || pn->qbsProject().targetExecutable(m_qbsProductData, qbs::InstallOptions()).isEmpty()) {
        return result;
    }

    foreach (ProjectExplorer::RunConfiguration *rc, pn->project()->activeTarget()->runConfigurations()) {
        QbsRunConfiguration *qbsRc = qobject_cast<QbsRunConfiguration *>(rc);
        if (!qbsRc)
            continue;
        if (qbsRc->qbsProduct() == qbsProductData().name())
            result << qbsRc;
    }

    return result;
}

QbsGroupNode *QbsProductNode::findGroupNode(const QString &name)
{
    foreach (ProjectExplorer::ProjectNode *n, subProjectNodes()) {
        QbsGroupNode *qn = static_cast<QbsGroupNode *>(n);
        if (qn->qbsGroupData()->name() == name)
            return qn;
    }
    return 0;
}

// --------------------------------------------------------------------
// QbsProjectNode:
// --------------------------------------------------------------------

QbsProjectNode::QbsProjectNode(QbsProject *project) :
    QbsBaseProjectNode(project->projectFilePath()),
    m_project(project)
{
    ctor();
}

QbsProjectNode::QbsProjectNode(const QString &path) :
    QbsBaseProjectNode(path),
    m_project(0)
{
    ctor();
}

QbsProjectNode::~QbsProjectNode()
{
    // do not delete m_project
}

void QbsProjectNode::update(const qbs::Project &prj)
{
    update(prj.isValid() ? prj.projectData() : qbs::ProjectData());
    m_qbsProject = prj;
}

void QbsProjectNode::update(const qbs::ProjectData &prjData)
{
    QList<ProjectExplorer::ProjectNode *> toAdd;
    QList<ProjectExplorer::ProjectNode *> toRemove = subProjectNodes();

    foreach (const qbs::ProjectData &subData, prjData.subProjects()) {
        QbsProjectNode *qn = findProjectNode(subData.name());
        if (!qn) {
            QbsProjectNode *subProject = new QbsProjectNode(subData.location().fileName());
            subProject->update(subData);
            toAdd << subProject;
        } else {
            qn->update(subData);
            toRemove.removeOne(qn);
        }
    }

    foreach (const qbs::ProductData &prd, prjData.products()) {
        QbsProductNode *qn = findProductNode(prd.name());
        if (!qn) {
            toAdd << new QbsProductNode(prd);
        } else {
            qn->setQbsProductData(prd);
            toRemove.removeOne(qn);
        }
    }

    if (!prjData.name().isEmpty())
        setDisplayName(prjData.name());
    else
        setDisplayName(m_project->displayName());

    removeProjectNodes(toRemove);
    addProjectNodes(toAdd);

    m_qbsProjectData = prjData;
}

QbsProject *QbsProjectNode::project() const
{
    if (!m_project && projectNode())
        return static_cast<QbsProjectNode *>(projectNode())->project();
    return m_project;
}

const qbs::Project QbsProjectNode::qbsProject() const
{
    QbsProjectNode *parent = qobject_cast<QbsProjectNode *>(projectNode());
    if (!m_qbsProject.isValid() && parent != this)
        return parent->qbsProject();
    return m_qbsProject;
}

const qbs::ProjectData QbsProjectNode::qbsProjectData() const
{
    return m_qbsProjectData;
}

bool QbsProjectNode::showInSimpleTree() const
{
    return true;
}

void QbsProjectNode::ctor()
{
    if (m_projectIcon.isNull())
        m_projectIcon = generateIcon(QString::fromLatin1(QtSupport::Constants::ICON_QT_PROJECT));

    setIcon(m_projectIcon);
    addFileNodes(QList<ProjectExplorer::FileNode *>()
                 << new ProjectExplorer::FileNode(path(), ProjectExplorer::ProjectFileType, false));
}

QbsProductNode *QbsProjectNode::findProductNode(const QString &name)
{
    foreach (ProjectExplorer::ProjectNode *n, subProjectNodes()) {
        QbsProductNode *qn = qobject_cast<QbsProductNode *>(n);
        if (qn && qn->qbsProductData().name() == name)
            return qn;
    }
    return 0;
}

QbsProjectNode *QbsProjectNode::findProjectNode(const QString &name)
{
    foreach (ProjectExplorer::ProjectNode *n, subProjectNodes()) {
        QbsProjectNode *qn = qobject_cast<QbsProjectNode *>(n);
        if (qn && qn->qbsProjectData().name() == name)
            return qn;
    }
    return 0;
}

} // namespace Internal
} // namespace QbsProjectManager
