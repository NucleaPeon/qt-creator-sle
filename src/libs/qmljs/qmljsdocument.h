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
#ifndef QMLJSDOCUMENT_H
#define QMLJSDOCUMENT_H

#include <QList>
#include <QSharedPointer>
#include <QString>

#include <languageutils/fakemetaobject.h>

#include "parser/qmldirparser_p.h"
#include "parser/qmljsengine_p.h"
#include "qmljs_global.h"
#include "qmljsconstants.h"
#include "qmljsimportdependencies.h"

namespace QmlJS {

class Bind;
class Snapshot;
class ImportDependencies;

class QMLJS_EXPORT Document
{
public:
    typedef QSharedPointer<const Document> Ptr;
    typedef QSharedPointer<Document> MutablePtr;

    static bool isQmlLikeLanguage(Language::Enum languge);
    static bool isFullySupportedLanguage(Language::Enum language);
    static bool isQmlLikeOrJsLanguage(Language::Enum language);
    static QList<Language::Enum> companionLanguages(Language::Enum language);
protected:
    Document(const QString &fileName, Language::Enum language);

public:
    ~Document();

    static MutablePtr create(const QString &fileName, Language::Enum language);

    Document::Ptr ptr() const;

    bool isQmlDocument() const;
    Language::Enum language() const;
    void setLanguage(Language::Enum l);

    QString importId() const;
    QByteArray fingerprint() const;
    AST::UiProgram *qmlProgram() const;
    AST::Program *jsProgram() const;
    AST::ExpressionNode *expression() const;
    AST::Node *ast() const;

    const QmlJS::Engine *engine() const;

    QList<DiagnosticMessage> diagnosticMessages() const;

    QString source() const;
    void setSource(const QString &source);

    bool parse();
    bool parseQml();
    bool parseJavaScript();
    bool parseExpression();

    bool isParsedCorrectly() const
    { return _parsedCorrectly; }

    Bind *bind() const;

    int editorRevision() const;
    void setEditorRevision(int revision);

    QString fileName() const;
    QString path() const;
    QString componentName() const;

private:
    bool parse_helper(int kind);

private:
    QmlJS::Engine *_engine;
    AST::Node *_ast;
    Bind *_bind;
    QList<QmlJS::DiagnosticMessage> _diagnosticMessages;
    QString _fileName;
    QString _path;
    QString _componentName;
    QString _source;
    QWeakPointer<Document> _ptr;
    QByteArray _fingerprint;
    int _editorRevision;
    Language::Enum _language;
    bool _parsedCorrectly;

    // for documentFromSource
    friend class Snapshot;
};

class QMLJS_EXPORT ModuleApiInfo
{
public:
    QString uri;
    LanguageUtils::ComponentVersion version;
    QString cppName;

    void addToHash(QCryptographicHash &hash) const;
};

class QMLJS_EXPORT LibraryInfo
{
public:
    enum PluginTypeInfoStatus {
        NoTypeInfo,
        DumpDone,
        DumpError,
        TypeInfoFileDone,
        TypeInfoFileError
    };

    enum Status {
        NotScanned,
        NotFound,
        Found
    };

private:
    Status _status;
    QList<QmlDirParser::Component> _components;
    QList<QmlDirParser::Plugin> _plugins;
    QList<QmlDirParser::TypeInfo> _typeinfos;
    typedef QList<LanguageUtils::FakeMetaObject::ConstPtr> FakeMetaObjectList;
    FakeMetaObjectList _metaObjects;
    QList<ModuleApiInfo> _moduleApis;
    QByteArray _fingerprint;

    PluginTypeInfoStatus _dumpStatus;
    QString _dumpError;

public:
    explicit LibraryInfo(Status status = NotScanned);
    explicit LibraryInfo(const QmlDirParser &parser, const QByteArray &fingerprint = QByteArray());
    ~LibraryInfo();

    QByteArray calculateFingerprint() const;
    void updateFingerprint();
    QByteArray fingerprint() const
    { return _fingerprint; }

    QList<QmlDirParser::Component> components() const
    { return _components; }

    QList<QmlDirParser::Plugin> plugins() const
    { return _plugins; }

    QList<QmlDirParser::TypeInfo> typeInfos() const
    { return _typeinfos; }

    FakeMetaObjectList metaObjects() const
    { return _metaObjects; }

    void setMetaObjects(const FakeMetaObjectList &objects)
    { _metaObjects = objects; }

    QList<ModuleApiInfo> moduleApis() const
    { return _moduleApis; }

    void setModuleApis(const QList<ModuleApiInfo> &apis)
    { _moduleApis = apis; }

    bool isValid() const
    { return _status == Found; }

    bool wasScanned() const
    { return _status != NotScanned; }

    bool wasFound() const
    { return _status != NotFound; }

    PluginTypeInfoStatus pluginTypeInfoStatus() const
    { return _dumpStatus; }

    QString pluginTypeInfoError() const
    { return _dumpError; }

    void setPluginTypeInfoStatus(PluginTypeInfoStatus dumped, const QString &error = QString())
    { _dumpStatus = dumped; _dumpError = error; }
};

class QMLJS_EXPORT Snapshot
{
    typedef QHash<QString, Document::Ptr> _Base;
    QHash<QString, Document::Ptr> _documents;
    QHash<QString, QList<Document::Ptr> > _documentsByPath;
    QHash<QString, LibraryInfo> _libraries;
    ImportDependencies _dependencies;

public:
    Snapshot();
    Snapshot(const Snapshot &o);
    ~Snapshot();

    typedef _Base::iterator iterator;
    typedef _Base::const_iterator const_iterator;

    const_iterator begin() const { return _documents.begin(); }
    const_iterator end() const { return _documents.end(); }

    void insert(const Document::Ptr &document, bool allowInvalid = false);
    void insertLibraryInfo(const QString &path, const LibraryInfo &info);
    void remove(const QString &fileName);

    const ImportDependencies *importDependencies() const;
    ImportDependencies *importDependencies();

    Document::Ptr document(const QString &fileName) const;
    QList<Document::Ptr> documentsInDirectory(const QString &path) const;
    LibraryInfo libraryInfo(const QString &path) const;

    Document::MutablePtr documentFromSource(const QString &code,
                                     const QString &fileName,
                                     Language::Enum language) const;
};

} // namespace QmlJS

#endif // QMLJSDOCUMENT_H
