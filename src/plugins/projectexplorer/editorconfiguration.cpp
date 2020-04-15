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

#include "editorconfiguration.h"
#include "session.h"
#include "projectexplorer.h"
#include "project.h"

#include <coreplugin/id.h>
#include <coreplugin/icore.h>
#include <texteditor/basetexteditor.h>
#include <texteditor/texteditorsettings.h>
#include <texteditor/simplecodestylepreferences.h>
#include <texteditor/typingsettings.h>
#include <texteditor/storagesettings.h>
#include <texteditor/behaviorsettings.h>
#include <texteditor/extraencodingsettings.h>
#include <texteditor/tabsettings.h>
#include <texteditor/marginsettings.h>
#include <texteditor/icodestylepreferencesfactory.h>

#include <QLatin1String>
#include <QByteArray>
#include <QTextCodec>
#include <QDebug>

static const QLatin1String kPrefix("EditorConfiguration.");
static const QLatin1String kUseGlobal("EditorConfiguration.UseGlobal");
static const QLatin1String kCodec("EditorConfiguration.Codec");
static const QLatin1String kCodeStylePrefix("EditorConfiguration.CodeStyle.");
static const QLatin1String kCodeStyleCount("EditorConfiguration.CodeStyle.Count");

using namespace TextEditor;

namespace ProjectExplorer {

struct EditorConfigurationPrivate
{
    EditorConfigurationPrivate()
        : m_useGlobal(true)
        , m_typingSettings(TextEditorSettings::typingSettings())
        , m_storageSettings(TextEditorSettings::storageSettings())
        , m_behaviorSettings(TextEditorSettings::behaviorSettings())
        , m_extraEncodingSettings(TextEditorSettings::extraEncodingSettings())
        , m_textCodec(Core::EditorManager::defaultTextCodec())
    {
    }

    bool m_useGlobal;
    ICodeStylePreferences *m_defaultCodeStyle;
    TypingSettings m_typingSettings;
    StorageSettings m_storageSettings;
    BehaviorSettings m_behaviorSettings;
    ExtraEncodingSettings m_extraEncodingSettings;
    MarginSettings m_marginSettings;
    QTextCodec *m_textCodec;

