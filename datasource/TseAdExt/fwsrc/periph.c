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

#pragma NOIV               // Do not generate interrupt vectors
//-----------------------------------------------------------------------------
//  File:      periph.c
//  Contents:   Firmware for TSEADEX data acquisition board for TSE Project
//
//	Copyright (c) 2005 HES-SO EIG Laboratoire de Système Numériques
//	Damien Baumann and Stéphane Magnenat
//
//-----------------------------------------------------------------------------
#include "fx2.h"
#include "fx2regs.h"
#include "fx2sdly.h"  //pour SYNCDELAY
#include "debug.h"

// Firmware version number
#define REVISION_NUMBER "7"

extern void GpifInit_multiChannel( void );
extern void GpifInit_singleChannel( void );

extern BOOL   GotSUD;         // Received setup data flag
extern BOOL   Sleep;
extern BOOL   Rwuen;
extern BOOL   Selfpwr;

BYTE   Configuration;      // Current configuration
BYTE   AlternateSetting;   // Alternate settings

//En définissant SINGLE_CHANNEL_OPTIMISATION,
//une optimisation est effectuée pour la lecture d'un seul canal.
//Sans cette définition, la technique utilisée pour un seul
//canal est la même que pour 2-8 canaux
#define SINGLE_CHANNEL_OPTIMISATION

//Permet l'affichage de messages de debug sur la ligne série
#define DEBUG_MESSAGES

// EP1-OUT bit for EPIE and EPIRQ
#define bmEP1OUT bmBIT3

// Byte set to 1 by EP1OUT ISR
BYTE	NewDataEP1OUT;

//-----------------------------------------------------------------------------
// Task Dispatcher hooks
//   The following hooks are called by the task dispatcher.
//-----------------------------------------------------------------------------

void TD_Init(void)             // Called once at startup
{
	TermInit();
	TermPuts("Firmware for TSEADEX data acquisition board for TSE Project\n");
	TermPuts("HES-SO EIG Laboratoire de Systeme Numeriques, 2005\n");
	TermPuts("Damien Baumann and Stephane Magnenat\n\n");
	TermPuts("Revision ");
	TermPuts(REVISION_NUMBER);
	TermPuts(" - ");
	TermPuts(__DATE__);
	TermPuts("\n\n");
	DEBUG_PRINT("*** DEBUG MODE ***\n\n");
	
	//CLKOUT inactive et CPU à 48 Mhz (page 15-13)
	CPUCS = 0x10;	
	SYNCDELAY;
	
	//Chip revision control. DYN_OUT et ENH_PKT (page 15-24)
	REVCTL = 0x03;
	SYNCDELAY;
	
	//IFLCK sort à 48 Mhz (page 15-13)
	//on utilisera un diviseur par 4 sur la carte (jumper W4 en position 1-2) -> 12 Mhz
	IFCONFIG |= (1<<5); 	
	SYNCDELAY;	
	
	//initialisation de l'endpoint EP1OUT
	EP1OUTBC = 0x01; //armement (page 15-69) -> busy = 1
	SYNCDELAY;
	EP1OUTCFG = 0xA0;  //valid + bulk (page 15-26)
	SYNCDELAY;
	
	//initialisation de l'endpoint 6 pour être utilisé en mode GPIF
	EP6CFG = 0xE2 ;  //Valid, IN, BULK, 512o, double buffer (page 15-27)
	//EP6CFG = 0xE3 ;  //Valid, IN, BULK, 512o, triple buffer (page 15-27)
	//EP6CFG = 0xE0 ;  //Valid, IN, BULK, 512o, quad buffer (page 15-27)
	SYNCDELAY;
	EP6FIFOCFG = 0x0D; //AUTOIN=1, ZEROLENIN=1, WORDWIDE=(16 bits) (page 15-29)
	SYNCDELAY;
	EP6AUTOINLENH = 0x02; //auto commit 512 byte packets (page 15-31)
	SYNCDELAY;
	EP6AUTOINLENL = 0x00; //(page 15-31)                  
	SYNCDELAY;
	EP6GPIFPFSTOP = 0x00; //GPIF transitions to DONE when TC has been met (page 15-98) 
	SYNCDELAY;		
}

