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
#include <QFontMetrics>

#include "SpectroGraph.h"
#include <SpectroGraph.moc>
#include <DataSource.h>
#include <stdio.h>

#include <fftw3.h>

QString ProcessingSpectroGraphDescription::systemName() const
{
	return QString("SpectroGraph");
}

QString ProcessingSpectroGraphDescription::name() const
{
	return QString("SpectroGraph");
}

QString ProcessingSpectroGraphDescription::description() const
{
	return QString("Returns frequency plot.");
}

unsigned ProcessingSpectroGraphDescription::inputCount() const
{
  return 1;
}

unsigned ProcessingSpectroGraphDescription::outputCount() const
{
  return 0;
}

ProcessingPlugin *ProcessingSpectroGraphDescription::create(const DataSource *dataSource) const
{
  return new ProcessingSpectroGraph(this, dataSource);
}


ProcessingSpectroGraph::ProcessingSpectroGraph(const ProcessingPluginDescription *description, const DataSource *dataSource) :
  ProcessingPlugin(description),
   dataSource(dataSource)
{
	newGUI = new SpectroGraphGUI;
    
        newGUI->samplingRate = dataSource->samplingRate();
        newGUI->setFftSize(newGUI->fftSize);
	
	newGUI->show();
}

void ProcessingSpectroGraph::peakDisplayChanged(int val) {

  newGUI->showPeak = false;
  if(val)
    newGUI->showPeak = true;

}

void ProcessingSpectroGraph::meanModeChanged(int val) {
	newGUI->fftMean = val;
}

void ProcessingSpectroGraph::fftSizeChanged(int val) {
  int size = 128;
 
  for (int i =0; i<val;i++){
    size += size;
  }
  newGUI->setFftSize(size);
}

void ProcessingSpectroGraph::wTableChanged(int val){
  newGUI->wTableType = val;
  newGUI->initWtable(newGUI->fftSize);
  newGUI->gridInvalidate = true;
}

void ProcessingSpectroGraph::dBminChanged(int val){
  /*checks */
  newGUI->mindB = (val+1)*(-10);
  qDebug("Min %d",newGUI->mindB);
  newGUI->setFftSize(newGUI->fftSize);
}
void ProcessingSpectroGraph::dBmaxChanged(int val){
  /*checks */
  newGUI->maxdB = val*(-10);
  qDebug("Max %d",newGUI->maxdB);
  newGUI->setFftSize(newGUI->fftSize);
}

QWidget *ProcessingSpectroGraph::createGUI(void)
{   
  int i;
	QWidget *guiBase = new QWidget;

	QGridLayout *layout = new QGridLayout(guiBase);

	layout->addWidget(new QLabel(tr("FFT Points")), 1, 0);
        QComboBox *fftSizeComboBox = new QComboBox;
        fftSizeComboBox->clear();
        fftSizeComboBox->addItem( tr( "128" ) );
        fftSizeComboBox->addItem( tr( "256" ) );
        fftSizeComboBox->addItem( tr( "512" ) );
        fftSizeComboBox->addItem( tr( "1024" ) );
        fftSizeComboBox->addItem( tr( "2048" ) );
        fftSizeComboBox->addItem( tr( "4096" ) );
        
        fftSizeComboBox->setCurrentIndex(2);
        layout->addWidget(fftSizeComboBox, 1, 1);
        connect(fftSizeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(fftSizeChanged(int)));
      
        layout->addWidget(new QLabel(tr("Peak Mark")), 2, 0);
        QCheckBox *peakDisplay = new  QCheckBox;
        layout->addWidget(peakDisplay, 2,1);
        connect(peakDisplay, SIGNAL(stateChanged(int)), SLOT(peakDisplayChanged(int)));

        layout->addWidget(new QLabel(tr("Window")), 3, 0);
        QComboBox *fftWindowBox = new QComboBox;
        fftWindowBox->clear();
        for(i=0; i<10;i++){
          fftWindowBox->addItem( newGUI->wNames[i].toLatin1() );
        }

        fftWindowBox->setCurrentIndex(0);
        layout->addWidget(fftWindowBox, 3, 1);
         connect(fftWindowBox, SIGNAL(currentIndexChanged(int)), SLOT(wTableChanged(int)));

        layout->addWidget(new QLabel(tr("Mean Mode")), 4, 0);
        QCheckBox *meanModeCheck = new  QCheckBox;
        layout->addWidget(meanModeCheck, 4,1);
        connect(meanModeCheck, SIGNAL(stateChanged(int)), SLOT(meanModeChanged(int)));

        layout->addWidget(new QLabel(tr("Scale Max")), 5, 0);
        QComboBox *maxdBBox = new QComboBox;
        maxdBBox->clear();
        maxdBBox->addItem("0 dB");
        QString str;
        for(i=1; i<9;i++){
          str.sprintf("-%2d dB", i*10);
          maxdBBox->addItem( str.toLatin1() );
         }
         maxdBBox->setCurrentIndex(0);
         layout->addWidget(maxdBBox,5,1);
         connect(maxdBBox, SIGNAL(currentIndexChanged(int)), SLOT(dBmaxChanged(int)));

         layout->addWidget(new QLabel(tr("Scale Min")), 6, 0);
         QComboBox *mindBBox = new QComboBox;
         mindBBox->clear();
         for(i=1; i<10;i++){
          str.sprintf("-%2d dB", i*10);
          mindBBox->addItem( str.toLatin1());
         }
         mindBBox->setCurrentIndex(5);
         layout->addWidget(mindBBox,6,1);
         connect(mindBBox, SIGNAL(currentIndexChanged(int)), SLOT(dBminChanged(int)));

	return guiBase;
}

