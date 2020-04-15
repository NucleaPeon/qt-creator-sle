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

#include "helpfindsupport.h"
#include "helpviewer.h"

#include <utils/qtcassert.h>

using namespace Core;
using namespace Help::Internal;

HelpFindSupport::HelpFindSupport(CentralWidget *centralWidget)
    : m_centralWidget(centralWidget)
{
}

HelpFindSupport::~HelpFindSupport()
{
}

Core::FindFlags HelpFindSupport::supportedFindFlags() const
{
    return FindBackward | FindCaseSensitively;
}

QString HelpFindSupport::currentFindString() const
{
    QTC_ASSERT(m_centralWidget, return QString());
    HelpViewer *viewer = m_centralWidget->currentHelpViewer();
    if (!viewer)
        return QString();
    return viewer->selectedText();
}

QString HelpFindSupport::completedFindString() const
{
    return QString();
}

Core::IFindSupport::Result HelpFindSupport::findIncremental(const QString &txt,
    Core::FindFlags findFlags)
{
    findFlags &= ~FindBackward;
    return find(txt, findFlags, true) ? Found : NotFound;
}

Core::IFindSupport::Result HelpFindSupport::findStep(const QString &txt,
    Core::FindFlags findFlags)
{
    return find(txt, findFlags, false) ? Found : NotFound;
}

bool HelpFindSupport::find(const QString &txt, Core::FindFlags findFlags, bool incremental)
{
    QTC_ASSERT(m_centralWidget, return false);
    bool wrapped = false;
    bool found = m_centralWidget->find(txt, findFlags, incremental, &wrapped);
    if (wrapped)
        showWrapIndicator(m_centralWidget);
    return found;
}

// -- HelpViewerFindSupport

HelpViewerFindSupport::HelpViewerFindSupport(HelpViewer *viewer)
    : m_viewer(viewer)
{
}

Core::FindFlags HelpViewerFindSupport::supportedFindFlags() const
{
    return FindBackward | FindCaseSensitively;
}

QString HelpViewerFindSupport::currentFindString() const
{
    QTC_ASSERT(m_viewer, return QString());
    return m_viewer->selectedText();
}

Core::IFindSupport::Result HelpViewerFindSupport::findIncremental(const QString &txt,
    Core::FindFlags findFlags)
{
    QTC_ASSERT(m_viewer, return NotFound);
    findFlags &= ~FindBackward;
    return find(txt, findFlags, true) ? Found : NotFound;
}

Core::IFindSupport::Result HelpViewerFindSupport::findStep(const QString &txt,
    Core::FindFlags findFlags)
{
    QTC_ASSERT(m_viewer, return NotFound);
    return find(txt, findFlags, false) ? Found : NotFound;
}

bool HelpViewerFindSupport::find(const QString &txt,
    Core::FindFlags findFlags, bool incremental)
{
    QTC_ASSERT(m_viewer, return false);
    bool wrapped = false;
    bool found = m_viewer->findText(txt, findFlags, incremental, false, &wrapped);
    if (wrapped)
        showWrapIndicator(m_viewer);
    return found;
}
