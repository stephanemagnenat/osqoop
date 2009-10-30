/*

This file is part of the TSEAdExt board firmware.
It is released as an example only, as a part of Osqoop.
Osqoop, an open source software oscilloscope.
Copyright (C) 2006 Stephane Magnenat <stephane at magnenat dot net>
Laboratory of Digital Systems http://www.eig.ch/labsynum.htm
Engineering School of Geneva http://www.eig.ch

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
//  Fichier     : hal.c
//  Crée        : 14.11.2003
//  Modifié     : 26.02.2005
//  Auteur(s)   : G.Wisner
//  Cible       : FX2 (compilateur Keil)
//
//  Description : Abstraction CPU.
//
//#############################################################################



//#############################################################################
//
//                              SUPPORT TERMINAL
//
//#############################################################################


#include "fx2.h"
#include "fx2regs.h"
#include <types.h>
#include <string.h>  // fonction strlen()

#ifndef __TERM_PORT__
   #define __TERM_PORT__ 0
#endif

#if __TERM_PORT__ == 0
   #define RCTL PCON
   #define SBUF SBUF0
   #define SCON SCON0
   #define SCON_TI TI
   #define SCON_RI RI
#elif __TERM_PORT__ == 1
   #define RCTL EICON
   #define SBUF SBUF1
   #define SCON SCON1
   #define SCON_TI TI1
   #define SCON_RI RI1
#else
   #error
#endif

#define NCOL 8
static CHAR xdata StrBuf[10+(NCOL*5)+1];

#define Dig2Char(d) ((d) > 9) ? (d)+'A'-10 : (d)+'0'


//=============================================================================
// Routine     : itoa()
//
// Description : Convertit un entier en une chaîne de caractère et stock le
//               résultat vers la location pointée par l'argument "string".
//               L'argument "radix" est ignoré (représentation hexa toujours
//               utilisée).
//
// e           : value - valeur entière dont on veut la représentation
//               string - pointeur sur le tampon de destination (! à la taille)
//               radix - ignoré (hexa toujours considéré)
// s           : pointeur sur la chaîne (tampon de destination)
//=============================================================================
static PCHAR itoa(LONG value, PCHAR string /*, LONG radix*/)
{
   BYTE  xdata DigNum;
   PCHAR xdata pStr = string;

   if ((ULONG)value < 0x100) DigNum = 2;
   else if ((ULONG)value < 0x10000) DigNum = 4;
   else DigNum = 8;

   while (DigNum--) *pStr++ = (CHAR) Dig2Char((value >> (4*DigNum)) & 0xF);
   *pStr = '\0';

   return (string);
}

//=============================================================================
// Routine     : TermInit()
//
// Description : Initialise le port série avec les paramètres : 115200 bits/s ,
//               sans parité , sans contrôle de flux
//=============================================================================
VOID TermInit(VOID)
{
   SCON  = 0x50;
   UART230 |= (1 << __TERM_PORT__);
   RCTL &= ~0x80;
   SCON_TI = 1;
}

//=============================================================================
// Routine     : TermGetc()
//
// Description : Lecture d'un caractère à partir de l'interface de communica-
//               tion série asynchrone (polling mode). Cette routine est non
//               blocante. Si il n'y a pas de caractère disponible lors de
//               l'appel, la routine retourne -1.
//
// s           : caractère obtenu de l'interface (-1 si pas de carac. dispo.)
//=============================================================================
INT TermGetc(VOID)
{
   INT c = (INT)(-1);

   if (SCON_RI) {
      c = (INT)SBUF;
      SCON_RI = 0;
   }
   return c;
}

//=============================================================================
// Routine     : TermPutc()
//
// Description : Ecriture d'un caractère vers l'interface de communication sé-
//               rie asynchrone (polling mode). Cette routine est blocante.
//               Elle ne retourne que lorsque le caractère transmis en argument
//               a pu être écrit vers l'interface.
//
// e           : c - caractère à écrire
// s           : caractère écrit
//=============================================================================
CHAR TermPutc(CHAR c)
{
   if (c == '\n')  {
      while (!SCON_TI);
      SCON_TI = 0;
      SBUF = '\r';
   }
   while (!SCON_TI);
   SCON_TI = 0;
   return (SBUF = c);
}

//=============================================================================
// Routine     : TermPuts()
//
// Description : Ecriture d'une chaîne de caractères vers l'interface de com-
//               munication série asynchrone (polling mode). Le caractère de
//               fin de chaîne n'est pas transmis.
//
//               remarque : TermPuts() ajoute l'échappement '\n' à la chaîne.
//
// e           : string - chaîne de caractères à écrire vers l'interface
//=============================================================================
VOID TermPuts(const PCHAR string)
{
   //EnterCS();
   while (*string) TermPutc(*string++);
   //ExitCS();
}

//=============================================================================
// Routine     : TermPuti()
//
// Description : Ecriture de la représentation hexa d'un nombre vers l'inter-
//               face de communication série asynchrone (polling mode).
//
// e           : v - nombre dont on veut transmettre la représentation
//=============================================================================
VOID TermPuti(DWORD v)
{
   StrBuf[0] = '0';
   StrBuf[1] = 'x';
   itoa(v, &StrBuf[2] /*, 16*/);
   TermPuts(StrBuf);
}

//=============================================================================
// Routine     : TermDump()
//
// Description : Dump mémoire via l'interface de communication série async.
//               Cette routine permet de visualiser sur un terminal "Len" oc-
//               tets mémoire à partir de l'adresse de base "pData".
//
// e           : pData - adresse de base de la zone mémoire à visualiser
//               Len   - nombre d'octets à visualiser
//=============================================================================
VOID TermDump(PBYTE pData, BYTE Len)
{
   BYTE  xdata Col;
   PCHAR xdata pStr;

   while (Len) {

      // adresse
      pStr = StrBuf;
      *pStr++ = '@';
      *pStr++ = ' ';
      *pStr++ = '0';
      *pStr++ = 'x';
      pStr += strlen(itoa((WORD)pData, pStr /*, 16*/));
	  *pStr++ = ' ';
	  *pStr++ = '|';

	  // 8 octets à compter de l'adresse
	  for (Col = 0; Col < NCOL; Col++) {
         *pStr++ = ' ';
         *pStr++ = '0';
         *pStr++ = 'x';
         itoa(*(PBYTE)pData++, pStr /*, 16*/);
		 pStr += 2;
		 if ( --Len == 0 ) break;
      }

      *pStr = '\0';
      TermPuts(StrBuf);
   }
}




