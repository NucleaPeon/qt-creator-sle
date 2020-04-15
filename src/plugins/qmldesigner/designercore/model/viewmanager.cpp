#include "viewmanager.h"

#include <rewriterview.h>
#include <nodeinstanceview.h>
#include <itemlibraryview.h>
#include <navigatorview.h>
#include <stateseditorview.h>
#include <formeditorview.h>
#include <propertyeditorview.h>
#include <componentview.h>
#include <debugview.h>
#include <importmanagerview.h>
#include <designeractionmanagerview.h>

#include "componentaction.h"
#include "designmodewidget.h"
#include "crumblebar.h"

#include <qmldesigner/qmldesignerplugin.h>

#include <qtsupport/qtversionmanager.h>
#include <qtsupport/qtsupportconstants.h>

namespace QmlDesigner {

class ViewManagerData
{
public:
    QmlModelState savedState;
    Internal::DebugView debugView;
    ComponentView componentView;
    FormEditorView formEditorView;
    ItemLibraryView itemLibraryView;
    NavigatorView navigatorView;
    PropertyEditorView propertyEditorView;
    StatesEditorView statesEditorView;
    NodeInstanceView nodeInstanceView;
    DesignerActionManagerView designerActionManagerView;

    QList<QWeakPointer<AbstractView> > additionalViews;
};


static CrumbleBar *crumbleBar() {
    return QmlDesignerPlugin::instance()->mainWidget()->crumbleBar();
}

ViewManager::ViewManager()
    : d(new ViewManagerData)
{
}

ViewManager::~ViewManager()
{
    foreach (const QWeakPointer<AbstractView> &view, d->additionalViews)
        delete view.data();

    delete d;
}

DesignDocument *ViewManager::currentDesignDocument() const
{
    return QmlDesignerPlugin::instance()->documentManager().currentDesignDocument();
}

QString ViewManager::pathToQt() const
{
    QtSupport::BaseQtVersion *activeQtVersion = QtSupport::QtVersionManager::version(currentDesignDocument()->qtVersionId());
    if (activeQtVersion && (activeQtVersion->qtVersion() >= QtSupport::QtVersionNumber(4, 7, 1))
            && (activeQtVersion->type() == QLatin1String(QtSupport::Constants::DESKTOPQT)
                || activeQtVersion->type() == QLatin1String(QtSupport::Constants::SIMULATORQT)))
        return activeQtVersion->qmakeProperty("QT_INSTALL_DATA");

    return QString();
}

void ViewManager::attachNodeInstanceView()
{
    setNodeInstanceViewQtPath(pathToQt());
    currentModel()->setNodeInstanceView(&d->nodeInstanceView);
}

void ViewManager::attachRewriterView()
{
    if (currentDesignDocument()->rewriterView()) {
        currentModel()->setRewriterView(currentDesignDocument()->rewriterView());
        currentDesignDocument()->rewriterView()->reactivateTextMofifierChangeSignals();
    }
}

void ViewManager::detachRewriterView()
{
    if (currentDesignDocument()->rewriterView()) {
        currentDesignDocument()->rewriterView()->deactivateTextMofifierChangeSignals();
        currentModel()->setRewriterView(0);
    }
}

void ViewManager::switchStateEditorViewToBaseState()
{
    if (d->statesEditorView.isAttached()) {
        d->savedState = d->statesEditorView.currentState();
        d->statesEditorView.setCurrentState(d->statesEditorView.baseState());
    }
}

void ViewManager::switchStateEditorViewToSavedState()
{
    if (d->savedState.isValid() && d->statesEditorView.isAttached())
        d->statesEditorView.setCurrentState(d->savedState);
}

void ViewManager::resetPropertyEditorView()
{
    d->propertyEditorView.resetView();
}

void ViewManager::registerFormEditorToolTakingOwnership(AbstractCustomTool *tool)
{
    d->formEditorView.registerTool(tool);
}

void ViewManager::registerViewTakingOwnership(AbstractView *view)
{
    d->additionalViews.append(view);
}

void ViewManager::detachViewsExceptRewriterAndComponetView()
{
    switchStateEditorViewToBaseState();
    detachAdditionalViews();
    currentModel()->detachView(&d->designerActionManagerView);
    currentModel()->detachView(&d->formEditorView);
    currentModel()->detachView(&d->navigatorView);
    currentModel()->detachView(&d->itemLibraryView);
    currentModel()->detachView(&d->statesEditorView);
    currentModel()->detachView(&d->propertyEditorView);

    if (d->debugView.isAttached())
        currentModel()->detachView(&d->debugView);

    currentModel()->setNodeInstanceView(0);
}

void ViewManager::attachItemLibraryView()
{
    setItemLibraryViewResourcePath(QFileInfo(currentDesignDocument()->fileName()).absolutePath());
    currentModel()->attachView(&d->itemLibraryView);
}

void ViewManager::attachAdditionalViews()
{
    foreach (const QWeakPointer<AbstractView> &view, d->additionalViews)
        currentModel()->attachView(view.data());
}

void ViewManager::detachAdditionalViews()
{
    foreach (const QWeakPointer<AbstractView> &view, d->additionalViews)
        currentModel()->detachView(view.data());
}

void ViewManager::attachComponentView()
{
    documentModel()->attachView(&d->componentView);
    QObject::connect(d->componentView.action(), SIGNAL(currentComponentChanged(ModelNode)), currentDesignDocument(), SLOT(changeToSubComponent(ModelNode)));
    QObject::connect(d->componentView.action(), SIGNAL(changedToMaster()), currentDesignDocument(), SLOT(changeToMaster()));
}

void ViewManager::detachComponentView()
{
    QObject::disconnect(d->componentView.action(), SIGNAL(currentComponentChanged(ModelNode)), currentDesignDocument(), SLOT(changeToSubComponent(ModelNode)));
    QObject::disconnect(d->componentView.action(), SIGNAL(changedToMaster()), currentDesignDocument(), SLOT(changeToMaster()));

    documentModel()->detachView(&d->componentView);
}

void ViewManager::attachViewsExceptRewriterAndComponetView()
{
    if (QmlDesignerPlugin::instance()->settings().enableDebugView)
        currentModel()->attachView(&d->debugView);

    attachNodeInstanceView();
    currentModel()->attachView(&d->formEditorView);
    currentModel()->attachView(&d->navigatorView);
    attachItemLibraryView();
    currentModel()->attachView(&d->statesEditorView);
    currentModel()->attachView(&d->propertyEditorView);
    currentModel()->attachView(&d->designerActionManagerView);
    attachAdditionalViews();
    switchStateEditorViewToSavedState();
}

void ViewManager::setItemLibraryViewResourcePath(const QString &resourcePath)
{
    d->itemLibraryView.setResourcePath(resourcePath);
}

void ViewManager::setComponentNode(const ModelNode &componentNode)
{
    d->componentView.setComponentNode(componentNode);
}

void ViewManager::setComponentViewToMaster()
{
    d->componentView.setComponentToMaster();
}

void ViewManager::setNodeInstanceViewQtPath(const QString &qtPath)
{
    d->nodeInstanceView.setPathToQt(qtPath);
}

static bool widgetInfoLessThan(const WidgetInfo &firstWidgetInfo, const WidgetInfo &secondWidgetInfo)
{
    return firstWidgetInfo.placementPriority < secondWidgetInfo.placementPriority;
}

QList<WidgetInfo> ViewManager::widgetInfos()
{
    QList<WidgetInfo> widgetInfoList;

    widgetInfoList.append(d->formEditorView.widgetInfo());
    widgetInfoList.append(d->itemLibraryView.widgetInfo());
    widgetInfoList.append(d->navigatorView.widgetInfo());
    widgetInfoList.append(d->propertyEditorView.widgetInfo());
    widgetInfoList.append(d->statesEditorView.widgetInfo());
    if (d->debugView.hasWidget())
        widgetInfoList.append(d->debugView.widgetInfo());

    foreach (const QWeakPointer<AbstractView> &abstractView, d->additionalViews) {
        if (abstractView && abstractView->hasWidget())
            widgetInfoList.append(abstractView->widgetInfo());
    }

    qSort(widgetInfoList.begin(), widgetInfoList.end(), widgetInfoLessThan);

    return widgetInfoList;
}

void ViewManager::disableWidgets()
{
    foreach (const WidgetInfo &widgetInfo, widgetInfos())
        widgetInfo.widget->setEnabled(false);
}

void ViewManager::enableWidgets()
{
    foreach (const WidgetInfo &widgetInfo, widgetInfos())
        widgetInfo.widget->setEnabled(true);
}

void ViewManager::pushFileOnCrumbleBar(const QString &fileName)
{
    crumbleBar()->pushFile(fileName);
}

void ViewManager::pushInFileComponentOnCrumbleBar(const ModelNode &modelNode)
{
    crumbleBar()->pushInFileComponent(modelNode);
}

void ViewManager::nextFileIsCalledInternally()
{
    crumbleBar()->nextFileIsCalledInternally();
}

NodeInstanceView *ViewManager::nodeInstanceView()
{
    return &d->nodeInstanceView;
}

QWidgetAction *ViewManager::componentViewAction()
{
    return d->componentView.action();
}

DesignerActionManager &ViewManager::designerActionManager()
{
    return d->designerActionManagerView.designerActionManager();
}

const DesignerActionManager &ViewManager::designerActionManager() const
{
    return d->designerActionManagerView.designerActionManager();
}

Model *ViewManager::currentModel() const
{
    return currentDesignDocument()->currentModel();
}

Model *ViewManager::documentModel() const
{
    return currentDesignDocument()->documentModel();
}

} // namespace QmlDesigner
