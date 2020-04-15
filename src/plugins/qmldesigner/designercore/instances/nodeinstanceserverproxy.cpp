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

#include "nodeinstanceserverproxy.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QProcess>
#include <QCoreApplication>
#include <QUuid>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QTextStream>
#include <QMessageBox>

#include "createinstancescommand.h"
#include "createscenecommand.h"
#include "changevaluescommand.h"
#include "changebindingscommand.h"
#include "changeauxiliarycommand.h"
#include "changefileurlcommand.h"
#include "removeinstancescommand.h"
#include "clearscenecommand.h"
#include "removepropertiescommand.h"
#include "reparentinstancescommand.h"
#include "changeidscommand.h"
#include "changestatecommand.h"
#include "completecomponentcommand.h"
#include "changenodesourcecommand.h"

#include "informationchangedcommand.h"
#include "pixmapchangedcommand.h"
#include "valueschangedcommand.h"
#include "childrenchangedcommand.h"
#include "statepreviewimagechangedcommand.h"
#include "componentcompletedcommand.h"
#include "tokencommand.h"
#include "removesharedmemorycommand.h"
#include "endpuppetcommand.h"
#include "synchronizecommand.h"
#include "debugoutputcommand.h"

#include "nodeinstanceview.h"

#include "puppetdialog.h"

#include "import.h"

#include "qmldesignerplugin.h"

#include <coreplugin/icore.h>
#include <utils/hostosinfo.h>

#include <QMessageBox>

namespace {
#ifdef Q_OS_MAC
#  define SHARE_PATH "/../Resources"
#else
#  define SHARE_PATH "/../share/qtcreator"
#endif

static QString applicationDirPath()
{
    return QCoreApplication::applicationDirPath();
}

static inline QString sharedDirPath()
{
    QString appPath = applicationDirPath();

    return QFileInfo(appPath + SHARE_PATH).absoluteFilePath();
}

static QLatin1String qmlPuppetApplicationDirectoryForTests()
{
    if (Utils::HostOsInfo::isWindowsHost())
        //one more - debug/release dir
        return QLatin1String("/../../../../../../bin/");
    return QLatin1String("/../../../../../bin/");
}
} //namespace

namespace QmlDesigner {

static bool hasQtQuick2(NodeInstanceView *nodeInstanceView)
{
    if (nodeInstanceView && nodeInstanceView->model()) {
        foreach (const Import &import ,nodeInstanceView->model()->imports()) {
            if (import.url() ==  "QtQuick" && import.version().toDouble() >= 2.0)
                return true;
        }
    }

    return false;
}

static bool hasQtQuick1(NodeInstanceView *nodeInstanceView)
{
    if (nodeInstanceView && nodeInstanceView->model()) {
        foreach (const Import &import ,nodeInstanceView->model()->imports()) {
            if (import.url() ==  "QtQuick" && import.version().toDouble() < 2.0)
                return true;
        }
    }

    return false;
}

QString NodeInstanceServerProxy::creatorQmlPuppetPath()
{
    QString applicationPath =  QCoreApplication::applicationDirPath();
    applicationPath = macOSBundlePath(applicationPath);
    applicationPath += QLatin1Char('/') + qmlPuppetApplicationName();

    return applicationPath;
}

bool NodeInstanceServerProxy::checkPuppetVersion(const QString &qmlPuppetPath)
{
    QProcess qmlPuppetVersionProcess;
    qmlPuppetVersionProcess.start(qmlPuppetPath, QStringList() << "--version");
    qmlPuppetVersionProcess.waitForReadyRead(6000);

    QByteArray versionString = qmlPuppetVersionProcess.readAll();

    bool canConvert;
    unsigned int versionNumber = versionString.toUInt(&canConvert);

    return canConvert && versionNumber == 2;
}

NodeInstanceServerProxy::NodeInstanceServerProxy(NodeInstanceView *nodeInstanceView, RunModus runModus, const QString &pathToQt)
    : NodeInstanceServerInterface(nodeInstanceView),
      m_localServer(new QLocalServer(this)),
      m_nodeInstanceView(nodeInstanceView),
      m_firstBlockSize(0),
      m_secondBlockSize(0),
      m_thirdBlockSize(0),
      m_writeCommandCounter(0),
      m_firstLastReadCommandCounter(0),
      m_secondLastReadCommandCounter(0),
      m_thirdLastReadCommandCounter(0),
      m_runModus(runModus),
      m_synchronizeId(-1)
{
   QString applicationPath =  pathToQt + QLatin1String("/bin");
   if (runModus == TestModus) {
       applicationPath = QCoreApplication::applicationDirPath()
           + qmlPuppetApplicationDirectoryForTests()
           + qmlPuppetApplicationName();
   } else {
       applicationPath = macOSBundlePath(applicationPath);
       applicationPath += QLatin1Char('/') + qmlPuppetApplicationName();


#if defined(QT_NO_DEBUG) || defined(SEARCH_PUPPET_IN_CREATOR_BINPATH) // to prevent of choosing the wrong puppet in debug
       if (!QFileInfo(applicationPath).exists()) { //No qmlpuppet in Qt
           //We have to find out how to give not too intrusive feedback
           applicationPath = creatorQmlPuppetPath();
       }
#endif
   }


   QByteArray envImportPath = qgetenv("QTCREATOR_QMLPUPPET_PATH");
   if (!envImportPath.isEmpty())
       applicationPath = envImportPath;

   QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0)) && (defined(Q_OS_MAC) || defined(Q_OS_LINUX))
   environment.insert(QLatin1String("DESIGNER_DONT_USE_SHARED_MEMORY"), QLatin1String("1"));
