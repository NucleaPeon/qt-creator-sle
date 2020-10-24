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

#ifndef OBJECTIVEC_SOURCEGENERATOR_H
#define OBJECTIVEC_SOURCEGENERATOR_H

#include <utils/newclasswidget.h>
#include <QSet>
#include <QString>

namespace ObjectiveCEditor {
namespace Internal {

class SourceGenerator
{
public:
    enum QtBinding {
        
    };

    enum QtVersion {
        Qt4,
        Qt5
    };

    SourceGenerator();
    ~SourceGenerator();

    void setObjectiveCQtBinding(QtBinding binding);
    void setObjectiveCQtVersion(QtVersion version);

    QString generateClass(const QString &className,
                          Utils::NewClassWidget::ClassType classType) const;

    QString generateObjectiveCPPMain(const QString &windowTitle) const;
    QString generateObjectiveCMain(const QString &windowTitle) const;
private:
    QString qtModulesImport(const QSet<QString> &modules) const;
    QString moduleForQWidget() const;

    QtBinding m_pythonQtBinding;
    QtVersion m_pythonQtVersion;
};

} // namespace Internal
} // namespace ObjectiveCEditor

#endif // OBJECTIVEC_SOURCEGENERATOR_H
