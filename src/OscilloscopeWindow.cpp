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

#include "OscilloscopeWindow.h"
#include "SignalViewWidget.h"
#include "ProcessingPlugin.h"
#include "ProcessingPluginDialog.h"
#include "DataConverter.h"
#include "DataSource.h"
#include "Version.h"

#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QMenuBar>
#include <QStatusBar>
#include <QString>
#include <QPrintDialog>
#include <QFileDialog>
#include <QPainter>
#include <QColor>
#include <QMessageBox>
#include <QPrinter>
#include <QCoreApplication>
#include <QPluginLoader>
#include <QTableWidget>
#include <QListWidget>
#include <QFile>
#include <QTextStream>
#include <QSplitter>
#include <QApplication>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QScrollArea>
#include <QLabel>
#include <QSizePolicy>
#include <QTextStream>
#include <QStringList>
#include <QSettings>
#include "Settings.h"
#include <QtDebug>
#include <cassert>

using namespace std;

//! Dialog box for choosing data source
class DataSourceDialog : public QDialog
{
public:
	QListWidget *dataSourceList; //!< the list holding available data sources

	//! Creates the widgets, especially the list of available data sources, which is filled from the dataSources parameter
	DataSourceDialog(const std::vector<DataSourceDescription *> &dataSources)
	{
		QVBoxLayout *layout = new QVBoxLayout(this);

		layout->addWidget(new QLabel(tr("Please select the desired data source")));

		dataSourceList = new QListWidget();
		for (size_t i = 0; i < dataSources.size(); i++)
			dataSourceList->addItem(dataSources[i]->name());
		connect(dataSourceList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), SLOT(accept()));
		layout->addWidget(dataSourceList);
	}
};

//! Constructor. Try to create data source, and, if successfull, create the data converter and main window. Otherwise set data converter to null and return.
OscilloscopeWindow::OscilloscopeWindow()
{
	loadPlugins();
	if (createDataSource())
	{
		// load and prepare channel names
		setDataSourceChannelCount(dataSource->inputCount());
		loadCustomChannelNames();
		
		// read number of channel and timescale, we use scope to destroythe QSettings asap
		{
			QSettings settings(ORGANISATION_NAME, APPLICATION_NAME);
			signalInfo.channelCount = dataSource->inputCount();
			if (settings.contains("extendedChannelCount"))
				signalInfo.channelCount += settings.value("extendedChannelCount").toUInt();
			else
				signalInfo.channelCount += 1;
			if (settings.contains("timeScale"))
				signalInfo.duration = settings.value("timeScale").toUInt();
			else
				signalInfo.duration = 20;
		}
		
		// create converter
		signalInfo.dataConverter = new DataConverter(dataSource, signalInfo.channelCount, signalInfo.duration);

		// create widgets
		wasFrozen = false;
		splitter = new QSplitter(Qt::Vertical, this);
		mainView = new SignalViewWidget(&signalInfo, dataSource->unitPerVoltCount());
		zoomedView = new SignalViewWidget(&signalInfo, dataSource->unitPerVoltCount(), true);
		zoomedView->setVisible(false);
		connect(mainView, SIGNAL(zoomPosChanged(int,int)), zoomedView, SLOT(setZoomPos(int,int)));
		
		splitter->addWidget(mainView);
		splitter->addWidget(zoomedView);
		
		setCentralWidget(splitter);
		
		// plugin dock area
		pluginDock = NULL;
		
		createActionsAndMenus();

		resize(600, 440);
		/*
		#ifdef WIN32
		// Works for windows
		qRegisterMetaType<std::valarray<signed short> >("std::valarray<signed short>");
		#else
		// Works for Linux
		qRegisterMetaType<valarray<signed short> >("valarray<signed short>");
		#endif*/

		qRegisterMetaType<std::valarray<signed short> >("std::valarray<signed short>");
		qRegisterMetaType<valarray<signed short> >("valarray<signed short>");


		qRegisterMetaType<unsigned>("unsigned");
		connectToDataConverter();
		//connect(&reader, SIGNAL(statusUpdated(const QString &)), this, SLOT(updateStatusBar(const QString &)), Qt::QueuedConnection);
		connect(mainView, SIGNAL(timeScaleChanged(unsigned)), this, SLOT(mainTimeScaleChanged(unsigned)));
		connect(mainView, SIGNAL(customChannelNameChanged()), this, SLOT(recreateChannelActionsAndMenu()));
		connect(zoomedView, SIGNAL(timeScaleChanged(unsigned)), this, SLOT(zoomedTimeScaleChanged(unsigned)));
		connect(zoomedView, SIGNAL(customChannelNameChanged()), this, SLOT(recreateChannelActionsAndMenu()));
		
		// restore GUI states
		loadGUISettings();

		// start acquisition
		signalInfo.dataConverter->start(QThread::HighestPriority);
	}
	else
		signalInfo.dataConverter = NULL;
}

//! Destructor. Delete the signal converter
OscilloscopeWindow::~OscilloscopeWindow()
{
	// destroy data paths
	if (signalInfo.dataConverter)
		delete signalInfo.dataConverter;
}

//! Return true if the window if a valid data source has been found
bool OscilloscopeWindow::isValid()
{
	return signalInfo.dataConverter != NULL;
}

//! Create the data source. Show a chooser window if multiple data source are available. Return false if no valid data source has been found
bool OscilloscopeWindow::createDataSource()
{
	switch (dataSourceDescriptions.size())
	{
		case 0:
		// no source available, return false
		QMessageBox::critical(this, "Oscilloscope", tr("No data source available, quitting"), QMessageBox::Ok, QMessageBox::NoButton);

		return false;

		case 1:
		{
			// single source available, try to open it and return false otherwise
			dataSource = dataSourceDescriptions[0]->create();
			if (!dataSource->init())
			{
				delete dataSource;
				dataSource = NULL;
				QMessageBox::critical(this, "Oscilloscope", tr("Data source initialization failed, quitting"), QMessageBox::Ok, QMessageBox::NoButton);
				return false;
			}
			else
				return true;
		}

		default:
		{
			// multiple sources available, give choice
			dataSource = NULL;
			while (!dataSource)
			{
				DataSourceDialog dataSourceDialog(dataSourceDescriptions);
				int ret = dataSourceDialog.exec();
				if (ret == QDialog::Rejected)
					return false;

				unsigned sourceId = dataSourceDialog.dataSourceList->currentRow();
				Q_ASSERT(sourceId < dataSourceDescriptions.size());
				dataSource = dataSourceDescriptions[sourceId]->create();
				if (!dataSource->init())
				{
					delete dataSource;
					dataSource = NULL;
					QMessageBox::warning(this, "Oscilloscope", tr("Data source initialization failed"), QMessageBox::Ok, QMessageBox::NoButton);
				}
			}
			return true;
		}
	}
}