#endif

   if (QFileInfo(applicationPath).exists()) {
       if (checkPuppetVersion(applicationPath)) {
           QString socketToken(QUuid::createUuid().toString());
           m_localServer->listen(socketToken);
           m_localServer->setMaxPendingConnections(3);

           m_qmlPuppetEditorProcess = new QProcess;
           m_qmlPuppetEditorProcess->setProcessEnvironment(environment);
           m_qmlPuppetEditorProcess->setObjectName("EditorProcess");
           connect(m_qmlPuppetEditorProcess.data(), SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processFinished(int,QProcess::ExitStatus)));
           connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), m_qmlPuppetEditorProcess.data(), SLOT(kill()));
           bool fowardQmlpuppetOutput = !qgetenv("FORWARD_QMLPUPPET_OUTPUT").isEmpty();
           if (fowardQmlpuppetOutput) {
               m_qmlPuppetEditorProcess->setProcessChannelMode(QProcess::MergedChannels);
               connect(m_qmlPuppetEditorProcess.data(), SIGNAL(readyRead()), this, SLOT(printEditorProcessOutput()));
           }
           m_qmlPuppetEditorProcess->start(applicationPath, QStringList() << socketToken << "editormode" << "-graphicssystem raster");

           if (runModus == NormalModus) {
               m_qmlPuppetPreviewProcess = new QProcess;
               m_qmlPuppetPreviewProcess->setProcessEnvironment(environment);
               m_qmlPuppetPreviewProcess->setObjectName("PreviewProcess");
               connect(m_qmlPuppetPreviewProcess.data(), SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processFinished(int,QProcess::ExitStatus)));
               connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), m_qmlPuppetPreviewProcess.data(), SLOT(kill()));
               if (fowardQmlpuppetOutput) {
                   m_qmlPuppetPreviewProcess->setProcessChannelMode(QProcess::MergedChannels);
                   connect(m_qmlPuppetPreviewProcess.data(), SIGNAL(readyRead()), this, SLOT(printPreviewProcessOutput()));
               }
               m_qmlPuppetPreviewProcess->start(applicationPath, QStringList() << socketToken << "previewmode" << "-graphicssystem raster");

               m_qmlPuppetRenderProcess = new QProcess;
               m_qmlPuppetRenderProcess->setProcessEnvironment(environment);
               m_qmlPuppetRenderProcess->setObjectName("RenderProcess");
               connect(m_qmlPuppetRenderProcess.data(), SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processFinished(int,QProcess::ExitStatus)));
               connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), m_qmlPuppetRenderProcess.data(), SLOT(kill()));
               if (fowardQmlpuppetOutput) {
                   m_qmlPuppetRenderProcess->setProcessChannelMode(QProcess::MergedChannels);
                   connect(m_qmlPuppetRenderProcess.data(), SIGNAL(readyRead()), this, SLOT(printRenderProcessOutput()));
               }
               m_qmlPuppetRenderProcess->start(applicationPath, QStringList() << socketToken << "rendermode" << "-graphicssystem raster");

           }

           connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));

           if (m_qmlPuppetEditorProcess->waitForStarted(10000)) {
               connect(m_qmlPuppetEditorProcess.data(), SIGNAL(finished(int)), m_qmlPuppetEditorProcess.data(),SLOT(deleteLater()));

               if (runModus == NormalModus) {
                   m_qmlPuppetPreviewProcess->waitForStarted();
                   connect(m_qmlPuppetPreviewProcess.data(), SIGNAL(finished(int)), m_qmlPuppetPreviewProcess.data(),SLOT(deleteLater()));

                   m_qmlPuppetRenderProcess->waitForStarted();
                   connect(m_qmlPuppetRenderProcess.data(), SIGNAL(finished(int)), m_qmlPuppetRenderProcess.data(),SLOT(deleteLater()));
               }

               if (!m_localServer->hasPendingConnections())
                   m_localServer->waitForNewConnection(10000);

               m_firstSocket = m_localServer->nextPendingConnection();
               connect(m_firstSocket.data(), SIGNAL(readyRead()), this, SLOT(readFirstDataStream()));

               if (runModus == NormalModus) {
                   if (!m_localServer->hasPendingConnections())
                       m_localServer->waitForNewConnection(10000);

                   m_secondSocket = m_localServer->nextPendingConnection();
                   connect(m_secondSocket.data(), SIGNAL(readyRead()), this, SLOT(readSecondDataStream()));

                   if (!m_localServer->hasPendingConnections())
                       m_localServer->waitForNewConnection(10000);

                   m_thirdSocket = m_localServer->nextPendingConnection();
                   connect(m_thirdSocket.data(), SIGNAL(readyRead()), this, SLOT(readThirdDataStream()));
               }

           } else {
               PuppetDialog::warning(Core::ICore::dialogParent(),
                                     tr("Cannot Start QML Puppet Executable"),
                                     missingQmlPuppetErrorMessage(tr("The executable of the QML Puppet process (%1) cannot be started. "
                                                                     "Please check your installation. "
                                                                     "QML Puppet is a process which runs in the background to render the items."
                                                                    ).arg(applicationPath)),
                                     copyAndPasterMessage(pathToQt));

               QmlDesignerPlugin::instance()->switchToTextModeDeferred();
           }

           m_localServer->close();

       } else {
           PuppetDialog::warning(Core::ICore::dialogParent(),
                                 tr("Wrong QML Puppet Executable Version"),
                                 missingQmlPuppetErrorMessage(tr("The QML Puppet version is incompatible with the Qt Creator version.")),
                                 copyAndPasterMessage(pathToQt)
                                 );
           QmlDesignerPlugin::instance()->switchToTextModeDeferred();
       }
   } else {
       PuppetDialog::warning(Core::ICore::dialogParent(),
                             tr("Cannot Find QML Puppet Executable"),
                             missingQmlPuppetErrorMessage(tr("The executable of the QML Puppet process (<code>%1</code>) cannot be found. "
                                                             "Check your installation. "
                                                            "QML Puppet is a process which runs in the background to render the items.").
                                                         arg(QDir::toNativeSeparators(applicationPath))),
                             copyAndPasterMessage(pathToQt));
       QmlDesignerPlugin::instance()->switchToTextModeDeferred();
   }

   int indexOfCapturePuppetStream = QCoreApplication::arguments().indexOf("-capture-puppet-stream");
   if (indexOfCapturePuppetStream > 0) {
       m_captureFileForTest.setFileName(QCoreApplication::arguments().at(indexOfCapturePuppetStream + 1));
       bool isOpen = m_captureFileForTest.open(QIODevice::WriteOnly);
       qDebug() << "file is open: " << isOpen;
   }
}

