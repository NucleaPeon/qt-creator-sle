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

#ifndef OBJECTIVECEDITOR_CLASSWIZARDDIALOG_H
#define OBJECTIVECEDITOR_CLASSWIZARDDIALOG_H

#include <utils/wizard.h>
#include <utils/newclasswidget.h>
#include <QScopedPointer>
#include <QVariantMap>

namespace ObjectiveCEditor {
namespace Internal {

class ClassNamePage;

class ClassWizardParameters
{
public:
    QString className;
    QString fileName;
    QString path;
    QString baseClass;
    Utils::NewClassWidget::ClassType classType;
};

class ClassWizardDialog : public Utils::Wizard
{
    Q_OBJECT
public:
    explicit ClassWizardDialog(QWidget *parent = 0);
    virtual ~ClassWizardDialog();

    void setPath(const QString &path);
    ClassWizardParameters parameters() const;

    void setExtraValues(const QVariantMap &extraValues);
    const QVariantMap &extraValues() const;

private:
    QScopedPointer<ClassNamePage> m_classNamePage;
    QVariantMap m_extraValues;
};

} // namespace Internal
} // namespace ObjectiveCEditor

#endif // OBJECTIVECEDITOR_CLASSWIZARDDIALOG_H
