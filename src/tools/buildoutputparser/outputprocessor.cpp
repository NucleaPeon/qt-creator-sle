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

#include "outputprocessor.h"

#include <projectexplorer/clangparser.h>
#include <projectexplorer/gccparser.h>
#include <projectexplorer/gnumakeparser.h>
#include <projectexplorer/osparser.h>
#include <qmakeprojectmanager/qmakeparser.h>
#include <qtsupport/qtparser.h>
#include <utils/fileutils.h>

#ifdef HAS_MSVC_PARSER
#include <projectexplorer/msvcparser.h>
#endif

#include <QIODevice>
#include <QTextStream>

#include <stdio.h>

CompilerOutputProcessor::CompilerOutputProcessor(CompilerType compilerType, QIODevice &source)
    : m_compilerType(compilerType)
    , m_source(source)
    , m_ostream(new QTextStream(stdout, QIODevice::WriteOnly))
{
}

CompilerOutputProcessor::~CompilerOutputProcessor()
{
    delete m_ostream;
}

void CompilerOutputProcessor::start()
{
    ProjectExplorer::OsParser parser;
    parser.appendOutputParser(new QmakeProjectManager::QMakeParser);
    parser.appendOutputParser(new ProjectExplorer::GnuMakeParser);
    parser.appendOutputParser(new QtSupport::QtParser);
    switch (m_compilerType) {
    case CompilerTypeGcc:
        parser.appendOutputParser(new ProjectExplorer::GccParser);
        break;
    case CompilerTypeClang:
        parser.appendOutputParser(new ProjectExplorer::ClangParser);
        break;
#ifdef HAS_MSVC_PARSER
    case CompilerTypeMsvc:
        parser.appendOutputParser(new ProjectExplorer::MsvcParser);
        break;
#endif
    }

    connect(&parser, SIGNAL(addTask(ProjectExplorer::Task)),
            SLOT(handleTask(ProjectExplorer::Task)));
    while (!m_source.atEnd())
        parser.stdError(QString::fromLocal8Bit(m_source.readLine().trimmed()));
    qApp->quit();
}

void CompilerOutputProcessor::handleTask(const ProjectExplorer::Task &task)
{
    const QString &fileName = task.file.toString();
    if (!fileName.isEmpty()) {
        *m_ostream << fileName;
        if (task.line != -1)
            *m_ostream << ':' << task.line;
        *m_ostream << ": ";
    }
    *m_ostream << task.description << endl;
}