NodeInstanceServerProxy::~NodeInstanceServerProxy()
{
    disconnect(this, SLOT(processFinished(int,QProcess::ExitStatus)));

    writeCommand(QVariant::fromValue(EndPuppetCommand()));

    if (m_firstSocket)
        m_firstSocket->close();

    if (m_secondSocket)
        m_secondSocket->close();

    if (m_thirdSocket)
        m_thirdSocket->close();


    if (m_qmlPuppetEditorProcess)
        QTimer::singleShot(3000, m_qmlPuppetEditorProcess.data(), SLOT(terminate()));

    if (m_qmlPuppetPreviewProcess)
        QTimer::singleShot(3000, m_qmlPuppetPreviewProcess.data(), SLOT(terminate()));

    if (m_qmlPuppetRenderProcess)
         QTimer::singleShot(3000, m_qmlPuppetRenderProcess.data(), SLOT(terminate()));
}

void NodeInstanceServerProxy::dispatchCommand(const QVariant &command)
{
    static const int informationChangedCommandType = QMetaType::type("InformationChangedCommand");
    static const int valuesChangedCommandType = QMetaType::type("ValuesChangedCommand");
    static const int pixmapChangedCommandType = QMetaType::type("PixmapChangedCommand");
    static const int childrenChangedCommandType = QMetaType::type("ChildrenChangedCommand");
    static const int statePreviewImageChangedCommandType = QMetaType::type("StatePreviewImageChangedCommand");
    static const int componentCompletedCommandType = QMetaType::type("ComponentCompletedCommand");
    static const int synchronizeCommandType = QMetaType::type("SynchronizeCommand");
    static const int tokenCommandType = QMetaType::type("TokenCommand");
    static const int debugOutputCommandType = QMetaType::type("DebugOutputCommand");

    if (command.userType() ==  informationChangedCommandType) {
        nodeInstanceClient()->informationChanged(command.value<InformationChangedCommand>());
    } else if (command.userType() ==  valuesChangedCommandType) {
        nodeInstanceClient()->valuesChanged(command.value<ValuesChangedCommand>());
    } else if (command.userType() ==  pixmapChangedCommandType) {
        nodeInstanceClient()->pixmapChanged(command.value<PixmapChangedCommand>());
    } else if (command.userType() == childrenChangedCommandType) {
        nodeInstanceClient()->childrenChanged(command.value<ChildrenChangedCommand>());
    } else if (command.userType() == statePreviewImageChangedCommandType) {
        nodeInstanceClient()->statePreviewImagesChanged(command.value<StatePreviewImageChangedCommand>());
    } else if (command.userType() == componentCompletedCommandType) {
        nodeInstanceClient()->componentCompleted(command.value<ComponentCompletedCommand>());
    } else if (command.userType() == tokenCommandType) {
        nodeInstanceClient()->token(command.value<TokenCommand>());
    } else if (command.userType() == debugOutputCommandType) {
        nodeInstanceClient()->debugOutput(command.value<DebugOutputCommand>());
    } else if (command.userType() == synchronizeCommandType) {
        SynchronizeCommand synchronizeCommand = command.value<SynchronizeCommand>();
        m_synchronizeId = synchronizeCommand.synchronizeId();
    }  else
        Q_ASSERT(false);
}

