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

#ifndef OBJECTIVECEDITOR_CLASSWIZARD_H
#define OBJECTIVECEDITOR_CLASSWIZARD_H

#include <coreplugin/basefilewizard.h>
#include <utils/wizard.h>

#include <QWizardPage>

namespace Utils { class NewClassWidget; }
namespace ProjectExplorer { class Kit; }

namespace ObjectiveCEditor {
namespace Internal {

class ClassWizardDialog;

struct ClassWizardParameters
{
    QString className;
    QString headerFile;
    QString sourceFile;
    QString baseClass;
    QString path;
    int classType;
};

class ClassWizard : public Core::BaseFileWizard
{
    Q_OBJECT

public:
    ClassWizard();

private:
    QWizard *createWizardDialog(QWidget *parent,
                                const Core::WizardDialogParameters &params) const;


    Core::GeneratedFiles generateFiles(const QWizard *w,
                                               QString *errorMessage) const;
    QString sourceSuffix() const;
    QString headerSuffix() const;

    ProjectExplorer::Kit *kitForWizard(const ClassWizardDialog *wizard) const;

    static bool generateHeaderAndSource(const ClassWizardParameters &params,
                                        QString *header, QString *source);
};


} // namespace Internal
} // namespace ObjectiveCEditor

#endif // OBJECTIVECEDITOR_CLASSWIZARD_H
