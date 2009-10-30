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

#include "ProcessingPluginDialog.h"
#include <ProcessingPluginDialog.moc>
#include "ProcessingPlugin.h"
#include "OscilloscopeWindow.h"
#include "Utilities.h"

#include <QLabel>
#include <QGridLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTableWidget>
#include <QPushButton>
#include <QSpinBox>

#include <algorithm>
#include <cassert>

ProcessingPluginDialog::ProcessingPluginDialog(unsigned dataSourceInputCount, OscilloscopeWindow *parent) :
	QDialog(parent),
	channelChooserGroup(this)
{
	mainWindow = parent;
	inputCount = outputCount = 0;
	this->dataSourceInputCount = dataSourceInputCount;
	
	// layout
	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->setColumnMinimumWidth(1, 5);
	
	// plugins list
	mainLayout->addWidget(new QLabel(tr("Available")), 0, 0);
	pluginsAvailableList = new QListWidget;
	for (size_t i = 0; i < parent->processingPluginsDescriptions.size(); i++)
	{
		pluginsAvailableList->addItem(parent->processingPluginsDescriptions[i]->name());
		pluginsAvailableList->item(i)->setToolTip(parent->processingPluginsDescriptions[i]->description());
	}
	QPushButton *addPlugin = new QPushButton(tr("&Use"));
	
	// active plugins
	mainLayout->addWidget(new QLabel(tr("Active")), 0, 2);
	pluginsActiveTable = new QTableWidget(0, 4);
	pluginsActiveTable->setSortingEnabled(false);
	pluginsActiveTable->setDragEnabled(false);
	pluginsActiveTable->setShowGrid(false);
	pluginsActiveTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	pluginsActiveTable->setSelectionMode(QAbstractItemView::NoSelection);
	pluginsActiveTable->setContextMenuPolicy(Qt::CustomContextMenu);

	// max channel number
	extendedChannelChooser = new QSpinBox(this);
	extendedChannelChooser->setButtonSymbols(QAbstractSpinBox::PlusMinus);
	
	// ok / cancel buttons
	QPushButton *okButton = new QPushButton(tr("OK"));
	okButton->setDefault(true);
	QPushButton *cancelButton = new QPushButton(tr("Cancel"));
	
	// connections
	connect(pluginsAvailableList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), SLOT(useSelectedPlugins(QListWidgetItem *)));
	connect(addPlugin, SIGNAL(clicked()), SLOT(useSelectedPlugins()));
	connect(okButton, SIGNAL(clicked()), SLOT(accept()));
	connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
	connect(pluginsActiveTable, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(pluginChannelEdit(const QPoint &)));
	connect(pluginsActiveTable, SIGNAL(itemClicked(QTableWidgetItem *)), SLOT(pluginEdit(QTableWidgetItem *)));
	connect(extendedChannelChooser, SIGNAL(valueChanged(int)), SLOT(recreateChannelMenu(int)));
	
	// layout
	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(okButton);
	buttonLayout->addWidget(cancelButton);

	QHBoxLayout *maxChannelLayout = new QHBoxLayout;
	maxChannelLayout->addWidget(new QLabel(tr("Number of extended channel")));
	maxChannelLayout->addWidget(extendedChannelChooser);
	
	mainLayout->addWidget(pluginsAvailableList, 1, 0);
	mainLayout->addWidget(addPlugin, 2, 0);
	mainLayout->setColumnStretch(0, 1);
	mainLayout->addWidget(pluginsActiveTable, 1, 2);
	mainLayout->addLayout(maxChannelLayout, 2, 2);
	mainLayout->setColumnStretch(2, 2);
	mainLayout->addLayout(buttonLayout, 3, 0, 1, -1);
	
	setLayout(mainLayout);
	
	setWindowTitle(tr("Plugins configuration"));
	
	recomputeUseChannelCount();

	recreateChannelMenu(extendedChannelChooser->minimum());
	toChangeItem = NULL;
}

//! Return the choosen number of channel. This number is asserted to be sufficient for all plugins
unsigned ProcessingPluginDialog::channelCount()
{
	return dataSourceInputCount + extendedChannelChooser->value();
}

//! Set the init content of extendedChannelChooser
void ProcessingPluginDialog::setChannelCount(unsigned channelCount)
{
	extendedChannelChooser->setValue((int)channelCount - (int)dataSourceInputCount);
}

//! Do the real job of adding selected plugin
void ProcessingPluginDialog::useSelectedPlugins(int row)
{
	usePlugin(mainWindow->processingPluginsDescriptions[row]);
}

