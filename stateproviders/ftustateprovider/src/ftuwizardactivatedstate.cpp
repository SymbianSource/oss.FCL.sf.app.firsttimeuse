/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Implements the wizard activation and interactions.
*
*/


#include "ftuwizardactivatedstate.h"
#include "fturuntimeservices_global.h"
#include "ftucontentservice.h"
#include <ftuwizard.h>

#include <hbmainwindow.h>
#include <hbdocumentloader.h>
#include <hbview.h>
#include <hblabel.h>
#include <hbstackedwidget.h>
#include <hbicon.h>
#include <hbinstance.h>
#include <hbaction.h>
#include <hbgridview.h>
#include <QStandardItemModel>
#include <QDebug>
#include <QGraphicsWidget>
#include <QDir>

const int gridRowCount = 1;

const char *FTUSTATEPROVIDER_DOCML2 = ":/xml/ftustateprovider.docml";
const char *WIZARD_VIEW = "wizardView";
const char *WIZARD_INFOTEXT_LABEL = "wizardInfoTextLabel";
const char *WIZARD_STACKEDWIDGET = "wizardStackedWidget";
const char *WIZARD_GRID_VIEW = "wizardGridView";

/*
#define LOG_GEOM(txt, r) logGeometry(txt, r)

static void logGeometry(const char *txt, const QRectF& r)
{
    qDebug() << "ftu:" << txt << " x: " << r.topLeft().x() << " y: "  << r.topLeft().y() 
             << " w: " << r.width() << " h: " << r.height();
}
*/

// ---------------------------------------------------------------------------
// FtuWizardActivatedState::FtuWizardActivatedState
// ---------------------------------------------------------------------------
//
FtuWizardActivatedState::FtuWizardActivatedState(QState *parent) :
    QState(parent),
    mMainWindow(NULL),
    mDocumentLoader(NULL),
    mPluginView(NULL),
    mActiveWizard(NULL),
    mPluginTitleLabel(NULL),
    mWizardStackedWidget(NULL),
    mBackAction(NULL),
    mMenustrip(NULL), 
    mPreviousView(NULL), 
    mCurrentView(NULL),
    mPluginDisplayMode(PartialScreen) 
{
    mMainWindow = hbInstance->allMainWindows().at(0);
    mDocumentLoader = new HbDocumentLoader();
    mPluginView = new HbView();

    bool ok = false;
	mDocumentLoader->load(FTUSTATEPROVIDER_DOCML2, &ok);
	QGraphicsWidget *widget = mDocumentLoader->findWidget(WIZARD_VIEW);
	Q_ASSERT_X(ok && (widget != 0), "ftustateprovider", "invalid DocML file");

    mPluginView->setWidget(widget);
    mMainWindow->addView(mPluginView);

    mPluginView->setTitle(qtTrId("txt_ftu_title_setup"));

    mPluginTitleLabel = qobject_cast<HbLabel *>(mDocumentLoader->findWidget(WIZARD_INFOTEXT_LABEL)); 

    mWizardStackedWidget = qobject_cast<HbStackedWidget *>(mDocumentLoader->findWidget(WIZARD_STACKEDWIDGET));

    QString path = QDir::currentPath();
    
    mBackAction = new HbAction(Hb::BackAction, this);
    
}

// ---------------------------------------------------------------------------
// FtuWizardActivatedState::~FtuWizardActivatedState
// ---------------------------------------------------------------------------
//
FtuWizardActivatedState::~FtuWizardActivatedState()
{
    delete mDocumentLoader;

	if(mBackAction)
	{		
		delete mBackAction;
	}
}

// ---------------------------------------------------------------------------
// FtuWizardActivatedState::onEntry
// ---------------------------------------------------------------------------
//
void FtuWizardActivatedState::onEntry(QEvent *event)
{
    qDebug() << "ftu:FtuWizardActivatedState::onEntry";
    QState::onEntry(event);    

    int activeIndex = content()->activeWizard();
    mActiveWizard = content()->wizard(activeIndex);
    
    mPluginView->setNavigationAction(mBackAction);
    // Make menustrip for testing purposes.
    if(!mMenustrip)
    {
        constructGrid();
    }
    mMenustrip->setVisible(true);

    connect(mBackAction, SIGNAL(triggered()),
            this, SLOT(handleBackEvent()));

    setActiveWizardConnections();

    mActiveWizard->activateWizard();
}

