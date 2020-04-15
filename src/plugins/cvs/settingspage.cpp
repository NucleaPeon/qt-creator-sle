/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#include "settingspage.h"
#include "cvssettings.h"
#include "cvsplugin.h"

#include <coreplugin/icore.h>
#include <extensionsystem/pluginmanager.h>
#include <vcsbase/vcsbaseconstants.h>
#include <utils/pathchooser.h>

#include <QCoreApplication>
#include <QTextStream>
#include <QFileDialog>

using namespace Cvs::Internal;
using namespace Utils;

SettingsPageWidget::SettingsPageWidget(QWidget *parent) :
    QWidget(parent)
{
    m_ui.setupUi(this);
    m_ui.commandPathChooser->setExpectedKind(PathChooser::ExistingCommand);
    m_ui.commandPathChooser->setHistoryCompleter(QLatin1String("Cvs.Command.History"));
    m_ui.commandPathChooser->setPromptDialogTitle(tr("CVS Command"));
}

CvsSettings SettingsPageWidget::settings() const
{
    CvsSettings rc;
    rc.setValue(CvsSettings::binaryPathKey, m_ui.commandPathChooser->rawPath());
    rc.setValue(CvsSettings::cvsRootKey, m_ui.rootLineEdit->text());
    rc.setValue(CvsSettings::diffOptionsKey, m_ui.diffOptionsLineEdit->text());
    rc.setValue(CvsSettings::timeoutKey, m_ui.timeOutSpinBox->value());
    rc.setValue(CvsSettings::promptOnSubmitKey, m_ui.promptToSubmitCheckBox->isChecked());
    rc.setValue(CvsSettings::describeByCommitIdKey, m_ui.describeByCommitIdCheckBox->isChecked());
    return rc;
}

void SettingsPageWidget::setSettings(const CvsSettings &s)
{
    m_ui.commandPathChooser->setPath(s.binaryPath());
    m_ui.rootLineEdit->setText(s.stringValue(CvsSettings::cvsRootKey));
    m_ui.diffOptionsLineEdit->setText(s.stringValue(CvsSettings::diffOptionsKey));
    m_ui.timeOutSpinBox->setValue(s.intValue(CvsSettings::timeoutKey));
    m_ui.promptToSubmitCheckBox->setChecked(s.boolValue(CvsSettings::promptOnSubmitKey));
    m_ui.describeByCommitIdCheckBox->setChecked(s.boolValue(CvsSettings::describeByCommitIdKey));
}

SettingsPage::SettingsPage()
{
    setId(VcsBase::Constants::VCS_ID_CVS);
    setDisplayName(tr("CVS"));
}

QWidget *SettingsPage::widget()
{
    if (!m_widget) {
        m_widget = new SettingsPageWidget;
        m_widget->setSettings(CvsPlugin::instance()->settings());
    }
    return m_widget;
}

void SettingsPage::apply()
{
    CvsPlugin::instance()->setSettings(m_widget->settings());
}

void SettingsPage::finish()
{
    delete m_widget;
}
