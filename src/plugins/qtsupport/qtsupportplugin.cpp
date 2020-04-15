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

#include "qtsupportplugin.h"

#include "customexecutablerunconfiguration.h"
#include "desktopqtversionfactory.h"
#include "qtfeatureprovider.h"
#include "qtkitinformation.h"
#include "qtoptionspage.h"
#include "qtversionmanager.h"
#include "simulatorqtversionfactory.h"
#include "uicodemodelsupport.h"
#include "winceqtversionfactory.h"

#include "profilereader.h"

#include <coreplugin/icore.h>
#include <coreplugin/mimedatabase.h>
#include <coreplugin/variablemanager.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/target.h>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
#include "gettingstartedwelcomepage.h"
#endif

#include <QtPlugin>

static const char kHostBins[] = "CurrentProject:QT_HOST_BINS";
static const char kInstallBins[] = "CurrentProject:QT_INSTALL_BINS";

using namespace Core;
using namespace QtSupport;
using namespace QtSupport::Internal;

bool QtSupportPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments);
    Q_UNUSED(errorMessage);
    QMakeParser::initialize();
    ProFileEvaluator::initialize();
    new ProFileCacheManager(this);

    if (!MimeDatabase::addMimeTypes(QLatin1String(":qtsupport/QtSupport.mimetypes.xml"), errorMessage))
        return false;

    addAutoReleasedObject(new QtVersionManager);
    addAutoReleasedObject(new DesktopQtVersionFactory);
    addAutoReleasedObject(new SimulatorQtVersionFactory);
    addAutoReleasedObject(new WinCeQtVersionFactory);
    addAutoReleasedObject(new UiCodeModelManager);

    QtFeatureProvider *featureMgr = new QtFeatureProvider;
    addAutoReleasedObject(featureMgr);

    addAutoReleasedObject(new QtOptionsPage);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
    ExamplesWelcomePage *welcomePage;
    welcomePage = new ExamplesWelcomePage;
    addAutoReleasedObject(welcomePage);
    welcomePage->setShowExamples(true);

    welcomePage = new ExamplesWelcomePage;
    addAutoReleasedObject(welcomePage);
#endif
    addAutoReleasedObject(new CustomExecutableRunConfigurationFactory);

    ProjectExplorer::KitManager::registerKitInformation(new QtKitInformation);

    QtVersionManager::initialized();

    return true;
}

void QtSupportPlugin::extensionsInitialized()
{
    VariableManager::registerVariable(kHostBins,
        tr("Full path to the host bin directory of the current project's Qt version."));
    VariableManager::registerVariable(kInstallBins,
        tr("Full path to the target bin directory of the current project's Qt version."
           " You probably want %1 instead.").arg(QString::fromLatin1(kHostBins)));
    connect(VariableManager::instance(), SIGNAL(variableUpdateRequested(QByteArray)),
            this, SLOT(updateVariable(QByteArray)));
}

bool QtSupportPlugin::delayedInitialize()
{
    return QtVersionManager::delayedInitialize();
}

void QtSupportPlugin::updateVariable(const QByteArray &variable)
{
    if (variable != kHostBins && variable != kInstallBins)
        return;

    ProjectExplorer::Project *project = ProjectExplorer::ProjectExplorerPlugin::currentProject();
    if (!project || !project->activeTarget()) {
        VariableManager::remove(variable);
        return;
    }

    const BaseQtVersion *qtVersion = QtKitInformation::qtVersion(project->activeTarget()->kit());
    if (!qtVersion) {
        VariableManager::remove(variable);
        return;
    }

    QString value = qtVersion->qmakeProperty(variable == kHostBins ? "QT_HOST_BINS" : "QT_INSTALL_BINS");
    VariableManager::insert(variable, value);
}

Q_EXPORT_PLUGIN(QtSupportPlugin)