void ProcessingSpectroGraph::processData(const std::valarray<signed short *> &inputs, const std::valarray<signed short *> &outputs, unsigned sampleCount)
{  

  newGUI->processRunning.lock();

  unsigned fftSize = newGUI->fftSize;
  unsigned staticSampleCount = newGUI->sampleCount;
  unsigned nsample = (fftSize-staticSampleCount > sampleCount) ? sampleCount : fftSize-staticSampleCount; 

  /* Read in values */  
  if(fftSize < sampleCount){
    /* Discard data in excess */
    for(unsigned sample = 0; sample < fftSize; sample++) {
       newGUI->fftIn[sample][0] = (double)inputs[0][sample]*
        newGUI->wTable[sample];
       newGUI->fftIn[sample][1] = 0.0;
    }
    newGUI->sampleCount += fftSize;
  }
  else{
    /* cumulate more calls to fftIn buffer */
    for(unsigned sample = 0; sample < nsample; sample++) {
      newGUI->fftIn[staticSampleCount+sample][0] = (double)inputs[0][sample]*
        newGUI->wTable[staticSampleCount+sample];
      newGUI->fftIn[staticSampleCount+sample][1] = 0.0;
    }
    newGUI->sampleCount += nsample;
  }

  /* if there are enought data, let's go! */
  if (newGUI->sampleCount >= fftSize){
    newGUI->sampleCount = 0;
    /* ****************************************************
    * Execute FFT from b1 into b2
    * ****************************************************/
    fftw_execute( newGUI->p1);
    
    /* Post Process */
    const float norm = (float)newGUI->fftSize;
    
    for(unsigned i=0;i<fftSize/2;i++){
      const float re  = newGUI->fftOut[i][0]  / norm;
      const float im = newGUI->fftOut[i][1]  / norm;
      
      if(newGUI->fftMean){
        if(newGUI->meanCnt == 0)
          newGUI->fftData[i] = sqrt( re*re+im*im );
        else
          newGUI->fftData[i] += sqrt( re*re+im*im );
        
        if(newGUI->meanCnt == MEANSIZE-1)
          newGUI->fftData[i] /= MEANSIZE;
      }
      else
        newGUI->fftData[i] =  sqrt( re*re+im*im );
    }
    
    if(newGUI->fftMean)
      if(newGUI->meanCnt == MEANSIZE-1){
        newGUI->update();
        newGUI->meanCnt = 0;
      }  
      else
        newGUI->meanCnt++;
    else
      newGUI->update();
  }
  
  newGUI->processRunning.unlock();
}

SpectroGraphGUI::SpectroGraphGUI(QWidget *parent) : QWidget(parent) {
  
  displayMouseMeasure = false;
  fftMean = false;
  fftSize = 512;
  fftIn = NULL;
  fftOut = NULL;
  setWindowTitle("Power Spectrum");
  persistantBuffer = NULL;
  gridInvalidate = true;
  // funny
  // setWindowFlags(Qt::FramelessWindowHint);
  fftType = Power;
  showPeak = false;
  meanCnt = 0;
  sampleCount = 0;
  wTableType = 0;
  cursor = false;
  mindB = -60;
  maxdB = 0;
  
  wNames[0] = tr("Hamming");
  wNames[1] = tr( "Blackman" );
  wNames[2] = tr( "Blackman-Harris" );
  wNames[3] =  tr( "Hann" );
  wNames[4] = tr( "Bartllet-Hann" ) ;
  wNames[5] = tr( "Gaussian" ) ;
  wNames[6] =  tr( "Welch" ) ;
  wNames[7] = tr( "Barlet" ) ;
  wNames[8] =  tr( "Kaiser-Bessel" );
  wNames[9] = tr( "Flat Top" );

}