//! Set new datas. If fullSampleCount is 0, set is incremental, and no resize is required. Otherwise each channel are resized to fullSampleCount
void OscilloscopeWindow::setData(const std::valarray<signed short> &data, unsigned fullSampleCount, unsigned fullSampleDuration, unsigned channelCount, unsigned startingPos, unsigned flags)
{
	if (flags & DataConverter::DATA_FRAME_START)
	{
		// resize
		unsigned oldChannelCount = signalInfo.channelCount;
		if ((oldChannelCount != channelCount) || (signalInfo.samplePerChannelCount != fullSampleCount))
			signalInfo.data.resize(fullSampleCount * channelCount);
		signalInfo.channelCount = channelCount;
		signalInfo.samplePerChannelCount = fullSampleCount;
		signalInfo.incrementalPos = 0;
		signalInfo.duration = fullSampleDuration;

		// recreate menu
		if (oldChannelCount != channelCount)
			recreateChannelActionsAndMenu();

		// reset timescale
		QAction *oldCheckedTimeScaleAction = timescaleGroup->checkedAction();
		unsigned actionIndex = getScaleFactorInvert(signalInfo.duration);
		Q_ASSERT(actionIndex < ScaleFactorCount);
		QAction *newCheckedTimeScaleAction = timeScaleAct[actionIndex];
		if (oldCheckedTimeScaleAction != newCheckedTimeScaleAction)
			newCheckedTimeScaleAction->setChecked(true);

		// ready again to accept datas
		wasFrozen = false;
	}
	
	if (!wasFrozen)
	{
		// new datas
		Q_ASSERT(data.size() / channelCount <= signalInfo.samplePerChannelCount);

		// copy data
		size_t sampleCount = data.size() / channelCount;
		int startSample = signalInfo.incrementalPos;
		for (size_t channel = 0; channel < channelCount; channel++)
		{
			const signed short *shiftedData = &data[channel * sampleCount];
			signed short *alignedData = &signalInfo.data[channel * signalInfo.samplePerChannelCount];
			for (size_t sample = 0; sample < sampleCount; sample++)
			{
				alignedData[(signalInfo.incrementalPos + sample) % signalInfo.samplePerChannelCount] = shiftedData[(sample + startingPos) % sampleCount];
			}
		}
		signalInfo.incrementalPos += sampleCount;
		int endSample = signalInfo.incrementalPos;

		// if single, stop getting datas
		if ((flags & DataConverter::DATA_FRAME_END) && (triggerSingleAct->isChecked()))
			displayFreezeAct->setChecked(true);

		// inform widgets
		mainView->newDataReady(startSample, endSample);
		zoomedView->newDataReady(startSample, endSample);
	}
}


//! Print display
void OscilloscopeWindow::print()
{
	disconnectFromDataConverter();

	QPrinter printer(QPrinter::HighResolution);
	#if QT_VERSION >= 0x040100
	printer.setOutputFormat(QPrinter::NativeFormat);
	#endif
	printer.setColorMode(QPrinter::Color);

	int answer;
	if (printer.colorMode() != QPrinter::GrayScale)
		answer = QMessageBox::question(this, tr("Export to PD&F"), tr("Do you want monochrome output?"), QMessageBox::Yes, QMessageBox::No);
	
	QPrintDialog printDialog(&printer, this);
	printDialog.setEnabledOptions(QAbstractPrintDialog::None);
	if (printDialog.exec() == QDialog::Accepted)
	{
		QPainter painter(&printer);
		mainView->drawForPrinting(&painter, printer.pageRect(), (printer.colorMode() == QPrinter::GrayScale) || (answer == QMessageBox::Yes), 5);
	}

	connectToDataConverter();
}

//! Export display to PDF
void OscilloscopeWindow::exportToPDF()
{
	#if QT_VERSION >= 0x040100
	disconnectFromDataConverter();

	int answer = QMessageBox::question(this, tr("Export to PD&F"), tr("Do you want monochrome output?"), QMessageBox::Yes, QMessageBox::No);
	QPrinter printer(QPrinter::HighResolution);
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setOrientation(QPrinter::Landscape);
	printer.setPageSize(QPrinter::A4);

	QString filename = QFileDialog::getSaveFileName(this, "", "", "PDF (*.pdf)");
	if (filename != "")
	{
		printer.setOutputFileName(filename);
		QPainter painter(&printer);
		mainView->drawForPrinting(&painter, painter.window(), answer == QMessageBox::Yes, 0.5);
	}

	connectToDataConverter();
	#endif
}

//! Export data to a space separated file
void OscilloscopeWindow::exportData()
{
	QString filename = QFileDialog::getSaveFileName(this, "", "", "Txt (*.txt)");
	if (filename != "")
	{
		QFile file(filename);
		if (file.open(QIODevice::WriteOnly))
		{
			QTextStream out(&file);
			unsigned rowCount = signalInfo.data.size() / signalInfo.channelCount;
			for (unsigned row = 0; row < rowCount; row++)
			{
				for (unsigned channel = 0; channel < signalInfo.channelCount; channel++)
				{
					out << signalInfo.data[channel*rowCount + row] << " ";
				}
				out << "\n";
			}
		}
	}
}

//! Export visible channels data to a space separated file
void OscilloscopeWindow::exportVisibleData()
{
	QString filename = QFileDialog::getSaveFileName(this, "", "", "Txt (*.txt)");
	if (filename != "")
	{
		QFile file(filename);
		if (file.open(QIODevice::WriteOnly))
		{
			QTextStream out(&file);
			unsigned rowCount = signalInfo.data.size() / signalInfo.channelCount;
			for (unsigned row = 0; row < rowCount; row++)
			{
				for (unsigned channel = 0; channel < signalInfo.channelCount; channel++)
				{
					if (mainView->channelEnabled(channel))
						out << signalInfo.data[channel*rowCount + row] << " ";
				}
				out << "\n";
			}
		}
	}
}

