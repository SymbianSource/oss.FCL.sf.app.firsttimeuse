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
* Description:  Implementation of the wizard menu state.
*
*/


#include "ftuwizardmenustate.h"
#include "ftucontentservice.h"
#include <ftuwizard.h>

#include <hbmainwindow.h>
#include <hbview.h>
#include <hblabel.h>
#include <hbicon.h>
#include <hbinstance.h>
#include <hblistview.h>
#include <hbdocumentloader.h>

#include <QStandardItemModel>
#include <QDate>

#include <xqsettingsmanager.h>

#include "ftustatecenrephandler.h"

const int progressCompelete = 100;
const char* emptyLine = " ";

#define WMS_LOG qDebug() << "ftu:FtuWizardMenuState"

const char *FTUSTATEPROVIDER_DOCML = ":/xml/ftustateprovider.docml";
const char *TOC_VIEW = "tocView";
const char *TOC_INFOTEXT_LABEL = "tocInfoTextLabel";
const char *TOC_LIST_VIEW = "tocListView";


// ---------------------------------------------------------------------------
// FtuWizardMenuState::FtuWizardMenuState
// ---------------------------------------------------------------------------
//
FtuWizardMenuState::FtuWizardMenuState(QState *parent) :
    QState(parent),
    mMainWindow(NULL),
    mTocView(NULL),
    mInfoText(NULL),
    mListView(NULL),
	mDocumentLoader(NULL),
    mModel(NULL)
{
    mMainWindow = hbInstance->allMainWindows().at(0);
    mModel = new QStandardItemModel(this);
    
    //TODO Read number of wizards supported and init the following list
    //mCompletedWizardList << false << false << false << false << false <<false;

	mDocumentLoader = new HbDocumentLoader();
	bool ok = false;
	mDocumentLoader->load(FTUSTATEPROVIDER_DOCML, &ok);
	QGraphicsWidget *widget = mDocumentLoader->findWidget(TOC_VIEW);
	Q_ASSERT_X(ok && (widget != 0), "ftustateprovider", "invalid DocML file");
	mTocView = qobject_cast<HbView*>(widget);

    mTocView->setTitle(qtTrId("txt_ftu_title_setup"));

	mMainWindow->addView(mTocView);
    // Set as initial view.
    mMainWindow->setCurrentView(mTocView);
    mCenrepHandler = new FtuStateCenrepHandler(NULL);
    int registeredPlugins = mCenrepHandler->registeredPluginCount();
    for(int counter = 0; counter < registeredPlugins; counter ++){
        mCompletedWizardList << mCenrepHandler->getPluginInfo(counter);
    }
        
}

// ---------------------------------------------------------------------------
// FtuWizardMenuState::~FtuWizardMenuState
// ---------------------------------------------------------------------------
//
FtuWizardMenuState::~FtuWizardMenuState()
{
	if(mModel)
	{
		delete mModel;
	}
	delete mDocumentLoader;
	delete mCenrepHandler;
}

// ---------------------------------------------------------------------------
// FtuWizardMenuState::onEntry
// ---------------------------------------------------------------------------
//
void FtuWizardMenuState::onEntry(QEvent *event)
{    
    WMS_LOG << "::onEntry";
    QState::onEntry(event);

    if(!mInfoText)
    {    
        createInfoText();
    }    
    if(!mListView)
    {
        createTocList();
    }

    // If this is not the current view, we're getting back from plugin view
    if(mMainWindow->currentView() != mTocView)
    {
        mMainWindow->setCurrentView(mTocView, true);
    }

    mMainWindow->show();
}

// ---------------------------------------------------------------------------
// FtuWizardMenuState::onExit
// ---------------------------------------------------------------------------
//
void FtuWizardMenuState::onExit(QEvent *event)
{
    //remove the options menu so that plugin can perform the cleanup
    mMainWindow->currentView()->takeMenu();
    QState::onExit(event);
}

// ---------------------------------------------------------------------------
// FtuWizardMenuState::content
// ---------------------------------------------------------------------------
//
FtuContentService *FtuWizardMenuState::content() const
{
    return property(FTU_CONTENT_SERVICE_KEY).value<FtuContentService*>();
}