// ---------------------------------------------------------------------------
// FtuWizardActivatedState::onExit
// ---------------------------------------------------------------------------
//
void FtuWizardActivatedState::onExit(QEvent *event)
{
    QState::onExit(event);
    
    disconnect(mBackAction, SIGNAL(triggered()),
               this, SLOT(handleBackEvent()));
    
    disconnect(mActiveWizard, SIGNAL(viewChanged(FtuWizard *, 
                                                 QGraphicsWidget* )),
               this, SLOT(changeWizardView(FtuWizard*, QGraphicsWidget*)));
    
    disconnect(mActiveWizard, SIGNAL(fullScreenModeRequested(FtuWizard *)),
               this, SLOT(enableFullScreenMode(FtuWizard*)));
    
    disconnect(mActiveWizard, SIGNAL(partialScreenModeRequested(FtuWizard * )),
               this, SLOT(enablePartialScreenMode(FtuWizard*)));

    disconnect(mActiveWizard, SIGNAL(infoTextUpdated(FtuWizard *, QString )),
            this, SLOT(updateInfoText(FtuWizard *, QString)));
}

// ---------------------------------------------------------------------------
// FtuWizardActivatedState::setActiveWizardConnections
// ---------------------------------------------------------------------------
//
void FtuWizardActivatedState::setActiveWizardConnections()
{
    // first disconnect possible old connections with active wizard
    disconnect(mActiveWizard, SIGNAL(viewChanged(FtuWizard *, 
                                                 QGraphicsWidget* )),
               this, SLOT(changeWizardView(FtuWizard*, QGraphicsWidget*)));
    
    disconnect(mActiveWizard, SIGNAL(fullScreenModeRequested(FtuWizard *)),
               this, SLOT(enableFullScreenMode(FtuWizard*)));
    
    disconnect(mActiveWizard, SIGNAL(partialScreenModeRequested(FtuWizard * )),
               this, SLOT(enablePartialScreenMode(FtuWizard*)));

    disconnect(mActiveWizard, SIGNAL(infoTextUpdated(FtuWizard *, QString )),
            this, SLOT(updateInfoText(FtuWizard *, QString)));

    // then connect new ones to active wizard
    connect(mActiveWizard, SIGNAL(viewChanged(FtuWizard *, QGraphicsWidget* )),
            this, SLOT(changeWizardView(FtuWizard*, QGraphicsWidget*)));
    
    connect(mActiveWizard, SIGNAL(fullScreenModeRequested(FtuWizard *)),
            this, SLOT(enableFullScreenMode(FtuWizard*)));
    
    connect(mActiveWizard, SIGNAL(partialScreenModeRequested(FtuWizard * )),
            this, SLOT(enablePartialScreenMode(FtuWizard*)));
    
    connect(mActiveWizard, SIGNAL(infoTextUpdated(FtuWizard *, QString )),
            this, SLOT(updateInfoText(FtuWizard *, QString)));
}

// ---------------------------------------------------------------------------
// FtuWizardActivatedState::content
// ---------------------------------------------------------------------------
//
FtuContentService *FtuWizardActivatedState::content() const
{
    return property(FTU_CONTENT_SERVICE_KEY).value<FtuContentService*>();
}

// ---------------------------------------------------------------------------
// FtuWizardActivatedState::handleBackEvent
// ---------------------------------------------------------------------------
//

void FtuWizardActivatedState::handleBackEvent()
{
    if(!mActiveWizard->handleBackEvent())
    {
        emit backEventTriggered();
    }    
}

// ---------------------------------------------------------------------------
// FtuWizardActivatedState::changeWizardView
// ---------------------------------------------------------------------------
//
void FtuWizardActivatedState::changeWizardView(FtuWizard *caller, 
                                            QGraphicsWidget* viewWidget)
{
    if(caller == mActiveWizard)        
    {
        if(viewWidget)
        {
            if (mWizardStackedWidget->indexOf(viewWidget) == -1)
            {
                // add wizard's widget to stacked widget if not yet there
                mWizardStackedWidget->addWidget(viewWidget);
            }
            // set wizard's widget as current widget
            mWizardStackedWidget->setCurrentWidget(viewWidget);
            
            qDebug() << "Ftu: switching view due plugin view change";
            mainWindow()->setCurrentView(mPluginView, true);
            
            QList<FtuWizard*> wizards = content()->wizards();
            // get index of active wizard
            int index = wizards.indexOf(mActiveWizard);
            
            // temp solution to skip edge indexes - start
            if (index == 0) { index++; }
            if (index == wizards.count()-1) { index--; }
            // temp solution to skip edge indexes - end  
                      
            // create model index for active wizard
            QModelIndex modelIndex = mMenustrip->model()->index(index, 0);
            // scroll to correct position in menustrip
            HbAbstractItemView::ScrollHint hint = HbAbstractItemView::PositionAtCenter;
            mMenustrip->scrollTo(modelIndex, hint);
        }        
    }
}

// ---------------------------------------------------------------------------
// FtuWizardActivatedState::enableFullScreenMode
// ---------------------------------------------------------------------------
//
void FtuWizardActivatedState::enableFullScreenMode(FtuWizard *caller)
{
    if(caller == mActiveWizard)
    {
        mPluginDisplayMode = FullScreen;
        // Remove menustrip
		mMenustrip->setVisible(false);
        caller->resizeWizard(calculateWizardGeometry()); 
    }    
}