//! Action that set the channel view configuration of the zoomed view to the same mask as the main view
void OscilloscopeWindow::zoomAction()
{
	zoomedView->channelEnabledMask = mainView->channelEnabledMask;
	zoomedView->autoLayoutChannels();
}

//! Freeze/unfreeze display
void OscilloscopeWindow::freezeDisplayToggled(bool toggeled)
{
	if (toggeled)
	{
		disconnectFromDataConverter();
		wasFrozen = true;
	}
	else
		connectToDataConverter();
}

//! Change the drawing mode
void OscilloscopeWindow::changeDrawingMode()
{
	QAction *action = static_cast<QAction *>(sender());
	bool conversionResult;
	unsigned mode = action->data().toUInt(&conversionResult);
	assert(conversionResult);
	mainView->drawingMode = (SignalViewWidget::DrawingMode)(mode);
}

//! Enable/disable a channel
void OscilloscopeWindow::channelAction()
{
	QAction *action = static_cast<QAction *>(sender());
	bool conversionResult;
	unsigned channel = action->data().toUInt(&conversionResult);
	assert(conversionResult);
	mainView->setChannelEnabled(channel, channelAct[channel]->isChecked());
}

//! Show all channels
void OscilloscopeWindow::channelAllAction()
{
	for (size_t channel = 0; channel < signalInfo.channelCount; channel ++)
	{
		channelAct[channel]->setChecked(true);
		mainView->enableChannel(channel);
	}
}

//! Hide all channels
void OscilloscopeWindow::channelNoneAction()
{
	mainView->channelEnabledMask = 0;
	for (size_t channel = 0; channel < signalInfo.channelCount; channel ++)
		channelAct[channel]->setChecked(false);
}

//! Change timescale. Timescale action will provide the data width in ms as a QVariant in its data() member
void OscilloscopeWindow::timeScaleAction()
{
	QAction *action = static_cast<QAction *>(sender());
	unsigned duration = action->data().toUInt();
	mainTimeScaleChanged(duration);
}

//! Change timescale on the main view to duration
void OscilloscopeWindow::mainTimeScaleChanged(unsigned duration)
{
	signalInfo.dataConverter->setTimeScale(duration);
}

//! Change timescale of the zoomed view to duration
void OscilloscopeWindow::zoomedTimeScaleChanged(unsigned duration)
{
	int middlePos = (mainView->zoomPosEnd() + mainView->zoomPosStart()) >> 1;
	unsigned sampleWidth = signalInfo.timeToSample(duration);
	mainView->setZoomPos(middlePos - (sampleWidth>>1), middlePos + (sampleWidth>>1));
}

//! Choose the trigger to trigger on
void OscilloscopeWindow::triggerChannel()
{
	QAction *action = static_cast<QAction *>(sender());
	bool conversionResult;
	signalInfo.dataConverter->setTrigger(
		signalInfo.dataConverter->triggerType(),
		signalInfo.dataConverter->triggerTimeout(),
		action->data().toUInt(&conversionResult),
		signalInfo.dataConverter->triggerPos(),
		signalInfo.dataConverter->triggerValue()
	);
	assert(conversionResult);
}

//! Disable trigger
void OscilloscopeWindow::triggerNone()
{
	// set trigger type to NONE
	signalInfo.dataConverter->setTrigger(
		DataConverter::TRIGGER_NONE,
		signalInfo.dataConverter->triggerTimeout(),
		signalInfo.dataConverter->triggerChannel(),
		signalInfo.dataConverter->triggerPos(),
		signalInfo.dataConverter->triggerValue()
	);
	
	// disable trigger type and channel selection
	triggerTypeGroup->setEnabled(false);
	triggerChannelMenu->setEnabled(false);
	
	// unfreeze
	if (displayFreezeAct->isEnabled())
		displayFreezeAct->setChecked(false);
}

//! Set trigger on single shot mode
void OscilloscopeWindow::triggerSingle()
{
	qDebug() << "OscilloscopeWindow::triggerSingle";
	// set trigger type
	const QAction *triggerTypeAct = triggerTypeGroup->checkedAction();
	Q_ASSERT(triggerTypeAct);
	DataConverter::TriggerType triggerType = (DataConverter::TriggerType)triggerTypeAct->data().toUInt();
	signalInfo.dataConverter->setTrigger(
		triggerType,
		false,
		signalInfo.dataConverter->triggerChannel(),
		signalInfo.dataConverter->triggerPos(),
		signalInfo.dataConverter->triggerValue()
	);
	
	// enable trigger type and channel selection
	triggerTypeGroup->setEnabled(true);
	triggerChannelMenu->setEnabled(true);
	
	// unfreeze
	if (displayFreezeAct->isEnabled())
		displayFreezeAct->setChecked(false);

	qDebug() << "OscilloscopeWindow::triggerSingle::end";
	// zz
}

//! Set trigger to auto mode
void OscilloscopeWindow::triggerAuto()
{
	// set trigger type
	const QAction *triggerTypeAct = triggerTypeGroup->checkedAction();
	Q_ASSERT(triggerTypeAct);
	DataConverter::TriggerType triggerType = (DataConverter::TriggerType)triggerTypeAct->data().toUInt();

	signalInfo.dataConverter->setTrigger(
		triggerType,
		true,
		signalInfo.dataConverter->triggerChannel(),
		signalInfo.dataConverter->triggerPos(),
		signalInfo.dataConverter->triggerValue()
	);
	
	// enable trigger type selection
	triggerTypeGroup->setEnabled(true);
	triggerChannelMenu->setEnabled(true);
	
	// unfreeze
	if (displayFreezeAct->isEnabled())
		displayFreezeAct->setChecked(false);
}

//! Set trigger on up signal
void OscilloscopeWindow::triggerUp()
{
	signalInfo.dataConverter->setTrigger(
		DataConverter::TRIGGER_UP,
		signalInfo.dataConverter->triggerTimeout(),
		signalInfo.dataConverter->triggerChannel(),
		signalInfo.dataConverter->triggerPos(),
		signalInfo.dataConverter->triggerValue()
	);
}

//! Set trigger on down signal
void OscilloscopeWindow::triggerDown()
{
	signalInfo.dataConverter->setTrigger(
		DataConverter::TRIGGER_DOWN,
		signalInfo.dataConverter->triggerTimeout(),
		signalInfo.dataConverter->triggerChannel(),
		signalInfo.dataConverter->triggerPos(),
		signalInfo.dataConverter->triggerValue()
	);
}

