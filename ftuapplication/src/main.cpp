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
* Description:  First Time Use application main implementation.
*
*/


#include "ftufirsttimeuse.h"
#include "ftutest_global.h"

#include <QtGui>
#include <hbapplication.h>
#include <QTranslator>
#include <QLocale>

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
//
int main(int argc, char *argv[])
{
    FTUTEST_FUNC_ENTRY("FTU::FtuFirstTimeUse::main");
    
    HbApplication app(argc, argv);    

    QTranslator translator;
    
    QString translation = "ftu_";
    if(argc > 1){
        translation.append(argv[1]);
    }
    else{
        // another option to load locale could be QLocale::system().language())
        translation.append(QLocale::system().name()); // this seems to be ftu_en_US in Windows and ftu_en_GB in WINSCW emulator and hardware
    }
    translator.load(translation, ":/translations" );
    app.installTranslator(&translator);

    QString exampleWizardsTranslation = "ftuexamplewizards_";
    if(argc > 1){
        exampleWizardsTranslation.append(argv[1]);
    }
    else{
        exampleWizardsTranslation.append(QLocale::system().name());
    }
    QTranslator exampleWizardsTranslator;

    exampleWizardsTranslator.load(exampleWizardsTranslation, ":/translations" );
    app.installTranslator(&exampleWizardsTranslator);

    FtuFirstTimeUse ftuFirstTimeUse;
#ifdef ROM
    QDir::setCurrent("Z:/");    
#else
    QDir::setCurrent("C:/");
    FTUDEBUG("main() - FtuApplication's current dir set to C:/");
#endif //ROM
    QObject::connect(&app,SIGNAL(aboutToQuit()),&ftuFirstTimeUse,SLOT(stop()));
    QObject::connect(&ftuFirstTimeUse, SIGNAL(exit()), &app, SLOT(quit()),Qt::QueuedConnection);    
    ftuFirstTimeUse.start();
    int ret = app.exec();
    FTUTEST_FUNC_EXIT("FTU::FtuFirstTimeUse::main");
    return ret;       
}