//! Add a new plugin instance in pluginsActiveTable, with a given description. Optional inputs and outputs mapping as well as a pointer to instance can be given 
void ProcessingPluginDialog::usePlugin(const ProcessingPluginDescription *pluginDescription, std::vector<unsigned> *inputs, std::vector<unsigned> *outputs, ProcessingPlugin *instance)
{
	int oldInputCount = inputCount;
	int oldOutputCount = outputCount;
	int myInputCount = pluginDescription->inputCount();
	int myOutputCount = pluginDescription->outputCount();
	inputCount = std::max(oldInputCount, myInputCount);
	outputCount = std::max(oldOutputCount, myOutputCount);
	
	// resize
	if (outputCount > oldOutputCount)
		for (int i = 0; i < outputCount - oldOutputCount; i++)
		{
			pluginsActiveTable->insertColumn(1 + oldInputCount + oldOutputCount);
			for (int row = 0; row < pluginsActiveTable->rowCount(); row++)
			{
				QTableWidgetItem *item = new QTableWidgetItem();
				item->setFlags(0);
				pluginsActiveTable->setItem(row, 1 + oldInputCount + oldOutputCount, item);
			}
		}
	if (inputCount > oldInputCount)
		for (int i = 0; i < inputCount - oldInputCount; i++)
		{
			pluginsActiveTable->insertColumn(1 + oldInputCount);
			for (int row = 0; row < pluginsActiveTable->rowCount(); row++)
			{
				QTableWidgetItem *item = new QTableWidgetItem();
				item->setFlags(0);
				pluginsActiveTable->setItem(row, 1 + oldInputCount, item);
			}
		}
	
	// horizontal headers
	QStringList horizontalHeaderStrings;
	horizontalHeaderStrings << tr("Name");
	for (int i = 0; i < inputCount; i++)
		horizontalHeaderStrings << QString("I");
	for (int i = 0; i < outputCount; i++)
		horizontalHeaderStrings << QString("O");
	horizontalHeaderStrings << "";
	horizontalHeaderStrings << "";
	horizontalHeaderStrings << "";
	pluginsActiveTable->setHorizontalHeaderLabels(horizontalHeaderStrings);
	
	// new row
	int rowCount = pluginsActiveTable->rowCount();
	pluginsActiveTable->insertRow(rowCount);
	pluginsInstanceTable.push_back(instance);
	QTableWidgetItem *item = new QTableWidgetItem(pluginDescription->name());
	item->setFlags(Qt::ItemIsEnabled);
	pluginsActiveTable->setItem(rowCount, 0, item);
	// my inputs are enabled on this line
	for (int i = 1; i < 1 + myInputCount; i++)
	{
		item = new QTableWidgetItem(channelNumberToString(0));
		item->setFlags(Qt::ItemIsEnabled);
		if (inputs)
			item->setText(channelNumberToString((*inputs)[i - 1]));
		pluginsActiveTable->setItem(rowCount, i, item);
	}
	// others inputs are disabled on this line
	for (int i = 1 + myInputCount; i < 1 + inputCount; i++)
	{
		item = new QTableWidgetItem();
		item->setFlags(0);
		pluginsActiveTable->setItem(rowCount, i, item);
	}
	// my outputs are enabled on this line
	for (int i = 1 + inputCount; i < 1 + inputCount + myOutputCount; i++)
	{
		item = new QTableWidgetItem(channelNumberToString(dataSourceInputCount));
		item->setFlags(Qt::ItemIsEnabled);
		if (outputs)
			item->setText(channelNumberToString((*outputs)[i - inputCount - 1]));
		pluginsActiveTable->setItem(rowCount, i, item);
	}
	// others outputs are disabled on this line
	for (int i = 1 + inputCount + myOutputCount; i < 1 + inputCount + outputCount; i++)
	{
		item = new QTableWidgetItem();
		item->setFlags(0);
		pluginsActiveTable->setItem(rowCount, i, item);
	}
	// rest is enabled
	for (int i = 1 + inputCount + outputCount; i < pluginsActiveTable->columnCount(); i++)
	{
		item = new QTableWidgetItem();
		item->setFlags(Qt::ItemIsEnabled);
		pluginsActiveTable->setItem(rowCount, i, item);
	}
	// draw icons
	int iconPos = 1 + inputCount + outputCount;
	pluginsActiveTable->item(rowCount, iconPos)->setIcon(QIcon(":/images/up.png"));
	pluginsActiveTable->item(rowCount, iconPos + 1)->setIcon(QIcon(":/images/down.png"));
	pluginsActiveTable->item(rowCount, iconPos + 2)->setIcon(QIcon(":/images/delete.png"));
	
	// resize table
	for (int row = 0; row < pluginsActiveTable->rowCount(); row++)
		pluginsActiveTable->resizeRowToContents(row);
	//pluginsActiveTable->resizeRowsToContents();
	for (int col = 0; col < pluginsActiveTable->columnCount(); col++)
		pluginsActiveTable->resizeColumnToContents(col);
	//pluginsActiveTable->resizeColumnsToContents();
}

