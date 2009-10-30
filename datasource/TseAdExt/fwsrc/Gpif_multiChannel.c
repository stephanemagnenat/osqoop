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

// This program configures the General Programmable Interface (GPIF) for FX2.              
// Parts of this program are automatically generated using the GPIFTool.                  
// Please do not modify sections of text which are marked as "DO NOT EDIT ...".            
// You can modify the comments section of this GPIF program file using the dropdown menus  
// and pop-up dialogs. These controls are available as hot spots in the text. Modifying the
// comments section will generate program code which will implement your GPIF program.     
//                                                                                         
// DO NOT EDIT ...                                                                         
// GPIF Initialization                                                                     
// Interface Timing      Sync                                                              
// Internal Ready Init   IntRdy=1                                                          
// CTL Out Tristate-able Binary                                                            
// SingleWrite WF Select     3                                                             
// SingleRead WF Select      2                                                             
// FifoWrite WF Select       1                                                             
// FifoRead WF Select        0                                                             
// Data Bus Idle Drive   Tristate                                                          
// END DO NOT EDIT                                                                         
                                                                                           
// DO NOT EDIT ...                                                                         
// GPIF Wave Names                                                                         
// Wave 0   = FIFORd                                                                       
// Wave 1   = FIFOWr                                                                       
// Wave 2   = SnglRd                                                                       
// Wave 3   = SnglWr                                                                       
                                                                                           
// GPIF Ctrl Outputs   Level                                                               
// CTL 0    = CONVST   CMOS                                                                
// CTL 1    = CS       CMOS                                                                
// CTL 2    = RD       CMOS                                                                
// CTL 3    = WR       CMOS                                                                
// CTL 4    = CTL 4    CMOS                                                                
// CTL 5    = CTL 5    CMOS                                                                
                                                                                           
