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
* Description:  Library header.
*
*/


#ifndef FTUSTATEPROVIDER_GLOBAL_H
#define FTUSTATEPROVIDER_GLOBAL_H

#include <Qt/qglobal.h>


#ifdef FTUSTATEPROVIDER_LIB
#define FTUSTATEPROVIDER_EXPORT Q_DECL_EXPORT
#else
#ifdef FTUSTATEPROVIDER_TEST
        #define FTUSTATEPROVIDER_EXPORT
    #else
        #define FTUSTATEPROVIDER_EXPORT Q_DECL_IMPORT
    #endif	
#endif

#endif // FTUSTATEPROVIDER_GLOBAL_H
