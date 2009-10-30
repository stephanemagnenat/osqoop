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
#include <QDoubleSpinBox>
#include "XYMode.h"
#include <XYMode.moc>
#include <stdio.h>


QString ProcessingXYModeDescription::systemName() const
{
	return QString("XYMode");
}

QString ProcessingXYModeDescription::name() const
{
	return QString("XYMode");
}

QString ProcessingXYModeDescription::description() const
{
	return QString("Returns a plot of X vs. Y");
}

unsigned ProcessingXYModeDescription::inputCount() const
{
	return 2;
}

unsigned ProcessingXYModeDescription::outputCount() const
{
	return 0;
}

ProcessingPlugin *ProcessingXYModeDescription::create(const DataSource *dataSource) const
{
	return new ProcessingXYMode(this);
}


ProcessingXYMode::ProcessingXYMode(const ProcessingPluginDescription *description) :
	ProcessingPlugin(description)
{
	newGUI = new XYModeGUI;

	/* Allocate memory here.
		Cannot find a max limit for sample count, hence using 512 experimentally */
	newGUI->xData = (short *) malloc(512 * sizeof(short));
	newGUI->yData = (short *) malloc(512 * sizeof(short)); 

	xPrescaleFactor = 0.02;
	yPrescaleFactor = 0.04;

	xTranslationFactor = 350;
	yTranslationFactor = 200;
	
	newGUI->show();
}

void ProcessingXYMode::yPrescaleFactorChanged(double value) {
	yPrescaleFactor = value;
}

void ProcessingXYMode::xPrescaleFactorChanged(double value) {
	xPrescaleFactor = value;
}

void ProcessingXYMode::xTranslationFactorChanged(double value) {
	xTranslationFactor = value;
}

void ProcessingXYMode::yTranslationFactorChanged(double value) {
	yTranslationFactor = value;
}

QWidget *ProcessingXYMode::createGUI(void)
{   
	QWidget *guiBase = new QWidget;
	QGridLayout *layout = new QGridLayout(guiBase);

	QDoubleSpinBox *xPrescaleFactorSpinBox = new QDoubleSpinBox;
	xPrescaleFactorSpinBox->setRange(0.0,1.0);
	xPrescaleFactorSpinBox->setSingleStep(0.01);
	xPrescaleFactorSpinBox->setValue(xPrescaleFactor);
	connect(xPrescaleFactorSpinBox, SIGNAL(valueChanged(double)), SLOT(xPrescaleFactorChanged(double)));

	QDoubleSpinBox *yPrescaleFactorSpinBox = new QDoubleSpinBox;
	yPrescaleFactorSpinBox->setRange(0.0,1.0);
	yPrescaleFactorSpinBox->setSingleStep(0.01);
	yPrescaleFactorSpinBox->setValue(yPrescaleFactor);
	connect(yPrescaleFactorSpinBox, SIGNAL(valueChanged(double)), SLOT(yPrescaleFactorChanged(double)));

	QDoubleSpinBox *xTranslationFactorSpinBox = new QDoubleSpinBox;
	xTranslationFactorSpinBox->setRange(0.0,500.0);
	xTranslationFactorSpinBox->setValue(xTranslationFactor);
	connect(xTranslationFactorSpinBox, SIGNAL(valueChanged(double)), SLOT(xTranslationFactorChanged(double)));

	QDoubleSpinBox *yTranslationFactorSpinBox = new QDoubleSpinBox;
	yTranslationFactorSpinBox->setRange(0.0,500.0);
	yTranslationFactorSpinBox->setValue(yTranslationFactor);
	connect(yTranslationFactorSpinBox, SIGNAL(valueChanged(double)), SLOT(yTranslationFactorChanged(double)));
	
	layout->addWidget(new QLabel(tr("X Axis Prescale Factor [0..1]")), 1, 0);
	layout->addWidget(xPrescaleFactorSpinBox, 1, 1);
	layout->addWidget(new QLabel(tr("Y Axis Prescale Factor [0..1]")), 2, 0);
	layout->addWidget(yPrescaleFactorSpinBox, 2, 1);

	layout->addWidget(new QLabel(tr("X Axis Translation Factor [0..500]")), 3, 0);
	layout->addWidget(xTranslationFactorSpinBox, 3, 1);
	layout->addWidget(new QLabel(tr("Y Axis Translation Factor [0..500]")), 4, 0);
	layout->addWidget(yTranslationFactorSpinBox, 4, 1);
	
	return guiBase;
}