// GPIF Rdy Inputs                                                                         
// RDY0     = INT                                                                          
// RDY1     = RDY1                                                                         
// RDY2     = RDY2                                                                         
// RDY3     = RDY3                                                                         
// RDY4     = RDY4                                                                         
// RDY5     = RDY5                                                                         
// FIFOFlag = FIFOFlag                                                                     
// IntReady = IntReady                                                                     
// END DO NOT EDIT                                                                         
// DO NOT EDIT ...                                                                         
//                                                                                         
// GPIF Waveform 0: FIFORd                                                                 
//                                                                                         
// Interval     0         1         2         3         4         5         6     Idle (7) 
//          _________ _________ _________ _________ _________ _________ _________ _________
//                                                                                         
// AddrMode Same Val  Same Val  Same Val  Same Val  Same Val  Same Val  Same Val           
// DataMode NO Data   Activate  NO Data   Activate  NO Data   NO Data   NO Data            
// NextData SameData  SameData  SameData  SameData  SameData  SameData  SameData           
// Int Trig No Int    No Int    No Int    No Int    No Int    No Int    Trig Int           
// IF/Wait  Wait 2    IF        Wait 1    Wait 1    Wait 1    Wait 1    Wait 1             
//   Term A           INT                                                                  
//   LFunc            OR                                                                   
//   Term B           INT                                                                  
// Branch1            ThenIdle                                                             
// Branch0            ElseIdle                                                             
// Re-Exec            No                                                                   
// Sngl/CRC Default   Default   Default   Default   Default   Default   Default            
// CONVST       1         1         1         1         1         1         1         1    
// CS           0         1         1         1         1         1         1         1    
// RD           0         1         1         1         1         1         1         1    
// WR           1         1         1         1         1         1         1         1    
// CTL 4        1         1         1         1         1         1         1         1    
// CTL 5        1         1         1         1         1         1         1         1    
//                                                                                         
// END DO NOT EDIT                                                                         
// DO NOT EDIT ...                                                                         
//                                                                                         
// GPIF Waveform 1: FIFOWr                                                                 
//                                                                                         
// Interval     0         1         2         3         4         5         6     Idle (7) 
//          _________ _________ _________ _________ _________ _________ _________ _________
//                                                                                         
// AddrMode Same Val  Same Val  Same Val  Same Val  Same Val  Same Val  Same Val           
// DataMode Activate  NO Data   NO Data   NO Data   NO Data   NO Data   NO Data            
// NextData SameData  SameData  NextData  SameData  SameData  SameData  SameData           
// Int Trig No Int    No Int    No Int    No Int    No Int    No Int    No Int             
// IF/Wait  Wait 1    Wait 1    Wait 1    Wait 1    Wait 1    Wait 1    Wait 1             
//   Term A                                                                                
//   LFunc                                                                                 
//   Term B                                                                                
// Branch1                                                                                 
// Branch0                                                                                 
// Re-Exec                                                                                 
// Sngl/CRC Default   Default   Default   Default   Default   Default   Default            
// CONVST       1         1         1         1         1         1         1         1    
// CS           1         1         1         1         1         1         1         1    
// RD           1         1         1         1         1         1         1         1    
// WR           1         1         1         1         1         1         1         1    
// CTL 4        1         1         1         1         1         1         1         1    
// CTL 5        1         1         1         1         1         1         1         1    
//                                                                                         
// END DO NOT EDIT                                                                         
// DO NOT EDIT ...                                                                         
//                                                                                         
// GPIF Waveform 2: SnglRd                                                                 
//                                                                                         
// Interval     0         1         2         3         4         5         6     Idle (7) 
//          _________ _________ _________ _________ _________ _________ _________ _________
//                                                                                         
// AddrMode Same Val  Same Val  Same Val  Same Val  Same Val  Same Val  Same Val           
// DataMode NO Data   NO Data   NO Data   NO Data   NO Data   NO Data   NO Data            
// NextData SameData  SameData  SameData  SameData  SameData  SameData  SameData           
// Int Trig No Int    No Int    No Int    No Int    No Int    No Int    Trig Int           
// IF/Wait  Wait 2    IF        Wait 1    Wait 1    Wait 1    Wait 1    Wait 1             
//   Term A           INT                                                                  
//   LFunc            OR                                                                   
//   Term B           INT                                                                  
// Branch1            Then 1                                                               
// Branch0            ElseIdle                                                             
// Re-Exec            No                                                                   
// Sngl/CRC Default   Default   Default   Default   Default   Default   Default            
// CONVST       0         1         1         1         1         1         1         1    
// CS           1         1         1         1         1         1         1         1    
// RD           1         1         1         1         1         1         1         1    
// WR           1         1         1         1         1         1         1         1    
// CTL 4        1         1         1         1         1         1         1         1    
// CTL 5        1         1         1         1         1         1         1         1    
//                                                                                         
// END DO NOT EDIT                                                                         
// DO NOT EDIT ...                                                                         
//                                                                                         
// GPIF Waveform 3: SnglWr                                                                 
//                                                                                         
// Interval     0         1         2         3         4         5         6     Idle (7) 
//          _________ _________ _________ _________ _________ _________ _________ _________
//                                                                                         
// AddrMode Same Val  Same Val  Same Val  Same Val  Same Val  Same Val  Same Val           
// DataMode Activate  NO Data   NO Data   NO Data   NO Data   NO Data   NO Data            
// NextData SameData  SameData  SameData  SameData  SameData  SameData  SameData           
// Int Trig No Int    No Int    No Int    No Int    No Int    No Int    No Int             
// IF/Wait  Wait 2    Wait 5    IF        Wait 1    Wait 1    Wait 1    Wait 1             
//   Term A                     INT                                                        
//   LFunc                      OR                                                         
//   Term B                     INT                                                        
// Branch1                      ThenIdle                                                   
// Branch0                      ElseIdle                                                   
// Re-Exec                      No                                                         
// Sngl/CRC Default   Default   Default   Default   Default   Default   Default            
// CONVST       1         1         1         1         1         1         1         1    
// CS           0         1         1         1         1         1         1         1    
// RD           1         1         1         1         1         1         1         1    
// WR           0         1         1         1         1         1         1         1    
// CTL 4        1         1         1         1         1         1         1         1    
// CTL 5        1         1         1         1         1         1         1         1    
//                                                                                         
// END DO NOT EDIT                                                                         
                                                                                           
// GPIF Program Code                                                                       
                                                                                           
