/****************************************************************************
**
** Copyright (C) 2014 Andre Hartmann.
** Contact: aha_1980@gmx.de
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

#ifndef CUSTOMPARSER_H
#define CUSTOMPARSER_H

#include "ioutputparser.h"

#include <projectexplorer/task.h>

#include <QRegExp>

namespace ProjectExplorer {

class CustomParserSettings
{
public:
    CustomParserSettings();

    bool operator ==(const CustomParserSettings &other) const;
    bool operator !=(const CustomParserSettings &other) const { return !operator==(other); }

    QString errorPattern;
    int fileNameCap;
    int lineNumberCap;
    int messageCap;
};

class CustomParser : public ProjectExplorer::IOutputParser
{
public:
    enum CustomParserChannels {
        ParseNoChannel = 0,
        ParseStdErrChannel = 1,
        ParseStdOutChannel = 2,
        ParseBothChannels = 3
    };

    CustomParser(const CustomParserSettings &settings = CustomParserSettings());
    ~CustomParser();
    void stdError(const QString &line);
    void stdOutput(const QString &line);

    void setSettings(const CustomParserSettings &settings);

    void setErrorPattern(const QString &errorPattern);
    QString errorPattern() const;
    int fileNameCap() const;
    void setFileNameCap(int fileNameCap);
    int lineNumberCap() const;
    void setLineNumberCap(int lineNumberCap);
    int messageCap() const;
    void setMessageCap(int messageCap);

private:
    bool parseLine(const QString &rawLine);

    QRegExp m_errorRegExp;
    int m_fileNameCap;
    int m_lineNumberCap;
    int m_messageCap;

    CustomParserChannels m_parserChannels;
};

} // namespace ProjectExplorer

#endif // CUSTOMPARSER_H