NodeInstanceClientInterface *NodeInstanceServerProxy::nodeInstanceClient() const
{
    return m_nodeInstanceView.data();
}

static QString generatePuppetCompilingHelp(const QString &puppetName, const QString &pathToQt)
{

    QString buildDirectory =  QDir::toNativeSeparators(QDir::cleanPath(QDir::tempPath() + QStringLiteral("/") + puppetName));
    QString qmakePath = QStringLiteral("\"") +QDir::toNativeSeparators(QDir::cleanPath(pathToQt + QStringLiteral("/bin/qmake\" -r ")));
    QString projectPath = QStringLiteral("\"") + QDir::toNativeSeparators(QDir::cleanPath(sharedDirPath() + QStringLiteral("/qml/qmlpuppet/%1/%1.pro\"\n")));

    QString puppetCompileHelp;

    puppetCompileHelp.append(QStringLiteral("<p><code><pre>"));
    puppetCompileHelp.append(QStringLiteral("<form><input></input></form>"));
    puppetCompileHelp.append(QStringLiteral("mkdir \"") + buildDirectory+ QStringLiteral("\"\n"));
    puppetCompileHelp.append(QStringLiteral("cd \"") + buildDirectory + QStringLiteral("\"\n"));
    puppetCompileHelp.append(qmakePath + projectPath);
    puppetCompileHelp.append(QStringLiteral("make"));
    puppetCompileHelp.append(QStringLiteral("</pre></code></p>"));

    puppetCompileHelp = puppetCompileHelp.arg(puppetName);

    return puppetCompileHelp;
}