// Fonction d'acquisition des données du MAX125 via GPIF
// Lit continuellement tous les canaux pour toujours
void ProcessAllContinuousGpif()
{
	BYTE dummy;

	//armement de l'endpoint
	EP1OUTBC = 0x01;
	SYNCDELAY;
	
	//initialisation du GPIF
	GpifInit_multiChannel( );
	
	//GpifInit_singleChannel modifie IFCONFIG...
	IFCONFIG |= (1<<5);
	
	//reset des FIFOs (page 15-20)
	//ne pas le déplacer avant GpifInit
	FIFORESET = 0x80;
	SYNCDELAY;
	FIFORESET = 0x02;
	SYNCDELAY;
	FIFORESET = 0x04;
	SYNCDELAY;
	FIFORESET = 0x06;
	SYNCDELAY;
	FIFORESET = 0x08;
	SYNCDELAY;
	FIFORESET = 0x00;
	SYNCDELAY;
	
	//setup transaction count  (page 15-96)    
	//les 24 bits de poids fort sont toujours à 0
	//les 8 bits de poids faible seront initialisés par la suite (ne pas le faire avant le lancement
	//des autres modes que FIFORead, car le transaction count peut être alors décrémenté)
	GPIFTCB3 = 0;
	SYNCDELAY;
	GPIFTCB2 = 0;
	SYNCDELAY;
	GPIFTCB1 = 0;	
	SYNCDELAY;
				
	//dummy read to trigger GPIF (FIFO read transaction) : que CS et RD, pas CONVST
	//Objectif : si nINT était à 0, cela le remettra pas à 1 !!! (vécu)
	//on ne lit pas le résultat. On le met dans l'endpoint 4 qui n'est pas utilisé
	GPIFTCB0 = 1; //Transaction Count == 1
	SYNCDELAY;
	GPIFTRIG = 5 ; // R/W=1, EP[1:0]=1 (== EP4)
	SYNCDELAY;
	
	// Lecture pour toujours
	NewDataEP1OUT = 0; // For now, no new data on EP1OIT
	EPIRQ = bmEP1OUT;
	EPIE |= bmEP1OUT; // Enable Endpoint interrupt for EP1-OUT
	while (NewDataEP1OUT == 0)
	{
		// DEBUG_PRINT(".");
		// POUR LES 4 PREMIERS CANAUX
			
		//initialisation du mode de fonctionnement du convertisseur MAX125
		//trigger GPIF (single byte write transaction). Impulsion sur CS et WR
		XGPIFSGLDATLX = 3; 
		while( !( GPIFTRIG & 0x80 ) )
			{ }
			
		//trigger single read (impulsion sur CONVST et attente de nINT)
		dummy = XGPIFSGLDATLX; 
					 
		GPIFTCB0 = 4; //transaction count
					
		//trigger pour transfert GPIF FIFORead
		while( !( GPIFTRIG & 0x80 ) )
			{ }
	  	GPIFTRIG = 6 ; // R/W=1, EP[1:0]=2 (== EP6) 
					
		//attente de la fin du transfert GPIF
		while( !( GPIFTRIG & 0x80 ) )
			{ }		
			
		// POUR LES 4 DERNIERS CANAUX
			
		//initialisation du mode de fonctionnement du convertisseur MAX125
		//trigger GPIF (single byte write transaction). Impulsion sur CS et WR
		XGPIFSGLDATLX = 7; 
		while( !( GPIFTRIG & 0x80 ) )
			{ }
			
		//trigger single read (impulsion sur CONVST, puis attente de nINT)
		dummy = XGPIFSGLDATLX; 
					 
		GPIFTCB0 = 4;
					
		//trigger pour transfert GPIF FIFORead
		while( !( GPIFTRIG & 0x80 ) )
			{ }
	  	GPIFTRIG = 6 ; // R/W=1, EP[1:0]=2 (== EP6) 
	
		//attente de la fin du transfert GPIF
		while( !( GPIFTRIG & 0x80 ) )
			{ }
	}
	EPIE &= ~bmEP1OUT; // Disable Endpoint interrupt for EP1-OUT
}