void ProcessingXYMode::processData(const std::valarray<signed short *> &inputs, const std::valarray<signed short *> &outputs, unsigned sampleCount)
{  
	newGUI->sampleCount = sampleCount;

	newGUI->xPrescaleFactor = xPrescaleFactor;
	newGUI->yPrescaleFactor = yPrescaleFactor;
	newGUI->xTranslationFactor = xTranslationFactor;
	newGUI->yTranslationFactor = yTranslationFactor;


	/* Initialize (duh) */
	for(unsigned sample = 0;sample < sampleCount;sample++) {
		newGUI->xData[sample]  = 0;
		newGUI->yData[sample]  = 0;
	}

	/* Read in values (duh) */
	for(unsigned sample = 0;sample < sampleCount;sample++) {
		newGUI->xData[sample]  = inputs[0][sample];
		newGUI->yData[sample]  = inputs[1][sample];
	}

	newGUI->update();  
}



/**** This class is a custom widget for displaying XY Mode
Designed by Bharathwaj Muthuswamy following instructions by Stephane Magnenat
Reference: http://doc.trolltech.com/4.2/widgets-scribble.html 
***/

XYModeGUI::XYModeGUI(QWidget *parent) : QWidget(parent) {
	/* You can change the color among these defaults, 
	   check the createGUI function.  */
	modified = false;
	myPenWidth = 1;
	myPenColor = Qt::red;
}

void XYModeGUI::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	/* Although I initialized xData and yData using
		sampleCount, I used a fixed 512 sample count 
		because of runtime errors in Windows */
	const int maxSample = 512;
	int xCoord1,yCoord1,xCoord2,yCoord2;
	
	/* This code is from SignalViewWidget::paintEvent, SignalViewWidget::drawGrid and SignalViewWidget::drawData 
	check ../src/SignalViewWidget.cpp */

	/* Draw the grid */
	int xMean = rect().width() >> 1;
	int yMean = rect().height() >> 1;
	int tickWidth = rect().width() / 100;

	painter.setPen(QPen(Qt::darkGray, 1));
	
	for (int pos = 1; pos < 10; pos ++)
	{
		int x = (pos * rect().width()) / 10;
		painter.drawLine(x + rect().x(), 0 + rect().y(), x + rect().x(), rect().height() + rect().y());
	}
	if (rect().width() >= 300)
		for (int pos = 1; pos < 100; pos ++)
		{
			int x = (pos * rect().width()) / 100;
			painter.drawLine(x + rect().x(), yMean-tickWidth + rect().y(), x + rect().x(), yMean+tickWidth + rect().y());
		}
	for (int pos = 1; pos < 10; pos ++)
	{
		int y = (pos * rect().height()) / 10;
		painter.drawLine(0 + rect().x(), y + rect().y(), rect().width() + rect().x(), y + rect().y());
	}
	if (rect().height() >= 300)
		for (int pos = 1; pos < 100; pos ++)
		{
			int y = (pos * rect().height()) / 100;
			painter.drawLine(xMean-tickWidth + rect().x(), y + rect().y(), xMean+tickWidth + rect().x(), y + rect().y());
		}

	/* Now, draw the data */

	/* Setup draw area */
	QRect validRect = rect() & event->rect();
	painter.setClipRect(validRect);
	
	/* New pen style for drawing */
	painter.setPen(QPen(Qt::red,1,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));

	/*drawRect is validRect, targetRect is rect */
	for(unsigned sample = 0;sample < maxSample-1;sample++) {

		xCoord1 =  (qint64) ((xData[sample] * xPrescaleFactor) + xTranslationFactor);
		yCoord1 =  (qint64) ((yData[sample] * yPrescaleFactor) + yTranslationFactor);

		xCoord2 =  (qint64) ((xData[sample+1] * xPrescaleFactor) + xTranslationFactor);
		yCoord2 =  (qint64) ((yData[sample+1] * yPrescaleFactor) + yTranslationFactor);
		
		painter.drawLine(xCoord1,yCoord1,xCoord2,yCoord2);
	}
}

Q_EXPORT_PLUGIN(ProcessingXYModeDescription)