static void formatQmlPuppetCompilationMessage(const QString &puppetName,
                                              const QString &sharedDirPath,
                                              QTextStream &messageStream)
{
    const QString sourcePath = sharedDirPath + QStringLiteral("/qml/qmlpuppet/") + puppetName + QLatin1Char('/');

    //: %1 Puppet binary name ("qmlpuppet", "qml2puppet"), %2 source path.
    messageStream << QChar(' ') << NodeInstanceServerProxy::tr("You can build <code>%1</code> yourself with Qt 5.2.0 or higher. "
                                                 "The source can be found in <code>%2</code>.")
                     .arg(puppetName, QDir::toNativeSeparators(sourcePath))
                  << QChar(' ') << NodeInstanceServerProxy::tr("<code>%1</code> will be installed to the <code>bin</code> directory of your Qt version. "
                                                 "Qt Quick Designer will check the <code>bin</code> directory of the currently active Qt version "
                                                 "of your project.").arg(puppetName);
}

QString NodeInstanceServerProxy::missingQmlPuppetErrorMessage(const QString &preMessage) const
{
    QString message;
    QTextStream messageStream(&message);

    messageStream << QStringLiteral("<html><head/><body><p>")
                  << preMessage;

    if (hasQtQuick2(m_nodeInstanceView.data()))
        formatQmlPuppetCompilationMessage(QStringLiteral("qml2puppet"), sharedDirPath(), messageStream);
    else if (hasQtQuick1(m_nodeInstanceView.data()))
        formatQmlPuppetCompilationMessage(QStringLiteral("qmlpuppet"), sharedDirPath(), messageStream);

    messageStream << QStringLiteral("</p></body></html>");

    return message;
}

QString NodeInstanceServerProxy::copyAndPasterMessage(const QString &pathToQt) const
{
    QString message;
    QTextStream messageStream(&message);
    messageStream << QStringLiteral("<html><head/><body>");

    if (hasQtQuick2(m_nodeInstanceView.data()))
        messageStream << generatePuppetCompilingHelp(QStringLiteral("qml2puppet"), pathToQt);
    else if (hasQtQuick1(m_nodeInstanceView.data()))
        messageStream << generatePuppetCompilingHelp(QStringLiteral("qmlpuppet"), pathToQt);

    messageStream << QStringLiteral("</body></html>");

    return message;
}

static void writeCommandToIODecive(const QVariant &command, QIODevice *ioDevice, unsigned int commandCounter)
{
    if (ioDevice) {
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_8);
        out << quint32(0);
        out << quint32(commandCounter);
        out << command;
        out.device()->seek(0);
        out << quint32(block.size() - sizeof(quint32));

        ioDevice->write(block);
    }
}

void NodeInstanceServerProxy::writeCommand(const QVariant &command)
{
    writeCommandToIODecive(command, m_firstSocket.data(), m_writeCommandCounter);
    writeCommandToIODecive(command, m_secondSocket.data(), m_writeCommandCounter);
    writeCommandToIODecive(command, m_thirdSocket.data(), m_writeCommandCounter);

    if (m_captureFileForTest.isWritable()) {
        qDebug() << "Write strean to file: " << m_captureFileForTest.fileName();
        writeCommandToIODecive(command, &m_captureFileForTest, m_writeCommandCounter);
        qDebug() << "\twrite file: " << m_captureFileForTest.pos();
    }

    m_writeCommandCounter++;
    if (m_runModus == TestModus) {
        static int synchronizeId = 0;
        synchronizeId++;
        SynchronizeCommand synchronizeCommand(synchronizeId);

        writeCommandToIODecive(QVariant::fromValue(synchronizeCommand), m_firstSocket.data(), m_writeCommandCounter);
        m_writeCommandCounter++;

        while (m_firstSocket->waitForReadyRead(100)) {
                readFirstDataStream();
                if (m_synchronizeId == synchronizeId)
                    return;
        }
    }
}

