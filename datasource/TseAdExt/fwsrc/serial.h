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
//  Fichier     : serial.h
//  Crée        : ?
//  Modifié     : ?
//  Auteur(s)   : ?
//  Cible       : FX2 (compilateur Keil)
//
//  Description : serial dump features
//
//#############################################################################

#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "types.h"


VOID TermInit(VOID);
INT  TermGetc(VOID);
CHAR TermPutc(CHAR c);
VOID TermPuts(const PCHAR string);
VOID TermPuti(DWORD v);
VOID TermDump(PBYTE pData, BYTE Len);

#endif // !defined(__SERIAL_H__)

