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

#ifndef __SIGNAL_VIEW_WIDGET
#define __SIGNAL_VIEW_WIDGET

#include <QWidget>
#include <QColor>
#include <QPoint>
#include <QRect>
#include <valarray>
#include <vector>
#include "Utilities.h"
#include "SignalDisplayData.h"

class DataConverter;
class QMenu;
class QActionGroup;
class QAction;
class QPixmap;
class QLineEdit;
class QSettings;

//! Widget for signal viwing
class SignalViewWidget : public QWidget
{
	Q_OBJECT
	Q_PROPERTY(bool antialiasing READ antialiasing WRITE setAntialiasing) //!< antialiased display: nicer but slower display
	Q_PROPERTY(bool alphaBlending READ alphaBlending WRITE setAlphaBlending) //!< alpha-blended infos: nicer but slower display
	Q_PROPERTY(bool displayPersistance READ displayPersistance WRITE setDisplayPersistance) //!< persistant display: old signal are still visible behind new ones)
	Q_PROPERTY(bool persistanceFadeout READ persistanceFadeout WRITE setPersistanceFadeout) //!< persistance fadeout: if displayPersistance is enabled, fadeout progressively old signals
	Q_PROPERTY(bool zoomMarker READ zoomMarker WRITE setZoomMarker) //!< zoom marker enabled: in non-zoomed view, allow user to set and move zoom markers

public:
	SignalViewWidget(const SignalDisplayData *signalInfo, int yPrescaleFactor, bool zoomed = false, QWidget *parent = 0);
	~SignalViewWidget();

	virtual QSize sizeHint () const;
	virtual QSize minimumSizeHint () const;
	virtual QSizePolicy sizePolicy () const;
	void enableChannel(unsigned channel);
	void disableChannel(unsigned channel);
	void setChannelEnabled(unsigned channel, bool enabled);
	bool channelEnabled(unsigned channel) const;
	bool antialiasing() const;
	bool alphaBlending() const;
	bool displayPersistance() const;
	bool persistanceFadeout() const;
	bool zoomMarker() const;
	int zoomPosStart() const;
	int zoomPosEnd() const;
	void drawForPrinting(QPainter *painter, const QRect &targetRect, bool blackAndWhite, qreal penWidth);

	// gui settings save/load
	void saveGUISettings(QSettings *settings);
	void loadGUISettings(QSettings *settings);

signals:
	void zoomPosChanged(int,int); //!< The zoom marker have been changed
	void timeScaleChanged(unsigned); //!< The timescale has been changed by the timescale menu
	void customChannelNameChanged(); //!< Name of a custom channel has been changed

public slots:
	void newDataReady(int startSample, int endSample);
	void setAntialiasing(bool enabled);
	void setAlphaBlending(bool enabled);
	void setDisplayPersistance(bool enabled);
	void setPersistanceFadeout(bool enabled);
	void setZoomMarker(bool enabled);
	void setZoomPos(int start, int end);
	void autoLayoutChannels(void);
	
protected slots:
	void timeScaleAction(void);
	void yScaleChanged();
	void channelNameChanged();

protected:
	// events
	void paintEvent(QPaintEvent *event);	
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void resizeEvent(QResizeEvent *event);

	// options
	void enableDisplayPersistance(void);
	void disableDisplayPersistance(void);

	// helper methods
	inline int shiftToScreenY(unsigned channel, int screenHeight);
	inline int sampleToScreenY(unsigned channel, signed short sample, int screenHeight);
	inline signed short screenToSampleY(unsigned channel, int screen, int screenHeight);
	inline int sampleToScreenX(int sample, int screenWidth);
	inline int screenToSampleX(int screen, int screenWidth);

	void regenerateOptimisedData(int startSample, int endSample);
	
	// drawing helper methods
	void drawGrid(QPainter *painter, const QRect &targetRect, bool blackAndWhite = false, qreal penWidth = 0);
	void drawData(QPainter *painter, const QRect &drawRect, const QRect &targetRect, bool blackAndWhite = false, qreal penWidth = 0);
	void drawDataOptimised(QPainter *painter, const QRect &clipRect);
	int drawInfoBubble(QPainter *painter, int x, int y, const QString &text, const QColor &color = Qt::gray);
	QRect getInfoBubbleRect(const QString &text, int x, int y);
	void drawYTriangle(QPainter *painter, int yPos, unsigned channel, bool selected, bool trigger);
	void drawXTriangle(QPainter *painter, int xPos, unsigned channel, bool selected, bool trigger);
	QRect getZoomInfoRect(int middle, QString *os1 = NULL, QString *os2 = NULL);

	// menu and actions
	void createActionsAndMenus(void);

public:
	unsigned channelEnabledMask; //!< which channel are enabled
	enum DrawingMode
	{
		LineDrawing, //!< link points with lines
		PointDrawing //!< draw only points
	} drawingMode; //!< active mode of drawing
	
protected:
	// signal display element
	const SignalDisplayData *signalInfo; //!< pointer to signal information struct
	bool zoomed; //!< is this widget a zoomed view
	int zoomStartPos; //!< position in sample to zoom from
	int zoomEndPos; //!< position in sample to zoom to
	int yPrescaleFactor; //!< Prescale factor for Y axis, corresponds to number of source unit per V
	std::vector<int> yDivisionFactor; //!< division factor, number of mV per height
	std::vector<int> yShiftFactor; //!< shift factor. In mV
	bool useAntialiasing; //!< do we use antialiasing
	bool useAlphaBlending; //!< do we use alpha-blending
	QPixmap *persistantBuffer; //!< buffer for persistant drawing
	bool doPersistanceFadeout; //!< do the persistance buffer fadeout
	bool persistantBufferDirty; //!< persistant buffer needs redraw
	bool isZoomMarker; //!< do we show zoom marker
	std::valarray<QPoint> optimisedData; //!< data optimised for display

	// gui interaction elements
	int movingChannelShift; //!< number of channel being shifted. -1 if no channel is being shifted
	bool movingTrigger; //!< true if trigger is being shifted.
	bool movingZoomMarker; //!< true if zoom marker are being shifted
	int movingZoomMarkerPos; //!< mouse position when zoom marker is clicked
	int editingChannelName; //!< number of the channel being edited, if any
	QLineEdit *channelNameEditing; //!< widget for entering new channel name

	// menu and actions
	QMenu *timeScaleMenu; //!< timescale menu
	QActionGroup *timescaleGroup; //!< action group for timescale menu
	QAction *timeScaleAct[ScaleFactorCount]; //!< timescale actions
	QMenu *yScaleMenu; //!< menu for y scale
	QActionGroup *yScaleGroup; //!< group for y scale menu
	QAction *yScaleAct[ScaleFactorCount]; //!< actions for yscale
	unsigned yScaleChannel; //!< the number of channel on which the last y scale change occured. -1 if no channel
};

#endif
