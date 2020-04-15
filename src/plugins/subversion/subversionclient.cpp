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

#include "subversionclient.h"
#include "subversionsettings.h"

#include <vcsbase/vcsbaseplugin.h>
#include <vcsbase/vcsbaseconstants.h>
#include <vcsbase/vcsbaseeditorparameterwidget.h>
#include <utils/synchronousprocess.h>

#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>

namespace Subversion {
namespace Internal {

// Collect all parameters required for a diff to be able to associate them
// with a diff editor and re-run the diff with parameters.
struct SubversionDiffParameters
{
    QString workingDir;
    QStringList extraOptions;
    QStringList files;
};

// Parameter widget controlling whitespace diff mode, associated with a parameter
class SubversionDiffParameterWidget : public VcsBase::VcsBaseEditorParameterWidget
{
    Q_OBJECT
public:
    explicit SubversionDiffParameterWidget(SubversionClient *client,
                                           const SubversionDiffParameters &p,
                                           QWidget *parent = 0);
    QStringList arguments() const;
    void executeCommand();

private:
    SubversionClient *m_client;
    const SubversionDiffParameters m_params;
};

SubversionDiffParameterWidget::SubversionDiffParameterWidget(SubversionClient *client,
                                                             const SubversionDiffParameters &p,
                                                             QWidget *parent)
    : VcsBase::VcsBaseEditorParameterWidget(parent), m_client(client), m_params(p)
{
    mapSetting(addToggleButton(QLatin1String("w"), tr("Ignore Whitespace")),
               client->settings()->boolPointer(SubversionSettings::diffIgnoreWhiteSpaceKey));
}

QStringList SubversionDiffParameterWidget::arguments() const
{
    QStringList args;
    // Subversion wants" -x -<ext-args>", default being -u
    const QStringList formatArguments = VcsBaseEditorParameterWidget::arguments();
    if (!formatArguments.isEmpty()) {
        args << QLatin1String("-x")
             << (QLatin1String("-u") + formatArguments.join(QString()));
    }
    return args;
}

void SubversionDiffParameterWidget::executeCommand()
{
    m_client->diff(m_params.workingDir, m_params.files, m_params.extraOptions);
}

SubversionClient::SubversionClient(SubversionSettings *settings) :
    VcsBase::VcsBaseClient(settings)
{
}

SubversionSettings *SubversionClient::settings() const
{
    return dynamic_cast<SubversionSettings *>(VcsBase::VcsBaseClient::settings());
}

Core::Id SubversionClient::vcsEditorKind(VcsCommand cmd) const
{
    switch (cmd) {
    case DiffCommand:
        return "Subversion Diff Editor"; // TODO: create subversionconstants.h
    default:
        return Core::Id();
    }
}

SubversionClient::Version SubversionClient::svnVersion()
{
    if (m_svnVersionBinary != settings()->binaryPath()) {
        QStringList args;
        args << QLatin1String("--version") << QLatin1String("-q");
        const Utils::SynchronousProcessResponse response =
                VcsBase::VcsBasePlugin::runVcs(QDir().absolutePath(), settings()->binaryPath(),
                                               args, settings()->timeOutMs());
        if (response.result == Utils::SynchronousProcessResponse::Finished &&
                response.exitCode == 0) {
            m_svnVersionBinary = settings()->binaryPath();
            m_svnVersion = response.stdOut.trimmed();
        } else {
            m_svnVersionBinary.clear();
            m_svnVersion.clear();
        }
    }

    SubversionClient::Version v;
    if (::sscanf(m_svnVersion.toLatin1().constData(), "%d.%d.%d",
                 &v.majorVersion, &v.minorVersion, &v.patchVersion) != 3) {
        v.majorVersion = v.minorVersion = v.patchVersion = -1;
    }

    return v;
}

// Add authorization options to the command line arguments.
// SVN pre 1.5 does not accept "--userName" for "add", which is most likely
// an oversight. As no password is needed for the option, generally omit it.
QStringList SubversionClient::addAuthenticationOptions(const QStringList &args,
                                                       const QString &userName,
                                                       const QString &password)
{
    if (userName.isEmpty())
        return args;
    if (!args.empty() && args.front() == QLatin1String("add"))
        return args;
    QStringList rc;
    rc.push_back(QLatin1String("--username"));
    rc.push_back(userName);
    if (!password.isEmpty()) {
        rc.push_back(QLatin1String("--password"));
        rc.push_back(password);
    }
    rc.append(args);
    return rc;
}

void SubversionClient::diff(const QString &workingDir, const QStringList &files,
                           const QStringList &extraOptions)
{
    QStringList args(extraOptions);
    Version v = svnVersion();

    // --internal-diff is new in v1.7.0
    if (v.majorVersion > 1 || (v.majorVersion == 1 && v.minorVersion >= 7))
        args.append(QLatin1String("--internal-diff"));

    const bool hasAuth = settings()->hasAuthentication();
    const QString userName = hasAuth ? settings()->stringValue(SubversionSettings::userKey) : QString();
    const QString password = hasAuth ? settings()->stringValue(SubversionSettings::passwordKey) : QString();
    args = addAuthenticationOptions(args, userName, password);

    VcsBaseClient::diff(workingDir, files, args);
}

QString SubversionClient::findTopLevelForFile(const QFileInfo &file) const
{
    Q_UNUSED(file)
    return QString();
}

QStringList SubversionClient::revisionSpec(const QString &revision) const
{
    Q_UNUSED(revision)
    return QStringList();
}

VcsBase::VcsBaseClient::StatusItem SubversionClient::parseStatusLine(const QString &line) const
{
    Q_UNUSED(line)
    return VcsBase::VcsBaseClient::StatusItem();
}

VcsBase::VcsBaseEditorParameterWidget *SubversionClient::createDiffEditor(
        const QString &workingDir, const QStringList &files, const QStringList &extraOptions)
{
    Q_UNUSED(extraOptions)
    SubversionDiffParameters p;
    p.workingDir = workingDir;
    p.files = files;
    p.extraOptions = extraOptions;
    return new SubversionDiffParameterWidget(this, p);
}

} // namespace Internal
} // namespace Subversion

#include "subversionclient.moc"
