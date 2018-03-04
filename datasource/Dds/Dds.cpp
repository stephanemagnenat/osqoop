/*



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

#include <QtCore>
#include <QWidget>
#include <QString>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QLCDNumber>

#include "Dds.h"

//! Dialog box for controlling datasource frequency
ControlDialog::ControlDialog(QWidget *parent) : QWidget(parent)
{
  ready = false;
  freqA = 100 ;
  freqB =  10;
  
  QHBoxLayout *layout = new QHBoxLayout(this);
 
  /* Channel A*/
  QVBoxLayout *layoutChannelA = new QVBoxLayout;

  QGroupBox *controlsGroupA;
  controlsGroupA = new QGroupBox( tr("Channel A") );

  layoutChannelA->addWidget(new QLabel(tr("Waveform")));
  QComboBox *waveTypeA = new QComboBox;
  waveTypeA->clear();
  waveTypeA->addItem( tr( "Sine" ) );
  waveTypeA->addItem( tr( "Square" ) );
  waveTypeA->addItem( tr( "Triangular" ) );
  layoutChannelA->addWidget(waveTypeA);

  layoutChannelA->addWidget(new QLabel(tr("Amplitude")));
  QDial *ampADial = new QDial;
  ampADial->setFocusPolicy(Qt::StrongFocus);
  connect(ampADial, SIGNAL(valueChanged(int)), this, SLOT(ampAsetValue(int)));
  layoutChannelA->addWidget(ampADial);
  ampADial->setValue(50);

  layoutChannelA->addWidget(new QLabel(tr("Frequency")));   
  QDial *freqADial = new QDial;
  freqADial->setFocusPolicy(Qt::StrongFocus);
  connect(freqADial, SIGNAL(valueChanged(int)), this, SLOT(freqAsetValue(int)));
  layoutChannelA->addWidget(freqADial);
  
  //QHBoxLayout *lcdAlayout =   new QHBoxLayout;
  lcdA = new QLCDNumber( 6, this );
  lcdA->setSegmentStyle(QLCDNumber::Filled);
  //lcdAlayout->addWidget( lcdA );
  //lcdAlayout->addWidget( new QLabel(tr("kHz")));
  /* here !!*/
  freqADial->setValue(10);                      

  layoutChannelA->addWidget(lcdA);
  layoutChannelA->addWidget(new QLabel(tr("Offset")));   
  QDial *offsetADial = new QDial;
  offsetADial->setFocusPolicy(Qt::StrongFocus);
  connect(offsetADial, SIGNAL(valueChanged(int)), this, SLOT(offsetAsetValue(int)));
  layoutChannelA->addWidget(offsetADial);
  offsetADial->setValue(50);
  controlsGroupA->setLayout(layoutChannelA);
  
  /* Channel B*/
 QVBoxLayout *layoutChannelB = new QVBoxLayout;

  QGroupBox *controlsGroupB;
  controlsGroupB = new QGroupBox( tr("Channel B") );

  layoutChannelB->addWidget(new QLabel(tr("Waveform")));
  QComboBox *waveTypeB = new QComboBox;
  waveTypeB->clear();
  waveTypeB->addItem( tr( "Sine" ) );
  waveTypeB->addItem( tr( "Square" ) );
  waveTypeB->addItem( tr( "Triangular" ) );
  layoutChannelB->addWidget(waveTypeB);

  layoutChannelB->addWidget(new QLabel(tr("Amplitude")));
  QDial *ampBDial = new QDial;
  ampBDial->setFocusPolicy(Qt::StrongFocus);
  connect(ampBDial, SIGNAL(valueChanged(int)), this, SLOT(ampBsetValue(int)));
  layoutChannelB->addWidget(ampBDial);
  ampBDial->setValue(50);

  layoutChannelB->addWidget(new QLabel(tr("Frequency")));   
  QDial *freqBDial = new QDial;
  freqBDial->setFocusPolicy(Qt::StrongFocus);
  connect(freqBDial, SIGNAL(valueChanged(int)), this, SLOT(freqBsetValue(int)));
  layoutChannelB->addWidget(freqBDial);
  lcdB = new QLCDNumber( 6, this );
  lcdB->setSegmentStyle(QLCDNumber::Filled);
  layoutChannelB->addWidget(lcdB);
  freqBDial->setValue(15);

  layoutChannelB->addWidget(new QLabel(tr("Offset")));   
  QDial *offsetBDial = new QDial;
  offsetBDial->setFocusPolicy(Qt::StrongFocus);
  connect(offsetBDial, SIGNAL(valueChanged(int)), this, SLOT(offsetBsetValue(int)));
  layoutChannelB->addWidget(offsetBDial);
  offsetBDial->setValue(50);
  controlsGroupB->setLayout(layoutChannelB);

  /* Math */
  QVBoxLayout *layoutMath = new QVBoxLayout;
  QGroupBox *controlsMath;
  controlsMath = new QGroupBox( tr("Math") );
  
  layoutMath->addWidget(new QLabel(tr("Output 1")));
  QComboBox *mathOut1 = new QComboBox;
  mathOut1->clear();
  mathOut1->addItem( tr( "A" ) );
  mathOut1->addItem( tr( "A + B" ) );
  mathOut1->addItem( tr( "A - B" ) );
  mathOut1->addItem( tr( "A * B" ) );
  mathOut1->addItem( tr( "A /  B" ) );
  mathOut1->addItem( tr( "-A" ) );
  connect(mathOut1,SIGNAL(currentIndexChanged(int)), this, SLOT(mathSetTypeA(int)));
  mathOut1->setCurrentIndex(0);
  layoutMath->addWidget(mathOut1);

  layoutMath->addWidget(new QLabel(tr("Additive Noise")));   
  QDial *noiseDial = new QDial;
  noiseDial->setFocusPolicy(Qt::StrongFocus);
  connect(noiseDial, SIGNAL(valueChanged(int)), this, SLOT(noiseSetValue(int)));
  noiseDial->setValue(0);
  layoutMath->addWidget(noiseDial);

  layoutMath->addWidget(new QLabel(tr("Output 2")));
  QComboBox *mathOut2 = new QComboBox;
  mathOut2->clear();
  mathOut2->addItem( tr( "Off" ) );
  mathOut2->addItem( tr( "B" ) );
  mathOut2->addItem( tr( "A + B" ) );
  mathOut2->addItem( tr( "B - A" ) );
  mathOut2->addItem( tr( "-B" ) );
  layoutMath->addWidget(mathOut2);
  mathOut2->setCurrentIndex(0);
  controlsMath->setLayout(layoutMath);

  /* Global layout*/
  layout->addWidget(controlsGroupA);
  layout->addWidget(controlsMath);
  layout->addWidget(controlsGroupB);

  setWindowTitle("Dds Parameters");
  ready = true;
}