// DO NOT EDIT ...                                                                         
#include "fx2.h"                                                                           
#include "fx2regs.h"                                                                       
#include "fx2sdly.h"            // SYNCDELAY macro
// END DO NOT EDIT                                                                         
                                                                                           
// DO NOT EDIT ...                                                                         
const char xdata WaveData_multiChannel[128] =                                                           
{                                                                                          
// Wave 0                                                                                  
/* LenBr */ 0x02,     0x3F,     0x01,     0x01,     0x01,     0x01,     0x01,     0x07,    
/* Opcode*/ 0x00,     0x03,     0x00,     0x02,     0x00,     0x00,     0x10,     0x00,    
/* Output*/ 0xF9,     0xFF,     0xFF,     0xFF,     0xFF,     0xFF,     0xFF,     0xFF,    
/* LFun  */ 0x00,     0x40,     0x12,     0x40,     0x00,     0x2D,     0x12,     0x3F,    
// Wave 1                                                                                  
/* LenBr */ 0x01,     0x01,     0x01,     0x01,     0x01,     0x01,     0x01,     0x07,    
/* Opcode*/ 0x02,     0x00,     0x04,     0x00,     0x00,     0x00,     0x00,     0x00,    
/* Output*/ 0xFF,     0xFF,     0xFF,     0xFF,     0xFF,     0xFF,     0xFF,     0xFF,    
/* LFun  */ 0x09,     0x40,     0x12,     0x40,     0x00,     0x2D,     0x36,     0x3F,    
// Wave 2                                                                                  
/* LenBr */ 0x02,     0x0F,     0x01,     0x01,     0x01,     0x01,     0x01,     0x07,    
/* Opcode*/ 0x00,     0x01,     0x00,     0x00,     0x00,     0x00,     0x10,     0x00,    
/* Output*/ 0xFE,     0xFF,     0xFF,     0xFF,     0xFF,     0xFF,     0xFF,     0xFF,    
/* LFun  */ 0x00,     0x40,     0x12,     0x40,     0x40,     0x2D,     0x12,     0x3F,    
// Wave 3                                                                                  
/* LenBr */ 0x02,     0x05,     0x3F,     0x01,     0x01,     0x01,     0x01,     0x07,    
/* Opcode*/ 0x02,     0x00,     0x01,     0x00,     0x00,     0x00,     0x00,     0x00,    
/* Output*/ 0xF5,     0xFF,     0xFF,     0xFF,     0xFF,     0xFF,     0xFF,     0xFF,    
/* LFun  */ 0x09,     0x09,     0x40,     0x40,     0x00,     0x2D,     0x36,     0x3F     
};                                                                                         
// END DO NOT EDIT                                                                         
                                                                                           
// DO NOT EDIT ...                                                                         
const char xdata InitData_multiChannel[7] =                                                             
{                                                                                          
/* Regs  */ 0xC0,0x00,0x00,0xFF,0x06,0xE4,0x11                                     
};                                                                                         
// END DO NOT EDIT                                                                         

// TO DO: You may add additional code below.

