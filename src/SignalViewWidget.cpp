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

#include "SignalViewWidget.h"
#include <SignalViewWidget.moc>
#include "DataConverter.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMessageBox>
#include <QSizePolicy>
#include <QMenu>
#include <QPixmap>
#include <QDateTime>
#include <QApplication>
#include <QLineEdit>
#include <QStringList>
#include <QSettings>
#include <QtDebug>
#include <QtGlobal>
#include <cassert>


//! Constructor. channelCount is the number of channel. parent is the parent widget, if any. signalInfo is a struct shared by parent that actually holds datas
SignalViewWidget::SignalViewWidget(const SignalDisplayData *signalInfo, int yPrescaleFactor, bool zoomed, QWidget *parent) :
	QWidget(parent),
	channelEnabledMask((unsigned)-1),
	signalInfo(signalInfo),
	zoomed(zoomed),
	yPrescaleFactor(yPrescaleFactor),
	yDivisionFactor(signalInfo->channelCount, 1000),
	yShiftFactor(signalInfo->channelCount, 0)
{

	Q_ASSERT(signalInfo->channelCount != 0);
	for (unsigned i = 0; i < signalInfo->channelCount; i++)
	{
		yShiftFactor[i] = 160 - i * (320 / signalInfo->channelCount);
	}

	useAntialiasing = false;
	doPersistanceFadeout = false;
	useAlphaBlending = false;
	isZoomMarker = false;
	movingChannelShift = -1;
	movingTrigger = false;
	movingZoomMarker = false;
	editingChannelName = -1;
	channelNameEditing = new QLineEdit(this);
	connect(channelNameEditing, SIGNAL(returnPressed()), SLOT(channelNameChanged()));
	channelNameEditing->hide();
	drawingMode = LineDrawing;
	persistantBuffer = NULL;
	zoomStartPos = 0;
	zoomEndPos = 0;

	createActionsAndMenus();
	
	if (!zoomed)
		setFocusPolicy(Qt::StrongFocus);
}

//! Destructor, cleanup things
SignalViewWidget::~SignalViewWidget()
{
	disableDisplayPersistance();
}

//! Return the best size for this widget
QSize SignalViewWidget::sizeHint () const
{
	return QSize(300, 300);
}

//! Return the minimum size for this widget
QSize SignalViewWidget::minimumSizeHint () const
{
	return QSize(20, 20);
}