    QMap<Core::Id, ICodeStylePreferences *> m_languageCodeStylePreferences;
};

EditorConfiguration::EditorConfiguration() : d(new EditorConfigurationPrivate)
{
    const QMap<Core::Id, ICodeStylePreferences *> languageCodeStylePreferences = TextEditorSettings::codeStyles();
    QMapIterator<Core::Id, ICodeStylePreferences *> itCodeStyle(languageCodeStylePreferences);
    while (itCodeStyle.hasNext()) {
        itCodeStyle.next();
        Core::Id languageId = itCodeStyle.key();
        // global prefs for language
        ICodeStylePreferences *originalPreferences = itCodeStyle.value();
        ICodeStylePreferencesFactory *factory = TextEditorSettings::codeStyleFactory(languageId);
        // clone of global prefs for language - it will became project prefs for language
        ICodeStylePreferences *preferences = factory->createCodeStyle();
        // project prefs can point to the global language pool, which contains also the global language prefs
        preferences->setDelegatingPool(TextEditorSettings::codeStylePool(languageId));
        preferences->setId(languageId.name() + "Project");
        preferences->setDisplayName(tr("Project %1", "Settings, %1 is a language (C++ or QML)").arg(factory->displayName()));
        // project prefs by default point to global prefs (which in turn can delegate to anything else or not)
        preferences->setCurrentDelegate(originalPreferences);
        d->m_languageCodeStylePreferences.insert(languageId, preferences);
    }

    // clone of global prefs (not language specific), for project scope
    d->m_defaultCodeStyle = new SimpleCodeStylePreferences(this);
    d->m_defaultCodeStyle->setDelegatingPool(TextEditorSettings::codeStylePool());
    d->m_defaultCodeStyle->setDisplayName(tr("Project", "Settings"));
    d->m_defaultCodeStyle->setId("Project");
    // if setCurrentDelegate is 0 values are read from *this prefs
    d->m_defaultCodeStyle->setCurrentDelegate(d->m_useGlobal
                    ? TextEditorSettings::codeStyle() : 0);

    connect(SessionManager::instance(), SIGNAL(aboutToRemoveProject(ProjectExplorer::Project*)),
            this, SLOT(slotAboutToRemoveProject(ProjectExplorer::Project*)));
}

EditorConfiguration::~EditorConfiguration()
{
    qDeleteAll(d->m_languageCodeStylePreferences);
    delete d;
}

bool EditorConfiguration::useGlobalSettings() const
{
    return d->m_useGlobal;
}

void EditorConfiguration::cloneGlobalSettings()
{
    d->m_defaultCodeStyle->setTabSettings(TextEditorSettings::codeStyle()->tabSettings());
    setTypingSettings(TextEditorSettings::typingSettings());
    setStorageSettings(TextEditorSettings::storageSettings());
    setBehaviorSettings(TextEditorSettings::behaviorSettings());
    setExtraEncodingSettings(TextEditorSettings::extraEncodingSettings());
    setMarginSettings(TextEditorSettings::marginSettings());
    d->m_textCodec = Core::EditorManager::defaultTextCodec();
}

QTextCodec *EditorConfiguration::textCodec() const
{
    return d->m_textCodec;
}

const TypingSettings &EditorConfiguration::typingSettings() const
{
    return d->m_typingSettings;
}

const StorageSettings &EditorConfiguration::storageSettings() const
{
    return d->m_storageSettings;
}

const BehaviorSettings &EditorConfiguration::behaviorSettings() const
{
    return d->m_behaviorSettings;
}

const ExtraEncodingSettings &EditorConfiguration::extraEncodingSettings() const
{
    return d->m_extraEncodingSettings;
}

const MarginSettings &EditorConfiguration::marginSettings() const
{
    return d->m_marginSettings;
}

ICodeStylePreferences *EditorConfiguration::codeStyle() const
{
    return d->m_defaultCodeStyle;
}

ICodeStylePreferences *EditorConfiguration::codeStyle(Core::Id languageId) const
{
    return d->m_languageCodeStylePreferences.value(languageId, codeStyle());
}

QMap<Core::Id, ICodeStylePreferences *> EditorConfiguration::codeStyles() const
{
    return d->m_languageCodeStylePreferences;
}

QVariantMap EditorConfiguration::toMap() const
{
    QVariantMap map;
    map.insert(kUseGlobal, d->m_useGlobal);
    map.insert(kCodec, d->m_textCodec->name());

    map.insert(kCodeStyleCount, d->m_languageCodeStylePreferences.count());
    QMapIterator<Core::Id, ICodeStylePreferences *> itCodeStyle(d->m_languageCodeStylePreferences);
    int i = 0;
    while (itCodeStyle.hasNext()) {
        itCodeStyle.next();
        QVariantMap settingsIdMap;
        settingsIdMap.insert(QLatin1String("language"), itCodeStyle.key().toSetting());
        QVariantMap value;
        itCodeStyle.value()->toMap(QString(), &value);
        settingsIdMap.insert(QLatin1String("value"), value);
        map.insert(kCodeStylePrefix + QString::number(i), settingsIdMap);
        i++;
    }

    d->m_defaultCodeStyle->tabSettings().toMap(kPrefix, &map);
    d->m_typingSettings.toMap(kPrefix, &map);
    d->m_storageSettings.toMap(kPrefix, &map);
    d->m_behaviorSettings.toMap(kPrefix, &map);
    d->m_extraEncodingSettings.toMap(kPrefix, &map);
    d->m_marginSettings.toMap(kPrefix, &map);

    return map;
}

void EditorConfiguration::fromMap(const QVariantMap &map)
{
    d->m_useGlobal = map.value(kUseGlobal, d->m_useGlobal).toBool();

    const QByteArray &codecName = map.value(kCodec, d->m_textCodec->name()).toByteArray();
    d->m_textCodec = QTextCodec::codecForName(codecName);
    if (!d->m_textCodec)
        d->m_textCodec = Core::EditorManager::defaultTextCodec();

    const int codeStyleCount = map.value(kCodeStyleCount, 0).toInt();
    for (int i = 0; i < codeStyleCount; ++i) {
        QVariantMap settingsIdMap = map.value(kCodeStylePrefix + QString::number(i)).toMap();
        if (settingsIdMap.isEmpty()) {
            qWarning() << "No data for code style settings list" << i << "found!";
            continue;
        }
        Core::Id languageId = Core::Id::fromSetting(settingsIdMap.value(QLatin1String("language")));
        QVariantMap value = settingsIdMap.value(QLatin1String("value")).toMap();
        ICodeStylePreferences *preferences = d->m_languageCodeStylePreferences.value(languageId);
        if (preferences)
             preferences->fromMap(QString(), value);
    }

    d->m_defaultCodeStyle->fromMap(kPrefix, map);
    d->m_typingSettings.fromMap(kPrefix, map);
    d->m_storageSettings.fromMap(kPrefix, map);
    d->m_behaviorSettings.fromMap(kPrefix, map);
    d->m_extraEncodingSettings.fromMap(kPrefix, map);
    d->m_marginSettings.fromMap(kPrefix, map);
}

void EditorConfiguration::configureEditor(ITextEditor *textEditor) const
{
    BaseTextEditorWidget *baseTextEditor = qobject_cast<BaseTextEditorWidget *>(textEditor->widget());
    if (baseTextEditor)
        baseTextEditor->setCodeStyle(codeStyle(baseTextEditor->languageSettingsId()));
    if (!d->m_useGlobal) {
        textEditor->textDocument()->setCodec(d->m_textCodec);
        if (baseTextEditor)
            switchSettings(baseTextEditor);
    }
}

void EditorConfiguration::deconfigureEditor(ITextEditor *textEditor) const
{
    BaseTextEditorWidget *baseTextEditor = qobject_cast<BaseTextEditorWidget *>(textEditor->widget());
    if (baseTextEditor)
        baseTextEditor->setCodeStyle(TextEditorSettings::codeStyle(baseTextEditor->languageSettingsId()));

    // TODO: what about text codec and switching settings?
}

void EditorConfiguration::setUseGlobalSettings(bool use)
{
    d->m_useGlobal = use;
    d->m_defaultCodeStyle->setCurrentDelegate(d->m_useGlobal
                    ? TextEditorSettings::codeStyle() : 0);
    QList<Core::IEditor *> opened = Core::EditorManager::documentModel()->editorsForDocuments(
                Core::EditorManager::documentModel()->openedDocuments());
    foreach (Core::IEditor *editor, opened) {
        if (BaseTextEditorWidget *baseTextEditor = qobject_cast<BaseTextEditorWidget *>(editor->widget())) {
            Project *project = SessionManager::projectForFile(editor->document()->filePath());
            if (project && project->editorConfiguration() == this)
                switchSettings(baseTextEditor);
        }
    }
}

static void switchSettings_helper(const QObject *newSender, const QObject *oldSender,
                                                BaseTextEditorWidget *baseTextEditor)
{
    QObject::disconnect(oldSender, SIGNAL(marginSettingsChanged(TextEditor::MarginSettings)),
                        baseTextEditor, SLOT(setMarginSettings(TextEditor::MarginSettings)));
    QObject::disconnect(oldSender, SIGNAL(typingSettingsChanged(TextEditor::TypingSettings)),
               baseTextEditor, SLOT(setTypingSettings(TextEditor::TypingSettings)));
    QObject::disconnect(oldSender, SIGNAL(storageSettingsChanged(TextEditor::StorageSettings)),
               baseTextEditor, SLOT(setStorageSettings(TextEditor::StorageSettings)));
    QObject::disconnect(oldSender, SIGNAL(behaviorSettingsChanged(TextEditor::BehaviorSettings)),
               baseTextEditor, SLOT(setBehaviorSettings(TextEditor::BehaviorSettings)));
    QObject::disconnect(oldSender, SIGNAL(extraEncodingSettingsChanged(TextEditor::ExtraEncodingSettings)),
               baseTextEditor, SLOT(setExtraEncodingSettings(TextEditor::ExtraEncodingSettings)));

    QObject::connect(newSender, SIGNAL(marginSettingsChanged(TextEditor::MarginSettings)),
                     baseTextEditor, SLOT(setMarginSettings(TextEditor::MarginSettings)));
    QObject::connect(newSender, SIGNAL(typingSettingsChanged(TextEditor::TypingSettings)),
            baseTextEditor, SLOT(setTypingSettings(TextEditor::TypingSettings)));
    QObject::connect(newSender, SIGNAL(storageSettingsChanged(TextEditor::StorageSettings)),
            baseTextEditor, SLOT(setStorageSettings(TextEditor::StorageSettings)));
    QObject::connect(newSender, SIGNAL(behaviorSettingsChanged(TextEditor::BehaviorSettings)),
            baseTextEditor, SLOT(setBehaviorSettings(TextEditor::BehaviorSettings)));
    QObject::connect(newSender, SIGNAL(extraEncodingSettingsChanged(TextEditor::ExtraEncodingSettings)),
            baseTextEditor, SLOT(setExtraEncodingSettings(TextEditor::ExtraEncodingSettings)));
}

void EditorConfiguration::switchSettings(BaseTextEditorWidget *baseTextEditor) const
{
    if (d->m_useGlobal) {
        baseTextEditor->setMarginSettings(TextEditorSettings::marginSettings());
        baseTextEditor->setTypingSettings(TextEditorSettings::typingSettings());
        baseTextEditor->setStorageSettings(TextEditorSettings::storageSettings());
        baseTextEditor->setBehaviorSettings(TextEditorSettings::behaviorSettings());
        baseTextEditor->setExtraEncodingSettings(TextEditorSettings::extraEncodingSettings());
        switchSettings_helper(TextEditorSettings::instance(), this, baseTextEditor);
    } else {
        baseTextEditor->setMarginSettings(marginSettings());
        baseTextEditor->setTypingSettings(typingSettings());
        baseTextEditor->setStorageSettings(storageSettings());
        baseTextEditor->setBehaviorSettings(behaviorSettings());
        baseTextEditor->setExtraEncodingSettings(extraEncodingSettings());
        switchSettings_helper(this, TextEditorSettings::instance(), baseTextEditor);
    }
}

void EditorConfiguration::setTypingSettings(const TextEditor::TypingSettings &settings)
{
    d->m_typingSettings = settings;
    emit typingSettingsChanged(d->m_typingSettings);
}

void EditorConfiguration::setStorageSettings(const TextEditor::StorageSettings &settings)
{
    d->m_storageSettings = settings;
    emit storageSettingsChanged(d->m_storageSettings);
}

void EditorConfiguration::setBehaviorSettings(const TextEditor::BehaviorSettings &settings)
{
    d->m_behaviorSettings = settings;
    emit behaviorSettingsChanged(d->m_behaviorSettings);
}

void EditorConfiguration::setExtraEncodingSettings(const TextEditor::ExtraEncodingSettings &settings)
{
    d->m_extraEncodingSettings = settings;
    emit extraEncodingSettingsChanged(d->m_extraEncodingSettings);
}

void EditorConfiguration::setMarginSettings(const MarginSettings &settings)
{
    if (d->m_marginSettings != settings) {
        d->m_marginSettings = settings;
        emit marginSettingsChanged(d->m_marginSettings);
    }
}

void EditorConfiguration::setTextCodec(QTextCodec *textCodec)
{
    d->m_textCodec = textCodec;
}

void EditorConfiguration::setShowWrapColumn(bool onoff)
{
    if (d->m_marginSettings.m_showMargin != onoff) {
        d->m_marginSettings.m_showMargin = onoff;
        emit marginSettingsChanged(d->m_marginSettings);
    }
}

void EditorConfiguration::setWrapColumn(int column)
{
    if (d->m_marginSettings.m_marginColumn != column) {
        d->m_marginSettings.m_marginColumn = column;
        emit marginSettingsChanged(d->m_marginSettings);
    }
}

void EditorConfiguration::slotAboutToRemoveProject(ProjectExplorer::Project *project)
{
    if (project->editorConfiguration() != this)
        return;

    Core::DocumentModel *model = Core::EditorManager::documentModel();
    QList<Core::IEditor *> editors = model->editorsForDocuments(model->openedDocuments());
    foreach (Core::IEditor *editor, editors) {
        if (TextEditor::ITextEditor *textEditor = qobject_cast<TextEditor::ITextEditor*>(editor)) {
            Core::IDocument *document = editor->document();
            if (document) {
                Project *editorProject = SessionManager::projectForFile(document->filePath());
                if (project == editorProject)
                    deconfigureEditor(textEditor);
            }
        }
    }
}

TabSettings actualTabSettings(const QString &fileName,
                              const BaseTextDocument *baseTextdocument)
{
    if (baseTextdocument)
        return baseTextdocument->tabSettings();
    if (Project *project = SessionManager::projectForFile(fileName))
        return project->editorConfiguration()->codeStyle()->tabSettings();
    return TextEditorSettings::codeStyle()->tabSettings();
}

} // ProjectExplorer
