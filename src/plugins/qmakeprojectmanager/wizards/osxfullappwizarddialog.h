/****************************************************************************
**
** Copyright (C) 2020 Daniel Kettle
** Contact: initial.dann@gmail.com
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
****************************************************************************/

#ifndef OSXAPPWIZARDDIALOG_H
#define OSXAPPWIZARDDIALOG_H

#include "qtwizard.h"
#include "guistructs.h"

namespace QmakeProjectManager {
namespace Internal {

struct QtProjectParameters;
class FilesPage;

class OSXFullAppWizardDialog : public BaseQmakeProjectWizardDialog
{
    Q_OBJECT

public:
    explicit OSXFullAppWizardDialog(const QString &templateName,
                                const QIcon &icon,
                                bool showModulesPage,
                                QWidget *parent,
                                const Core::WizardDialogParameters &parameters);

    void setBaseClasses(const QStringList &baseClasses);
    void setSuffixes(const QString &header, const QString &source,  const QString &form);
    void setLowerCaseFiles(bool l);

    QtProjectParameters projectParameters() const;
    GuiAppParameters parameters() const;

private:
    FilesPage *m_filesPage;
};

} // namespace Internal
} // namespace QmakeProjectManager

#endif // OSXAPPWIZARDDIALOG_H