//! Return the size policy for this widget
QSizePolicy SignalViewWidget::sizePolicy () const
{
	return QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

//! Enable channel a channel which will then be visible
void SignalViewWidget::enableChannel(unsigned channel)
{
	channelEnabledMask |= (1<<channel);
	update();
}

//! Disable channel a channel which will then be hidden
void SignalViewWidget::disableChannel(unsigned channel)
{
	channelEnabledMask &= ~(1<<channel);
	update();
}

//! Enable/disable channel a channel which will then be visible/hidden
void SignalViewWidget::setChannelEnabled(unsigned channel, bool enabled)
{
	if (enabled)
		enableChannel(channel);
	else
		disableChannel(channel);
	update();
}

//! Return true if channel is enabled (i.e. visible)
bool SignalViewWidget::channelEnabled(unsigned channel) const
{
	return (channelEnabledMask & (1<<channel)) != 0;
}

//! Return true if antialiasing is enabled
bool SignalViewWidget::antialiasing() const
{
	return useAntialiasing;
}

//! Return true if alpha-blending is enabled
bool SignalViewWidget::alphaBlending() const
{
	return useAlphaBlending;
}

//! Return true if display is persistant (i.e. old signals are visible under new ones)
bool SignalViewWidget::displayPersistance() const
{
	return persistantBuffer != NULL;
}

//! Return true if persistance fadeout (i.e. old signals get lighter and lighter)
bool SignalViewWidget::persistanceFadeout() const
{
	return doPersistanceFadeout;
}

//! Return true if zoom marker are enabled
bool SignalViewWidget::zoomMarker() const
{
	return isZoomMarker;
}

//! Return the left zoom marker in sample coordinates
int SignalViewWidget::zoomPosStart() const
{
	return zoomStartPos;
}

//! Return the right zoom marker in sample coordinates
int SignalViewWidget::zoomPosEnd() const
{
	return zoomEndPos;
}

//! New data are available in the SignalDisplayData structure
void SignalViewWidget::newDataReady(int startSample, int endSample)
{
	// we have new data, resize vector if required
	if (signalInfo->channelCount != yDivisionFactor.size())
		yDivisionFactor.resize(signalInfo->channelCount, 1000);
	if (signalInfo->channelCount != yShiftFactor.size())
		yShiftFactor.resize(signalInfo->channelCount, 0);

	if (!zoomed)
	{
		// optimised data
		if (width() * signalInfo->channelCount != optimisedData.size())
			optimisedData.resize(width() * signalInfo->channelCount);
		regenerateOptimisedData(startSample, endSample);	
	
		// checkable timescale menu
		QAction *oldCheckedTimeScaleAction = timescaleGroup->checkedAction();
		unsigned actionIndex = getScaleFactorInvert(signalInfo->duration);
		Q_ASSERT(actionIndex < ScaleFactorCount);
		QAction *newCheckedTimeScaleAction = timeScaleAct[actionIndex];
		if (oldCheckedTimeScaleAction != newCheckedTimeScaleAction)
			newCheckedTimeScaleAction->setChecked(true);
	}
		
	// clip zoom positions
	zoomStartPos = signalInfo->clipSamplePos(zoomStartPos);
	zoomEndPos = signalInfo->clipSamplePos(zoomEndPos);

	// update display
	persistantBufferDirty = true;
	update();
}

//! Regenerate pixel-aligned data for optimised view
void SignalViewWidget::regenerateOptimisedData(int startSample, int endSample)
{
	int w = width();
	int startPixel = sampleToScreenX(startSample, w);
	int endPixel = sampleToScreenX(endSample, w);

	Q_ASSERT(startPixel >= 0);
	endPixel = std::min(endPixel, w);
	Q_ASSERT(endPixel <= (int)(optimisedData.size() / signalInfo->channelCount));

	for (unsigned channel = 0; channel < signalInfo->channelCount; channel++)
	{
		const short int *dataPtr = &(signalInfo->data[channel * signalInfo->sampleCount()]);
		for (int pixel = startPixel; pixel < endPixel; pixel++)
		{
			unsigned startSubSample = screenToSampleX(pixel, w);
			unsigned endSubSample = screenToSampleX(pixel+1, w);
			endSubSample = std::min(endSubSample, signalInfo->sampleCount()-1);

			int min = dataPtr[startSubSample];
			int max = min;
			for (unsigned sample = startSubSample + 1; sample < endSubSample; sample++)
			{
				min = std::min(min, (int)dataPtr[sample]);
				max = std::max(min, (int)dataPtr[sample]);
			}
			optimisedData[w * channel + pixel] = QPoint(min, max);
		}
	}
}

//! Enable/disable antialiasing
void SignalViewWidget::setAntialiasing(bool enabled)
{
	useAntialiasing = enabled;
	update();
}

//! Enable/disable alpha-blending
void SignalViewWidget::setAlphaBlending(bool enabled)
{
	useAlphaBlending = enabled;
	update();
}

//! Enable/disable display persistance
void SignalViewWidget::setDisplayPersistance(bool enabled)
{
	if (enabled)
		enableDisplayPersistance();
	else
		disableDisplayPersistance();
	update();
}

//! Enable/disable display persistance fadeout
void SignalViewWidget::setPersistanceFadeout(bool enabled)
{
	doPersistanceFadeout = enabled;
	update();
}

//! Enable/disable the zoom markers, thus enable/disable zoom. Emit zoomPosChanged
void SignalViewWidget::setZoomMarker(bool enabled)
{
	isZoomMarker = enabled;
	setZoomPos(signalInfo->sampleCount() / 4, (signalInfo->sampleCount() * 3) / 4);
}

//! Set the zoom markers. Emit zoomPosChanged
void SignalViewWidget::setZoomPos(int start, int end)
{
	if (start > end)
		std::swap(start, end);
	zoomStartPos = signalInfo->clipSamplePos(start);
	zoomEndPos = signalInfo->clipSamplePos(end);
	emit(zoomPosChanged(zoomStartPos, zoomEndPos));
	update();
}

//! Layout channels nicely and automatically
void SignalViewWidget::autoLayoutChannels(void)
{
	// count the number of active channel
	int channelEnabledCount = 0;
	for (unsigned channel = 0; channel < signalInfo->channelCount; channel++)
		channelEnabledCount += channelEnabled(channel) ? 1 : 0;
	if (channelEnabledCount == 0)
		return;
	
	// handcrafted nice values for low channel count
	int newShiftInc;
	int newShift;
	if (channelEnabledCount == 2)
	{
		newShiftInc = -40;
		newShift = 20;
	}
	else if (channelEnabledCount == 3)
	{
		newShiftInc = -30;
		newShift = 30;
	}
	else if (channelEnabledCount == 4)
	{
		newShiftInc = -20;
		newShift = 30;
	}
	else if (channelEnabledCount == 5)
	{
		newShiftInc = -20;
		newShift = 40;
	}
	else if (channelEnabledCount == 6)
	{
		newShiftInc = -15;
		newShift = 40;
	}
	else if (channelEnabledCount == 7)
	{
		newShiftInc = -15;
		newShift = 45;
	}
	else if (channelEnabledCount % 2 == 0)
	{
		newShiftInc = -100 / (channelEnabledCount+2);
		newShift = 50 + newShiftInc;
	}
	else
	{
		 newShiftInc = -100 / (channelEnabledCount+1);
		 newShift = 50 + newShiftInc;
	}
	
	// we set yDivisionFactor[channel]
	int limit = -newShiftInc / 2; // limit in 100
	for (unsigned channel = 0; channel < signalInfo->channelCount; channel++)
	{
		int amplitude;
		signalInfo->channelAmplitude(channel, NULL, &amplitude, NULL);

		// find the most appropriate scale
		unsigned scale;
		for (scale = 0; scale < 15; scale++)
		{
			// transform amplitude to amount of screen and want it lower than -newShiftInc / 2 in fix point
			int ratio = ((qint64)amplitude * (qint64)1000 * (qint64)100) / ((qint64)yPrescaleFactor * (qint64)getScaleFactor(scale));
			if (ratio < limit)
				break;
		}
		scale = std::min(scale, (unsigned)14);
		yDivisionFactor[channel] = getScaleFactor(scale);
	}
	
	// we count as 1/10 of grid line, which yDivisionFactor[channel] / 100
	if (channelEnabledCount % 2 == 0)
	{
		// even (pair)
		unsigned channel = 0;
		unsigned chanUsed = 0;
		while ((channel < signalInfo->channelCount) && (chanUsed <  (unsigned)channelEnabledCount / 2))
		{
			if (channelEnabled(channel))
			{
				yShiftFactor[channel] = (newShift * yDivisionFactor[channel]) / 100;
				newShift += newShiftInc;
				chanUsed++; 
			}
			channel++;
		}
		
		newShift = -newShift + newShiftInc;
		while (channel < signalInfo->channelCount)
		{
			if (channelEnabled(channel))
			{
				yShiftFactor[channel] = (newShift * yDivisionFactor[channel]) / 100;
				newShift += newShiftInc;
			}
			channel++;
		}
	}
	else
	{
		// odd (impaire)
		for (unsigned channel = 0; channel < signalInfo->channelCount; channel++)
		{
			if (channelEnabled(channel))
			{
				yShiftFactor[channel] = (newShift * yDivisionFactor[channel]) / 100;
				newShift += newShiftInc;
			}
		}
	}
}

//! Enable display persistance
void SignalViewWidget::enableDisplayPersistance(void)
{
	disableDisplayPersistance();
	persistantBuffer = new QPixmap(size());
	QPainter painter(persistantBuffer);
	painter.fillRect(rect(), Qt::white);
	persistantBufferDirty = true;
}

//! Disable display persistance
void SignalViewWidget::disableDisplayPersistance(void)
{
	if (persistantBuffer)
		delete persistantBuffer;
	persistantBuffer = NULL;
}

//! Draw a printable friendly version of signal on painter in targetRect area
void SignalViewWidget::drawForPrinting(QPainter *painter, const QRect &targetRect, bool blackAndWhite, qreal penWidth)
{
	drawGrid(painter, targetRect, blackAndWhite, penWidth);

	// draw data nicely
	QPen newPen = painter->pen();
	newPen.setCapStyle(Qt::RoundCap);
	newPen.setJoinStyle(Qt::RoundJoin);
	painter->setPen(newPen);
	drawData(painter, targetRect, targetRect, blackAndWhite, penWidth);

	// font dimensions
	QFontMetrics fm(painter->font());
	int lineSpacing = fm.lineSpacing();
	
	// compute time and scale dimensions
	int totalHeight = lineSpacing;
	int maxWidth = fm.width(QString("%0").arg(timeScaleToStringWidthToDiv(signalInfo->duration)));
	for (unsigned channel = 0; channel < signalInfo->channelCount; channel++)
		if (channelEnabled(channel))
		{
			totalHeight += lineSpacing;
			maxWidth = std::max(maxWidth, fm.width(QString("%0 : %1").arg(channelNumberToString(channel)).arg(YScaleToString(yDivisionFactor[channel]))));
		}

	// fill background
	painter->fillRect(targetRect.x(), targetRect.y(), maxWidth, totalHeight, QColor(255, 255, 255, 180));
	
	// draw time and scale
	QPoint textPos(targetRect.x(), targetRect.y());
	painter->setPen(QPen(Qt::black, penWidth));
	painter->drawText(textPos, QString("%0").arg(timeScaleToStringWidthToDiv(signalInfo->duration)));
	textPos.ry() += lineSpacing;
	for (unsigned channel = 0; channel < signalInfo->channelCount; channel++)
		if (channelEnabled(channel))
		{
			if (!blackAndWhite)
				painter->setPen(QPen(getChannelColor(channel), penWidth));
			painter->drawText(textPos, QString("%0 : %1").arg(channelNumberToString(channel)).arg(YScaleToString(yDivisionFactor[channel])));
			textPos.ry() += lineSpacing;
		}

	// draw date
	QString dateTime = QDateTime::currentDateTime().toString();
	painter->setPen(QPen(Qt::black, penWidth));
	painter->fillRect(targetRect.width() - fm.width(dateTime), targetRect.y(), fm.width(dateTime), fm.height(), QColor(255, 255, 255, 180));
	painter->drawText(targetRect.width() - fm.width(dateTime), targetRect.y(), dateTime);
}

//! Implementation of painting. event is the zone to redraw
void SignalViewWidget::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	QRect validRect = rect() & event->rect();
	painter.setClipRect(validRect);
	painter.fillRect(validRect, Qt::white);
	
	// draw persistant buffer if any
	if (persistantBuffer)
	{
		if (persistantBufferDirty)
		{
			QPainter painter(persistantBuffer);
			if (doPersistanceFadeout)
				painter.fillRect(persistantBuffer->rect(), QColor(255, 255, 255, 8));

			if (!zoomed && (int)signalInfo->sampleCount() > width())
				drawDataOptimised(&painter, persistantBuffer->rect());
			else
				drawData(&painter, persistantBuffer->rect(), persistantBuffer->rect(), false);
			persistantBufferDirty = false;
		}
		painter.drawPixmap(0, 0, *persistantBuffer);
	}

	// draw grid
	drawGrid(&painter, rect(), false);

	// draw data if not persistant buffer
	if (persistantBuffer == NULL)
	{
		if (!zoomed && (int)signalInfo->sampleCount() > width())
			drawDataOptimised(&painter, validRect);
		else
			drawData(&painter, validRect, rect(), false);
	}

	// channel shift
	for (unsigned channel = 0; channel < signalInfo->channelCount; channel++)
		if (channelEnabled(channel))
			drawYTriangle(&painter, shiftToScreenY(channel, height()), channel, (int)channel == movingChannelShift, false);
	
	// trigger
	if (!zoomed && (signalInfo->dataConverter->triggerType() != DataConverter::TRIGGER_NONE) && signalInfo->data.size())
	{
		unsigned channel = signalInfo->dataConverter->triggerChannel();
		if (channelEnabled(channel))
		{
			drawYTriangle(&painter, sampleToScreenY(channel, signalInfo->dataConverter->triggerValue(), height()) + shiftToScreenY(channel, height()), channel, true, true);
			drawXTriangle(&painter, sampleToScreenX(signalInfo->dataConverter->triggerPos(), width()), channel, true, true);
		}
	}
	
	// zoom marker
	if (!zoomed && isZoomMarker)
	{
		painter.setPen(Qt::darkGray);
		int xPosLeft = sampleToScreenX(zoomStartPos, width());
		painter.drawLine(xPosLeft, 0, xPosLeft, height());
		int xPosRight = sampleToScreenX(zoomEndPos, width());
		painter.drawLine(xPosRight, 0, xPosRight, height());
		QString s1, s2;
		QRect infoRect = getZoomInfoRect((xPosLeft + xPosRight) / 2, &s1, &s2);
		if (useAlphaBlending)
		{
			painter.setPen(QColor(127,127,127,200));
			painter.setBrush(QBrush(QColor(255,255,255,200)));
		}
		else
		{
			painter.setPen(QColor(127,127,127));
			painter.setBrush(QBrush(QColor(255,255,255)));
		}
		painter.drawRect(infoRect);
		painter.drawText(infoRect, Qt::AlignTop | Qt::AlignHCenter, s1);
		painter.drawText(infoRect, Qt::AlignBottom | Qt::AlignHCenter, s2);
	}

	// draw informations
	if (!zoomed)
		drawInfoBubble(&painter, 12, 12, timeScaleToStringWidthToDiv(signalInfo->duration));
	else
		drawInfoBubble(&painter, 6, 12, signalInfo->sampleToTimeString((double)(zoomEndPos - zoomStartPos) / 10));
	int xPos = 65;
	for (unsigned channel = 0; channel < signalInfo->channelCount; channel++)
	{
		if (channelEnabled(channel))
			drawInfoBubble(&painter, xPos, 12, QString("%0:%1").arg(channelNumberToString(channel)).arg(YScaleToString(yDivisionFactor[channel])), getChannelColor(channel));
		else
			drawInfoBubble(&painter, xPos, 12, QString("%0:disabled").arg(channelNumberToString(channel)), Qt::gray);
		xPos += 65;
	}
}