void NodeInstanceServerProxy::processFinished(int /*exitCode*/, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Process finished:" << sender();

    if (m_captureFileForTest.isOpen()) {
        m_captureFileForTest.close();
        m_captureFileForTest.remove();
        QMessageBox::warning(Core::ICore::dialogParent(), tr("QML Puppet Crashed"),
                             tr("You are recording a puppet stream and the puppet crashed. "
                                "It is recommended to reopen the Qt Quick Designer and start again."));
    }


    writeCommand(QVariant::fromValue(EndPuppetCommand()));

    if (m_firstSocket)
        m_firstSocket->close();
    if (m_secondSocket)
        m_secondSocket->close();
    if (m_thirdSocket)
        m_thirdSocket->close();

    if (exitStatus == QProcess::CrashExit)
        emit processCrashed();
}


void NodeInstanceServerProxy::readFirstDataStream()
{
    QList<QVariant> commandList;

    while (!m_firstSocket->atEnd()) {
        if (m_firstSocket->bytesAvailable() < int(sizeof(quint32)))
            break;

        QDataStream in(m_firstSocket.data());
        in.setVersion(QDataStream::Qt_4_8);

        if (m_firstBlockSize == 0)
            in >> m_firstBlockSize;

        if (m_firstSocket->bytesAvailable() < m_firstBlockSize)
            break;

        quint32 commandCounter;
        in >> commandCounter;
        bool commandLost = !((m_firstLastReadCommandCounter == 0 && commandCounter == 0) || (m_firstLastReadCommandCounter + 1 == commandCounter));
        if (commandLost)
            qDebug() << "server command lost: " << m_firstLastReadCommandCounter <<  commandCounter;
        m_firstLastReadCommandCounter = commandCounter;


        QVariant command;
        in >> command;
        m_firstBlockSize = 0;

        commandList.append(command);
    }

    foreach (const QVariant &command, commandList) {
        dispatchCommand(command);
    }
}

void NodeInstanceServerProxy::readSecondDataStream()
{
    QList<QVariant> commandList;

    while (!m_secondSocket->atEnd()) {
        if (m_secondSocket->bytesAvailable() < int(sizeof(quint32)))
            break;

        QDataStream in(m_secondSocket.data());
        in.setVersion(QDataStream::Qt_4_8);

        if (m_secondBlockSize == 0)
            in >> m_secondBlockSize;

        if (m_secondSocket->bytesAvailable() < m_secondBlockSize)
            break;

        quint32 commandCounter;
        in >> commandCounter;
        bool commandLost = !((m_secondLastReadCommandCounter == 0 && commandCounter == 0) || (m_secondLastReadCommandCounter + 1 == commandCounter));
        if (commandLost)
            qDebug() << "server command lost: " << m_secondLastReadCommandCounter <<  commandCounter;
        m_secondLastReadCommandCounter = commandCounter;


        QVariant command;
        in >> command;
        m_secondBlockSize = 0;

        commandList.append(command);
    }

    foreach (const QVariant &command, commandList) {
        dispatchCommand(command);
    }
}

void NodeInstanceServerProxy::readThirdDataStream()
{
    QList<QVariant> commandList;

    while (!m_thirdSocket->atEnd()) {
        if (m_thirdSocket->bytesAvailable() < int(sizeof(quint32)))
            break;

        QDataStream in(m_thirdSocket.data());
        in.setVersion(QDataStream::Qt_4_8);

        if (m_thirdBlockSize == 0)
            in >> m_thirdBlockSize;

        if (m_thirdSocket->bytesAvailable() < m_thirdBlockSize)
            break;

        quint32 commandCounter;
        in >> commandCounter;
        bool commandLost = !((m_thirdLastReadCommandCounter == 0 && commandCounter == 0) || (m_thirdLastReadCommandCounter + 1 == commandCounter));
        if (commandLost)
            qDebug() << "server command lost: " << m_thirdLastReadCommandCounter <<  commandCounter;
        m_thirdLastReadCommandCounter = commandCounter;


        QVariant command;
        in >> command;
        m_thirdBlockSize = 0;

        commandList.append(command);
    }

    foreach (const QVariant &command, commandList) {
        dispatchCommand(command);
    }
}

