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

#include "qmlprofilerviewmanager.h"

#include "qmlprofilertraceview.h"
#include "qmlprofilereventview.h"
#include "qmlprofilertool.h"
#include "qmlprofilerstatemanager.h"
#include "qmlprofilermodelmanager.h"
#include "qmlprofilerstatewidget.h"
#include "qv8profilereventview.h"

#include <utils/qtcassert.h>
#include <utils/fancymainwindow.h>
#include <analyzerbase/analyzermanager.h>

#include <QDockWidget>

using namespace Analyzer;

namespace QmlProfiler {
namespace Internal {

class QmlProfilerViewManager::QmlProfilerViewManagerPrivate {
public:
    QmlProfilerViewManagerPrivate(QmlProfilerViewManager *qq) { Q_UNUSED(qq); }

    QmlProfilerTraceView *traceView;
    QmlProfilerEventsWidget *eventsView;
    QV8ProfilerEventsWidget *v8profilerView;
    QmlProfilerStateManager *profilerState;
    QmlProfilerModelManager *profilerModelManager;
    QmlProfilerTool *profilerTool;
};

QmlProfilerViewManager::QmlProfilerViewManager(QObject *parent,
                                               QmlProfilerTool *profilerTool,
                                               QmlProfilerModelManager *modelManager,
                                               QmlProfilerStateManager *profilerState)
    : QObject(parent), d(new QmlProfilerViewManagerPrivate(this))
{
    setObjectName(QLatin1String("QML Profiler View Manager"));
    d->traceView = 0;
    d->eventsView = 0;
    d->v8profilerView = 0;
    d->profilerState = profilerState;
    d->profilerModelManager = modelManager;
    d->profilerTool = profilerTool;
    createViews();
}

QmlProfilerViewManager::~QmlProfilerViewManager()
{
    delete d;
}

////////////////////////////////////////////////////////////
// Views
void QmlProfilerViewManager::createViews()
{

    QTC_ASSERT(d->profilerModelManager, return);
    QTC_ASSERT(d->profilerState, return);

    Utils::FancyMainWindow *mw = AnalyzerManager::mainWindow();

    d->traceView = new QmlProfilerTraceView(mw,
                                            d->profilerTool,
                                            this,
                                            d->profilerModelManager,
                                            d->profilerState);
    connect(d->traceView, SIGNAL(gotoSourceLocation(QString,int,int)),
            this, SIGNAL(gotoSourceLocation(QString,int,int)));
    d->traceView->reset();


    d->eventsView = new QmlProfilerEventsWidget(mw, d->profilerTool, this,
                                                d->profilerModelManager);
    connect(d->eventsView, SIGNAL(gotoSourceLocation(QString,int,int)), this,
            SIGNAL(gotoSourceLocation(QString,int,int)));
    connect(d->eventsView, SIGNAL(eventSelectedByHash(QString)), d->traceView,
            SLOT(selectNextEventByHash(QString)));
    connect(d->traceView, SIGNAL(gotoSourceLocation(QString,int,int)),
            d->eventsView, SLOT(selectBySourceLocation(QString,int,int)));

    d->v8profilerView = new QV8ProfilerEventsWidget(mw, d->profilerTool, this,
                                                    d->profilerModelManager);
    connect(d->v8profilerView, SIGNAL(gotoSourceLocation(QString,int,int)), this,
            SIGNAL(gotoSourceLocation(QString,int,int)));
    connect(d->traceView, SIGNAL(gotoSourceLocation(QString,int,int)),
            d->v8profilerView, SLOT(selectBySourceLocation(QString,int,int)));
    connect(d->v8profilerView, SIGNAL(gotoSourceLocation(QString,int,int)),
            d->traceView, SLOT(selectNextEventByLocation(QString,int,int)));
    connect(d->v8profilerView, SIGNAL(gotoSourceLocation(QString,int,int)),
            d->eventsView, SLOT(selectBySourceLocation(QString,int,int)));
    connect(d->eventsView, SIGNAL(gotoSourceLocation(QString,int,int)),
            d->v8profilerView, SLOT(selectBySourceLocation(QString,int,int)));

    QDockWidget *eventsDock = AnalyzerManager::createDockWidget
            (d->profilerTool, tr("Events"), d->eventsView, Qt::BottomDockWidgetArea);
    QDockWidget *timelineDock = AnalyzerManager::createDockWidget
            (d->profilerTool, tr("Timeline"), d->traceView, Qt::BottomDockWidgetArea);
    QDockWidget *v8profilerDock = AnalyzerManager::createDockWidget(
                d->profilerTool, tr("JavaScript"), d->v8profilerView, Qt::BottomDockWidgetArea);

    eventsDock->show();
    timelineDock->show();
    v8profilerDock->show();

    mw->splitDockWidget(mw->toolBarDockWidget(), timelineDock, Qt::Vertical);
    mw->tabifyDockWidget(timelineDock, eventsDock);
    mw->tabifyDockWidget(eventsDock, v8profilerDock);

    new QmlProfilerStateWidget(d->profilerState, d->profilerModelManager, d->eventsView);
    new QmlProfilerStateWidget(d->profilerState, d->profilerModelManager, d->traceView);
    new QmlProfilerStateWidget(d->profilerState, d->profilerModelManager, d->v8profilerView);
}

bool QmlProfilerViewManager::hasValidSelection() const
{
    return d->traceView->hasValidSelection();
}

qint64 QmlProfilerViewManager::selectionStart() const
{
    return d->traceView->selectionStart();
}

qint64 QmlProfilerViewManager::selectionEnd() const
{
    return d->traceView->selectionEnd();
}

bool QmlProfilerViewManager::hasGlobalStats() const
{
    return d->eventsView->hasGlobalStats();
}

void QmlProfilerViewManager::getStatisticsInRange(qint64 rangeStart, qint64 rangeEnd)
{
    d->eventsView->getStatisticsInRange(rangeStart, rangeEnd);
}

void QmlProfilerViewManager::clear()
{
    d->traceView->clear();
    d->eventsView->clear();
    d->v8profilerView->clear();
}

} // namespace Internal
} // namespace QmlProfiler