//! Draw the grid in targetRect area
void SignalViewWidget::drawGrid(QPainter *painter, const QRect &targetRect, bool blackAndWhite, qreal penWidth)
{
	int xMean = targetRect.width() >> 1;
	int yMean = targetRect.height() >> 1;
	int tickWidth = targetRect.width() / 100;

	if (blackAndWhite)
		painter->setPen(QPen(Qt::darkGray, penWidth));
	else
		painter->setPen(QPen(QColor(220, 220, 220), penWidth));
	for (int pos = 1; pos < 10; pos ++)
	{
		int x = (pos * targetRect.width()) / 10;
		painter->drawLine(x + targetRect.x(), 0 + targetRect.y(), x + targetRect.x(), targetRect.height() + targetRect.y());
	}
	if (targetRect.width() >= 300)
		for (int pos = 1; pos < 100; pos ++)
		{
			int x = (pos * targetRect.width()) / 100;
			painter->drawLine(x + targetRect.x(), yMean-tickWidth + targetRect.y(), x + targetRect.x(), yMean+tickWidth + targetRect.y());
		}
	for (int pos = 1; pos < 10; pos ++)
	{
		int y = (pos * targetRect.height()) / 10;
		painter->drawLine(0 + targetRect.x(), y + targetRect.y(), targetRect.width() + targetRect.x(), y + targetRect.y());
	}
	if (targetRect.height() >= 300)
		for (int pos = 1; pos < 100; pos ++)
		{
			int y = (pos * targetRect.height()) / 100;
			painter->drawLine(xMean-tickWidth + targetRect.x(), y + targetRect.y(), xMean+tickWidth + targetRect.x(), y + targetRect.y());
		}
}