void GpifInit_multiChannel( void )
{
  BYTE i;

  // Registers which require a synchronization delay, see section 15.14
  // FIFORESET        FIFOPINPOLAR
  // INPKTEND         OUTPKTEND
  // EPxBCH:L         REVCTL
  // GPIFTCB3         GPIFTCB2
  // GPIFTCB1         GPIFTCB0
  // EPxFIFOPFH:L     EPxAUTOINLENH:L
  // EPxFIFOCFG       EPxGPIFFLGSEL
  // PINFLAGSxx       EPxFIFOIRQ
  // EPxFIFOIE        GPIFIRQ
  // GPIFIE           GPIFADRH:L
  // UDMACRCH:L       EPxGPIFTRIG
  // GPIFTRIG
  
  // Note: The pre-REVE EPxGPIFTCH/L register are affected, as well...
  //      ...these have been replaced by GPIFTC[B3:B0] registers

  // 8051 doesn't have access to waveform memories 'til
  // the part is in GPIF mode.

  IFCONFIG = 0xCE;
  // IFCLKSRC=1   , FIFOs executes on internal clk source
  // xMHz=1       , 48MHz internal clk rate
  // IFCLKOE=0    , Don't drive IFCLK pin signal at 48MHz
  // IFCLKPOL=0   , Don't invert IFCLK pin signal from internal clk
  // ASYNC=1      , master samples asynchronous
  // GSTATE=1     , Drive GPIF states out on PORTE[2:0], debug WF
  // IFCFG[1:0]=10, FX2 in GPIF master mode

  GPIFABORT = 0xFF;  // abort any waveforms pending

  GPIFREADYCFG = InitData_multiChannel[ 0 ];
  GPIFCTLCFG = InitData_multiChannel[ 1 ];
  GPIFIDLECS = InitData_multiChannel[ 2 ];
  GPIFIDLECTL = InitData_multiChannel[ 3 ];
  GPIFWFSELECT = InitData_multiChannel[ 5 ];
  GPIFREADYSTAT = InitData_multiChannel[ 6 ];

  // use dual autopointer feature... 
  AUTOPTRSETUP = 0x07;          // inc both pointers, 
                                // ...warning: this introduces pdata hole(s)
                                // ...at E67B (XAUTODAT1) and E67C (XAUTODAT2)
  
  // source
  APTR1H = MSB( &WaveData_multiChannel );
  APTR1L = LSB( &WaveData_multiChannel );
  
  // destination
  AUTOPTRH2 = 0xE4;
  AUTOPTRL2 = 0x00;

  // transfer
  for ( i = 0x00; i < 128; i++ )
  {
    EXTAUTODAT2 = EXTAUTODAT1;
  }

// Configure GPIF Address pins, output initial value,
  PORTCCFG = 0xFF;    // [7:0] as alt. func. GPIFADR[7:0]
  OEC = 0xFF;         // and as outputs
  PORTECFG |= 0x80;   // [8] as alt. func. GPIFADR[8]
  OEC |= 0x80;        // and as output

// ...OR... tri-state GPIFADR[8:0] pins
//  PORTCCFG = 0x00;  // [7:0] as port I/O
//  OEC = 0x00;       // and as inputs
//  PORTECFG &= 0x7F; // [8] as port I/O
//  OEC &= 0x7F;      // and as input

// GPIF address pins update when GPIFADRH/L written
  SYNCDELAY;                    // 
  GPIFADRH = 0x00;    // bits[7:1] always 0
  SYNCDELAY;                    // 
  GPIFADRL = 0x00;    // point to PERIPHERAL address 0x0000
}

//#define TESTING_GPIF // NOTE: Comment this line out for frameworks based firmware
                     // See the example GPIF Tool Utility under Application
                     // Reference Material for more advanced development info
#ifdef TESTING_GPIF
// TODO: You may add additional code below.

void OtherInit( void )
{ // interface initialization
  // ...see TD_Init( );
}

// Set Address GPIFADR[8:0] to PERIPHERAL
void Peripheral_SetAddress( WORD gaddr )
{
  SYNCDELAY;                    // 
  GPIFADRH = gaddr >> 8;
  SYNCDELAY;                    // 
  GPIFADRL = ( BYTE )gaddr; // setup GPIF address 
}

// Set EP2GPIF Transaction Count
void Peripheral_SetEP2GPIFTC( WORD xfrcnt )
{
  SYNCDELAY;                    // 
  EP2GPIFTCH = xfrcnt >> 8;  // setup transaction count
  SYNCDELAY;                    // 
  EP2GPIFTCL = ( BYTE )xfrcnt;
}

// Set EP4GPIF Transaction Count
void Peripheral_SetEP4GPIFTC( WORD xfrcnt )
{
  SYNCDELAY;                    // 
  EP4GPIFTCH = xfrcnt >> 8;  // setup transaction count
  SYNCDELAY;                    // 
  EP4GPIFTCL = ( BYTE )xfrcnt;
}

// Set EP6GPIF Transaction Count
void Peripheral_SetEP6GPIFTC( WORD xfrcnt )
{
  SYNCDELAY;                    // 
  EP6GPIFTCH = xfrcnt >> 8;  // setup transaction count
  SYNCDELAY;                    // 
  EP6GPIFTCL = ( BYTE )xfrcnt;
}

// Set EP8GPIF Transaction Count
void Peripheral_SetEP8GPIFTC( WORD xfrcnt )
{
  SYNCDELAY;                    // 
  EP8GPIFTCH = xfrcnt >> 8;  // setup transaction count
  SYNCDELAY;                    // 
  EP8GPIFTCL = ( BYTE )xfrcnt;
}

