/**************************************************************************
**
** Copyright (c) 2014 Hugues Delorme
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
#include "uncommitdialog.h"

#include "ui_uncommitdialog.h"
#include "bazaarclient.h"
#include "bazaarplugin.h"
#include <utils/qtcassert.h>

#include <QPushButton>

namespace Bazaar {
namespace Internal {

UnCommitDialog::UnCommitDialog(QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui::UnCommitDialog)
{
    m_ui->setupUi(this);

    QPushButton* dryRunBtn = new QPushButton(tr("Dry Run"));
    dryRunBtn->setToolTip(tr("Test the outcome of removing the last committed revision, without actually removing anything."));
    m_ui->buttonBox->addButton(dryRunBtn, QDialogButtonBox::ApplyRole);
    connect(dryRunBtn, SIGNAL(clicked()), this, SLOT(dryRun()));
}

UnCommitDialog::~UnCommitDialog()
{
    delete m_ui;
}

QStringList UnCommitDialog::extraOptions() const
{
    QStringList opts;
    if (m_ui->keepTagsCheckBox->isChecked())
        opts += QLatin1String("--keep-tags");
    if (m_ui->localCheckBox->isChecked())
        opts += QLatin1String("--local");
    return opts;
}

QString UnCommitDialog::revision() const
{
    return m_ui->revisionLineEdit->text().trimmed();
}

void UnCommitDialog::dryRun()
{
    BazaarPlugin* bzrPlugin = BazaarPlugin::instance();
    QTC_ASSERT(bzrPlugin->currentState().hasTopLevel(), return);
    bzrPlugin->client()->synchronousUncommit(bzrPlugin->currentState().topLevel(),
                                             revision(),
                                             extraOptions() << QLatin1String("--dry-run"));
}

} // namespace Internal
} // namespace Bazaar