//! Draw the curves in targetRect area, clip using drawRect area
void SignalViewWidget::drawData(QPainter *painter, const QRect &drawRect, const QRect &targetRect, bool blackAndWhite, qreal penWidth)
{
	if (blackAndWhite)
	{
		QPen newPen = painter->pen();
		newPen.setColor(Qt::black);
		painter->setPen(newPen);
	}
	painter->setRenderHint(QPainter::Antialiasing, useAntialiasing && (drawingMode == LineDrawing));
	int yMean = targetRect.height() >> 1;
	if (signalInfo->data.size())
	{
		int sampleSize = static_cast<int>(signalInfo->sampleCount());
		int sampleStart;
		if (zoomed)
			sampleStart = zoomStartPos + (drawRect.x() * (zoomEndPos - zoomStartPos)) / targetRect.width();
		else
			sampleStart = (drawRect.x() * sampleSize) / targetRect.width();
		int sampleEnd;
		if (zoomed)
			sampleEnd = zoomStartPos + 1 + ((drawRect.x() + drawRect.width()) * (zoomEndPos - zoomStartPos)) / targetRect.width();
		else
			sampleEnd = ((drawRect.x() + drawRect.width()) * sampleSize) / targetRect.width();
		sampleEnd = std::min(sampleEnd, sampleSize);

		assert(sampleStart >= 0);
		assert(sampleStart <= sampleSize);
		assert(sampleEnd >= 0);
		assert(sampleEnd <= sampleSize);

		for (unsigned channel = 0; channel < signalInfo->channelCount; channel++)
			if (channelEnabled(channel))
			{
				if (!blackAndWhite)
					painter->setPen(QPen(getChannelColor(channel), penWidth));

				/*
				// Version based on vector of point. Does not work like this because both start and end for each line must be specified
				QVector<QPoint> linesToDraw(sampleEnd - sampleStart);

				for (int sample = sampleStart; sample < sampleEnd; sample++)
				{
					int newXPos = sampleToScreenX(sample, targetRect.width()) + targetRect.x();
					int newSample = sampleToScreenY(channel, signalInfo->data[channel * sampleSize + sample], targetRect.height());
					int yShift = shiftToScreenY(channel, targetRect.height());
					linesToDraw[sample - sampleStart] = QPoint(newXPos, yMean - yShift - newSample + targetRect.y());
				}

				painter->drawLines(linesToDraw);*/
			
				// get original positions
				int oldXPos = drawRect.x() + targetRect.x();
				int oldSample = sampleToScreenY(channel, signalInfo->data[channel * sampleSize + sampleStart], targetRect.height());
				
				for (int sample = sampleStart + 1; sample < sampleEnd; sample++)
				{
					int newXPos = sampleToScreenX(sample, targetRect.width()) + targetRect.x();
					int newSample = sampleToScreenY(channel, signalInfo->data[channel * sampleSize + sample], targetRect.height());
					int yShift = shiftToScreenY(channel, targetRect.height());
					
					switch (drawingMode)
					{
						case LineDrawing:
						painter->drawLine(oldXPos, yMean - yShift - oldSample + targetRect.y(), newXPos, yMean - yShift - newSample + targetRect.y());
						break;

						case PointDrawing:
						painter->drawPoint(newXPos, yMean - yShift - newSample + targetRect.y());
						break;
					};

					oldSample = newSample;
					oldXPos = newXPos;
				}
			}
	}
	painter->setRenderHint(QPainter::Antialiasing, false);
}