#define GPIF_FLGSELPF 0
#define GPIF_FLGSELEF 1
#define GPIF_FLGSELFF 2

// Set EP2GPIF Decision Point FIFO Flag Select (PF, EF, FF)
void SetEP2GPIFFLGSEL( WORD DP_FIFOFlag )
{
  EP2GPIFFLGSEL = DP_FIFOFlag;
}

// Set EP4GPIF Decision Point FIFO Flag Select (PF, EF, FF)
void SetEP4GPIFFLGSEL( WORD DP_FIFOFlag )
{
  EP4GPIFFLGSEL = DP_FIFOFlag;
}

// Set EP6GPIF Decision Point FIFO Flag Select (PF, EF, FF)
void SetEP6GPIFFLGSEL( WORD DP_FIFOFlag )
{
  EP6GPIFFLGSEL = DP_FIFOFlag;
}

// Set EP8GPIF Decision Point FIFO Flag Select (PF, EF, FF)
void SetEP8GPIFFLGSEL( WORD DP_FIFOFlag )
{
  EP8GPIFFLGSEL = DP_FIFOFlag;
}

// Set EP2GPIF Programmable Flag STOP, overrides Transaction Count
void SetEP2GPIFPFSTOP( void )
{
  EP2GPIFPFSTOP = 0x01;
}

// Set EP4GPIF Programmable Flag STOP, overrides Transaction Count
void SetEP4GPIFPFSTOP( void )
{
  EP4GPIFPFSTOP = 0x01;
}

// Set EP6GPIF Programmable Flag STOP, overrides Transaction Count
void SetEP6GPIFPFSTOP( void )
{
  EP6GPIFPFSTOP = 0x01;
}

// Set EP8GPIF Programmable Flag STOP, overrides Transaction Count
void SetEP8GPIFPFSTOP( void )
{
  EP8GPIFPFSTOP = 0x01;
}

// write single byte to PERIPHERAL, using GPIF
void Peripheral_SingleByteWrite( BYTE gdata )
{
  while( !( GPIFTRIG & 0x80 ) ) // poll GPIFTRIG.7 Done bit
  {
     ;
  }

  XGPIFSGLDATLX = gdata;        // trigger GPIF 
                                // ...single byte write transaction
}

// write single word to PERIPHERAL, using GPIF
void Peripheral_SingleWordWrite( WORD gdata )
{
  while( !( GPIFTRIG & 0x80 ) )  // poll GPIFTRIG.7 Done bit
  {
    ;
  }

// using register(s) in XDATA space
  XGPIFSGLDATH = gdata >> 8;              
  XGPIFSGLDATLX = gdata;        // trigger GPIF 
                                // ...single word write transaction
}

// read single byte from PERIPHERAL, using GPIF
void Peripheral_SingleByteRead( BYTE xdata *gdata )
{
  static BYTE g_data = 0x00;

  while( !( GPIFTRIG & 0x80 ) )  // poll GPIFTRIG.7 Done bit
  {
     ;
  }

// using register(s) in XDATA space, dummy read
  g_data = XGPIFSGLDATLX;       // trigger GPIF 
                                // ...single byte read transaction
  while( !( GPIFTRIG & 0x80 ) ) // poll GPIFTRIG.7 Done bit
  {
     ;
  }

// using register(s) in XDATA space, 
  *gdata = XGPIFSGLDATLNOX;     // ...GPIF reads byte from PERIPHERAL
}

// read single word from PERIPHERAL, using GPIF
void Peripheral_SingleWordRead( WORD xdata *gdata )
{
  BYTE g_data = 0x00;

  while( !( GPIFTRIG & 0x80 ) ) // poll GPIFTRIG.7 Done bit
  {
     ;
  }

// using register(s) in XDATA space, dummy read
  g_data = XGPIFSGLDATLX;       // trigger GPIF 
                                // ...single word read transaction

  while( !( GPIFTRIG & 0x80 ) ) // poll GPIFTRIG.7 Done bit
  {
     ;
  }

// using register(s) in XDATA space, GPIF reads word from PERIPHERAL
  *gdata = ( ( WORD )XGPIFSGLDATH << 8 ) | ( WORD )XGPIFSGLDATLNOX;
}

