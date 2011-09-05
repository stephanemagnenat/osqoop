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

#ifndef __PROCESSING_SpectroGraph
#define __PROCESSING_SpectroGraph

#include <ProcessingPlugin.h>

/** This is for the custom Spectrograph widget **/
#include <QWidget>
#include <QColor>
#include <QImage>
#include <QPoint>
#include <QtGui>
#include <QList>
#include <QMainWindow>
#include <QString>

#include <fftw3.h>

#define MEANSIZE 32
enum {Power, Phase};
 
//! Description of SpectroGraph plugin
class ProcessingSpectroGraphDescription : public QObject, public ProcessingPluginDescription
{
	Q_OBJECT
	Q_INTERFACES(ProcessingPluginDescription)

public:
	QString systemName() const;
	QString name() const;
	QString description() const;
	unsigned inputCount() const;
	unsigned outputCount() const;
	ProcessingPlugin *create(const DataSource *dataSource) const;
};


class SpectroGraphGUI : public QWidget
{
	Q_OBJECT

public:
	SpectroGraphGUI(QWidget *parent = 0);

	unsigned sampleCount;
        fftw_complex *fftIn,*fftOut;
        float *fftData;
        float *fftdB;
        float *wTable;
        fftw_plan p1;
        unsigned fftSize;
        unsigned fftType;
        float freqStep;
        unsigned samplingRate;
        bool showPeak;
        unsigned fftMean;
        unsigned meanCnt;
        int mindB;
        int maxdB;
        QPoint measurePoint;
        bool displayMouseMeasure;
        QMutex processRunning;
        QMutex paintRunning;
        int wTableType;
        QString wNames[10];
        void setFftSize(int size);
        void initWtable(int size);   
        QImage *persistantBuffer;
        QImage *grid;
        bool gridInvalidate;
        bool cursor;
        int cursorPos;
        unsigned maxIdx;
      
protected:
        void mousePressEvent(QMouseEvent *event);
        void keyPressEvent(QKeyEvent *event);
	void paintEvent(QPaintEvent *event);
        void resizeEvent(QResizeEvent *);

/*
private:
*/        

};

/** End of new class definition **/
class ProcessingSpectroGraph : public QObject, public ProcessingPlugin
{
	Q_OBJECT
	
public:
	QWidget *createGUI(void);
	void processData(const std::valarray<signed short *> &inputs, 
                                     const std::valarray<signed short *> &outputs, unsigned sampleCount);

	/* Modified function below to delete signal data in memory */
	void terminate(void) { 
          deleteLater();
          fftw_free(newGUI->fftIn);    
          fftw_free(newGUI->fftOut);    
          free(newGUI->fftData);  
          fftw_destroy_plan(newGUI->p1);
          newGUI->close();
          delete newGUI;
          }

private slots:
	void peakDisplayChanged(int val);
        void fftSizeChanged(int val);
        void meanModeChanged(int val);
        void wTableChanged(int val);
        void dBminChanged(int val);
        void dBmaxChanged(int val);

private:
	
	SpectroGraphGUI *newGUI;
        const DataSource *dataSource; //!< data source, to get sampling rate
      
	friend class ProcessingSpectroGraphDescription;
	friend class SpectroGraphGUI;
	ProcessingSpectroGraph(const ProcessingPluginDescription *description, const DataSource *dataSource);
};


#endif
