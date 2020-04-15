/**************************************************************************
**
** Copyright (c) 2014 Dmitry Savchenko
** Copyright (c) 2014 Vasiliy Sorokin
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
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

#include "optionspage.h"
#include "constants.h"
#include "optionsdialog.h"

#include <coreplugin/icore.h>

#include <QList>
#include <QIcon>
#include <QMessageBox>

Todo::Internal::OptionsDialog *some = 0;

namespace Todo {
namespace Internal {

OptionsPage::OptionsPage(const Settings &settings, QObject *parent) :
    IOptionsPage(parent),
    m_widget(0)
{
    setSettings(settings);

    setId("TodoSettings");
    setDisplayName(tr("To-Do"));
    setCategory("To-Do");
    setDisplayCategory(tr("To-Do"));
    setCategoryIcon(QLatin1String(Constants::ICON_TODO));
}

void OptionsPage::setSettings(const Settings &settings)
{
    m_settings = settings;
}

QWidget *OptionsPage::widget()
{
    if (!m_widget) {
        m_widget = new OptionsDialog;
        m_widget->setSettings(m_settings);
    }
    return m_widget;
}

void OptionsPage::apply()
{
    Settings newSettings = m_widget->settings();

    if (newSettings != m_settings) {
        m_settings = newSettings;
        emit settingsChanged(m_settings);
    }
}

void OptionsPage::finish()
{
    delete m_widget;
}

} // namespace Internal
} // namespace Todo