void SpectroGraphGUI::setFftSize(int size)
{
 
  processRunning.lock();
  paintRunning.lock();
   
  /* now should be safe proceding */
  sampleCount = 0;
  fftSize = size;
  /* free */
  if(fftIn != NULL){
    free(persistantBuffer);
    free(grid);
    persistantBuffer = NULL;
    fftw_free(fftIn);
    fftw_free(fftOut);
    free(fftData);
    free(fftdB);
    free(wTable);
    fftw_destroy_plan(p1);
  }

  /* allocate */
  fftIn=(fftw_complex* )fftw_malloc(sizeof(fftw_complex)*size);
  fftOut=(fftw_complex* )fftw_malloc(sizeof(fftw_complex)*size);

  fftData = (float *)malloc(sizeof(float)* size/2+1);
  fftdB =  (float *)malloc(sizeof(float)* size/2+1);
  /* ****************************************************
  * Create forward FFT plan from b1 into b2
  * ****************************************************/
  p1 = fftw_plan_dft_1d( fftSize, fftIn, fftOut, FFTW_FORWARD, FFTW_ESTIMATE);
  
   /* half should be enought */
  wTable = (float *)malloc(sizeof(float)*size);
  initWtable(size);

  processRunning.unlock();
  paintRunning.unlock();
}

void SpectroGraphGUI::initWtable(int size)
{
  /* 
  Have a look at http://en.wikipedia.org/wiki/Window_function 
  Code inspired from http://www.mtoussaint.de/qtdso.html
  */
  const float arg = 2.0*M_PI/(size-1);
  const float arg2 = 1./(size-1);

  float sum = 0.;
 
  /* Hamming */
  if(wTableType == 0){
    for (int i=0; i<size; i++){
      wTable[i] = (0.54-0.46*cos(arg*i));
      sum += wTable[i];  
    }
  }
  /* Blackman */
  if(wTableType == 1){
    for (int i=0; i<size; i++){
      wTable[i] = (0.42-0.5*cos(arg*i)+0.08*cos(2*arg*i));
      sum += wTable[i];  
    }
  }
  /* Blackman Harris*/
  if(wTableType == 2){
    for (int i=0; i<size; i++){
      wTable[i] =  (0.35875-0.48829*cos(arg*i)+
                    0.14128*cos(2.*arg*i)-
                    0.01168*cos(3*arg*i));
      sum += wTable[i];  
    }
  }
  /* Hann */
  if(wTableType == 3){
    for (int i=0; i<size; i++){
      wTable[i] =  2.*(0.5-0.5*cos(arg*i));
      sum += wTable[i];  
    }
  }
  /* Bartlett Hann*/
  if(wTableType == 4){
    for (int i=0; i<size; i++){
      wTable[i] = (0.62-0.48*fabs(arg2*i-0.5)+
                   0.38*cos(2*M_PI*(arg2*i-0.5)));
      sum += wTable[i];  
    }
  }
  /* Gaussian */
  if(wTableType == 5){
    for (int i=0; i<size; i++){
      float x = (i-size/2.)/(float)size*2.;
      wTable[i] = exp(-3*(x*x));
      sum += wTable[i];  
    }
  }
  /* Welch */
  if(wTableType == 6){
    for (int i=0; i<size; i++){
      float x = (i-size/2.)/(float)size*2.;
      wTable[i] = (1.-x*x);
      sum += wTable[i];  
    }
  }
  /* Bartlet */
  if(wTableType == 7){
    for (int i=0; i<size/2; i++){
      wTable[i] = 2.*((float)i/(float)size*2.); 
      sum += wTable[i];  
    }
    for (int i=size/2; i<size; i++){
      wTable[i] = 2.*((float)i/(float)size*2.); 
      sum += wTable[i];  
    }
  }
  /* Kaiser Bessel */
  if(wTableType == 8){
    for (int i=0; i<size; i++){
      wTable[i] =  (0.4021-0.4986*cos(arg*i)+
                    0.0981*cos(2.*arg*i)-
                    0.0012*cos(3*arg*i)); 
      sum += wTable[i];  
    }
  }
  /* Flat Top */
  if(wTableType == 9){
    for (int i=0; i<size; i++){
      wTable[i] = (0.2155 - 0.4159 * cos( arg * i ) +
                   0.2780 * cos( 2. * arg * i ) -
                   0.0836 * cos( 3. * arg * i ) +
                   0.0070 * cos( 4. * arg * i ) ); 
      sum += wTable[i];  
    }
  }

  /* Normalization */
  sum /= (float)size;
  for (int i=0; i<size; i++){
    wTable[i] /= sum;
  }
  
}


