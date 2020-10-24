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

#include "objectiveceditorplugin.h"
#include "objectiveceditorconstants.h"
#include "wizard/objectivecfilewizard.h"
#include "wizard/objectivecclasswizard.h"
#include "objectiveceditorwidget.h"
#include "objectiveceditorfactory.h"
#include "tools/objectivechighlighterfactory.h"

#include <coreplugin/icore.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/mimedatabase.h>
#include <coreplugin/fileiconprovider.h>
#include <coreplugin/id.h>
#include <coreplugin/editormanager/editormanager.h>
#include <extensionsystem/pluginmanager.h>
#include <texteditor/texteditorconstants.h>

#include <QtPlugin>
#include <QCoreApplication>

using namespace ObjectiveCEditor::Constants;

/*******************************************************************************
 * List of Objective C Keywords
 ******************************************************************************/
static const char *const LIST_OF_OBJECTIVE_C_KEYWORDS[] = {
    "break",
    "case",
    "continue",
    "default",
    "do",
    "else",
    "enum",
    "extern",
    "for",
    "goto",
    "if",
    "return",
    "sizeof",
    "struct",
    "switch",
    "typedef",
    "union",
    "while",
    "@class",
    "@defs",
    "@encode",
    "@end",
    "@implementation",
    "@interface",
    "@private",
    "@protected",
    "@protocol",
    "@public",
    "@selector",
    "self",
    "super"
};

/*******************************************************************************
 * List of Objective C++ Keywords
 ******************************************************************************/
static const char *const LIST_OF_OBJECTIVE_CPP_KEYWORDS[] = {
    "asm",
    "catch",
    "class",
    "const_cast",
    "delete",
    "dynamic_cast",
    "explicit",
    "export",
    "false",
    "friend",
    "inline",
    "namespace",
    "new",
    "operator",
    "private",
    "protected",
    "public",
    "qobject_cast",
    "reinterpret_cast",
    "static_cast",
    "template",
    "this",
    "throw",
    "true",
    "try",
    "typeid",
    "type_info",
    "typename",
    "using",
    "virtual",
    "and",
    "and_eq",
    "bad_cast",
    "bad_typeid",
    "bitand",
    "bitor",
    "compl",
    "not",
    "not_eq",
    "or",
    "or_eq",
    "xor",
    "xor_eq"
};

/*******************************************************************************
 * List of QT Extensions that can show up in objective c++ code
 ******************************************************************************/
static const char *const LIST_OF_QT_OBJCPP_EXTENSIONS[] = {
    "K_DCOP",
    "SLOT",
    "SIGNAL",
    "Q_CLASSINFO",
    "Q_ENUMS",
    "Q_EXPORT",
    "Q_OBJECT",
    "Q_OVERRIDE",
    "Q_PROPERTY",
    "Q_SETS",
    "Q_SIGNALS",
    "Q_SLOTS",
    "Q_FOREACH",
    "Q_DECLARE_FLAGS",
    "Q_INIT_RESOURCE",
    "Q_CLEANUP_RESOURCE",
    "Q_GLOBAL_STATIC",
    "Q_GLOBAL_STATIC_WITH_ARGS",
    "Q_DECLARE_INTERFACE",
    "Q_DECLARE_TYPEINFO",
    "Q_DECLARE_SHARED",
    "Q_DECLARE_FLAGS",
    "Q_DECLARE_OPERATORS_FOR_FLAGS",
    "Q_FOREVER",
    "Q_DECLARE_PRIVATE",
    "Q_DECLARE_PUBLIC",
    "Q_D",
    "Q_Q",
    "Q_DISABLE_COPY",
    "Q_INTERFACES",
    "Q_FLAGS",
    "Q_SCRIPTABLE",
    "Q_INVOKABLE",
    "Q_GADGET",
    "Q_ARG",
    "Q_RETURN_ARG",
    "Q_ASSERT",
    "Q_ASSERT_X",
    "TRUE",
    "FALSE",
    "connect",
    "disconnect",
    "emit",
    "signals",
    "slots",
    "foreach",
    "forever"
};

