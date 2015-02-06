////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2009 Nexell Co., Ltd All Rights Reserved
//  Nexell Co. Proprietary & Confidential
//
//  Nexell informs that this code and information is provided "as is" base
//  and without warranty of any kind, either expressed or implied, including
//  but not limited to the implied warranties of merchantability and/or fitness
//  for a particular puporse.
//
//
//  Module          :
//  File            : buildinfo.c
//  Description     :
//  Author          : Hans
//  History         :
//          2013-12-29  Hans
////////////////////////////////////////////////////////////////////////////////

#include "printf.h"
//------------------------------------------------------------------------------
void buildinfo(void)
{
    printf( "\r\n" );
    printf( "--------------------------------------------------------------------------------\r\n" );
    printf( " Second Boot by Nexell Co. : Ver0.5 - Built on %s %s\r\n", __DATE__, __TIME__ );
    printf( "--------------------------------------------------------------------------------\r\n" );
}