void SpectroGraphGUI::paintEvent(QPaintEvent *event) {

  unsigned sample;
  bool firstRun = false;

  if(persistantBuffer == NULL){
    persistantBuffer = new QImage(size(), QImage::Format_ARGB32_Premultiplied);
    grid = new QImage(size(), QImage::Format_ARGB32_Premultiplied);
    firstRun = true;
  }

  /* work around to clear info strings */
  if(gridInvalidate){
    free(grid);
    grid = new QImage(size(), QImage::Format_ARGB32_Premultiplied);
  }

  QPainter winPainter(this);
  QPainter gridPainter(grid);
  QPainter painter(persistantBuffer);

  /* beautiful, but extremelly slow */ 
  painter.setRenderHint(QPainter::Antialiasing, true);
        
  paintRunning.lock();
		
        /* Setup draw area */
	QRect validRect = rect() & event->rect();
	painter.setClipRect(validRect);
        //if (firstRun)
        painter.fillRect(validRect, Qt::black);

        int xMargin = 100; // all right
        int yMargin = 100; // half top, half bottom
        int xOff = 10;
        
        /* Draw area */
        int ySize = rect().height() - yMargin;
        int xSize =  rect().width() - xMargin- xOff;
        QFontMetrics font = painter.fontMetrics();
        QString label;

        if (firstRun || gridInvalidate){

          /* Window Title */
          gridPainter.setPen(QPen(Qt::lightGray, 1));
          gridPainter.drawText(validRect, Qt::AlignHCenter | Qt::AlignTop , tr("\nPower Spectrum"));

          cursorPos = fftSize/4;
        
          /* Informations line */
          gridPainter.setPen(QPen(Qt::cyan, 1));
          QString desc;
          desc.sprintf("\n(%d points, %s), %d kSample/s ", 
                       fftSize, (const char *)wNames[wTableType].toLatin1(),samplingRate/1000);
          gridPainter.drawText(validRect, Qt::AlignRight | Qt::AlignTop , desc);
    
          /* Draw Border */
          gridPainter.setPen(QPen(Qt::lightGray, 1));
          QRect drawRect(xMargin, yMargin/2, xSize, ySize);
          gridPainter.drawRect(drawRect);
              
          gridPainter.setPen(QPen(Qt::lightGray, 1));
          int pos;
          /* Scale */
          gridPainter.setPen(QPen(Qt::lightGray,1,Qt::DotLine,Qt::RoundCap,Qt::RoundJoin));
          for (pos = 1; pos < 10; pos ++){
            int x = (pos * (float) (xSize)) / 10 + xMargin;
            /* max freq = sampling/2 */
            label.clear();
            label.sprintf("%3.1f kHz", pos* (float)samplingRate/(2.0 *1000*10));
            gridPainter.drawLine(x + rect().x(), 0 + rect().y()+yMargin/2, x + rect().x(), ySize + rect().y()+yMargin/2);
            gridPainter.drawText(x + rect().x() - (font.averageCharWidth()*label.count()/2), ySize + rect().y()+yMargin/2+yMargin/4, label);
          }
    
          /* Horizontal */
          for (pos = 1; pos < 10; pos ++){
            int dB = (mindB-maxdB) / 10;
            label.clear();
            label.sprintf("%2d dB", pos*dB + maxdB);
            //qDebug("delta %d div 10 %d x pos %d", maxdB-mindB, dB, pos);
            int y = (pos * ySize) / 10 + yMargin/2;
            gridPainter.drawLine(rect().x()+xMargin, y + rect().y(), xSize + rect().x()+xMargin, y + rect().y());
            gridPainter.drawText(xMargin - font.averageCharWidth()*label.count()-font.averageCharWidth(), y+font.height()/2, label);
          }
          gridInvalidate = false;
        }
        /* VScale */
              
	/* Now, draw the data */
	painter.setPen(QPen(Qt::green,1,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));

        /* dB normalization */
        freqStep = (float)(xSize) / ((float)fftSize/2.0);
        double maxData = 0;
        for(sample = 0; sample < fftSize/2.0-1; sample++) {
          /* keep trace of max value */
          if (fftData[sample] > maxData){
            maxIdx = sample;
            maxData = fftData[sample];
          }          
        }
       
        for(sample = 0; sample < fftSize/2.0; sample++) {
          fftdB[sample] = 10 * log(fftData[sample]/maxData);
          /* saturate */
          if (fftdB[sample] > maxdB)
            fftdB[sample] = maxdB;
          if(fftdB[sample] < mindB)
            fftdB[sample] = mindB;
        }

        /* +5 and -10 are eye candy */
        int y0 = yMargin/2 + 5;
        float step = xMargin;
        float yScale =  (ySize -10)/ (float)(mindB-maxdB);  
        unsigned maxStep = xMargin;
	for(sample = 0; sample < fftSize/2.0-1; sample++) {
          painter.drawLine(step,(fftdB[sample]-maxdB)*yScale +y0 ,step+freqStep,(fftdB[sample+1]-maxdB)*yScale + y0);    
          step += freqStep;
        }
            
        painter.setPen(QPen(Qt::cyan, 1));
        /* show peak measure */
        if(showPeak){
          maxStep = maxIdx * freqStep + xMargin;
          label.clear();
          label.sprintf("%3.1f kHz", (float)(maxStep - xMargin)/freqStep * 
                        ((float)samplingRate / (float)fftSize) / 1000.0 );
          unsigned textPos = maxStep - (font.averageCharWidth()*label.count()/2);
          /* border check */
          if (textPos + (font.averageCharWidth()*label.count()) > rect().width())
            textPos = rect().width()-font.averageCharWidth()*label.count()-xOff;

          painter.drawText( textPos, 
                            fftData[maxIdx]-font.height(), label);
        }

        /* Show mouse measure */
        if(displayMouseMeasure){
          label.clear();
          label.sprintf("(%3.2f kHz)", (float)(measurePoint.x() - xMargin)/freqStep* ((float)samplingRate / fftSize) / 1000.0 );
          unsigned textPos = measurePoint.x() - (font.averageCharWidth()*label.count()/2);
          /* border check */
          if (textPos + (font.averageCharWidth()*label.count()) >  rect().width())
              textPos = rect().width()-font.averageCharWidth()*label.count()-xOff;
          painter.drawText(textPos, measurePoint.y(), label);
        }
        
        
        if(cursor){
          painter.setPen(QPen(Qt::cyan,1,Qt::DashDotLine,Qt::RoundCap,Qt::RoundJoin));
          /* hline */
          painter.drawLine(xMargin, fftdB[cursorPos]*yScale +y0, xSize+xMargin,  fftdB[cursorPos]*yScale +y0);
          /* vline*/
          painter.drawLine(freqStep*cursorPos+xMargin, fftdB[cursorPos]*yScale +y0-10 , 
                                      freqStep*cursorPos+xMargin, fftdB[cursorPos]*yScale +y0+10);

          label.clear();
          label.sprintf("  (%3.2f kHz)", cursorPos*samplingRate / (fftSize *1000.0));
          painter.drawText(xMargin , yMargin/2 - font.height(), label);
          
        }

        winPainter.setCompositionMode(QPainter::CompositionMode_Source);
        winPainter.drawImage(0,0, *persistantBuffer);        
        winPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        winPainter.drawImage(0,0, *grid);

        paintRunning.unlock();
}

