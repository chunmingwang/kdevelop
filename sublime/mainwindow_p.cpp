/***************************************************************************
 *   Copyright 2006-2007 Alexander Dymo  <adymo@kdevelop.org>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "mainwindow_p.h"

#include <QMenu>
#include <QLayout>
#include <QSplitter>
#include <QDockWidget>
#include <QWidgetAction>
#include <QComboBox>
#include <QHBoxLayout>
#include <QToolButton>
#include <QMenu>

#include <kdebug.h>
#include <kacceleratormanager.h>
#include <kactioncollection.h>

#include "area.h"
#include "view.h"
#include "areaindex.h"
#include "document.h"
#include "container.h"
#include "controller.h"
#include "mainwindow.h"
#include "ideal.h"

namespace Sublime {

AreaSelectorWidget::AreaSelectorWidget(QWidget *parent,
                                       MainWindow* window,
                                       Controller* controller)
: QWidget(parent), window_(window), controller_(controller)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    combo_ = new QComboBox(this);
    layout->addWidget(combo_);
    QToolButton* button = new QToolButton(this);
    button->setIcon(KIcon("configure"));
    button->setPopupMode(QToolButton::InstantPopup);
    layout->addWidget(button);

    QMenu *menu = new QMenu(button);
    menu->addAction("Reset Current Area", this,
                    SLOT(resetCurrentArea()));
    menu->addAction("New Area");
    menu->addAction("Delete Area");
    button->setMenu(menu);

    foreach (Area *a, controller_->defaultAreas())
    {
        areaIds_.push_back(a->objectName());
        combo_->addItem(a->title());
        if (a->title() == window->area()->title())
            combo_->setCurrentIndex(combo_->count()-1);
    }
    connect (combo_, SIGNAL(activated(int)), this, SLOT(activateArea(int)));
    connect (window_, SIGNAL(areaChanged(Sublime::Area*)),
             this, SLOT(areaChanged(Sublime::Area*)));
}

void AreaSelectorWidget::resetCurrentArea()
{
    controller_->resetCurrentArea(window_);
}

void AreaSelectorWidget::activateArea(int index)
{
    controller_->showArea(areaIds_[index], window_);
}

void AreaSelectorWidget::areaChanged(Sublime::Area* area)
{
    for (int i = 0; i < combo_->count(); ++i)
        if (combo_->itemText(i) == area->title())
        {
            combo_->setCurrentIndex(i);
            break;
        }
}

QWidget* AreaSelectionAction::createWidget(QWidget* parent)
{
    return new AreaSelectorWidget(parent, window_, controller_);
}

MainWindowPrivate::MainWindowPrivate(MainWindow *w, Controller* controller)
:controller(controller), area(0), activeView(0), activeToolView(0), centralWidget(0),
 ignoreDockShown(false), autoAreaSettingsSave(false), m_mainWindow(w)
{
    recreateCentralWidget();

    QAction* action = new AreaSelectionAction(this, controller);
    m_mainWindow->actionCollection()->addAction("switch_area", action);
}

Area::WalkerMode MainWindowPrivate::IdealToolViewCreator::operator() (View *view, Sublime::Position position)
{
    if (!d->docks.contains(view))
    {
        d->docks << view;
        d->idealMainWidget->addView(d->positionToDockArea(position), view);
    }
    return Area::ContinueWalker;
}

Area::WalkerMode MainWindowPrivate::ViewCreator::operator() (AreaIndex *index)
{
    kDebug() << "reconstructing views for area index" << index;
    QSplitter *parent = 0;
    QSplitter *splitter = d->m_indexSplitters.value(index);
    if (!splitter)
    {
        //no splitter - we shall create it and populate with views
        if (!index->parent())
        {
            kDebug() << "reconstructing root area";
            //this is root area
            splitter = new QSplitter(d->centralWidget);
            d->m_indexSplitters[index] = splitter;
            d->centralWidget->layout()->addWidget(splitter);
        }
        else
        {
            parent = d->m_indexSplitters[index->parent()];
            kDebug() << "adding new splitter to" << parent;
            splitter = new QSplitter(parent);
            d->m_indexSplitters[index] = splitter;
            parent->addWidget(splitter);
        }
    }
    splitter->show();

    if (index->isSplitted()) //this is a visible splitter
        splitter->setOrientation(index->orientation());
    else
    {
        Container *container = 0;
        if (!splitter->widget(0))
        {
            //we need to create view container
            container = new Container(splitter);
            connect(container, SIGNAL(activateView(Sublime::View*)), d->m_mainWindow, SLOT(activateView(Sublime::View*)));
            connect(container, SIGNAL(closeRequest(QWidget*)),
                    d, SLOT(widgetCloseRequest(QWidget*)));
            splitter->addWidget(container);
        }
        else
            container = qobject_cast<Container*>(splitter->widget(0));
        container->show();
        foreach (View *view, index->views())
        {
            QWidget *widget = view->widget(container);
            if (widget && !container->hasWidget(widget))
            {
                widget->installEventFilter(d);
                foreach (QWidget* w, widget->findChildren<QWidget*>())
                    w->installEventFilter(d);
                container->addWidget(view);
                d->viewContainers[view] = container;
                d->widgetToView[widget] = view;
            }
        }
    }
    return Area::ContinueWalker;
}

void MainWindowPrivate::reconstruct()
{
    IdealToolViewCreator toolViewCreator(this);
    area->walkToolViews(toolViewCreator, Sublime::AllPositions);

    ViewCreator viewCreator(this);
    area->walkViews(viewCreator, area->rootIndex());

    idealMainWidget->blockSignals(true);
    IdealMainLayout *l = idealMainWidget->mainLayout();
    l->setWidthForRole(IdealMainLayout::Left, area->thickness(Sublime::Left));
    l->setWidthForRole(IdealMainLayout::Right, area->thickness(Sublime::Right));
    l->setWidthForRole(IdealMainLayout::Bottom,
                       area->thickness(Sublime::Bottom));
    l->setWidthForRole(IdealMainLayout::Top, area->thickness(Sublime::Top));
    kDebug() << "RECONSTRUCT" << area << "  " << area->shownToolView(Sublime::Left) << "\n";
    foreach (View *view, area->toolViews())
    {
        QString id = view->document()->documentSpecifier();
        if (!id.isEmpty())
        {
            Sublime::Position pos = area->toolViewPosition(view);
            if (area->shownToolView(pos) == id)
                idealMainWidget->raiseView(view);
        }
    }
    idealMainWidget->blockSignals(false);
}

void MainWindowPrivate::clearArea()
{
    //reparent toolview widgets to 0 to prevent their deletion together with dockwidgets
    foreach (View *view, area->toolViews())
    {
        // FIXME should we really delete here??
        idealMainWidget->removeView(view);

        if (view->hasWidget())
            view->widget()->setParent(0);
    }

    docks.clear();

    //reparent all view widgets to 0 to prevent their deletion together with central
    //widget. this reparenting is necessary when switching areas inside the same mainwindow
    foreach (View *view, area->views())
    {
        if (view->hasWidget())
            view->widget()->setParent(0);
    }
    recreateCentralWidget();
    m_indexSplitters.clear();
    area = 0;
}

void MainWindowPrivate::recreateCentralWidget()
{
    idealMainWidget = new IdealMainWidget(m_mainWindow, m_mainWindow->actionCollection());
    m_mainWindow->setCentralWidget(idealMainWidget);

    centralWidget = new QWidget();
    idealMainWidget->setCentralWidget(centralWidget);

    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    layout->setMargin(0);
    centralWidget->setLayout(layout);

    connect(idealMainWidget,
            SIGNAL(dockShown(Sublime::View*, Sublime::Position, bool)),
            this,
            SLOT(slotDockShown(Sublime::View*, Sublime::Position, bool)));

    connect(idealMainWidget->mainLayout(),
            SIGNAL(widgetResized(IdealMainLayout::Role, int)),
            this,
            SLOT(widgetResized(IdealMainLayout::Role, int)));

}

void MainWindowPrivate::
slotDockShown(Sublime::View* view, Sublime::Position pos, bool shown)
{
    if (ignoreDockShown)
        return;

    QString id;
    if (shown)
        id = view->document()->documentSpecifier();
    kDebug() << "View " << view->document()->documentSpecifier() << " " << shown;
    area->setShownToolView(pos, id);
}

void MainWindowPrivate::viewAdded(Sublime::AreaIndex *index, Sublime::View */*view*/)
{
    ViewCreator viewCreator(this);
    QSplitter *splitter = m_indexSplitters[index];
    if (index->isSplitted() && (splitter->count() == 1) &&
            qobject_cast<Sublime::Container*>(splitter->widget(0)))
    {
        Container *container = qobject_cast<Sublime::Container*>(splitter->widget(0));
        //we need to remove extra container before reconstruction
        //first reparent widgets in container so that they are not deleted
        while (container->count())
        {
            container->widget(0)->setParent(0);
        }
        //and then delete the container
        delete container;
    }
    area->walkViews(viewCreator, index);
}