//! Draw data from optimised version (i.e. min/max sample value for each pixel in coordinates), using filled polygones
void SignalViewWidget::drawDataOptimised(QPainter *painter, const QRect &clipRect)
{
	int yMean = height() >> 1;
	int w = width();
	QPoint points[4];
	for (unsigned channel = 0; channel < signalInfo->channelCount; channel++)
		if (channelEnabled(channel))
		{
			painter->setPen(getChannelColor(channel).light());

			int yShift = shiftToScreenY(channel, height());
			int pixel = clipRect.left();
			int oy1 = yMean - yShift - sampleToScreenY(channel, optimisedData[w * channel + pixel].x(), height());
			int oy2 = yMean - yShift - sampleToScreenY(channel, optimisedData[w * channel + pixel].y(), height());
			
			for (pixel++; pixel <= clipRect.right(); pixel++)
			{
				Q_ASSERT(pixel < (int)(optimisedData.size() / signalInfo->channelCount));
				int y1 = yMean - yShift - sampleToScreenY(channel, optimisedData[w * channel + pixel].x(), height());
				int y2 = yMean - yShift - sampleToScreenY(channel, optimisedData[w * channel + pixel].y(), height());
				points[0] = QPoint(pixel - 1, oy1);
				points[1] = QPoint(pixel - 1, oy2);
				points[2] = QPoint(pixel, y2);
				points[3] = QPoint(pixel, y1);
				painter->drawConvexPolygon(points, 4);
				oy1 = y1;
				oy2 = y2;
			}
		}
}