//! Set trigger on both signal side
void OscilloscopeWindow::triggerBoth()
{
	signalInfo.dataConverter->setTrigger(
		DataConverter::TRIGGER_BOTH,
		signalInfo.dataConverter->triggerTimeout(),
		signalInfo.dataConverter->triggerChannel(),
		signalInfo.dataConverter->triggerPos(),
		signalInfo.dataConverter->triggerValue()
	);
}

//! Reset the trigger
void OscilloscopeWindow::triggerReset()
{
	triggerBothAct->trigger();
	triggerChannelAct[0]->trigger();

	signalInfo.dataConverter->setTrigger(
		signalInfo.dataConverter->triggerType(),
		signalInfo.dataConverter->triggerTimeout(),
		signalInfo.dataConverter->triggerChannel(),
		signalInfo.dataConverter->outputSampleCount() / 2,
		0
	);
}

//! Delete old plugin dock (if any), and recreate a new one by requesting plugins to generate their GUI (if any)
void OscilloscopeWindow::recreatePluginDock(const DataConverter::ActivePlugins &configuration)
{
	// delete old dock
	if (pluginDock)
	{
		removeDockWidget(pluginDock);
		pluginDock->deleteLater();
		pluginDock = NULL;
	}
	
	if (configuration.size() > 0)
	{
		// recreate new dock if there is plugins
		pluginDock = new QDockWidget(tr("Plugins"), this);
		pluginDock->setFeatures(QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetFloatable);
		
		QScrollArea *pluginScrollArea = new QScrollArea(pluginDock);
		pluginDock->setWidget(pluginScrollArea);
		
		QWidget *pluginListWidget = new QWidget();
		pluginScrollArea->setWidget(pluginListWidget);
		
		pluginListWidget->resize(200, 0);
		QVBoxLayout *pluginDockLayout = new QVBoxLayout(pluginListWidget);
		pluginDockLayout->setSizeConstraint(QLayout::SetMinimumSize);
		
		// add new plugins gui
		for (size_t plugin = 0; plugin < configuration.size(); plugin++)
		{
			ProcessingPlugin *pluginPtr = configuration[plugin].plugin;
		
			QLabel *subTitle;
			
			subTitle = new QLabel(pluginPtr->description()->name());
			subTitle->setBackgroundRole(QPalette::Base);
			subTitle->setMargin(1);
			subTitle->setAlignment(Qt::AlignHCenter);
			pluginDockLayout->addWidget(subTitle);
			
			QString subTitleText;
			for (size_t i = 0; i < configuration[plugin].inputs.size(); i++)
			{
				subTitleText += channelNumberToString(configuration[plugin].inputs[i]);
				if (i+1 < configuration[plugin].inputs.size())
					subTitleText += ", ";
			}
			subTitleText += ' ';
			subTitleText += QChar(0x2192);
			subTitleText += ' ';
			for (size_t i = 0; i < configuration[plugin].outputs.size(); i++)
			{
				subTitleText += channelNumberToString(configuration[plugin].outputs[i]);
				if (i+1 < configuration[plugin].outputs.size())
					subTitleText += ", ";
			}
			subTitle = new QLabel(subTitleText);
			subTitle->setAlignment(Qt::AlignHCenter);
			pluginDockLayout->addWidget(subTitle);
			
			QWidget *pluginGUI = pluginPtr->createGUI();
			if (pluginGUI)
				pluginDockLayout->addWidget(pluginGUI);
				
			pluginDockLayout->addSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
		}
		
		addDockWidget(Qt::RightDockWidgetArea, pluginDock);
	}
}

//! Reconfigure the plugins by getting the actual config from the data converter, calling the plugin dialog and setting the new config to the data converter
void OscilloscopeWindow::configurePlugins()
{
	ProcessingPluginDialog dialog(dataSource->inputCount(), this);
	
	// prepare
	unsigned channelCount;
	DataConverter::ActivePlugins activePlugins;
	signalInfo.dataConverter->getPluginMapping(&activePlugins, &channelCount);
	
	dialog.setChannelCount(channelCount);
	
	for (size_t plugin = 0; plugin < activePlugins.size(); plugin++)
		dialog.usePlugin(activePlugins[plugin].plugin->description(), &activePlugins[plugin].inputs, &activePlugins[plugin].outputs, activePlugins[plugin].plugin);

	dialog.recomputeUseChannelCount();

	// execute
	if (dialog.exec() == QDialog::Accepted)
	{
		Q_ASSERT(dialog.pluginsInstanceTable.size() == (unsigned)dialog.pluginsActiveTable->rowCount());
		// read results
		activePlugins.resize(dialog.pluginsActiveTable->rowCount());
		for (size_t plugin = 0; plugin < activePlugins.size(); plugin++)
		{
			// get the plugin id from its name
			QList<QListWidgetItem *> items = dialog.pluginsAvailableList->findItems(dialog.pluginsActiveTable->item(plugin, 0)->text(), Qt::MatchExactly);
			Q_ASSERT(!items.empty());
			unsigned id = dialog.pluginsAvailableList->row(items.first());

			// create the plugin
			Q_ASSERT(id < processingPluginsDescriptions.size());
			// try to use old instance, if it does not exists, create a new one
			if (dialog.pluginsInstanceTable[plugin])
				activePlugins[plugin].plugin = dialog.pluginsInstanceTable[plugin];
			else
				activePlugins[plugin].plugin = processingPluginsDescriptions[id]->create(dataSource);

			// set the inputs
			activePlugins[plugin].inputs.resize(processingPluginsDescriptions[id]->inputCount());
			for (size_t i = 0; i < activePlugins[plugin].inputs.size(); i++)
				activePlugins[plugin].inputs[i] = channelNumberFromString(dialog.pluginsActiveTable->item(plugin, 1 + i)->text());

			// set the outputs
			activePlugins[plugin].outputs.resize(processingPluginsDescriptions[id]->outputCount());
			for (size_t i = 0; i < activePlugins[plugin].outputs.size(); i++)
				activePlugins[plugin].outputs[i] = channelNumberFromString(dialog.pluginsActiveTable->item(plugin, 1 + dialog.inputCount + i)->text());
		}

		// assign new configuration to converter
		signalInfo.dataConverter->setPluginMapping(activePlugins, dialog.channelCount());
		
		// recerate plugin dock
		recreatePluginDock(activePlugins);
	}
}

