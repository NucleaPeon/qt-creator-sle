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

#include "kitchooser.h"

#include "kitconfigwidget.h"
#include "kitinformation.h"
#include "kitmanager.h"
#include "projectexplorerconstants.h"

#include <coreplugin/icore.h>

#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSettings>

namespace ProjectExplorer {

const char lastKitKey[] = "LastSelectedKit";

KitChooser::KitChooser(QWidget *parent) :
    QWidget(parent)
{
    m_chooser = new QComboBox(this);
    m_chooser->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_manageButton = new QPushButton(KitConfigWidget::msgManage(), this);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_chooser);
    layout->addWidget(m_manageButton);

    connect(m_chooser, SIGNAL(currentIndexChanged(int)), SLOT(onCurrentIndexChanged(int)));
    connect(m_chooser, SIGNAL(activated(int)), SIGNAL(activated(int)));
    connect(m_manageButton, SIGNAL(clicked()), SLOT(onManageButtonClicked()));
    connect(KitManager::instance(), SIGNAL(kitsChanged()), SLOT(populate()));
}

void KitChooser::onManageButtonClicked()
{
    Core::ICore::showOptionsDialog(Constants::PROJECTEXPLORER_SETTINGS_CATEGORY,
           Constants::KITS_SETTINGS_PAGE_ID, this);
}

void KitChooser::onCurrentIndexChanged(int index)
{
    if (Kit *kit = kitAt(index))
        setToolTip(kitToolTip(kit));
    else
        setToolTip(QString());
    emit currentIndexChanged(index);
}

bool KitChooser::kitMatches(const Kit *k) const
{
    return k->isValid();
}

QString KitChooser::kitText(const Kit *k) const
{
    return k->displayName();
}

QString KitChooser::kitToolTip(Kit *k) const
{
    return k->toHtml();
}

void KitChooser::populate()
{
    m_chooser->clear();
    foreach (Kit *kit, KitManager::kits()) {
        if (kitMatches(kit)) {
            m_chooser->addItem(kitText(kit), qVariantFromValue(kit->id()));
            m_chooser->setItemData(m_chooser->count() - 1, kitToolTip(kit), Qt::ToolTipRole);
        }
    }

    const int n = m_chooser->count();
    const int index = Core::ICore::settings()->value(QLatin1String(lastKitKey)).toInt();
    m_chooser->setCurrentIndex(0 <= index && index < n ? index : -1);
    m_chooser->setEnabled(n > 1);
}

Kit *KitChooser::currentKit() const
{
    const int index = m_chooser->currentIndex();
    Core::ICore::settings()->setValue(QLatin1String(lastKitKey), index);
    return index == -1 ? 0 : kitAt(index);
}

void KitChooser::setCurrentKitId(Core::Id id)
{
    for (int i = 0, n = m_chooser->count(); i != n; ++i) {
        if (kitAt(i)->id() == id) {
            m_chooser->setCurrentIndex(i);
            break;
        }
    }
}

Core::Id KitChooser::currentKitId() const
{
    Kit *kit = currentKit();
    return kit ? kit->id() : Core::Id();
}

Kit *KitChooser::kitAt(int index) const
{
    Core::Id id = qvariant_cast<Core::Id>(m_chooser->itemData(index));
    return KitManager::find(id);
}

} // namespace ProjectExplorer