// ---------------------------------------------------------------------------
// FtuWizardMenuState::updatedAsString
// ---------------------------------------------------------------------------
//
QString FtuWizardMenuState::updatedAsString(const QDate& date) const
{
    QString status;

    if(date == QDate().currentDate() )
    { 	
        status.append(qtTrId("txt_ftu_list_update_today"));
    }
    else
    {
        status.append(qtTrId("txt_ftu_list_update_date").arg(date.toString()));
    }
    return status;
}

// ---------------------------------------------------------------------------
// FtuWizardMenuState::addWizardToListModel
// ---------------------------------------------------------------------------
//
void FtuWizardMenuState::addWizardToListModel(int aIndex)
{    
    WMS_LOG << "::addWizardToListModel idx : " << aIndex;
    FtuContentService* ftuContentService = content();

    FtuWizard* addedWizard = ftuContentService->wizard(aIndex);
    if(addedWizard)
    {    
        // Connect for progress updates.
        connect(addedWizard, SIGNAL(progressUpdated(FtuWizard *, bool, int)), 
                this, SLOT(updateProgress(FtuWizard *, bool, int)));

    	const FtuWizardSetting& settings = addedWizard->wizardSettings();
	    QStandardItem* newItem = new QStandardItem();
	    HbIcon icon (settings.mTocDefaultIcon.absoluteFilePath());
        QStringList data;
        data << settings.mTocLabel;
        QDate date = addedWizard->wizardCompletedDate();
        if(date.isNull())
        {
            data << emptyLine;
        }
        else
        {
            data << updatedAsString(date);
        }

        newItem->setBackground(QBrush(Qt::lightGray));
        newItem->setData(icon, Qt::DecorationRole);
        newItem->setData(QVariant(data), Qt::DisplayRole);

        mModel->appendRow(newItem);        
    }
}

// ---------------------------------------------------------------------------
// FtuWizardMenuState::createTocList
// ---------------------------------------------------------------------------
//
void FtuWizardMenuState::createTocList()
{
    if (!mListView)
    {
		mListView = qobject_cast<HbListView *>(mDocumentLoader->findWidget(
															TOC_LIST_VIEW));
	    connect(mListView,
                SIGNAL(activated(const QModelIndex)),
                this,
                SLOT(activateWizard(const QModelIndex)));

        mListView->setModel(mModel);
    }
}

// ---------------------------------------------------------------------------
// FtWizardMenuState::createInfoText
// ---------------------------------------------------------------------------
//
void FtuWizardMenuState::createInfoText()
{
    mInfoText = qobject_cast<HbLabel *>(mDocumentLoader->findWidget(TOC_INFOTEXT_LABEL));                               
    mInfoText->setPlainText(qtTrId("txt_ftu_subtitle_toc"));
}

// ---------------------------------------------------------------------------
// FtuWizardMenuState::activateWizard
// ---------------------------------------------------------------------------
//
void FtuWizardMenuState::activateWizard(const QModelIndex index)
{
    //remove the main menu 
    mMainWindow->currentView()->takeMenu();
    // Set the active wizard index
    content()->setActiveWizard(index.row());
    FtuWizard * wizard=content()->wizard(index.row());
    
    // signal to activated state
    emit wizardSelected();
}

// ---------------------------------------------------------------------------
// FtuWizardMenuState::updateProgress
// ---------------------------------------------------------------------------
//
void FtuWizardMenuState::updateProgress(FtuWizard *caller, bool show, 
                                           int progress)
{
    Q_UNUSED(show)
    QList<FtuWizard*> wizards = content()->wizards();
    int index = -1;
    
    // Get the index of the wizard
    for(int i = 0 ; i < wizards.size() ; ++i)
    {
        if(caller == wizards[i])
        {
            WMS_LOG << "::updateProgress wizard found at: " << i;
            index = i;
        }
    }
    if(index != -1)
    {  
        QStringList data;
        data << wizards[index]->wizardSettings().mTocLabel;           
        
        if(progress < progressCompelete)
        {
            QString progressStr;
            QString progressNumber;
            progressNumber.setNum(progress);
            progressStr = qtTrId("txt_ftu_list_progress_status").arg(progressNumber);
            data << progressStr;
        }
        else
        {         
            QDate date = wizards[index]->wizardCompletedDate();
            data << updatedAsString(date);            

            XQSettingsManager settingsManager;
            
            if(false == mCompletedWizardList[index])
            {
            mCompletedWizardList[index] = true; 
            mCenrepHandler->updatePluginInfo(index);
            }

        }
        mModel->item(index)->setData(QVariant(data), Qt::DisplayRole);
    }
}