//Fonction d'acquisition des données du MAX125 via GPIF
//Valable pour 1 à 8 canaux
//Pour 1 canal, il existe une méthode optimisée : ProcessSingleChannelGpif
void ProcessMultiChannelGpif()
{
	BYTE dummy;
	DWORD nbr_transferts; //nombre de samples à acquérir (au total, PAS par canal)
	UCHAR nbr_canaux;
	DWORD i;

	DEBUG_PRINT("ProcessMultiChannelGpif\n");

	//nombre de samples à acquérir (au total, PAS par canal)
	//le casting (DWORD) est vital !! (vécu)
	nbr_transferts = ((DWORD)EP1OUTBUF[4]<<24) | ((DWORD)EP1OUTBUF[3]<<16) | (EP1OUTBUF[2]<<8) | EP1OUTBUF[1];
				
	//nombre de canaux
	nbr_canaux = EP1OUTBUF[0];

	//armement de l'endpoint
	EP1OUTBC = 0x01;
	SYNCDELAY;
	
	//initialisation du GPIF
	GpifInit_multiChannel( );
	
	//GpifInit_singleChannel modifie IFCONFIG...
	IFCONFIG |= (1<<5);
	
	//reset des FIFOs (page 15-20)
	//ne pas le déplacer avant GpifInit
	FIFORESET = 0x80;
	SYNCDELAY;
	FIFORESET = 0x02;
	SYNCDELAY;
	FIFORESET = 0x04;
	SYNCDELAY;
	FIFORESET = 0x06;
	SYNCDELAY;
	FIFORESET = 0x08;
	SYNCDELAY;
	FIFORESET = 0x00;
	SYNCDELAY;
	
	//setup transaction count  (page 15-96)    
	//les 24 bits de poids fort sont toujours à 0
	//les 8 bits de poids faible seront initialisés par la suite (ne pas le faire avant le lancement
	//des autres modes que FIFORead, car le transaction count peut être alors décrémenté)
	GPIFTCB3 = 0;
	SYNCDELAY;
	GPIFTCB2 = 0;
	SYNCDELAY;
	GPIFTCB1 = 0;	
	SYNCDELAY;	
			
	//dummy read to trigger GPIF (FIFO read transaction) : que CS et RD, pas CONVST
	//Objectif : si nINT était à 0, cela le remettra pas à 1 !!! (vécu)
	//on ne lit pas le résultat. On le met dans l'endpoint 4 qui n'est pas utilisé
	GPIFTCB0 = 1; //Transaction Count == 1
	SYNCDELAY;
	GPIFTRIG = 5 ; // R/W=1, EP[1:0]=1 (== EP4)
	SYNCDELAY;
			
	if (nbr_canaux <= 4)
	{
		DEBUG_PRINT("ProcessMultiChannelGpif. <= 4\n");
	
		//initialisation du mode de fonctionnement du convertisseur MAX125
		//trigger GPIF (single byte write transaction). Impulsion sur CS et WR
		//si nINT était à 0, cela NE le remettrait PAS à 1 !!! (vécu)
		//mais ici nINT est forcément à égal à 1 à cause de la dummy read d'avant
		while( !( GPIFTRIG & 0x80 ) )
			{ }
		XGPIFSGLDATLX = nbr_canaux-1; 
				
		for(i=0;i<nbr_transferts/nbr_canaux + 1;i++)
		{
			//le +1 est dû au fait que
			//nbr_transferts n'est pas forcément un multiple de nbr_canaux
			//il veut donc mieux stocker trop de samples que pas assez
			
			//trigger single read (impulsion sur CONVST, puis attente de nINT)
			dummy = XGPIFSGLDATLX; 
					 
			GPIFTCB0 = nbr_canaux; //transaction count
					
			//trigger pour transfert GPIF FIFORead
			while( !( GPIFTRIG & 0x80 ) )
				{ }
		  	GPIFTRIG = 6 ; // R/W=1, EP[1:0]=2 (== EP6) 
					
			//attente de la fin du transfert GPIF
			while( !( GPIFTRIG & 0x80 ) )
				{ }
		}
	}
	else
	{
		DEBUG_PRINT("ProcessMultiChannelGpif. > 4\n");
	
		for(i=0;i<nbr_transferts/nbr_canaux + 1;i++)
		{		
			//le +1 est dû au fait que
			//nbr_transferts n'est pas forcément un multiple de nbr_canaux
			//il veut donc mieux stocker trop de samples que pas assez
				
			//POUR LES 4 PREMIERS CANAUX
				
			//initialisation du mode de fonctionnement du convertisseur MAX125
			//trigger GPIF (single byte write transaction). Impulsion sur CS et WR
			XGPIFSGLDATLX = 3; 
			while( !( GPIFTRIG & 0x80 ) )
				{ }
				
			//trigger single read (impulsion sur CONVST et attente de nINT)
			dummy = XGPIFSGLDATLX; 
						 
			GPIFTCB0 = 4; //transaction count
						
			//trigger pour transfert GPIF FIFORead
			while( !( GPIFTRIG & 0x80 ) )
				{ }
		  	GPIFTRIG = 6 ; // R/W=1, EP[1:0]=2 (== EP6) 
						
			//attente de la fin du transfert GPIF
			while( !( GPIFTRIG & 0x80 ) )
				{ }	
				
			//POUR LES AUTRES CANAUX
				
			//initialisation du mode de fonctionnement du convertisseur MAX125
			//trigger GPIF (single byte write transaction). Impulsion sur CS et WR
			XGPIFSGLDATLX = nbr_canaux-1; 
			while( !( GPIFTRIG & 0x80 ) )
				{ }
				
			//trigger single read (impulsion sur CONVST, puis attente de nINT)
			dummy = XGPIFSGLDATLX; 
						 
			GPIFTCB0 = nbr_canaux-4;
						
			//trigger pour transfert GPIF FIFORead
			while( !( GPIFTRIG & 0x80 ) )
				{ }
		  	GPIFTRIG = 6 ; // R/W=1, EP[1:0]=2 (== EP6) 
		
			//attente de la fin du transfert GPIF
			while( !( GPIFTRIG & 0x80 ) )
				{ }
		}	
	}
}