void NodeInstanceServerProxy::printEditorProcessOutput()
{
    while (m_qmlPuppetEditorProcess->canReadLine()) {
        QByteArray line = m_qmlPuppetEditorProcess->readLine();
        line.chop(1);
        qDebug().nospace() << "Editor Puppet: " << qPrintable(line);
    }
    qDebug() << "\n";
}

void NodeInstanceServerProxy::printPreviewProcessOutput()
{
    while (m_qmlPuppetPreviewProcess->canReadLine()) {
        QByteArray line = m_qmlPuppetPreviewProcess->readLine();
        line.chop(1);
        qDebug().nospace() << "Preview Puppet: " << qPrintable(line);
    }
    qDebug() << "\n";
}

void NodeInstanceServerProxy::printRenderProcessOutput()
{
    while (m_qmlPuppetRenderProcess->canReadLine()) {
        QByteArray line = m_qmlPuppetRenderProcess->readLine();
        line.chop(1);
        qDebug().nospace() << "Render Puppet: " << qPrintable(line);
    }

    qDebug() << "\n";
}

QString NodeInstanceServerProxy::qmlPuppetApplicationName() const
{
    if (hasQtQuick2(m_nodeInstanceView.data()))
        return QLatin1String("qml2puppet" QTC_HOST_EXE_SUFFIX);
    return QLatin1String("qmlpuppet" QTC_HOST_EXE_SUFFIX);
}

QString NodeInstanceServerProxy::macOSBundlePath(const QString &path) const
{
    QString applicationPath = path;
    if (Utils::HostOsInfo::isMacHost()) {
        if (hasQtQuick2(m_nodeInstanceView.data()))
            applicationPath += QLatin1String("/qml2puppet.app/Contents/MacOS");
        else
            applicationPath += QLatin1String("/qmlpuppet.app/Contents/MacOS");

    }
   return applicationPath;
}

void NodeInstanceServerProxy::createInstances(const CreateInstancesCommand &command)
{
    writeCommand(QVariant::fromValue(command));
}

void NodeInstanceServerProxy::changeFileUrl(const ChangeFileUrlCommand &command)
{
    writeCommand(QVariant::fromValue(command));
}

void NodeInstanceServerProxy::createScene(const CreateSceneCommand &command)
{
    writeCommand(QVariant::fromValue(command));
}

void NodeInstanceServerProxy::clearScene(const ClearSceneCommand &command)
{
    writeCommand(QVariant::fromValue(command));
}

void NodeInstanceServerProxy::removeInstances(const RemoveInstancesCommand &command)
{
    writeCommand(QVariant::fromValue(command));
}

void NodeInstanceServerProxy::removeProperties(const RemovePropertiesCommand &command)
{
    writeCommand(QVariant::fromValue(command));
}

void NodeInstanceServerProxy::changePropertyBindings(const ChangeBindingsCommand &command)
{
    writeCommand(QVariant::fromValue(command));
}

void NodeInstanceServerProxy::changePropertyValues(const ChangeValuesCommand &command)
{
    writeCommand(QVariant::fromValue(command));
}

void NodeInstanceServerProxy::changeAuxiliaryValues(const ChangeAuxiliaryCommand &command)
{
    writeCommand(QVariant::fromValue(command));
}

void NodeInstanceServerProxy::reparentInstances(const ReparentInstancesCommand &command)
{
    writeCommand(QVariant::fromValue(command));
}

void NodeInstanceServerProxy::changeIds(const ChangeIdsCommand &command)
{
    writeCommand(QVariant::fromValue(command));
}

void NodeInstanceServerProxy::changeState(const ChangeStateCommand &command)
{
    writeCommand(QVariant::fromValue(command));
}

void NodeInstanceServerProxy::completeComponent(const CompleteComponentCommand &command)
{
    writeCommand(QVariant::fromValue(command));
}

void NodeInstanceServerProxy::changeNodeSource(const ChangeNodeSourceCommand &command)
{
    writeCommand(QVariant::fromValue(command));
}

void NodeInstanceServerProxy::token(const TokenCommand &command)
{
    writeCommand(QVariant::fromValue(command));
}

void NodeInstanceServerProxy::removeSharedMemory(const RemoveSharedMemoryCommand &command)
{
   writeCommand(QVariant::fromValue(command));
}

} // namespace QmlDesigner
