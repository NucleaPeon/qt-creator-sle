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

#ifndef OBJECTIVECEDITOR_CLASSNAMEPAGE_H
#define OBJECTIVECEDITOR_CLASSNAMEPAGE_H

#include <QWizardPage>
#include <QScopedPointer>

namespace Utils { class NewClassWidget; }

namespace ObjectiveCEditor {
namespace Internal {

class ClassNamePage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ClassNamePage(QWidget *parent = 0);
    virtual ~ClassNamePage();

    bool isComplete() const { return m_isValid; }
    Utils::NewClassWidget *newClassWidget() const { return m_newClassWidget.data(); }

private slots:
    void slotValidChanged();

private:
    void initParameters();

    QScopedPointer<Utils::NewClassWidget> m_newClassWidget;
    bool m_isValid;
};

} // namespace Internal
} // namespace ObjectiveCEditor

#endif // OBJECTIVECEDITOR_CLASSNAMEPAGE_H