void ControlDialog::noiseSetValue(int val){
  noise = (val+1)/100.0;
}

void ControlDialog::ampAsetValue(int val){
  ampA = (val+1)/100.0;
}

void ControlDialog::freqAsetValue(int val){
  freqA = val+1;
  lcdA->display((double)freqA*500);
}

void ControlDialog::offsetAsetValue(int val){
  offsetA = -50+val;
}

void ControlDialog::ampBsetValue(int val){
  ampB = (val+1)/100.0;
}

void ControlDialog::freqBsetValue(int val){
  freqB = val+1;
  lcdB->display((double)freqB*500);
}

void ControlDialog::offsetBsetValue(int val){
  offsetB = -50+val;
}

void ControlDialog::mathSetTypeA(int val){
  mathA = val;
}

QString DdsDataSourceDescription::name() const
{
	return "DDS generator";
}

QString DdsDataSourceDescription::description() const
{
	return "A simple DDS generator";
}

DataSource *DdsDataSourceDescription::create() const
{
	return new DdsDataSource(this);
}


DdsDataSource::DdsDataSource(const DataSourceDescription *description) :
	DataSource(description)
{
  	cDialog = new ControlDialog;
        cDialog->show();

        c0 = new signed short[512];
        c1 = new signed short[512];

	t = 0;
}