#define GPIFTRIGWR 0
#define GPIFTRIGRD 4

#define GPIF_EP2 0
#define GPIF_EP4 1
#define GPIF_EP6 2
#define GPIF_EP8 3

// write byte(s)/word(s) to PERIPHERAL, using GPIF and EPxFIFO
// if EPx WORDWIDE=0 then write byte(s)
// if EPx WORDWIDE=1 then write word(s)
void Peripheral_FIFOWrite( BYTE FIFO_EpNum )
{
  while( !( GPIFTRIG & 0x80 ) ) // poll GPIFTRIG.7 Done bit
  {
     ;
  }

  // trigger FIFO write transaction(s), using SFR
  GPIFTRIG = FIFO_EpNum;  // R/W=0, EP[1:0]=FIFO_EpNum for EPx write(s)
}

// read byte(s)/word(s) from PERIPHERAL, using GPIF and EPxFIFO
// if EPx WORDWIDE=0 then read byte(s)
// if EPx WORDWIDE=1 then read word(s)
void Peripheral_FIFORead( BYTE FIFO_EpNum )
{
  while( !( GPIFTRIG & 0x80 ) ) // poll GPIFTRIG.7 GPIF Done bit
  {
     ;
  }

  // trigger FIFO read transaction(s), using SFR
  GPIFTRIG = GPIFTRIGRD | FIFO_EpNum; // R/W=1, EP[1:0]=FIFO_EpNum for EPx read(s)
}

void main( void )
{
  WORD xdata wData = 0x0000;
  BYTE xdata bData = 0x00;
  WORD myi = 0x0000;
  BOOL bResult = 1;

  OtherInit( );
  GpifInit( );
  
  if( bResult == 0 ) 
  { // stub out unused functions; avoid UNCALLED SEGMENT link error
    Peripheral_SingleWordWrite( 0x5678 );
    Peripheral_SingleByteWrite( 0xAA );
    Peripheral_SingleWordRead( &wData );
    Peripheral_SingleByteRead( &bData );
    // GPIF automatically throttles when performing FIFO read(s)/write(s)
    // ...what this means is that it doesn't transition from IDLE, while...
    // ...the EPx OUT FIFO is empty... OR ...the EPx IN FIFO is full...
    Peripheral_FIFOWrite( GPIF_EP2 ); // GPIF will not read from an empty FIFO
    Peripheral_FIFORead( GPIF_EP8 );  // GPIF will not write to a full FIFO
    Peripheral_SetEP2GPIFTC( 0x0000 );
    Peripheral_SetEP4GPIFTC( 0x0000 );
    Peripheral_SetEP6GPIFTC( 0x0000 );
    Peripheral_SetEP8GPIFTC( 0x0000 );
    SetEP2GPIFFLGSEL( GPIF_FLGSELPF );
    SetEP4GPIFFLGSEL( GPIF_FLGSELEF );
    SetEP6GPIFFLGSEL( GPIF_FLGSELFF );
    SetEP8GPIFFLGSEL( GPIF_FLGSELPF );
    SetEP2GPIFPFSTOP(); // use GPIF_PF to transition GPIF to done
    SetEP4GPIFPFSTOP(); // use GPIF_PF to transition GPIF to done
    SetEP6GPIFPFSTOP(); // use GPIF_PF to transition GPIF to done
    SetEP8GPIFPFSTOP(); // use GPIF_PF to transition GPIF to done
  }

  Peripheral_SetAddress( 0x0155 );

  while( 1 )
  { // ...the EPx WORDWIDE bits must to be set to
    // ...configure PORTD as FD[15:8] for single transactions
    if( EP2FIFOCFG & 0x01 )  // If 16-bit mode - EPx WORDWIDE=1
    { // illustrate use of efficient 16 bit functions
      Peripheral_SingleWordWrite( 0xAA55 );
      Peripheral_SingleWordRead( &wData );
    }
    else
    {
      Peripheral_SingleByteWrite( 0xA5 );
      Peripheral_SingleByteRead( &bData );
    }
  }
}
#endif