void Sublime::MainWindowPrivate::raiseToolView(Sublime::View * view)
{
    idealMainWidget->raiseView(view);
}

void MainWindowPrivate::aboutToRemoveView(Sublime::AreaIndex *index, Sublime::View *view)
{
    if (!m_indexSplitters.contains(index))
        return;

    QSplitter *splitter = m_indexSplitters[index];
    kDebug() << "index " << index << " root " << area->rootIndex();
    kDebug() << "splitter " << splitter << " container " << splitter->widget(0);
    //find the container for the view and remove the widget
    Container *container = qobject_cast<Container*>(splitter->widget(0));
    if (!container) {
        kWarning() << "Splitter does not have a left widget!";
        return;
    }

    if (view->widget())
        widgetToView.remove(view->widget());
    viewContainers.remove(view);

    if (container->count() > 1)
    {
        bool wasActive = m_mainWindow->activeView() == view;
        //container is not empty or this is a root index
        //just remove a widget
        container->removeWidget(view->widget());
        view->widget()->setParent(0);
        //activate what is visible currently in the container if the removed view was active
        if (wasActive)
            return m_mainWindow->setActiveView(container->viewForWidget(container->currentWidget()));
    }
    else
    {
        // We've about to remove the last view of this container.  It will
        // be empty, so have to delete it, as well.

        // If we have a container, then it should be the only child of
        // the splitter.
        Q_ASSERT(splitter->count() == 1);
        container->removeWidget(view->widget());

        if (view->widget())
            view->widget()->setParent(0);
        else
            kWarning() << "View does not have a widget!";

        // We can be called from signal handler of container
        // (which is tab widget), so defer deleting it.
        container->deleteLater();

        /* If we're not at the top level, we get to collapse split views.  */
        if (index->parent())
        {
            /* The splitter used to have container as the only child, now it's
               time to get rid of it.  Make sure deleting splitter does not
               delete container -- per above comment, we'll delete it later.  */
            container->setParent(0);
            m_indexSplitters.remove(index);
            delete splitter;

            AreaIndex *parent = index->parent();
            QSplitter *parentSplitter = m_indexSplitters[parent];

            AreaIndex *sibling = parent->first() == index ? parent->second() : parent->first();
            QSplitter *siblingSplitter = m_indexSplitters[sibling];

            parentSplitter->setUpdatesEnabled(false);
            //save sizes and orientation of the sibling splitter
            parentSplitter->setOrientation(siblingSplitter->orientation());
            QList<int> sizes = siblingSplitter->sizes();

            /* Parent has two children -- 'index' that we've deleted and
               'sibling'.  We move all children of 'sibling' into parent,
               and delete 'sibling'.  sibling either contains a single
               Container instance, or a bunch of further QSplitters.  */
            while (siblingSplitter->count() > 0)
            {
                //reparent contents into parent splitter
                QWidget *siblingWidget = siblingSplitter->widget(0);
                siblingWidget->setParent(parentSplitter);
                parentSplitter->addWidget(siblingWidget);
            }

            m_indexSplitters.remove(sibling);
            delete siblingSplitter;

            parentSplitter->setSizes(sizes);
            parentSplitter->setUpdatesEnabled(true);

            kDebug() << "after deleation " << parent << " has "
                         << parentSplitter->count() << " elements";

            //find the container somewhere to activate
            Container *containerToActivate = parentSplitter->findChild<Sublime::Container*>();
            //activate the current view there
            if (containerToActivate)
                return m_mainWindow->setActiveView(containerToActivate->viewForWidget(containerToActivate->currentWidget()));
        }
    }

    m_mainWindow->setActiveView(0L);
}

