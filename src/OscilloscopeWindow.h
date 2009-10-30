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

#ifndef __OSCILLOSCOPE_WINDOW_H
#define __OSCILLOSCOPE_WINDOW_H

#include "SignalDisplayData.h"
#include "DataConverter.h"
#include "Utilities.h"

#include <QMainWindow>
#include <QMetaType>

class ProcessingPluginDescription;
class SignalViewWidget;
class DataSource;
class DataSourceDescription;
class QMenu;
class QAction;
class QActionGroup;
class QString;
class QSplitter;
class QDockWidget;
class QVBoxLayout;
class QScrollArea;

//! Main window for the oscilloscope.
/*!
	Contains two SignalViewWidget, one for normal view and the other for zoomed view.
	Also contains menus and status bar
*/
class OscilloscopeWindow : public QMainWindow
{
	friend class ProcessingPluginDialog;
	Q_OBJECT
	
public:
	OscilloscopeWindow();
	virtual ~OscilloscopeWindow();
	bool isValid();
	
private slots:
	void setData(const std::valarray<signed short> &, unsigned, unsigned, unsigned, unsigned, unsigned);
	void print();
	void exportToPDF();
	void exportData();
	void exportVisibleData();
	void zoomAction();
	void freezeDisplayToggled(bool);
	void changeDrawingMode();
	void channelAction();
	void channelAllAction();
	void channelNoneAction();
	void timeScaleAction();
	void mainTimeScaleChanged(unsigned);
	void zoomedTimeScaleChanged(unsigned);
	void triggerChannel();
	void triggerNone();
	void triggerSingle();
	void triggerAuto();
	void triggerUp();
	void triggerDown();
	void triggerBoth();
	void triggerReset();
	void configurePlugins();
	void savePluginsConfiguration();
	void loadPluginsConfiguration();
	void about();
	void recreateChannelActionsAndMenu();
	void saveGUISettings();

protected:
	// events
	void closeEvent(QCloseEvent * e);

private:
	bool createDataSource();
	void createActionsAndMenus();
	void loadPlugins();
	void recreatePluginDock(const DataConverter::ActivePlugins &configuration);
	void connectToDataConverter();
	void disconnectFromDataConverter();
	void loadGUISettings();
	
	std::vector<DataSourceDescription *> dataSourceDescriptions; //!< available data sources. Real data source instances can be created out of descriptions
	DataSource *dataSource; //!< source of data

	SignalDisplayData signalInfo; //!< signal data to be displayed
	QSplitter *splitter; //!< splitter for main and zoomed view
	SignalViewWidget *mainView; //!< main data view
	SignalViewWidget *zoomedView; //!< zoomed data view
	
	std::vector<ProcessingPluginDescription *> processingPluginsDescriptions; //!< available plugins. Real plugins instances can be created out of descriptions
	QDockWidget *pluginDock; //!< plugins dock

	QMenu *channelMenu; //!< menu for choosing channel to display. Dynamically recreated when number of menu are changed
	QMenu *triggerChannelMenu; //!< menu for choosing channel to trigger on
	
	// Only actions and actions group to which we need to keep a pointer are listed here.
	QAction *displayFreezeAct;
	bool wasFrozen; //!< was the display frozen since last DataConverter::DATA_FRAME_START ?
	std::valarray<QAction *> channelAct;

	QActionGroup *timescaleGroup; //!< group of timescale actions
	QAction *timeScaleAct[ScaleFactorCount]; //!< timescale actions
	
	QActionGroup *triggerTypeGroup; //!< group of trigger type actions
	QAction *triggerNoneAct; //!< no trigger action
	QAction *triggerSingleAct; //!< single trigger action
	QAction *triggerAutoAct; //!< auto triggert action

	QAction *triggerUpAct; //!< up slop trigger action
	QAction *triggerDownAct; //!< down slope trigger action
	QAction *triggerBothAct; //!< both signal slope trigger action
	
	QActionGroup *triggerChannelGroup; //!< group of trigger channel actions
	std::valarray<QAction *> triggerChannelAct; //!< trigger channel actions
};

#endif

