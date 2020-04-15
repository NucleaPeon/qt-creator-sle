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

#include "diagnostic.h"
#include "unit.h"
#include "utils.h"
#include "utils_p.h"

#include <clang-c/Index.h>

#include <QMutex>
#include <QMutexLocker>

namespace ClangCodeModel {
namespace Internal {

QPair<bool, QStringList> precompile(const PchInfo::Ptr &pchInfo)
{
//    qDebug() << "*** Precompiling" << pchInfo->inputFileName()
//             << "into" << pchInfo->fileName()
//             << "with options" << pchInfo->options();

    bool ok = false;

    Internal::Unit::Ptr unit = Internal::Unit::create(pchInfo->inputFileName());
    unit->setCompilationOptions(pchInfo->options());

    unsigned parseOpts = CXTranslationUnit_ForSerialization
            | CXTranslationUnit_Incomplete;
    unit->setManagementOptions(parseOpts);

    unit->parse();
    if (unit->isLoaded())
        ok = CXSaveError_None == unit->save(pchInfo->fileName());

    return qMakePair(ok, Internal::formattedDiagnostics(unit));
}

namespace {
static bool clangInitialised = false;
static QMutex initialisationMutex;
} // anonymous namespace

void initializeClang()
{
    if (clangInitialised)
        return;

    QMutexLocker locker(&initialisationMutex);
    if (clangInitialised)
        return;

    clang_toggleCrashRecovery(1);
    clang_enableStackTraces();
    clangInitialised = true;

    qRegisterMetaType<ClangCodeModel::Diagnostic>();
    qRegisterMetaType<QList<ClangCodeModel::Diagnostic> >();
}

} // Internal namespace
} // ClangCodeModel namespace