// ---------------------------------------------------------------------------
// FtuWizardActivatedState::enablePartialScreenMode
// ---------------------------------------------------------------------------
//
void FtuWizardActivatedState::enablePartialScreenMode(FtuWizard *caller)
{
    if(caller == mActiveWizard)
    {
        mPluginDisplayMode = PartialScreen;
        mMenustrip->setVisible(true);
        caller->resizeWizard(calculateWizardGeometry());
    }
}

// ---------------------------------------------------------------------------
// FtuWizardActivatedState::updateInfoText
// ---------------------------------------------------------------------------
//
void FtuWizardActivatedState::updateInfoText(FtuWizard *caller, 
                                                QString text)
{
    if(caller == mActiveWizard)
    {
        mPluginTitleLabel->setPlainText(text);
    }    
}

// ---------------------------------------------------------------------------
// FtuWizardActivatedState::mainWindow
// ---------------------------------------------------------------------------
//
HbMainWindow* FtuWizardActivatedState::mainWindow()
{
    return hbInstance->allMainWindows().at(0);
}

// ---------------------------------------------------------------------------
// FtuWizardActivatedState::constructGrid
// ---------------------------------------------------------------------------
//
void FtuWizardActivatedState::constructGrid()
{
    // fetch grid view from docml
    mMenustrip = qobject_cast<HbGridView *>(mDocumentLoader->findWidget(WIZARD_GRID_VIEW));
    // set row amount for grid
    mMenustrip->setRowCount(gridRowCount);

    mMenustrip->setScrollDirections(Qt::Horizontal);

    FtuContentService* ftuContentService = content();
    // get wizards
    QList<FtuWizard*> wizards = ftuContentService->wizards();
    
    QStandardItemModel* model = new QStandardItemModel(this);
    
    for(int i = 0 ; i < wizards.size() ; ++i)
    {
        // get wizard settings
        const FtuWizardSetting& settings = wizards.at(i)->wizardSettings();
        QStandardItem* item = new QStandardItem();
        HbIcon icon(settings.mMenustripDefaultIcon.absoluteFilePath());

        item->setBackground(QBrush(Qt::lightGray));
        item->setData(icon, Qt::DecorationRole);
        
        QStringList data;
        data << settings.mMenustripLabel;
        
        item->setData(QVariant(data), Qt::DisplayRole);

        model->appendRow(item);
    }
    // set above defined model for menustrip
    mMenustrip->setModel(model);

    connect(mMenustrip, SIGNAL(activated(const QModelIndex)),
            this, SLOT(activateWizard(const QModelIndex)));
}

// ---------------------------------------------------------------------------
// FtuWizardActivatedState::activateWizard
// ---------------------------------------------------------------------------
//
void FtuWizardActivatedState::activateWizard(const QModelIndex index)
{   
    // get index for selected wizard
    int wizardIndex = index.row();
    if (wizardIndex != -1)
    {
        // check if other wizard than current is activated
        if (mActiveWizard != content()->wizard(wizardIndex))
        {
            // first deactivate current active wizard
		    if(mActiveWizard)
		    {
			    mActiveWizard->deactivateWizard();
		    }
            // set selected wizard active (precaution, as state does not change)
            content()->setActiveWizard(wizardIndex);
            // save active wizard
            mActiveWizard = content()->wizard(wizardIndex);
            // reset connections for new active wizard
            setActiveWizardConnections();
            // set new active wizard visible
            mActiveWizard->activateWizard();
        }
        
        // temp solution to skip edge indexes - start
        if (wizardIndex == 0) { wizardIndex++; }
        if (wizardIndex == content()->wizards().count()-1) { wizardIndex--; }
        QModelIndex modelIndex = mMenustrip->model()->index(wizardIndex, 0);
        // temp solution to skip edge indexes - end         
              
        HbAbstractItemView::ScrollHint hint = HbAbstractItemView::PositionAtCenter;
        // scroll to correct position in menustrip
        mMenustrip->scrollTo(modelIndex, hint);
    }
}

// ---------------------------------------------------------------------------
// FtuWizardActivatedState::calculateWizardGeometry
// ---------------------------------------------------------------------------
//
QRectF FtuWizardActivatedState::calculateWizardGeometry()
{
	QRectF widgetRect = mWizardStackedWidget->geometry();
	QRectF pluginRect = mPluginView->geometry();	
    if(mPluginDisplayMode == FullScreen)
    {		
		pluginRect.setSize(QSizeF(widgetRect.width(),
								  (widgetRect.height() +
						           mMenustrip->geometry().height())));
    }
	else
	{	
		pluginRect.setSize(QSizeF(widgetRect.width(),
			                      widgetRect.height()));	
	}

    return pluginRect;
}