//! A mouse button has been pressed
void SignalViewWidget::mousePressEvent(QMouseEvent *event)
{
	bool served = false;

	channelNameEditing->hide();

	// timescale popup menu
	if (getInfoBubbleRect(timeScaleToStringWidthToDiv(signalInfo->duration), 12, 12).contains(event->pos()))
	{
		timeScaleMenu->popup(mapToGlobal(event->pos()));
		served = true;
	}

	if (event->button() == Qt::LeftButton)
	{
		// zoom marker drag
		if (!served && !zoomed && isZoomMarker)
		{
			QRect infoRect = getZoomInfoRect(sampleToScreenX((zoomStartPos + zoomEndPos) / 2, width()));
			if (infoRect.contains(event->pos()))
			{
				movingZoomMarker = true;
				movingZoomMarkerPos = event->pos().x() - sampleToScreenX(zoomStartPos, width());
				served = true;
			}
		}

		int yMean = height() / 2;
		// channel shift drag
		if (!served)
			for (size_t i = 0; i < yShiftFactor.size(); i++)
			{
				if (
						(
						(event->pos() - QPoint(0, yMean - shiftToScreenY(i, height())))
						.manhattanLength() < 6
						)
					&&
						channelEnabled(i)
					)
				{
					movingChannelShift = i;
					update();
					served = true;
					break;
				}
			}

		// channel on / off
		if (!served)
			for (unsigned channel = 0; channel < signalInfo->channelCount; channel++)
				if (
					getInfoBubbleRect(QString("%0:%1").arg(channelNumberToString(channel)).arg(YScaleToString(yDivisionFactor[channel])), 65 + channel * 65, 12).contains(event->pos())
					)
				{
					setChannelEnabled(channel, !channelEnabled(channel));
					served = true;
				}

		// trigger draw
		if (!zoomed && (signalInfo->dataConverter->triggerType() != DataConverter::TRIGGER_NONE))
		{
			DataConverter *conv = signalInfo->dataConverter;
			unsigned channel = conv->triggerChannel();
			int yMean = height() / 2;
			
			// TODO : move this call in its own function
			QPoint triggerPos = QPoint(
				sampleToScreenX(conv->triggerPos(),
				width()),
				yMean - sampleToScreenY(channel, conv->triggerValue(),
				height()) - shiftToScreenY(channel, height()));

			if ((event->pos() - triggerPos).manhattanLength() < 6)
			{
				movingTrigger = true;
				update();
				served = true;
			}
		}
		// zoom
		if (!served && !zoomed && isZoomMarker)
			setZoomPos(screenToSampleX(event->pos().x(), width()), zoomEndPos);
	}
	else if (event->button() == Qt::RightButton)
	{
		// y scale popup menus
		if (!served)
			for (unsigned channel = 0; channel < signalInfo->channelCount; channel++)
				if (
					getInfoBubbleRect(QString("%0:%1").arg(channelNumberToString(channel)).arg(YScaleToString(yDivisionFactor[channel])), 65 + channel * 65, 12).contains(event->pos())
					&&
					channelEnabled(channel)
					)
				{
					yScaleChannel = channel;
					yScaleAct[getScaleFactorInvert(yDivisionFactor[channel])]->setChecked(true);
					yScaleMenu->popup(mapToGlobal(event->pos()));
					served = true;
					break;
				}
			
		// zoom
		if (!served && !zoomed && isZoomMarker)
			setZoomPos(zoomStartPos, screenToSampleX(event->pos().x(), width()));
	}
}

//! Mouse has been moved
void SignalViewWidget::mouseMoveEvent(QMouseEvent *event)
{
	// TODO: compute yMean globally
	int yMean = height() / 2;
	if (movingZoomMarker)
	{
		int zoomPosWidthSample = zoomEndPos - zoomStartPos;
		int zoomPosWidthScreen = sampleToScreenX(zoomPosWidthSample, width());
		int newScreenPos = event->pos().x() - movingZoomMarkerPos;
		newScreenPos = std::max(newScreenPos, 0);
		newScreenPos = std::min(newScreenPos, width() - zoomPosWidthScreen);
		int newStartPos = screenToSampleX(newScreenPos, width());
		setZoomPos(newStartPos, newStartPos + zoomPosWidthSample);
	}
	if (movingChannelShift >= 0)
	{
		yShiftFactor[movingChannelShift] = ((yMean - event->pos().y()) * yDivisionFactor[movingChannelShift]) / height();
		update();
	}
	if (movingTrigger)
	{
		DataConverter *conv = signalInfo->dataConverter;
		unsigned channel = conv->triggerChannel();
		int triggerSample = screenToSampleX(event->pos().x(), width());
		int triggerValue = screenToSampleY(channel, yMean - event->pos().y() - shiftToScreenY(channel, height()), height());
		conv->setTrigger(conv->triggerType(), conv->triggerTimeout(), channel, triggerSample, triggerValue);
		update();
	}
}

//! A mouse button has been released
void SignalViewWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		movingChannelShift = -1;
		movingTrigger = false;
		movingZoomMarker = false;

		// channel name editing
		int yMean = height() / 2;
		for (size_t i = 0; i < yShiftFactor.size(); i++)
		{
			QFontMetrics fm(QApplication::font(this));
			QString channelName = channelNumberToString(i);
			int w = fm.width(channelName);
			int h = fm.height();
			QRect textRect(QPoint(1, yMean - shiftToScreenY(i, height()) - 5), QSize(w, -h));
			if (channelEnabled(i) && textRect.contains(event->pos()))
			{
				editingChannelName = i;
				channelNameEditing->setText(channelName);
				channelNameEditing->move(1, yMean - shiftToScreenY(i, height()) - 5 - h);
				channelNameEditing->setFocus();
				channelNameEditing->selectAll();
				channelNameEditing->show();
				break;
			}
		}
		update();
	}
}

