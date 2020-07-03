/****************************************************************************
**
** Copyright (C) 2020 PeonDevelopments 
** Contact: Daniel Kettle <initial.dann@gmail.com>
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
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

/**
 * @brief The FileWizard class - adds wizard for creating new ObjectiveC source file
 */

#include "objectivecfilewizard.h"
#include "../objectiveceditorconstants.h"

#include <utils/filewizarddialog.h>
#include <texteditor/textfilewizard.h>

#include <QWizard>

namespace ObjectiveCEditor {

/**
 * @brief Initialize wizard and add new option to "New..." dialog.
 * @param parent
 */
FileWizard::FileWizard()
{
    setWizardKind(Core::IWizard::FileWizard);
    setId(QLatin1String(Constants::C_OBJC_SOURCE_WIZARD_ID));
    setCategory(QLatin1String(Constants::C_OBJC_WIZARD_CATEGORY));
    setDisplayCategory(QLatin1String(Constants::C_OBJC_DISPLAY_CATEGORY));
    setDisplayName(FileWizard::tr(Constants::EN_OBJC_SOURCE_DISPLAY_NAME));
    setDescription(FileWizard::tr(Constants::EN_OBJC_SOURCE_DESCRIPTION));
}

/**
 * @brief FileWizard::createWizardDialog
 * @param parent
 * @param params
 * @return
 */
QWizard *FileWizard::createWizardDialog(QWidget *parent,
                                        const Core::WizardDialogParameters &params) const
{
    Utils::FileWizardDialog *pDialog = new Utils::FileWizardDialog(parent);
    pDialog->setWindowTitle(tr("New %1").arg(displayName()));
    pDialog->setPath(params.defaultPath());
    foreach (QWizardPage *p, params.extensionPages())
        applyExtensionPageShortTitle(pDialog, pDialog->addPage(p));

    return pDialog;
}

Core::GeneratedFiles FileWizard::generateFiles(const QWizard *dialog,
                                                QString *errorMessage) const
{
    Q_UNUSED(errorMessage)

    const Utils::FileWizardDialog *pWizard =
            qobject_cast<const Utils::FileWizardDialog *>(dialog);

    QString folder = pWizard->path();
    QString name = pWizard->fileName();

    name = Core::BaseFileWizard::buildFileName(
                folder, name, QLatin1String(Constants::C_OBJC_EXTENSION));
    Core::GeneratedFile file(name);
    file.setContents(QLatin1String(Constants::C_OBJC_SOURCE_CONTENT));
    file.setAttributes(Core::GeneratedFile::OpenEditorAttribute);

    return (Core::GeneratedFiles() << file);
}

} // namespace ObjectiveCEditor
