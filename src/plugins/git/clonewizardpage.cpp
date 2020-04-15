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

#include "clonewizardpage.h"
#include "gitplugin.h"
#include "gitclient.h"

#include <vcsbase/command.h>

#include <QCheckBox>

namespace Git {

struct CloneWizardPagePrivate {
    CloneWizardPagePrivate();

    bool urlIsLocal(const QString &url);

    const QString mainLinePostfix;
    const QString gitPostFix;
    const QString protocolDelimiter;
    QCheckBox *recursiveCheckBox;
};

CloneWizardPagePrivate::CloneWizardPagePrivate() :
    mainLinePostfix(QLatin1String("/mainline.git")),
    gitPostFix(QLatin1String(".git")),
    protocolDelimiter(QLatin1String("://")),
    recursiveCheckBox(0)
{
}

bool CloneWizardPagePrivate::urlIsLocal(const QString &url)
{
    if (url.startsWith(QLatin1String("file://"))
        || url.startsWith(QLatin1Char('/'))
        || (url.at(0).isLetter() && url.at(1) == QLatin1Char(':') && url.at(2) == QLatin1Char('\\')))
        return true;
    return false;
}

CloneWizardPage::CloneWizardPage(QWidget *parent) :
    VcsBase::BaseCheckoutWizardPage(parent),
    d(new CloneWizardPagePrivate)
{
    setTitle(tr("Location"));
    setSubTitle(tr("Specify repository URL, checkout directory and path."));
    setRepositoryLabel(tr("Clone URL:"));
    d->recursiveCheckBox = new QCheckBox(tr("Recursive"));
    addLocalControl(d->recursiveCheckBox);
}

CloneWizardPage::~CloneWizardPage()
{
    delete d;
}

QString CloneWizardPage::directoryFromRepository(const QString &urlIn) const
{
    /* Try to figure out a good directory name from something like:
     * 'user@host:qt/qt.git', 'http://host/qt/qt.git' 'local repo'
     * ------> 'qt' .  */
    const QChar slash = QLatin1Char('/');
    QString url = urlIn.trimmed().replace(QLatin1Char('\\'), slash);

    // remove host
    const int protocolDelimiterPos = url.indexOf(d->protocolDelimiter); // "://"
    const int startRepoSearchPos = protocolDelimiterPos == -1 ? 0 : protocolDelimiterPos + d->protocolDelimiter.size();
    int repoPos = url.indexOf(QLatin1Char(':'), startRepoSearchPos);
    if (repoPos == -1)
        repoPos = url.indexOf(slash, startRepoSearchPos);
    if (repoPos != -1)
        url.remove(0, repoPos + 1);
    // Remove postfixes
    if (url.endsWith(d->mainLinePostfix)) {
        url.truncate(url.size() - d->mainLinePostfix.size());
    } else {
        if (url.endsWith(d->gitPostFix))
            url.truncate(url.size() - d->gitPostFix.size());
    }
    // Check for equal parts, something like "qt/qt" -> "qt"
    const int slashPos = url.indexOf(slash);
    if (slashPos != -1 && slashPos == (url.size() - 1) / 2) {
        if (url.leftRef(slashPos) == url.rightRef(slashPos))
            url.truncate(slashPos);
    }
    // fix invalid characters
    const QChar dash = QLatin1Char('-');
    url.replace(QRegExp(QLatin1String("[^0-9a-zA-Z_.-]")), dash);
    // trim leading dashes (they are annoying and get created when using local pathes)
    url.replace(QRegExp(QLatin1String("^-+")), QString());
    return url;
}

VcsBase::Command *CloneWizardPage::createCheckoutJob(QString *checkoutPath) const
{
     const Internal::GitClient *client = Internal::GitPlugin::instance()->gitClient();
     const QString workingDirectory = path();
     const QString checkoutDir = directory();
     *checkoutPath = workingDirectory + QLatin1Char('/') + checkoutDir;

     const QString checkoutBranch = branch();

     QStringList args(QLatin1String("clone"));
     if (!checkoutBranch.isEmpty())
         args << QLatin1String("--branch") << checkoutBranch;
     if (d->recursiveCheckBox->isChecked())
         args << QLatin1String("--recursive");
     args << QLatin1String("--progress") << repository() << checkoutDir;
     VcsBase::Command *command = new VcsBase::Command(client->gitBinaryPath(), workingDirectory,
                                                      client->processEnvironment());
     command->addFlags(VcsBase::VcsBasePlugin::MergeOutputChannels);
     command->addJob(args, -1);
     return command;
}

QStringList CloneWizardPage::branches(const QString &repository, int *current)
{
    // Run git on remote repository if an URL was specified.
    *current = -1;

    if (repository.isEmpty())
        return QStringList();
     const QStringList branches = Internal::GitPlugin::instance()->gitClient()->synchronousRepositoryBranches(repository);
     if (!branches.isEmpty())
         *current = 0; // default branch is always returned first!
     return branches;
}

} // namespace Git