//Fonction d'acquisition des données du MAX125 via GPIF
//Valable pour 1 canal. Il s'agit de la version optimisée par rapport à ProcessMultiChannelGpif
void ProcessSingleChannelGpif()
{
	BYTE dummy;
	DEBUG_PRINT("ProcessSingleChannelGpif\n");

	//initialisation du GPIF
	GpifInit_singleChannel( );
	
	//GpifInit_singleChannel modifie IFCONFIG...
	IFCONFIG |= (1<<5);
	
	//reset des FIFOs (page 15-20)
	//ne pas le déplacer avant GpifInit
	FIFORESET = 0x80;
	SYNCDELAY;
	FIFORESET = 0x02;
	SYNCDELAY;
	FIFORESET = 0x04;
	SYNCDELAY;
	FIFORESET = 0x06;
	SYNCDELAY;
	FIFORESET = 0x08;
	SYNCDELAY;
	FIFORESET = 0x00;
	SYNCDELAY;
	
	//dummy read to trigger GPIF (single read transaction) : que CS et RD, pas CONVST
	//Objectif : si nINT était à 0, cela le remettra pas à 1 !!! (vécu)
	//on ne lit pas le résultat
	dummy = XGPIFSGLDATLX;  
			
	//initialisation du mode de fonctionnement du convertisseur MAX125
	//trigger GPIF (single byte write transaction). Impulsion sur CS et WR
	//input mux A, single-channel conversion
	//si nINT était à 0, cela NE le remettrait PAS à 1 !!! (vécu)
	//mais ici nINT est forcément à égal à 1 à cause de la dummy read d'avant
	while( !( GPIFTRIG & 0x80 ) ) { }
	XGPIFSGLDATLX = 0; 
	SYNCDELAY;
			
	//setup transaction count  (page 15-96)    
	//ne pas le faire avant, car il serait décrémenté par les transferts ci-dessus  
	GPIFTCB3 = EP1OUTBUF[4];
	SYNCDELAY;
	GPIFTCB2 = EP1OUTBUF[3];
	SYNCDELAY;
	GPIFTCB1 = EP1OUTBUF[2];
	SYNCDELAY;
	GPIFTCB0 = EP1OUTBUF[1];
	SYNCDELAY;

	//armement de l'endpoint
	EP1OUTBC = 0x01;
	SYNCDELAY;
			
	//trigger pour transfert GPIF FIFORead
	while( !( GPIFTRIG & 0x80 ) )
		{ }
	GPIFTRIG = 6 ; // R/W=1, EP[1:0]=2 (== EP6) 
	SYNCDELAY;
			
	//attente de la fin du transfert GPIF
	while( !( GPIFTRIG & 0x80 ) )
		{ }
}