//! Save the actual plugin configuration
void OscilloscopeWindow::savePluginsConfiguration()
{
	QString filename = QFileDialog::getSaveFileName(this, "", "", "Plugins configuration (*.pcfg)");
	if (filename != "")
	{
		if (filename.lastIndexOf(".") < 0)
			filename += ".pcfg";
		QFile file(filename);
		if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
		{
			unsigned channelCount;
			DataConverter::ActivePlugins activePlugins;
			QTextStream out(&file);

			// read actual configuration from converter
			signalInfo.dataConverter->getPluginMapping(&activePlugins, &channelCount);

			// save plugins configuration
			out << (channelCount - dataSource->inputCount()) << endl;
			for (size_t plugin = 0; plugin < activePlugins.size(); plugin++)
			{
				// save plugin system name
				out << activePlugins[plugin].plugin->description()->systemName() << " ";

				// save inputs and outputs configuration
				for (size_t i = 0; i < activePlugins[plugin].inputs.size(); i++)
					out << physicChannelIdToLogic(activePlugins[plugin].inputs[i]) << " ";
				for (size_t i = 0; i < activePlugins[plugin].outputs.size(); i++)
					out << physicChannelIdToLogic(activePlugins[plugin].outputs[i]) << " ";

				// save instance data
				activePlugins[plugin].plugin->save(&out);

				// new line
				if (plugin + 1 < activePlugins.size())
					out << endl;
			}
		}
	}
}

//! Load a new actual plugin configuration
void OscilloscopeWindow::loadPluginsConfiguration()
{
	QString filename = QFileDialog::getOpenFileName(this, "", "", "Plugins configuration (*.pcfg)");
	if (filename != "")
	{
		QFile file(filename);
		if (file.open(QIODevice::ReadOnly))
		{
			unsigned channelCount;
			DataConverter::ActivePlugins newConfiguration;
			QTextStream in(&file);
			
			// load plugins configuration
			in >> channelCount;
			channelCount += dataSource->inputCount();
			while (!in.atEnd())
			{
				// load plugin system name, break if null
				QString pluginSystemName;
				in >> pluginSystemName;
				if (pluginSystemName == "")
					break;

				size_t pos = newConfiguration.size();
				
				// get description from plugin name
				ProcessingPluginDescription *description = NULL;
				for (size_t i = 0; i < processingPluginsDescriptions.size(); i++)
				{
					if (processingPluginsDescriptions[i]->systemName() == pluginSystemName)
					{
						description = processingPluginsDescriptions[i];
						break;
					}
				}

				// no plugin with correct name found
				if (description == NULL)
				{
					QMessageBox::warning(this, pluginSystemName, tr("No plugin found of that name, ignoring"), QMessageBox::Ok, QMessageBox::NoButton);
					// consume line
					in.readLine();
					continue;
				}

				// create new plugin instance
				newConfiguration.resize(pos + 1);
				newConfiguration[pos].plugin = description->create(dataSource);
				
				// load inputs and outputs configuration
				newConfiguration[pos].inputs.resize(description->inputCount());
				for (size_t i = 0; i < newConfiguration[pos].inputs.size(); i++)
				{
					int logic;
					in >> logic;
					bool ok;
					unsigned physic = logicChannelIdToPhysic(logic, &ok);
					if (ok)
						newConfiguration[pos].inputs[i] = physic;
					else
						newConfiguration[pos].inputs[i] = dataSource->inputCount();
				}
				newConfiguration[pos].outputs.resize(description->outputCount());
				for (size_t i = 0; i < newConfiguration[pos].outputs.size(); i++)
				{
					int logic;
					in >> logic;
					bool ok;
					unsigned physic = logicChannelIdToPhysic(logic, &ok);
					if (ok)
						newConfiguration[pos].outputs[i] = physic;
					else
						newConfiguration[pos].outputs[i] = dataSource->inputCount();
				}
				
				// load instance data
				newConfiguration[pos].plugin->load(&in);
			}

			// assign new configuration to converter
			signalInfo.dataConverter->setPluginMapping(newConfiguration, channelCount);
		
			// recerate plugin dock
			recreatePluginDock(newConfiguration);
		}
	}
}

//! Display the about box
void OscilloscopeWindow::about()
{
	QMessageBox::about(this, tr("About OsQoop"),
	tr( "<h4 align=\"center\">OsQoop is a free software Oscilloscope.</h4>"
	"<h5 align=\"center\">Version " OSQOOP_VERSION_MAJOR "." OSQOOP_VERSION_MINOR "." OSQOOP_VERSION_REV "</h5>"
	"<p>Copyright (c) 2005-2006 Stéphane Magnenat and René Beuchat.<br/>"
	"Laboratoire de Systèmes Numériques (http://www.eig.ch/fr/laboratoires/systemes-numeriques/), "
	"Haute Ecole du paysage, d’ingénierie et d’architecture (http://hepia.hesge.ch/), Switzerland."
	"Osqoop was partially funded by the TSE project.</p>"
	"<p>Copyright (c) 2006-2009 Stéphane Magnenat (http://stephane.magnenat.net)</p>"
	"<p>See authors file in source distribution for details about contributors.</p>"));
}