//! A keyboard key has been pressed
void SignalViewWidget::keyPressEvent(QKeyEvent *event)
{
	if (zoomed || !isZoomMarker)
	{
		QWidget::keyPressEvent(event);
		return;
	}

	int delta = signalInfo->sampleCount() / 100;
	if (delta == 0)
		delta = 1;
	switch (event->key())
	{
	case Qt::Key_Left:
		{
			int newZoomPos = std::max(zoomStartPos - delta, 0);
			setZoomPos(newZoomPos, newZoomPos + zoomEndPos - zoomStartPos);
		}
		break;
	case Qt::Key_Right:
		{
			int newZoomPos = std::min(zoomStartPos + delta, (int)signalInfo->sampleCount() - (zoomEndPos - zoomStartPos) - 1);
			setZoomPos(newZoomPos, newZoomPos + zoomEndPos - zoomStartPos);
		}
		break;
	default:
		QWidget::keyPressEvent(event);
	}
}

//! Widget has been resized
void SignalViewWidget::resizeEvent(QResizeEvent *)
{
	if (persistantBuffer)
	{
		disableDisplayPersistance();
		enableDisplayPersistance();
	}

	if (!zoomed)
	{
		optimisedData.resize(width() * signalInfo->channelCount);
		regenerateOptimisedData(0, signalInfo->sampleCount());
	}
}

//! A timescale action has been triggered. Timescale action will provide the data width in ms as a QVariant in its data() member
void SignalViewWidget::timeScaleAction()
{
	QAction *action = static_cast<QAction *>(sender());
	unsigned duration = action->data().toUInt();
	emit timeScaleChanged(duration);
}

//! Returns the shift in screen coordinate for channel
int SignalViewWidget::shiftToScreenY(unsigned channel, int screenHeight)
{
	return (screenHeight * yShiftFactor[channel]) / yDivisionFactor[channel];
}

//! Convert a sample value to a screen coordinate
int SignalViewWidget::sampleToScreenY(unsigned channel, signed short sample, int screenHeight)
{
	return ((qint64)screenHeight * (qint64)sample * 1000) / ((qint64)yPrescaleFactor * (qint64)yDivisionFactor[channel]);
}

//! Convert a screen coordinate to a sample value
signed short SignalViewWidget::screenToSampleY(unsigned channel, int screen, int screenHeight)
{
	return ((qint64)yPrescaleFactor * (qint64)yDivisionFactor[channel] * (qint64)screen) / (1000 * (qint64)screenHeight);
}

//! Convert a sample number to a screen coordinate
int SignalViewWidget::sampleToScreenX(int sample, int screenWidth)
{
	if (zoomed)
	{
		if (zoomEndPos - zoomStartPos == 0)
			return 0;
		else
			return ((sample - zoomStartPos) * screenWidth) / (zoomEndPos - zoomStartPos);
	}
	else
		return (sample * screenWidth) / ((static_cast<int>(signalInfo->sampleCount())) - 1);
}

//! Convert a screen coordinate to a sample number
int SignalViewWidget::screenToSampleX(int screen, int screenWidth)
{
	if (zoomed)
		return zoomStartPos + (screen * (zoomEndPos - zoomStartPos)) / screenWidth;
	else
		return (screen * ((static_cast<int>(signalInfo->sampleCount())) - 1)) / screenWidth;
}

//! Draw info bubble. Returns the total width
int SignalViewWidget::drawInfoBubble(QPainter *painter, int x, int y, const QString &text, const QColor &color)
{
	QFontMetrics fm(painter->font());
	int w = fm.width(text);
	int h = fm.height();
	w += h;

	if (useAlphaBlending)
	{
		painter->setPen(QColor(127,127,127,200));
		painter->setBrush(QBrush(QColor(255,255,255,200)));
	}
	else
	{
		painter->setPen(QColor(127,127,127));
		painter->setBrush(QBrush(QColor(255,255,255)));
	}

	painter->setRenderHint(QPainter::Antialiasing, useAntialiasing);
	painter->drawRoundRect(x, y, w, h, (99 * h) / w, 99);
	painter->setRenderHint(QPainter::Antialiasing, false);
	
	QColor alphaColor = color;
	if (useAlphaBlending)
		alphaColor.setAlpha(200);
	painter->setPen(alphaColor);
	painter->drawText(x, y, w, h, Qt::AlignCenter, text);

	return w;
}

//! Return the bounding rect of info bubble
QRect SignalViewWidget::getInfoBubbleRect(const QString &text, int x, int y)
{
	QFontMetrics fm(QApplication::font(this));
	int w = fm.width(text);
	int h = fm.height();
	return QRect(x, y, w + h, h);
}

//! Draw a triangle on the left Y axis
void SignalViewWidget::drawYTriangle(QPainter *painter, int yPos, unsigned channel, bool selected, bool trigger)
{
	yPos = (height() / 2) - yPos;
	painter->setBrush(getChannelColor(channel));
	painter->setPen(getChannelColor(channel));

	if (trigger)
		painter->drawText(10, yPos-5, "T");
	else
		painter->drawText(1, yPos-5, channelNumberToString(channel));

	QPoint triangleShape[3];
	triangleShape[0] = QPoint(4, yPos);
	triangleShape[1] = QPoint(0, yPos-4);
	triangleShape[2] = QPoint(0, yPos+4);
	painter->drawPolygon(triangleShape, 3);

	if (selected)
		painter->drawLine(0, yPos, width(), yPos);
}

