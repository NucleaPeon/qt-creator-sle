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

#include "objectivecclasswizarddialog.h"
#include "objectivecclassnamepage.h"

#include <utils/newclasswidget.h>
#include <coreplugin/basefilewizard.h>

namespace ObjectiveCEditor {
namespace Internal {

ClassWizardDialog::ClassWizardDialog(QWidget *parent)
    : Utils::Wizard(parent)
    , m_classNamePage(new ClassNamePage(this))
{
    setWindowTitle(tr("ObjectiveC Class Wizard"));
    const int classNameId = addPage(m_classNamePage.data());
    wizardProgress()->item(classNameId)->setTitle(tr("Details"));
}

ClassWizardDialog::~ClassWizardDialog()
{
}

ClassWizardParameters ClassWizardDialog::parameters() const
{
    ClassWizardParameters p;
    const Utils::NewClassWidget *ncw = m_classNamePage->newClassWidget();
    p.className = ncw->className();
    p.fileName = ncw->sourceFileName();
    p.baseClass = ncw->baseClassName();
    p.path = ncw->path();
    p.classType = ncw->classType();

    return p;
}

void ClassWizardDialog::setExtraValues(const QVariantMap &extraValues)
{
    m_extraValues = extraValues;
}

const QVariantMap &ClassWizardDialog::extraValues() const
{
    return m_extraValues;
}

void ClassWizardDialog::setPath(const QString &path)
{
    m_classNamePage->newClassWidget()->setPath(path);
}

} // namespace Internal
} // namespace ObjectiveCEditor