//! Create actions and menu for this window
void OscilloscopeWindow::createActionsAndMenus()
{
	QAction *printAct = new QAction(tr("&Print"), this);
	printAct->setShortcut(QString("Ctrl+P"));
	printAct->setStatusTip(tr("Print the actual display"));
	connect(printAct, SIGNAL(triggered()), SLOT(print()));

	QAction *pdfExportAct = new QAction(tr("Export to PD&F"), this);
	pdfExportAct->setStatusTip(tr("Export the actual display to PDF"));
	connect(pdfExportAct, SIGNAL(triggered()), SLOT(exportToPDF()));

	QAction *dataExportAct = new QAction(tr("&Export Data"), this);
	dataExportAct->setShortcut(QString("Ctrl+E"));
	dataExportAct->setStatusTip(tr("Export the data to text"));
	connect(dataExportAct, SIGNAL(triggered()), SLOT(exportData()));
	
	QAction *visibleDataExportAct = new QAction(tr("Export &visible Data"), this);
	visibleDataExportAct->setShortcut(QString("Ctrl+V"));
	visibleDataExportAct->setStatusTip(tr("Export the displayed data to text"));
	connect(visibleDataExportAct, SIGNAL(triggered()), SLOT(exportVisibleData()));

	QAction *exitAct = new QAction(tr("E&xit"), this);
	exitAct->setShortcut(QString("Ctrl+Q"));
	exitAct->setStatusTip(tr("Exit the application"));
	connect(exitAct, SIGNAL(triggered()), SLOT(close()));

	QAction *zoomAct = new QAction(tr("&Zoom"), this);
	zoomAct->setShortcut(QString("z"));
	zoomAct->setCheckable(true);
	zoomAct->setChecked(false);
	connect(zoomAct, SIGNAL(triggered()), SLOT(zoomAction()));
	connect(zoomAct, SIGNAL(toggled(bool)), mainView, SLOT(setZoomMarker(bool)));
	connect(zoomAct, SIGNAL(toggled(bool)), zoomedView, SLOT(setVisible(bool)));

	displayFreezeAct = new QAction(tr("&Freeze"), this);
	displayFreezeAct->setShortcut(QString(" "));
	displayFreezeAct->setCheckable(true);
	displayFreezeAct->setChecked(false);
	connect(displayFreezeAct, SIGNAL(toggled(bool)), SLOT(freezeDisplayToggled(bool)));

	QActionGroup *drawingModeGroup = new QActionGroup(this);
	
	QAction *drawingLineAct = new QAction(tr("&Line"), this);
	drawingLineAct->setShortcut(QString("l"));
	drawingLineAct->setData(SignalViewWidget::LineDrawing);
	drawingLineAct->setCheckable(true);
	drawingLineAct->setChecked(true);
	connect(drawingLineAct, SIGNAL(triggered()), SLOT(changeDrawingMode()));
	drawingModeGroup->addAction(drawingLineAct);
	
	QAction *drawingPointAct = new QAction(tr("&Point"), this);
	drawingPointAct->setShortcut(QString("p"));
	drawingPointAct->setData(SignalViewWidget::PointDrawing);
	drawingPointAct->setCheckable(true);
	connect(drawingPointAct, SIGNAL(triggered()), SLOT(changeDrawingMode()));
	drawingModeGroup->addAction(drawingPointAct);

	QAction *persistantDisplayAct = new QAction(tr("Pe&rsistent"), this);
	persistantDisplayAct->setShortcut(QString("ctrl+r"));
	persistantDisplayAct->setCheckable(true);
	connect(persistantDisplayAct, SIGNAL(triggered(bool)), mainView, SLOT(setDisplayPersistance(bool)));

	QAction *persistanceFadeoutAct = new QAction(tr("Persistence Fade&out"), this);
	persistanceFadeoutAct->setShortcut(QString("ctrl+f"));
	persistanceFadeoutAct->setCheckable(true);
	persistanceFadeoutAct->setEnabled(false);
	connect(persistantDisplayAct, SIGNAL(triggered(bool)), persistanceFadeoutAct, SLOT(setEnabled(bool)));
	connect(persistanceFadeoutAct, SIGNAL(triggered(bool)), mainView, SLOT(setPersistanceFadeout(bool)));

	QAction *antialiasedDisplayAct = new QAction(tr("&Antialiasing"), this);
	antialiasedDisplayAct->setCheckable(true);
	connect(antialiasedDisplayAct, SIGNAL(triggered(bool)), mainView, SLOT(setAntialiasing(bool)));
	connect(antialiasedDisplayAct, SIGNAL(triggered(bool)), zoomedView, SLOT(setAntialiasing(bool)));
	
	QAction *alphaBlendingAct = new QAction(tr("Alpha-&blending"), this);
	alphaBlendingAct->setCheckable(true);
	alphaBlendingAct->setChecked(true);
	connect(alphaBlendingAct, SIGNAL(triggered(bool)), mainView, SLOT(setAlphaBlending(bool)));
	connect(alphaBlendingAct, SIGNAL(triggered(bool)), zoomedView, SLOT(setAlphaBlending(bool)));
	
	timescaleGroup = new QActionGroup(this);
	for (size_t i = 0; i < ScaleFactorCount; i++)
	{
		unsigned factor = getScaleFactor(i);
		timeScaleAct[i] = new QAction(timeScaleToStringWidthToDiv(factor), this);
		timeScaleAct[i]->setData(QVariant(factor));
		timeScaleAct[i]->setCheckable(true);
		connect(timeScaleAct[i], SIGNAL(triggered()), SLOT(timeScaleAction()));
		timescaleGroup->addAction(timeScaleAct[i]);
	}
	timeScaleAct[getScaleFactorInvert(signalInfo.duration)]->setChecked(true);
	
	// trigger mode
	
	triggerNoneAct = new QAction(tr("&None"), this);
	triggerNoneAct->setShortcut(QString("n"));
	triggerNoneAct->setCheckable(true);
	triggerNoneAct->setChecked(signalInfo.dataConverter->triggerType() == DataConverter::TRIGGER_NONE);
	connect(triggerNoneAct, SIGNAL(triggered()), SLOT(triggerNone()));
	
	triggerSingleAct = new QAction(tr("&Single"), this);
	triggerSingleAct->setShortcut(QString("s"));
	triggerSingleAct->setCheckable(true);
	triggerSingleAct->setChecked(
		(signalInfo.dataConverter->triggerType() != DataConverter::TRIGGER_NONE) &&
		(signalInfo.dataConverter->triggerTimeout() == false)
	);
	connect(triggerSingleAct, SIGNAL(triggered()), SLOT(triggerSingle()));

	triggerAutoAct = new QAction(tr("&Auto"), this);
	triggerAutoAct->setShortcut(QString("a"));
	triggerAutoAct->setCheckable(true);
	triggerAutoAct->setChecked(
		(signalInfo.dataConverter->triggerType() != DataConverter::TRIGGER_NONE) &&
		(signalInfo.dataConverter->triggerTimeout() == true)
	);
	connect(triggerAutoAct, SIGNAL(triggered()), SLOT(triggerAuto()));
	
	QActionGroup *triggerModeGroup = new QActionGroup(this);
	triggerModeGroup->addAction(triggerNoneAct);
	triggerModeGroup->addAction(triggerSingleAct);
	triggerModeGroup->addAction(triggerAutoAct);
	
	// trigger type
	
	triggerUpAct = new QAction(tr("&Up signal"), this);
	triggerUpAct->setShortcut(QString("u"));
	triggerUpAct->setCheckable(true);
	triggerUpAct->setChecked(signalInfo.dataConverter->triggerType() == DataConverter::TRIGGER_UP);
	triggerUpAct->setData(QVariant((unsigned)DataConverter::TRIGGER_UP));
	connect(triggerUpAct, SIGNAL(triggered()), SLOT(triggerUp()));
	
	triggerDownAct = new QAction(tr("&Down signal"), this);
	triggerDownAct->setShortcut(QString("d"));
	triggerDownAct->setCheckable(true);
	triggerDownAct->setChecked(signalInfo.dataConverter->triggerType() == DataConverter::TRIGGER_DOWN);
	triggerDownAct->setData(QVariant((unsigned)DataConverter::TRIGGER_DOWN));
	connect(triggerDownAct, SIGNAL(triggered()), SLOT(triggerDown()));
	
	triggerBothAct = new QAction(tr("&Both direction"), this);
	triggerBothAct->setShortcut(QString("b"));
	triggerBothAct->setCheckable(true);
	triggerBothAct->setChecked((signalInfo.dataConverter->triggerType() == DataConverter::TRIGGER_BOTH) || (signalInfo.dataConverter->triggerType() == DataConverter::TRIGGER_NONE));
	triggerBothAct->setData(QVariant((unsigned)DataConverter::TRIGGER_BOTH));
	connect(triggerBothAct, SIGNAL(triggered()), SLOT(triggerBoth()));
	
	triggerTypeGroup = new QActionGroup(this);
	triggerTypeGroup->addAction(triggerUpAct);
	triggerTypeGroup->addAction(triggerDownAct);
	triggerTypeGroup->addAction(triggerBothAct);
	triggerTypeGroup->setEnabled(signalInfo.dataConverter->triggerType() != DataConverter::TRIGGER_NONE);
	
	triggerChannelGroup = new QActionGroup(this);
	/*for (size_t i = 0; i < 8; i ++)
	{
		QString actTitle(channelNumberToString(i));
		triggerChannelAct[i] = new QAction(actTitle, this);
		triggerChannelAct[i]->setData(QVariant(i));
		triggerChannelAct[i]->setCheckable(true);
		triggerChannelGroup->addAction(triggerChannelAct[i]);
		connect(triggerChannelAct[i], SIGNAL(triggered()), SLOT(triggerChannel()));
	}
	triggerChannelAct[signalInfo.dataConverter->triggerChannel()]->setChecked(true);*/
	
	QAction *triggerResetAct = new QAction(tr("&Reset"), this);
	triggerResetAct->setShortcut(QString("r"));
	triggerResetAct->setStatusTip(tr("Reset the trigger to default position"));
	connect(triggerResetAct, SIGNAL(triggered()), SLOT(triggerReset()));
	
	QAction *pluginsConfigureAct = new QAction(tr("&Configure"), this);
	pluginsConfigureAct->setStatusTip(tr("Choose which plugin to use on which channels"));
	connect(pluginsConfigureAct, SIGNAL(triggered()), SLOT(configurePlugins()));

	QAction *savePluginsConfiguration = new QAction(tr("&Save configuration"), this);
	savePluginsConfiguration->setStatusTip(tr("Save the actual plugins configuration to a text file"));
	connect(savePluginsConfiguration, SIGNAL(triggered()), SLOT(savePluginsConfiguration()));

	QAction *loadPluginsConfiguration = new QAction(tr("&Load configuration"), this);
	loadPluginsConfiguration->setStatusTip(tr("Load a new plugins configuration from a text file"));
	connect(loadPluginsConfiguration, SIGNAL(triggered()), SLOT(loadPluginsConfiguration()));

	QAction *aboutAct = new QAction(tr("&About"), this);
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

	QAction *aboutQtAct = new QAction(tr("&About Qt"), this);
	connect(aboutQtAct, SIGNAL(triggered()), QApplication::instance(), SLOT(aboutQt()));

	// menus
	
	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(printAct);
	fileMenu->addAction(pdfExportAct);
	fileMenu->addSeparator();
	fileMenu->addAction(dataExportAct);
	fileMenu->addAction(visibleDataExportAct);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAct);

	QMenu *displayMenu = menuBar()->addMenu(tr("&Display"));
	displayMenu->addAction(zoomAct);
	displayMenu->addSeparator();
	displayMenu->addAction(displayFreezeAct);
	displayMenu->addSeparator();
	displayMenu->addAction(drawingLineAct);
	displayMenu->addAction(drawingPointAct);
	displayMenu->addSeparator();
	displayMenu->addAction(persistantDisplayAct);
	displayMenu->addAction(persistanceFadeoutAct);
	displayMenu->addSeparator();
	displayMenu->addAction(antialiasedDisplayAct);
	displayMenu->addAction(alphaBlendingAct);

	channelMenu = menuBar()->addMenu(tr("&Channel"));
	
	QMenu *timescaleMenu = menuBar()->addMenu(tr("&Timescale"));
	for (size_t i = 0; i < ScaleFactorCount; i++)
		timescaleMenu->addAction(timeScaleAct[i]);
	
	QMenu *triggerMenu = menuBar()->addMenu(tr("T&rigger"));
	triggerMenu->addAction(triggerNoneAct);
	triggerMenu->addAction(triggerSingleAct);
	triggerMenu->addAction(triggerAutoAct);
	triggerMenu->addSeparator();
	triggerMenu->addAction(triggerUpAct);
	triggerMenu->addAction(triggerDownAct);
	triggerMenu->addAction(triggerBothAct);
	triggerMenu->addSeparator();

	triggerChannelMenu = triggerMenu->addMenu(tr("&Channel"));
	/*for (size_t i = 0; i < 8; i++)
		triggerChannelMenu->addAction(triggerChannelAct[i]);*/
	triggerMenu->addSeparator();
	triggerMenu->addAction(triggerResetAct);
	
	QMenu *pluginsMenu = menuBar()->addMenu(tr("&Plugins"));
	pluginsMenu->addAction(pluginsConfigureAct);
	pluginsMenu->addAction(savePluginsConfiguration);
	pluginsMenu->addAction(loadPluginsConfiguration);

	QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(aboutAct);
	helpMenu->addAction(aboutQtAct);
	
	recreateChannelActionsAndMenu();
}

