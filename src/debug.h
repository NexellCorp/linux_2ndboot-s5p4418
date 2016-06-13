/*
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved
 *      Nexell Co. Proprietary & Confidential
 *
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR
 *	FITNESS
 *      FOR A PARTICULAR PURPOSE.
 *
 *      Module          : 
 *      File            : debug.h
 *      Description     :
 *      Author          : Hans
 *      History         : 
 */
#ifndef __DEBUG_H__
#define __DEBUG_H__

//#include <stdio.h>
//#include <stdarg.h>
#include <nx_type.h>

CBOOL DebugInit(U32 port);
void DebugPutch(S8 ch);
// S8      DebugGetch( void );
CBOOL DebugIsUartTxDone(void);
#if 0
CBOOL	DebugIsTXEmpty(void);
CBOOL   DebugIsBusy( void );

void    DebugPutString( const S8* const String );
S32     DebugGetString( S8* const pStringBuffer );

void    DebugPutDec( S32 value );
void    DebugPutHex( S32 value );
void    DebugPutByte( S8 value );
void    DebugPutWord( S16 value );
void    DebugPutDWord( S32 value );

//void    DebugPrint( const S8* const FormatString, ... );
#endif
#endif // __DEBUG_H__