//! Draw a triangle on the top X axis
void SignalViewWidget::drawXTriangle(QPainter *painter, int xPos, unsigned channel, bool selected, bool trigger)
{
	painter->setBrush(getChannelColor(channel));
	painter->setPen(getChannelColor(channel));

	if (trigger)
		painter->drawText(xPos + 6, 12, "T");
	else
		painter->drawText(xPos + 6, 12, channelNumberToString(channel));

	QPoint triangleShape[3];
	triangleShape[0] = QPoint(xPos, 4);
	triangleShape[1] = QPoint(xPos-4, 0);
	triangleShape[2] = QPoint(xPos+4, 0);
	painter->drawPolygon(triangleShape, 3);

	if (selected)
		painter->drawLine(xPos, 0, xPos, height());
}

//! Return the coordinates of the zoom information rectangle
QRect SignalViewWidget::getZoomInfoRect(int middle, QString *os1, QString *os2)
{
	QString s1 = signalInfo->sampleToTimeString((double)(zoomEndPos - zoomStartPos));
	QString s2 = signalInfo->sampleToFreqString(zoomEndPos - zoomStartPos);
	QFontMetrics fm(QApplication::font(this));

	int sh = fm.height();
	int sw1 = fm.width(s1);
	int sw2 = fm.width(s2);
	int maxSw = std::max(sw1, sw2);
	
	if (os1)
		*os1 = s1;
	if (os2)
		*os2 = s2;

	return QRect(middle - ((maxSw + 10) / 2), height() - 2 * sh - 10, maxSw + 10, 2 * sh);
}



//! The Y scale has been changed. Action will provide the factor data in mv per height as a QVariant in its data() member
void SignalViewWidget::yScaleChanged()
{
	QAction *action = static_cast<QAction *>(sender());
	bool ok;
	unsigned yScale = action->data().toUInt(&ok);
	assert(ok);
	
	assert(yScaleChannel < signalInfo->channelCount);
	
	// correct shift
	yShiftFactor[yScaleChannel] = (yShiftFactor[yScaleChannel] * (int)yScale) / yDivisionFactor[yScaleChannel];

	// apply new division change
	yDivisionFactor[yScaleChannel] = yScale;
}

//! The channel name has been changed
void SignalViewWidget::channelNameChanged()
{
	setCustomChannelName(editingChannelName, channelNameEditing->text());
	channelNameEditing->hide();
	emit customChannelNameChanged();
}

//! Create actions and menu for this widget
void SignalViewWidget::createActionsAndMenus(void)
{
	// yscale menu
	yScaleMenu = new QMenu(this);
	yScaleGroup = new QActionGroup(this);
	for (size_t i = 0; i < ScaleFactorCount; i++)
	{
		unsigned factor = getScaleFactor(i);
		yScaleAct[i] = yScaleMenu->addAction(YScaleToString(factor));
		yScaleAct[i]->setData(QVariant(factor));
		yScaleAct[i]->setCheckable(true);
		connect(yScaleAct[i], SIGNAL(triggered()), this, SLOT(yScaleChanged()));
		yScaleGroup->addAction(yScaleAct[i]);
	}
	
	// timescale menu
	timeScaleMenu = new QMenu(this);
	timescaleGroup = new QActionGroup(this);
	for (size_t i = 0; i < ScaleFactorCount; i++)
	{
		unsigned factor = getScaleFactor(i);
		timeScaleAct[i] = timeScaleMenu->addAction(timeScaleToStringWidthToDiv(factor));
		timeScaleAct[i]->setData(QVariant(factor));
		if (!zoomed)
			timeScaleAct[i]->setCheckable(true);
		connect(timeScaleAct[i], SIGNAL(triggered()), SLOT(timeScaleAction()));
		timescaleGroup->addAction(timeScaleAct[i]);
	}
}

//! Save GUI parameters to settings in order to reload them at next start
void SignalViewWidget::saveGUISettings(QSettings *settings)
{
	settings->beginWriteArray("channelVisual");
	for (unsigned i = 0; i < signalInfo->channelCount; i++)
	{
		settings->setArrayIndex(i);
		settings->setValue("index", physicChannelIdToLogic(i));
		settings->setValue("yDivisionFactor", yDivisionFactor[i]);
		settings->setValue("yShiftFactor", yShiftFactor[i]);
		if (channelEnabledMask & (1<<i))
			settings->setValue("enabled", true);
		else
			settings->setValue("enabled", false);
	}
	settings->endArray();
}

//! Load GUI parameters from settings
void SignalViewWidget::loadGUISettings(QSettings *settings)
{
	channelEnabledMask = (unsigned)-1;
	QStringList groups = settings->childGroups();
	if (groups.contains("channelVisual"))
	{
		int size = settings->beginReadArray("channelVisual");
		for (int i = 0; i < size; i++)
		{
			settings->setArrayIndex(i);
			bool ok;
			unsigned id = logicChannelIdToPhysic(settings->value("index").toInt(), &ok);
			if (ok)
			{
				yDivisionFactor[id] = settings->value("yDivisionFactor").toInt();
				yShiftFactor[id] = settings->value("yShiftFactor").toInt();
				if (settings->value("enabled").toBool() == false)
					channelEnabledMask &= ~(1 << id);
			}
		}
		settings->endArray();
	}
}
