/*

This file is part of the TSEAdExt board firmware.
It is released as an example only, as a part of Osqoop.
Osqoop, an open source software oscilloscope.
Copyright (C) 2006--2009 Stephane Magnenat <stephane at magnenat dot net>
http://stephane.magnenat.net
Laboratory of Digital Systems
http://www.eig.ch/fr/laboratoires/systemes-numeriques/
Engineering School of Geneva
http://hepia.hesge.ch/
See authors file in source distribution for details about contributors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

//#############################################################################
//
//  HES-SO EIG Laboratoire de Système Numériques
//
//  Fichier     : types.h
//  Crée        : 31.12.2003
//  Modifié     : 30.12.2004
//  Auteur(s)   : G.Wisner
//  Cible       : FX2 / Carte Cyclone (compilateur Keil)
//
//  Description : Définitions de types.
//
//#############################################################################

#ifndef __TYPES_H__
#define __TYPES_H__

#define VOID            void
#define PVOID           VOID*

//#define BOOL            char
#define TRUE            1
#define FALSE           0

#define CHAR            signed char
#define UCHAR           unsigned char
#define BYTE            unsigned char
#define PCHAR           CHAR*
#define PUCHAR          UCHAR*
#define PBYTE           BYTE*

#define SHORT           signed short
#define USHORT          unsigned short
#define WORD            unsigned short
#define PSHORT          SHORT*
#define PUSHORT         USHORT*
#define PWORD           WORD*

#define INT             signed int
#define UINT            unsigned int

#define LONG            signed long
#define ULONG           unsigned long
#define DWORD           unsigned long
#define PLONG           LONG*
#define PULONG          ULONG*
#define PDWORD          DWORD*


#endif // !defined(__TYPES_H__)