unsigned DdsDataSource::getRawData(std::valarray<std::valarray<signed short> > *data)
{

   if(!cDialog->ready){
     return 100000;
   }

      int inputs = inputCount();
      float freqA =  M_PI * cDialog->freqA / 100.0;
      float freqB = M_PI * cDialog->freqB / 100.0;
      float ampA = unitPerVoltCount() * cDialog->ampA;
      float ampB = unitPerVoltCount() * cDialog->ampB;
      int offsetA = cDialog->offsetA * unitPerVoltCount()  / 100;
      int offsetB = cDialog->offsetB * unitPerVoltCount()  / 100;
      float noise =  unitPerVoltCount() * cDialog->noise;

      Q_ASSERT(data->size() >= inputs);

      for (size_t sample = 0; sample < 512; sample++){
         c0[sample] = 
           (signed short)(ampA * sin( t *  freqA )) + offsetA + (signed short)(noise * ((float)rand()/RAND_MAX - 0.5));
         c1[sample] =
           (signed short)(ampB * sin( t *  freqB )) + offsetB + (signed short)(noise * ((float)rand()/RAND_MAX - 0.5));
          t++;
      }

      switch (cDialog->mathA){
      case 1:
         for (size_t sample = 0; sample < 512; sample++)
           (*data)[0][sample] = c0[sample] + c1[sample];
         break;
      case 2:
        for (size_t sample = 0; sample < 512; sample++)
           (*data)[0][sample] = c0[sample] - c1[sample];
         break;
      case 3:
        for (size_t sample = 0; sample < 512; sample++)
          (*data)[0][sample] = c0[sample] * c1[sample];
        break;
      case 4:
        for (size_t sample = 0; sample < 512; sample++)
            if (c1[sample]) {
              (*data)[0][sample] = c0[sample] / c1[sample];
            } else {
              (*data)[0][sample] = c0[sample];
            }
        break; 
      case 5:
        for (size_t sample = 0; sample < 512; sample++)
          (*data)[0][sample] = -1 * c0[sample] ;
        break;    
      case 6:
        for (size_t sample = 0; sample < 512; sample++)
            if (c0[sample]) {
              (*data)[0][sample] = 1 / c0[sample];
            } else {
              (*data)[0][sample] = 1;
            }
        break; 
      case 0:
      default:
         for (size_t sample = 0; sample < 512; sample++)
           (*data)[0][sample] = c0[sample];
        }

    
   switch (cDialog->mathB){
   case 1:
     for (size_t sample = 0; sample < 512; sample++){
        (*data)[1][sample] = c1[sample];
     }
     break;
   case 2:
     for (size_t sample = 0; sample < 512; sample++){
        (*data)[1][sample] = c1[sample] + c0[sample];
     }
     break;
   case 3:
     for (size_t sample = 0; sample < 512; sample++){
        (*data)[1][sample] = c1[sample] - c0[sample];
     }
     break;
   case 4:
     for (size_t sample = 0; sample < 512; sample++){
        (*data)[1][sample] = -1 * c1[sample]; 
     }
     break;
   case 0:
   default:
     for (size_t sample = 0; sample < 512; sample++){
        (*data)[1][sample] = 0;
     }
   }
      /* elapsed microseconds */
      //return samplingRate() * 512 / 1000000 ;
      return 1000000 / (samplingRate() / 512);
}

unsigned DdsDataSource::inputCount() const
{
	return 2;
}

unsigned DdsDataSource::samplingRate() const
{
	return 100000;
}

unsigned DdsDataSource::unitPerVoltCount() const
{
    return 1000;
}
