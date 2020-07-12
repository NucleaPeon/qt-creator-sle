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


#ifndef OBJECTIVECEDITOR_PLUGIN_H
#define OBJECTIVECEDITOR_PLUGIN_H

#include <extensionsystem/iplugin.h>
#include <QSet>

namespace ObjectiveCEditor {
namespace Internal {

class EditorFactory;
class EditorWidget;

/**
  \class __Editor::Plugin implements ExtensionSystem::IPlugin
  Singleton object - __Editor plugin
  */
class ObjectiveCEditorPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "ObjectiveCEditor.json")

public:
    ObjectiveCEditorPlugin();
    virtual ~ObjectiveCEditorPlugin();

    virtual bool initialize(const QStringList &arguments, QString *errorMessage);
    virtual void extensionsInitialized();
    static ObjectiveCEditorPlugin *instance() { return m_instance; }

    static QSet<QString> keywords();
    static QSet<QString> magics();
    static QSet<QString> builtins();

private:
    static ObjectiveCEditorPlugin *m_instance;
    EditorFactory *m_factory;
    QSet<QString> m_keywords;
    QSet<QString> m_magics;
    QSet<QString> m_builtins;
};

} // namespace Internal
} // namespace ObjectiveCEditor

#endif // OBJECTIVECEDITOR_PLUGIN_H
