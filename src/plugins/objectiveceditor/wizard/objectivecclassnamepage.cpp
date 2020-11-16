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

#include "objectivecclassnamepage.h"
#include "../objectiveceditorconstants.h"

#include <coreplugin/icore.h>
#include <coreplugin/mimedatabase.h>
#include <utils/newclasswidget.h>

#include <QVBoxLayout>

namespace ObjectiveCEditor {
namespace Internal {

ClassNamePage::ClassNamePage(QWidget *parent)
    : QWizardPage(parent)
    , m_isValid(false)
{
    setTitle(tr("Enter Class Name"));
    setSubTitle(tr("The source and header filename will be derived from the class name"));

    m_newClassWidget.reset(new Utils::NewClassWidget);
    // Order, set extensions first before suggested name is derived
    m_newClassWidget->setBaseClassEditable(false);
    m_newClassWidget->setFormInputVisible(false);
    m_newClassWidget->setHeaderInputVisible(true);
    m_newClassWidget->setNamespacesEnabled(false);
    m_newClassWidget->setBaseClassInputVisible(true);
    m_newClassWidget->setNamesDelimiter(QLatin1String("."));
    m_newClassWidget->setAllowDirectories(true);

    connect(m_newClassWidget.data(), SIGNAL(validChanged()), this, SLOT(slotValidChanged()));

    QVBoxLayout *pageLayout = new QVBoxLayout(this);
    pageLayout->addWidget(m_newClassWidget.data());
    QSpacerItem *vSpacer = new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::Expanding);
    pageLayout->addItem(vSpacer);
    initParameters();
}

ClassNamePage::~ClassNamePage()
{
}

void ClassNamePage::initParameters()
{
    m_newClassWidget->setSourceExtension(QLatin1String(Constants::C_OBJC_EXTENSION));
}

void ClassNamePage::slotValidChanged()
{
    const bool validNow = m_newClassWidget->isValid();
    if (m_isValid != validNow) {
        m_isValid = validNow;
        emit completeChanged();
    }
}

} // namespace Internal
} // namespace ObjectiveCEditor
