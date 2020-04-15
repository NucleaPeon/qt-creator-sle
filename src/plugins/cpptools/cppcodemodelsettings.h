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

#ifndef CPPTOOLS_INTERNAL_CPPCODEMODELSETTINGS_H
#define CPPTOOLS_INTERNAL_CPPCODEMODELSETTINGS_H

#include <QHash>
#include <QList>
#include <QSettings>
#include <QString>

namespace CppTools {

class ModelManagerSupport;

namespace Internal {

class CppCodeModelSettings
{
public:
    enum PCHUsage {
        PchUse_None = 1,
        PchUse_BuildSystem = 2
    };

public:
    CppCodeModelSettings(): m_pchUsage(PchUse_None) {}

    void fromSettings(QSettings *s);
    void toSettings(QSettings *s);

    void setModelManagerSupports(const QList<ModelManagerSupport *> &supporters);

    QString modelManagerSupportId(const QString &mimeType) const;
    void setModelManagerSupportId(const QString &mimeType, const QString &supporter);

    const QHash<QString, QString> &availableModelManagerSupportersByName() const
    { return m_availableModelManagerSupportersByName; }

    QString defaultId() const
    { return m_defaultId; }

    void setDefaultId(const QString &defaultId)
    { m_defaultId = defaultId; }

    PCHUsage pchUsage() const { return m_pchUsage; }
    void setPCHUsage(PCHUsage pchUsage) { m_pchUsage = pchUsage; }

private:
    void setIdForMimeType(const QVariant &var, const QString &mimeType);

private:
    QHash<QString, QString> m_modelManagerSupportByMimeType;
    QHash<QString, QString> m_availableModelManagerSupportersByName;
    QString m_defaultId;
    PCHUsage m_pchUsage;
};

} // namespace Internal
} // namespace CppTools

#endif // CPPTOOLS_INTERNAL_CPPCODEMODELSETTINGS_H
