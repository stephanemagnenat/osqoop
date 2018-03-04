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
#include <QGridLayout>
#include <QLabel>
#include "XYMode.h"
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
	Q_UNUSED(dataSource);
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

	xPrescaleFactor = 1.0;
	yPrescaleFactor = 1.0;

	xOffset = 0;
	yOffset = 0;
		
	penWidth = 5;
	
	newGUI->show();
}

void ProcessingXYMode::yPrescaleFactorChanged(double value) {
	yPrescaleFactor = value;
}

void ProcessingXYMode::xPrescaleFactorChanged(double value) {
	xPrescaleFactor = value;
}

void ProcessingXYMode::xOffsetChanged(double value) {
	xOffset = value;
}

void ProcessingXYMode::yOffsetChanged(double value) {
	yOffset = value;
}

void ProcessingXYMode::penWidthChanged(int value) {
	penWidth = value;
}

QWidget *ProcessingXYMode::createGUI(void)
{   
	QWidget *guiBase = new QWidget;
	QGridLayout *layout = new QGridLayout(guiBase);

	QDoubleSpinBox *xPrescaleFactorSpinBox = new QDoubleSpinBox;
	xPrescaleFactorSpinBox->setRange(0.0,100.0);
	xPrescaleFactorSpinBox->setValue(xPrescaleFactor);
	xPrescaleFactorSpinBox->setSingleStep(0.01);
	connect(xPrescaleFactorSpinBox, SIGNAL(valueChanged(double)), SLOT(xPrescaleFactorChanged(double)));

	QDoubleSpinBox *yPrescaleFactorSpinBox = new QDoubleSpinBox;
	yPrescaleFactorSpinBox->setRange(0.0,100.0);
	yPrescaleFactorSpinBox->setValue(yPrescaleFactor);
	yPrescaleFactorSpinBox->setSingleStep(0.01);
	connect(yPrescaleFactorSpinBox, SIGNAL(valueChanged(double)), SLOT(yPrescaleFactorChanged(double)));

	QDoubleSpinBox *xOffsetSpinBox = new QDoubleSpinBox;
	xOffsetSpinBox->setRange(-1000.0,1000.0);
	xOffsetSpinBox->setValue(xOffset);
	connect(xOffsetSpinBox, SIGNAL(valueChanged(double)), SLOT(xOffsetChanged(double)));

	QDoubleSpinBox *yOffsetSpinBox = new QDoubleSpinBox;
	yOffsetSpinBox->setRange(-1000.0,1000.0);
	yOffsetSpinBox->setValue(yOffset);
	connect(yOffsetSpinBox, SIGNAL(valueChanged(double)), SLOT(yOffsetChanged(double)));
		
	QSpinBox *penWidthSpinBox = new QSpinBox;
	penWidthSpinBox->setRange(1,100);
	penWidthSpinBox->setValue(penWidth);
	connect(penWidthSpinBox, SIGNAL(valueChanged(int)), SLOT(penWidthChanged(int)));

  
	layout->addWidget(new QLabel(tr("X Axis Prescale Factor [0 100]")), 1, 0);
	layout->addWidget(xPrescaleFactorSpinBox, 1, 1);
	layout->addWidget(new QLabel(tr("Y Axis Prescale Factor [0 100]")), 2, 0);
	layout->addWidget(yPrescaleFactorSpinBox, 2, 1);

	layout->addWidget(new QLabel(tr("X Axis Offset [-1000 1000]")), 3, 0);
	layout->addWidget(xOffsetSpinBox, 3, 1);
	layout->addWidget(new QLabel(tr("Y Axis Offset [-1000 1000]")), 4, 0);
	layout->addWidget(yOffsetSpinBox, 4, 1);
		
	layout->addWidget(new QLabel(tr("Dot Width [1 100]")), 5, 0);
	layout->addWidget(penWidthSpinBox, 5, 1);

	return guiBase;
}

void ProcessingXYMode::processData(const std::valarray<signed short *> &inputs, const std::valarray<signed short *> &outputs, unsigned sampleCount)
{  
	Q_UNUSED(outputs);

	newGUI->sampleCount = sampleCount;

	newGUI->xPrescaleFactor = xPrescaleFactor;
	newGUI->yPrescaleFactor = yPrescaleFactor;
	newGUI->xOffset = xOffset;
	newGUI->yOffset = yOffset;
	newGUI->penWidth = penWidth;

	/* Read in values (duh) */
	for(unsigned sample = 0;sample < sampleCount;sample++)
	{
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
	myPenWidth = penWidth;
	myPenColor = Qt::red;
}

void XYModeGUI::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	/* Although I initialized xData and yData using
		sampleCount, I used a fixed 512 sample count 
		because of runtime errors in Windows */
	const int maxSample = 512;
	qint64 xCoord1,yCoord1,xCoord2,yCoord2;
	
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
	 painter.setPen(QPen(Qt::red,penWidth,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));

	/*drawRect is validRect, targetRect is rect */
	for(unsigned sample = 0;sample < maxSample-1;sample++)
	{
		xCoord1 =  (qint64) ((xData[sample] / 10000.0 * xPrescaleFactor + xOffset / 10) * rect().width() + xMean);
		yCoord1 =  (qint64) ((yData[sample] / 10000.0 * yPrescaleFactor + yOffset / 10) * (-rect().height()) + yMean);

		xCoord2 =  (qint64) ((xData[sample+1] / 10000.0 * xPrescaleFactor + xOffset / 10) * rect().width() + xMean);
		yCoord2 =  (qint64) ((yData[sample+1] / 10000.0 * yPrescaleFactor + yOffset / 10) * (-rect().height()) + yMean);
		
		painter.drawLine(xCoord1,yCoord1,xCoord2,yCoord2);
	}
}

