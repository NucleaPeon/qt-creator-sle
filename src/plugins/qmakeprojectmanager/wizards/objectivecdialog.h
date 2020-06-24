/****************************************************************************
**
** Copyright (C) 2020 Daniel Kettle <initial.dann@gmail.com>
** Contact: https://github.com/NucleaPeon/qt-creator-sle
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

#ifndef OBJCWIZARDDIALOG_H
#define OBJCWIZARDDIALOG_H

#include "qtwizard.h"

namespace QmakeProjectManager {
namespace Internal {

struct QtProjectParameters;

class ObjectiveCWizardDialog : public BaseQmakeProjectWizardDialog
{
    Q_OBJECT
public:
    explicit ObjectiveCWizardDialog(const QString &templateName,
                                    const QIcon &icon,
                                    bool showModulesPage,
                                    QWidget *parent, const Core::WizardDialogParameters &parameters);

    QtProjectParameters parameters() const;
};

} // namespace Internal
} // namespace QmakeProjectManager

#endif // OBJCWIZARDDIALOG_H