//! Recreate the channel menu and actions when the number of channel changes
void OscilloscopeWindow::recreateChannelActionsAndMenu()
{
	channelMenu->clear();
	
	// create action for auto layout
	QAction *channelAutoLayoutAct = new QAction(tr("Auto &Layout"), this);
	channelAutoLayoutAct->setShortcut(QString("Ctrl+l"));
	connect(channelAutoLayoutAct, SIGNAL(triggered()), mainView, SLOT(autoLayoutChannels()));
	connect(channelAutoLayoutAct, SIGNAL(triggered()), zoomedView, SLOT(autoLayoutChannels()));
	channelMenu->addAction(channelAutoLayoutAct);
	
	channelMenu->addSeparator();
	
	// create actions for all channels
	channelAct.resize(signalInfo.channelCount);
	for (unsigned channel = 0; channel < signalInfo.channelCount; channel++)
	{
		QAction *action = new QAction(channelNumberToString(channel), this);
		action->setData(QVariant(channel));
		if (channel < 9)
			action->setShortcut(QString("%1").arg(channel + 1));
		action->setCheckable(true);
		action->setChecked(mainView->channelEnabled(channel));
		connect(action, SIGNAL(triggered()), SLOT(channelAction()));
		channelAct[channel] = action;
		channelMenu->addAction(action);
	}

	// create action for set all
	QAction *channelAllAct = new QAction(tr("&All"), this);
	channelAllAct->setShortcut(QString("Ctrl+a"));
	connect(channelAllAct, SIGNAL(triggered()), SLOT(channelAllAction()));
	channelMenu->addAction(channelAllAct);

	// create action for set none
	QAction *channelNoneAct = new QAction(tr("&None"), this);
	channelNoneAct->setShortcut(QString("Ctrl+n"));
	connect(channelNoneAct, SIGNAL(triggered()), SLOT(channelNoneAction()));
	channelMenu->addAction(channelNoneAct);

	triggerChannelMenu->clear();

	// create actions for channel trigger menu
	triggerChannelAct.resize(signalInfo.channelCount);
	for (unsigned channel = 0; channel < signalInfo.channelCount; channel++)
	{
		QAction *action = new QAction(channelNumberToString(channel), this);
		action->setData(QVariant(channel));
		action->setCheckable(true);
		triggerChannelGroup->addAction(action);
		connect(action, SIGNAL(triggered()), SLOT(triggerChannel()));
		triggerChannelAct[channel] = action;
		triggerChannelMenu->addAction(action);
	}
	triggerChannelAct[signalInfo.dataConverter->triggerChannel()]->setChecked(true);
	triggerChannelMenu->setEnabled(signalInfo.dataConverter->triggerType() != DataConverter::TRIGGER_NONE);
}