// Called repeatedly while the device is idle
void TD_Poll(void)
{
	//attente d'une requête sur l'endpoint EP1OUT
	DEBUG_PRINT("Attente d'une requete sur l'EP1\n");
	
	while ( (EP1OUTCS & 0x02) != 0)  //page 15-72
		{}
	
	//la requête est arrivée
	//elle comporte le nombre de conversions à effectuer
	//ainsi que le nombre de canaux d'acquisition
	DEBUG_PRINT("Requete recue sur l'EP1\n");
	DEBUG_PRINTI(EP1OUTBUF[0]);
	DEBUG_PRINT(" canaux\n");
	if (EP1OUTBUF[0] != 0)
	{
		DWORD sampleCount = ((DWORD)EP1OUTBUF[4]<<24) | ((DWORD)EP1OUTBUF[3]<<16) | (EP1OUTBUF[2]<<8) | EP1OUTBUF[1];		
		DEBUG_PRINTI(sampleCount);
		DEBUG_PRINT(" echantillons en tout\n");
	}
	
	/*
		La requête est composée de deux parties. La première, un seul byte,
		indique	le nombre de canaux. Si ce dernier est nul, l'acquisition se
		fait en continu sur 8 canaux jusqu'à réception d'une nouvelle commande.

		Si le nombre de canaux est compris entre 1 et 8, alors les quatres
		prochains bytes indique le nombre de sample à acquérir.

		Chaque sample est représenté sur 16 bits.

		Le nombre de samples doit être un multiple de 256, c'est-à-dire de 
		512 octets pour être compatible avec EP6AUTOINLEN. Si le nombre
		de samples n'est pas multiple de 256 (512 octets), le dernier paquet
		ne sera jamais commité (AUTOCOMMIT)
		Le PC s'occupe de faire respecter cette contrainte.

	*/
	#ifdef SINGLE_CHANNEL_OPTIMISATION
	
	if (EP1OUTBUF[0] == 0)
		ProcessAllContinuousGpif();
	else if (EP1OUTBUF[0] == 1)
		ProcessSingleChannelGpif();	
	else
		ProcessMultiChannelGpif();
	
	#else

	if (EP1OUTBUF[0] == 0)
		ProcessAllContinuousGpif();
	else
		ProcessMultiChannelGpif();		
	
	#endif //SINGLE_CHANNEL_OPTIMISATION
	
	DEBUG_PRINT("Fin du traitement de la requete\n\n");
}

BOOL TD_Suspend(void)          // Called before the device goes into suspend mode
{
	return(TRUE);
}

BOOL TD_Resume(void)          // Called after the device resumes
{
	return(TRUE);
}

//-----------------------------------------------------------------------------
// Device Request hooks
//   The following hooks are called by the end point 0 device request parser.
//-----------------------------------------------------------------------------

BOOL DR_GetDescriptor(void)
{
	return(TRUE);
}

BOOL DR_SetConfiguration(void)   // Called when a Set Configuration command is received
{
	Configuration = SETUPDAT[2];
	return(TRUE);            // Handled by user code
}

BOOL DR_GetConfiguration(void)   // Called when a Get Configuration command is received
{
	EP0BUF[0] = Configuration;
	EP0BCH = 0;
	EP0BCL = 1;
	return(TRUE);            // Handled by user code
}

BOOL DR_SetInterface(void)       // Called when a Set Interface command is received
{
	AlternateSetting = SETUPDAT[2];
	return(TRUE);            // Handled by user code
}

BOOL DR_GetInterface(void)       // Called when a Set Interface command is received
{
	EP0BUF[0] = AlternateSetting;
	EP0BCH = 0;
	EP0BCL = 1;
	return(TRUE);            // Handled by user code
}

BOOL DR_GetStatus(void)
{
	return(TRUE);
}

BOOL DR_ClearFeature(void)
{
	return(TRUE);
}

BOOL DR_SetFeature(void)
{
	return(TRUE);
}

BOOL DR_VendorCmnd(void)
{
	return(TRUE);
}

//-----------------------------------------------------------------------------
// USB Interrupt Handlers
//   The following functions are called by the USB interrupt jump table.
//-----------------------------------------------------------------------------

