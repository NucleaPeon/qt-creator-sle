/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
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

#ifndef OSXFULLAPPWIZARD_H
#define OSXFULLAPPWIZARD_H

#include "qtwizard.h"
#include "guistructs.h"

namespace QmakeProjectManager {
namespace Internal {

class OSXFullAppWizard : public QtWizard
{
    Q_OBJECT

public:
    OSXFullAppWizard();

private:
    QWizard *createWizardDialog(QWidget *parent,
                                const Core::WizardDialogParameters &wizardDialogParameters) const;

    Core::GeneratedFiles generateFiles(const QWizard *w, QString *errorMessage) const;

private:
    static bool parametrizeTemplate(const QString &templatePath, const QString &templateName,
                                    const GuiAppParameters &params,
                                    QString *target, QString *errorMessage);
};

} // namespace Internal
} // namespace QmakeProjectManager

#endif // OSXFULLAPPWIZARD_H