//! Call useSelectedPlugins(row)
void ProcessingPluginDialog::useSelectedPlugins(QListWidgetItem * item)
{
	useSelectedPlugins(pluginsAvailableList->row(item));
}

//! Call useSelectedPlugins(row) for each selected plugin
void ProcessingPluginDialog::useSelectedPlugins()
{
	for (int row = 0; row < pluginsAvailableList->count(); row++)
		if (pluginsAvailableList->isItemSelected(pluginsAvailableList->item(row)))
			useSelectedPlugins(row);
}

//! A channel number has been selected
void ProcessingPluginDialog::pluginChannelEdit(const QPoint & pos)
{
	QTableWidgetItem *item = pluginsActiveTable->itemAt(pos);
	if (item)
	{
		int column = pluginsActiveTable->column(item);
		int columnCount = pluginsActiveTable->columnCount();
		if ((column >= 1) && (column < columnCount - 3) && (item->flags() & Qt::ItemIsEnabled))
		{
			//this is a valid editable item
			unsigned oldChannel = channelNumberFromString(item->text());
			Q_ASSERT(oldChannel < channelChooserAct.size());
			channelChooserAct[oldChannel]->setChecked(true);
			toChangeItem = item;
			channelChooserMenu.popup(pluginsActiveTable->mapToGlobal(pos));
		}
	}
}

//! A channel edit operation (move or deleted) has been done
void ProcessingPluginDialog::pluginEdit(QTableWidgetItem * item)
{
	if (item)
	{
		int row = pluginsActiveTable->row(item);
		int column = pluginsActiveTable->column(item);
		int rowCount = pluginsActiveTable->rowCount();
		int columnCount = pluginsActiveTable->columnCount();
		if (column  == columnCount - 1)
		{
			pluginsActiveTable->removeRow(row);
			pluginsInstanceTable.erase(pluginsInstanceTable.begin() + row);
			recomputeUseChannelCount();
		}
		else if (column == columnCount - 2)
		{
			// exchange row with the lower one
			if (row + 1 < rowCount)
			{
				for (int i = 0; i < columnCount; i++)
				{
					QTableWidgetItem *item = pluginsActiveTable->item(row, i)->clone();
					pluginsActiveTable->setItem(row, i, pluginsActiveTable->item(row + 1, i)->clone());
					pluginsActiveTable->setItem(row + 1, i, item);
				}
				std::swap(pluginsInstanceTable[row], pluginsInstanceTable[row+1]);
			}
		}
		else if (column == columnCount - 3)
		{
			// exchange row with the upper one
			if (row > 0)
			{
				for (int i = 0; i < columnCount; i++)
				{
					QTableWidgetItem *item = pluginsActiveTable->item(row, i)->clone();
					pluginsActiveTable->setItem(row, i, pluginsActiveTable->item(row - 1, i)->clone());
					pluginsActiveTable->setItem(row - 1, i, item);
				}
				std::swap(pluginsInstanceTable[row], pluginsInstanceTable[row-1]);
			}
		}
	}
}

//! A channel has been selected in the popup menu
void ProcessingPluginDialog::channelChanged()
{
	Q_ASSERT(toChangeItem);
	QAction *action = static_cast<QAction *>(sender());
	bool ok;
	unsigned channel = action->data().toUInt(&ok);
	Q_ASSERT(ok);
	toChangeItem->setText(channelNumberToString(channel));
	toChangeItem = NULL;
	recomputeUseChannelCount();
}

//! Recreate the channel popup menu for a given number of channel
void ProcessingPluginDialog::recreateChannelMenu(int extendedChannelCount)
{
    int channelCount = dataSourceInputCount + extendedChannelCount;
	channelChooserMenu.clear();
	channelChooserAct.resize(channelCount);
	
	for (unsigned channel = 0; channel < (unsigned)channelCount; channel++)
	{
		QAction *action = new QAction(channelNumberToString(channel), this);
		action->setData(QVariant(channel));
		action->setCheckable(true);
		connect(action, SIGNAL(triggered()), this, SLOT(channelChanged()));
		channelChooserGroup.addAction(action);
		channelChooserMenu.addAction(action);
		channelChooserAct[channel] = action;
	}
}

//! Recompute the number of used channel
void ProcessingPluginDialog::recomputeUseChannelCount(void)
{
	unsigned maxChannel = 0;
	for (int column = 1; column < 1 + inputCount + outputCount; column++)
		for (int row = 0; row < pluginsActiveTable->rowCount(); row++)
			if (pluginsActiveTable->item(row, column)->flags() & Qt::ItemIsEnabled)
				maxChannel = std::max(maxChannel, channelNumberFromString(pluginsActiveTable->item(row, column)->text()));
	extendedChannelChooser->setMinimum(std::max(dataSourceInputCount + 1, maxChannel + 1) - dataSourceInputCount);
}