/*******************************************************************************
 * List of objective c/c++ built-in data types
 ******************************************************************************/
static const char *const LIST_OF_OBJECTIVE_C_TYPES[] = {
    "auto",
    "char",
    "const",
    "double",
    "float",
    "int",
    "long",
    "register",
    "short",
    "signed",
    "static",
    "unsigned",
    "void",
    "volatile"
};

// This (objcpp) implies the inclusion of LIST_OF_OBJECTIVE_C_TYPES as well
static const char *const LIST_OF_OBJECTIVE_CPP_TYPES[] = {
    "bool",
    "mutable",
    "uchar",
    "uint",
    "int8_t",
    "int16_t",
    "int32_t",
    "int64_t",
    "uint8_t",
    "uint16_t",
    "uint32_t",
    "uint64_t",
    "wchar_t",
};

namespace ObjectiveCEditor {
namespace Internal {

ObjectiveCEditorPlugin *ObjectiveCEditorPlugin::m_instance = 0;

/// Copies identifiers from array to QSet
static void copyIdentifiers(const char * const words[], size_t bytesCount, QSet<QString> &result)
{
    const size_t count = bytesCount / sizeof(const char * const);
    for (size_t i = 0; i < count; ++i)
        result.insert(QLatin1String(words[i]));
}

ObjectiveCEditorPlugin::ObjectiveCEditorPlugin()
    : m_factory(0)
{
    m_instance = this;
     copyIdentifiers(LIST_OF_OBJECTIVE_C_KEYWORDS, sizeof(LIST_OF_OBJECTIVE_C_KEYWORDS), m_keywords);
//     copyIdentifiers(LIST_OF_OBJECTIVE_CPP_KEYWORDS, sizeof(LIST_OF_OBJECTIVE_CPP_KEYWORDS), m_keywords);
     copyIdentifiers(LIST_OF_OBJECTIVE_C_TYPES, sizeof(LIST_OF_OBJECTIVE_C_TYPES), m_magics);
//     copyIdentifiers(LIST_OF_PYTHON_BUILTINS, sizeof(LIST_OF_PYTHON_BUILTINS), m_builtins);
}

ObjectiveCEditorPlugin::~ObjectiveCEditorPlugin()
{
    removeObject(m_factory);
    m_instance = 0;
}

bool ObjectiveCEditorPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments)

    if (!Core::MimeDatabase::addMimeTypes(QLatin1String(RC_OBJC_MIME_XML), errorMessage))
        return false;

    m_factory = new EditorFactory(this);
    addObject(m_factory);

    // Initialize editor actions handler
    // Add MIME overlay icons (these icons displayed at Project dock panel)
    const QIcon icon = QIcon::fromTheme(QLatin1String(C_OBJC_MIME_ICON));
    if (!icon.isNull())
        Core::FileIconProvider::registerIconOverlayForMimeType(icon, OBJC_SOURCE_MIMETYPE);

    // Add Python files and classes creation dialogs
    addAutoReleasedObject(new FileWizard);
    addAutoReleasedObject(new ClassWizard);
    addAutoReleasedObject(new Internal::ObjectiveCHighlighterFactory);

    return true;
}

void ObjectiveCEditorPlugin::extensionsInitialized()
{
}

QSet<QString> ObjectiveCEditorPlugin::keywords()
{
    return instance()->m_keywords;
}

QSet<QString> ObjectiveCEditorPlugin::magics()
{
    return instance()->m_magics;
}

QSet<QString> ObjectiveCEditorPlugin::builtins()
{
    return instance()->m_builtins;
}

} // namespace Internal
} // namespace ObjectiveCEditor

Q_EXPORT_PLUGIN(ObjectiveCEditor::Internal::ObjectiveCEditorPlugin)