//! Load plugins from the plugin subdirectory
void OscilloscopeWindow::loadPlugins()
{
	QStringList potentialDirs;
	
	potentialDirs << QCoreApplication::applicationDirPath();
	potentialDirs << (QCoreApplication::applicationDirPath() + "/../share/osqoop");
	potentialDirs << ".osqoop/";
	
	foreach (QString dirName, potentialDirs)
	{
		if (!QFile::exists(dirName))
			continue;
		QDir pluginsDir = QDir(dirName);
		if (pluginsDir.cd("processing"))
		{
			foreach (QString fileName, pluginsDir.entryList(QDir::Files))
			{
				QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
				QObject *plugin = loader.instance();
				if (plugin)
				{
					ProcessingPluginDescription *iProcessingDescription = qobject_cast<ProcessingPluginDescription *>(plugin);
					if (iProcessingDescription)
						processingPluginsDescriptions.push_back(iProcessingDescription);
				}
				else
					qDebug() << "Processing plugin " << fileName << " failed to load: " << loader.errorString();
			}
			pluginsDir.cd("..");
		}
		if (pluginsDir.cd("datasource"))
		{
			foreach (QString fileName, pluginsDir.entryList(QDir::Files))
			{
				QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
				QObject *plugin = loader.instance();
				if (plugin)
				{
					DataSourceDescription *iDataSourceDescription = qobject_cast<DataSourceDescription *>(plugin);
					if (iDataSourceDescription)
						dataSourceDescriptions.push_back(iDataSourceDescription);
				}
				else
					qDebug() << "Datasource plugin " << fileName << " failed to load: " << loader.errorString();
			}
			pluginsDir.cd("..");
		}
	}
}

//! Connect to the data converter to get the datastream
void OscilloscopeWindow::connectToDataConverter()
{
	connect(
		signalInfo.dataConverter, 
		SIGNAL(dataReady(const std::valarray<signed short> &, unsigned, unsigned, unsigned, unsigned, unsigned)), 
		this,
		SLOT(setData(const std::valarray<signed short> &, unsigned, unsigned, unsigned, unsigned, unsigned)),
		Qt::QueuedConnection
	);
}

//! Disconnect from the data converter to stop getting the datastream
void OscilloscopeWindow::disconnectFromDataConverter()
{
	disconnect(
		signalInfo.dataConverter,
		SIGNAL(dataReady(const std::valarray<signed short> &, unsigned, unsigned, unsigned, unsigned, unsigned)),
		this,
		SLOT(setData(const std::valarray<signed short> &, unsigned, unsigned, unsigned, unsigned, unsigned))
	);
}

//! Save GUI parameters to settings in order to reload them at next start
void OscilloscopeWindow::saveGUISettings()
{
	QSettings settings(ORGANISATION_NAME, APPLICATION_NAME);
	settings.setValue("extendedChannelCount", signalInfo.channelCount - dataSource->inputCount());
	settings.setValue("timeScale", signalInfo.duration);
	
	settings.beginGroup("mainView");
	mainView->saveGUISettings(&settings);
	settings.endGroup();
}

//! Window closeed. Save GUI state for next reload
void OscilloscopeWindow::closeEvent(QCloseEvent *)
{
	if (signalInfo.dataConverter)
	{
		saveGUISettings();
		saveCustomChannelNames();
	}
}

//! Load GUI parameters from settings
void OscilloscopeWindow::loadGUISettings()
{
	QSettings settings(ORGANISATION_NAME, APPLICATION_NAME);
	QStringList groups = settings.childGroups();

	if (groups.contains("mainView"))
	{
		settings.beginGroup("mainView");
		mainView->loadGUISettings(&settings);
		settings.endGroup();
	}
}