// Setup Data Available Interrupt Handler
void ISR_Sudav(void) interrupt 0
{
	GotSUD = TRUE;            // Set flag
	EZUSB_IRQ_CLEAR();
	USBIRQ = bmSUDAV;         // Clear SUDAV IRQ
}

// Setup Token Interrupt Handler
void ISR_Sutok(void) interrupt 0
{
	EZUSB_IRQ_CLEAR();
	USBIRQ = bmSUTOK;         // Clear SUTOK IRQ
}

void ISR_Sof(void) interrupt 0
{
	EZUSB_IRQ_CLEAR();
	USBIRQ = bmSOF;            // Clear SOF IRQ
}

void ISR_Ures(void) interrupt 0
{
	if (EZUSB_HIGHSPEED())
	{
		pConfigDscr = pHighSpeedConfigDscr;
		pOtherConfigDscr = pFullSpeedConfigDscr;
	}
	else
	{
		pConfigDscr = pFullSpeedConfigDscr;
		pOtherConfigDscr = pHighSpeedConfigDscr;
	}
	
	EZUSB_IRQ_CLEAR();
	USBIRQ = bmURES;         // Clear URES IRQ
}

void ISR_Susp(void) interrupt 0
{
	Sleep = TRUE;
	EZUSB_IRQ_CLEAR();
	USBIRQ = bmSUSP;
}

void ISR_Highspeed(void) interrupt 0
{
	if (EZUSB_HIGHSPEED())
	{
		pConfigDscr = pHighSpeedConfigDscr;
		pOtherConfigDscr = pFullSpeedConfigDscr;
	}
	else
	{
		pConfigDscr = pFullSpeedConfigDscr;
		pOtherConfigDscr = pHighSpeedConfigDscr;
	}
	
	EZUSB_IRQ_CLEAR();
	USBIRQ = bmHSGRANT;
}
void ISR_Ep0ack(void) interrupt 0
{
}
void ISR_Stub(void) interrupt 0
{
}
void ISR_Ep0in(void) interrupt 0
{
}
void ISR_Ep0out(void) interrupt 0
{
}
void ISR_Ep1in(void) interrupt 0
{
}
void ISR_Ep1out(void) interrupt 0
{
	DEBUG_PRINT("Interrupt on EP1OUT\n");
	NewDataEP1OUT = 1;
	EZUSB_IRQ_CLEAR();
	EPIRQ = bmEP1OUT;
}
void ISR_Ep2inout(void) interrupt 0
{
}
void ISR_Ep4inout(void) interrupt 0
{
}
void ISR_Ep6inout(void) interrupt 0
{
}
void ISR_Ep8inout(void) interrupt 0
{
}
void ISR_Ibn(void) interrupt 0
{
}
void ISR_Ep0pingnak(void) interrupt 0
{
}
void ISR_Ep1pingnak(void) interrupt 0
{
}
void ISR_Ep2pingnak(void) interrupt 0
{
}
void ISR_Ep4pingnak(void) interrupt 0
{
}
void ISR_Ep6pingnak(void) interrupt 0
{
}
void ISR_Ep8pingnak(void) interrupt 0
{
}
void ISR_Errorlimit(void) interrupt 0
{
}
void ISR_Ep2piderror(void) interrupt 0
{
}
void ISR_Ep4piderror(void) interrupt 0
{
}
void ISR_Ep6piderror(void) interrupt 0
{
}
void ISR_Ep8piderror(void) interrupt 0
{
}
void ISR_Ep2pflag(void) interrupt 0
{
}
void ISR_Ep4pflag(void) interrupt 0
{
}
void ISR_Ep6pflag(void) interrupt 0
{
}
void ISR_Ep8pflag(void) interrupt 0
{
}
void ISR_Ep2eflag(void) interrupt 0
{
}
void ISR_Ep4eflag(void) interrupt 0
{
}
void ISR_Ep6eflag(void) interrupt 0
{
}
void ISR_Ep8eflag(void) interrupt 0
{
}
void ISR_Ep2fflag(void) interrupt 0
{
}
void ISR_Ep4fflag(void) interrupt 0
{
}
void ISR_Ep6fflag(void) interrupt 0
{
}
void ISR_Ep8fflag(void) interrupt 0
{
}
void ISR_GpifComplete(void) interrupt 0
{
}
void ISR_GpifWaveform(void) interrupt 0
{
}