void MainWindowPrivate::toolViewAdded(Sublime::View */*toolView*/, Sublime::Position position)
{
    IdealToolViewCreator toolViewCreator(this);
    area->walkToolViews(toolViewCreator, position);
}

void MainWindowPrivate::aboutToRemoveToolView(Sublime::View *toolView, Sublime::Position /*position*/)
{
    if (!docks.contains(toolView))
        return;

    idealMainWidget->removeView(toolView);
    // TODO are Views unique?
    docks.removeAll(toolView);
}

void MainWindowPrivate::toolViewMoved(
    Sublime::View *toolView, Sublime::Position position)
{
    if (!docks.contains(toolView))
        return;

    idealMainWidget->moveView(toolView, positionToDockArea(position));
}

Qt::DockWidgetArea MainWindowPrivate::positionToDockArea(Position position)
{
    switch (position)
    {
        case Sublime::Left: return Qt::LeftDockWidgetArea;
        case Sublime::Right: return Qt::RightDockWidgetArea;
        case Sublime::Bottom: return Qt::BottomDockWidgetArea;
        case Sublime::Top: return Qt::TopDockWidgetArea;
        default: return Qt::LeftDockWidgetArea;
    }
}

void MainWindowPrivate::switchToArea(QAction *action)
{
    kDebug() << "for" << action;
    controller->showArea(m_actionAreas[action], m_mainWindow);
}

void MainWindowPrivate::updateAreaSwitcher(Sublime::Area *area)
{
    if (m_areaActions.contains(area))
        m_areaActions[area]->setChecked(true);
}

void MainWindowPrivate::activateFirstVisibleView()
{
    if (area->views().count() > 0)
        m_mainWindow->activateView(area->views().first());
}

bool MainWindowPrivate::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::FocusIn)
        idealMainWidget->centralWidgetFocused();

    return false;
}

void Sublime::MainWindowPrivate::setStatusIcon(View * view, const QIcon & icon)
{
    if (viewContainers.contains(view)) {
        Container* c = viewContainers[view];
        int index = c->indexOf(view->widget());
        if (index != -1) {
            c->setTabIcon(index, icon);
        }
    }
}

void MainWindowPrivate::widgetResized(IdealMainLayout::Role role, int thickness)
{
    area->setThickness(IdealMainLayout::positionForRole(role), thickness);
}

void MainWindowPrivate::widgetCloseRequest(QWidget* widget)
{
    if (widgetToView.contains(widget))
    {
        View *view = widgetToView[widget];
        area->removeView(view);
        delete view;
    }
}

}

#include "mainwindow_p.moc"