void SpectroGraphGUI::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
    {
      measurePoint = event->pos();
      displayMouseMeasure = true;
    }
    if (event->button() == Qt::RightButton)
    {
      displayMouseMeasure = false;
    }
    if (event->button() ==Qt::MidButton){
       displayMouseMeasure = false;
    }
}

void SpectroGraphGUI::keyPressEvent(QKeyEvent *event){
  switch (event->key()){
  case Qt::Key_P:
    showPeak = !showPeak;
    break;
  case Qt::Key_C:
    cursor = !cursor;
    cursorPos = maxIdx;
    break;   
  case Qt::Key_Right:
    if(cursor)
      cursorPos++;
    break; 
  case Qt::Key_Left:
    if(cursor)
      cursorPos--;
    break; 
  case Qt::Key_Up:
    if(cursor)
      cursorPos+=100;
    break; 
  case Qt::Key_Down:
    if(cursor)
      cursorPos-=100;
    break; 
  case Qt::Key_D:
    qDebug("freqStep %f",freqStep);
    break;
  default:
    QWidget::keyPressEvent(event);
  }
}

void SpectroGraphGUI::resizeEvent(QResizeEvent *)
{
	if (persistantBuffer)
	{
          free(persistantBuffer);
          free(grid);
          persistantBuffer = NULL;
	}
}

Q_EXPORT_PLUGIN(ProcessingSpectroGraphDescription)
