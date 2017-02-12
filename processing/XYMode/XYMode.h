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

#ifndef __PROCESSING_XYMode
#define __PROCESSING_XYMode

#include <ProcessingPlugin.h>

/** This is for the custom XY mode widget **/
#include <QWidget>
#include <QColor>
#include <QImage>
#include <QPoint>
#include <QtGui>
#include <QList>
#include <QString>

//! Description of XYMode plugin
class ProcessingXYModeDescription : public QObject, public ProcessingPluginDescription
{
	Q_OBJECT
    Q_PLUGIN_METADATA(IID "ch.eig.lsn.Oscilloscope.ProcessingPluginDescription/1.0" FILE "xymode.json")
    Q_INTERFACES(ProcessingPluginDescription)

public:
	QString systemName() const;
	QString name() const;
	QString description() const;
	unsigned inputCount() const;
	unsigned outputCount() const;
	ProcessingPlugin *create(const DataSource *dataSource) const;
};



/**** This class is a custom widget for displaying XY Mode
Designed by Bharathwaj Muthuswamy following instructions by Stephane Magnenat
Reference: http://doc.trolltech.com/4.2/widgets-scribble.html
If you instantiate this class, you will create a new window for XY mode. 
***/

class XYModeGUI : public QWidget
{
	Q_OBJECT
public:
	XYModeGUI(QWidget *parent = 0);
	short *xData,*yData;
	unsigned sampleCount;
	double yPrescaleFactor;
	double xPrescaleFactor;
	double xOffset;
	double yOffset;
	int penWidth;

protected:
	void paintEvent(QPaintEvent *event);


private:
	bool modified;
	int myPenWidth;
	QColor myPenColor;
};

/** End of new class definition **/

class ProcessingXYMode : public QObject, public ProcessingPlugin
{
	Q_OBJECT
	
public:
	QWidget *createGUI(void);
	void processData(const std::valarray<signed short *> &inputs, const std::valarray<signed short *> &outputs, unsigned sampleCount);

	/* Modified function below to delete signal data in memory (used for plotting XY) */
	void terminate(void) { deleteLater();   free(newGUI->xData);  free(newGUI->yData);}

private slots:
	void yPrescaleFactorChanged(double);
	void xPrescaleFactorChanged(double);
	void xOffsetChanged(double);
	void yOffsetChanged(double);
	void penWidthChanged(int);

private:
	/* This is the new XY Mode GUI */
	XYModeGUI *newGUI;
	double yPrescaleFactor;
	double xPrescaleFactor;
	double xOffset;
	double yOffset;
	int penWidth;

	friend class ProcessingXYModeDescription;
	friend class XYModeGUI;
	ProcessingXYMode(const ProcessingPluginDescription *description);
};


#endif
