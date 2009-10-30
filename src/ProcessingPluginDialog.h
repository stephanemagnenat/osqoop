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

#ifndef __PROCESSING_PLUGIN_DIALOG_H
#define __PROCESSING_PLUGIN_DIALOG_H

#include <QDialog>
#include <QMenu>
#include <QActionGroup>
#include <vector>
#include <valarray>

class OscilloscopeWindow;
class QListWidget;
class QListWidgetItem;
class QTableWidget;
class QPushButton;
class QMenu;
class QTableWidgetItem;
class QAction;
class QSpinBox;
class ProcessingPluginDescription;
class ProcessingPlugin;

//! Dialog that let the user choose which plugin she wants to activate and with which inputs/outputs
class ProcessingPluginDialog : public QDialog
{
	Q_OBJECT
	
public:
	ProcessingPluginDialog(unsigned dataSourceInputCount, OscilloscopeWindow *parent = 0);
	unsigned channelCount();
	void setChannelCount(unsigned channelCount);

private slots:
	void useSelectedPlugins(int row);
	void useSelectedPlugins();
	void useSelectedPlugins(QListWidgetItem *);
	void pluginChannelEdit(const QPoint &);
	void pluginEdit(QTableWidgetItem *);
	void channelChanged();
	void recreateChannelMenu(int);

private:
	void usePlugin(const ProcessingPluginDescription *pluginDescription, std::vector<unsigned> *inputs = NULL, std::vector<unsigned> *outputs = NULL, ProcessingPlugin *instance = NULL);
	void recomputeUseChannelCount(void);

private:
	friend class OscilloscopeWindow;
	OscilloscopeWindow *mainWindow; //!< pointer to the main window, used to retrieve the plugin descriptions
	QListWidget *pluginsAvailableList; //!< list of all available plugins. Display the names from the ProcessingPluginDescription
	QTableWidget *pluginsActiveTable; //!< table of plugins instances configuration that will be activated after the dialog
	std::vector<ProcessingPlugin *> pluginsInstanceTable; //!< vector of pointers to actual plugins instances for configurations shown in pluginsActiveTable. Pointers are NULL if no instance exist yet
	QTableWidgetItem *toChangeItem; //!< item on which the channel popup menu has been called
	QSpinBox *extendedChannelChooser; //!< spinbox to change the number of extended channel. This can't be lower that the biggest used channel number
	
	QMenu channelChooserMenu; //!< menu for choosing channel number
	QActionGroup channelChooserGroup; //!< group for chossing channel number to ensure that only one channel can be choosen
	std::valarray<QAction *> channelChooserAct; //!< actions for every choosable number of channel
	
	int inputCount; //!< maximum number of inputs of all plugins in pluginsActiveTable
	int outputCount; //!< maximum number of outputs of all plugins in pluginsActiveTable
	unsigned dataSourceInputCount; //!< biggest used channel number
};

#endif
