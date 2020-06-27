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

#include "objectivechighlighterfactory.h"
#include "../objectiveceditorconstants.h"
#include "objectivechighlighter.h"

using namespace ObjectiveCEditor::Internal;

ObjectiveCHighlighterFactory::ObjectiveCHighlighterFactory()
{
    setId(Constants::C_OBJECTIVEC_EDITOR_ID);
    addMimeType(QLatin1String(Constants::OBJC_SOURCE_MIMETYPE));
}

TextEditor::SyntaxHighlighter *ObjectiveCHighlighterFactory::createHighlighter() const
{
    return new ObjectiveCHighlighter;
}
