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

#include "DataConverter.h"
#include "DataSource.h"
#include "ProcessingPlugin.h"
#include "DataConverter.h"
#include <set>
#include <QStringList>
#include <QSettings>
#include "Settings.h"
#include "Utilities.h"
#include <QtDebug>

const unsigned sampleCountForIncremental = 16384;
const unsigned toSendIncrementalThreshold = 4096;

//! Constructor. channelCount is the initial number of channel to create and timescale the initial acquisition duration
DataConverter::DataConverter(DataSource *dataSource, unsigned channelCount, unsigned timescale)
{
	Q_ASSERT(channelCount >= dataSource->inputCount());

	// init datas from parameters
	this->dataSource = dataSource;
	samplingRate = dataSource->samplingRate();
	_outputSampleCount = ((samplingRate * timescale) / 1000) + 1;
	_channelCount = channelCount;
	_outputTime = timescale;
	
	// load trigger settings if possible
	QSettings settings(ORGANISATION_NAME, APPLICATION_NAME);
	QStringList groups = settings.childGroups();
	if (groups.contains("trigger"))
	{
		// reload values from settings but take some care to bound them
		settings.beginGroup("trigger");
		_triggerType = (TriggerType)std::min(settings.value("triggerType").toUInt(), (unsigned)TRIGGER_BOTH);
		_triggerTimeout = settings.value("triggerTimeout").toBool();
		bool ok;
		unsigned id = logicChannelIdToPhysic(settings.value("triggerChannel").toInt(), &ok);
		if (ok && (id < _channelCount))
			_triggerChannel = id;
		else
			_triggerChannel = 0;
		_triggerValue = settings.value("triggerValue").toInt();
		_triggerPos = std::min(settings.value("triggerPos").toUInt(), _outputSampleCount);
		settings.endGroup();
	}
	else
	{
		_triggerType = TRIGGER_BOTH;
		_triggerTimeout = true;
		_triggerChannel = 0;
		_triggerValue = 0;
		_triggerPos = _outputSampleCount >> 1;
	}

	// internal parameters initialisation
	pluginConfigurationChanged = false;
	quit = false;
}

//! Destructor, set quit to true and wait until run is finished
DataConverter::~DataConverter()
{
	// save trigger settings
	QSettings settings(ORGANISATION_NAME, APPLICATION_NAME);
	settings.beginGroup("trigger");
	settings.setValue("triggerType", (unsigned)_triggerType);
	settings.setValue("triggerTimeout", _triggerTimeout);
	settings.setValue("triggerChannel", physicChannelIdToLogic(_triggerChannel));
	settings.setValue("triggerValue", _triggerValue);
	settings.setValue("triggerPos", _triggerPos);
	settings.endGroup();

	// first stop consumer
	quit = true;
	wait();

	// then stop producer (otherwise deadlock arises)
	delete dataSource;
}

//! Change the timescale of output data in millisecond
void DataConverter::setTimeScale(unsigned ms)
{
	QMutexLocker locker(&mutex);
	double oldTriggerPos = static_cast<double>(_triggerPos) / static_cast<double>(_outputSampleCount);
	_outputSampleCount = ((samplingRate * ms) / 1000)+1;
	_outputTime = ms;
	_triggerPos = static_cast<unsigned>(oldTriggerPos * static_cast<double>(_outputSampleCount));
}

//! Set the trigger
void DataConverter::setTrigger(TriggerType type, bool timeout, unsigned channel, unsigned pos, signed short value)
{
	QMutexLocker locker(&mutex);
	_triggerTimeout = timeout;
	_triggerType = type;
	_triggerChannel = channel;
	_triggerValue = value;
	_triggerPos = pos;
}

//! Get the actual plugin configuration. Simply copy our configuration to the caller's pointer
void DataConverter::getPluginMapping(ActivePlugins *configuration, unsigned *channelCount) const
{
	QMutexLocker locker(&mutex);
	*configuration = _plugins;
	*channelCount = _channelCount;
}

//! Set a new plugin configuration. The previous plugins instance are destroyed, the new one are kept for later destruction
void DataConverter::setPluginMapping(const ActivePlugins &configuration, unsigned channelCount)
{
	Q_ASSERT(channelCount >= dataSource->inputCount());

	QMutexLocker locker(&mutex);
	_plugins = configuration;
	_channelCount = channelCount;
	pluginConfigurationChanged = true;
}

//! Thread running method. Get sample from source, trigger and emits dataReady
void DataConverter::run()
{
	// read parameters
	mutex.lock();
	unsigned outputSampleCount = _outputSampleCount;
	unsigned outputTime = _outputTime;
	TriggerType triggerType = _triggerType;
	bool triggerTimeout = _triggerTimeout;
	unsigned triggerChannel = _triggerChannel;
	signed short triggerValue = _triggerValue;
	unsigned triggerPos = _triggerPos;
	unsigned channelCount = _channelCount;
	ActivePlugins plugins = _plugins;
	mutex.unlock();
	
	unsigned actOutputSample = 0;
	bool triggerLocked = false;
	bool incremental = outputSampleCount > sampleCountForIncremental;
	unsigned toSendIncremental = 0;
	bool firstIncrementalSent = true;
	unsigned leftToGet = 0;
	std::valarray<std::valarray<signed short> > linearSamples(channelCount);
	for (size_t i = 0; i < linearSamples.size(); i++)
		linearSamples[i].resize(512);
	std::valarray<signed short> outputSamples(outputSampleCount * channelCount);
	std::valarray<signed short> previousValues((signed short)0, (size_t)channelCount);

	while (!quit)
	{
		// process plugin add/remove
		unsigned oldChannelCount = channelCount;
		mutex.lock();
		// if plugin configuration has changed
		if (pluginConfigurationChanged)
		{
			// create a set of plug that will be deleted
			std::set<ProcessingPlugin *> toDelete;
			// populate the set
			for (unsigned plugin = 0; plugin < plugins.size(); plugin++)
				toDelete.insert(plugins[plugin].plugin);
			// remove the ones still in used
			for (unsigned plugin = 0; plugin < _plugins.size(); plugin++)
				if (toDelete.find(_plugins[plugin].plugin) != toDelete.end())
					toDelete.erase(_plugins[plugin].plugin);
			//deleteActivePlugins(&plugins);
			// copy plugins
			plugins = _plugins;
			channelCount = _channelCount;
			// delete unused
			for (std::set<ProcessingPlugin *>::iterator i = toDelete.begin(); i != toDelete.end(); ++i)
				(*i)->terminate();
			pluginConfigurationChanged = false;
		}
		// if trigger enabled and no trigger found, reread trigger datas position, channel, value
		if ((triggerType != TRIGGER_NONE) && (!triggerLocked))
		{
			triggerType = _triggerType;
			triggerTimeout = _triggerTimeout;
			triggerChannel = _triggerChannel;
			triggerValue = _triggerValue;
			triggerPos = _triggerPos;
		}
		mutex.unlock();
		if (channelCount != oldChannelCount)
		{
			linearSamples.resize(channelCount);
			for (size_t i = 0; i < linearSamples.size(); i++)
				linearSamples[i].resize(512);
			outputSamples.resize(outputSampleCount * channelCount);
			previousValues.resize(channelCount, 0);
		}

		// read data from source
		unsigned microSecondToSleep = dataSource->getRawData(&linearSamples);
		if (microSecondToSleep)
			QThread::usleep(microSecondToSleep);
		
		// apply plugins
		for (unsigned plugin = 0; plugin < plugins.size(); plugin++)
		{
			ActivePlugin &p = plugins[plugin];
			std::valarray<signed short *> inputs(p.inputs.size());
			std::valarray<signed short *> outputs(p.outputs.size());

			// assign inputs
			for (size_t i = 0; i < inputs.size(); i++)
				inputs[i] = &linearSamples[p.inputs[i]][0];

			// assign outputs
			for (size_t i = 0; i < outputs.size(); i++)
				outputs[i] = &linearSamples[p.outputs[i]][0];

			// call plugin
			Q_ASSERT(p.plugin);
			p.plugin->processData(inputs, outputs, 512);
		}

		// trigger
		for (size_t sample = 0; sample < 512; sample++)
		{
			unsigned actOutputSamplePos = actOutputSample % outputSampleCount;
			for (size_t channel = 0; channel < channelCount; channel++)
			{
				signed short value = linearSamples[channel][sample];
				
				if (
					!triggerLocked &&
					(triggerType != TRIGGER_NONE) && 
					(triggerChannel == channel) &&
					(actOutputSample >= triggerPos)
					)
				{
					if (	(	(triggerType == TRIGGER_UP || triggerType == TRIGGER_BOTH) &&
								(previousValues[channel] <= triggerValue) && (value > triggerValue)
							)
							||
							(	(triggerType == TRIGGER_DOWN || triggerType == TRIGGER_BOTH) &&
								(previousValues[channel] >= triggerValue) && (value < triggerValue)
							)
						)
					{
						// triger event
						triggerLocked = true;
						// what we still have to get
						if (outputSampleCount >= triggerPos)
							leftToGet = outputSampleCount - triggerPos;
						else
							leftToGet = 0;
					}
				}
				
				outputSamples[channel * outputSampleCount + actOutputSamplePos] = value;
				previousValues[channel] = value;
			}

			// incremental send
			if (incremental && (triggerLocked || (triggerType == TRIGGER_NONE)) && (toSendIncremental >= toSendIncrementalThreshold))
			{
				// fill buffer
				std::valarray<signed short> toSendBuffer(toSendIncremental * channelCount);
				for (size_t channel = 0; channel < channelCount; channel++)
					for (unsigned sample = 0; sample < toSendIncremental; sample ++)
					{
						unsigned actOutputSamplePos = (actOutputSample + sample - toSendIncremental + 1) % outputSampleCount;
						toSendBuffer[channel * toSendIncremental + sample] = outputSamples[channel * outputSampleCount + actOutputSamplePos];
					}
				// emit
				emit dataReady(toSendBuffer, outputSampleCount, outputTime, channelCount, 0, firstIncrementalSent ? DATA_FRAME_START : 0);
				// reset incremental
				toSendIncremental = 0;
				firstIncrementalSent = false;
			}
			
			// counters
			actOutputSample++;
			toSendIncremental++;
			if (triggerLocked)
			{
				if (leftToGet > 0)
                    leftToGet--;
			}

			// we have either get all the samples or we have elapsed time
			if (
				(triggerLocked && (leftToGet == 0)) || // all sample got
				((triggerType != TRIGGER_NONE) && (!triggerLocked) && (triggerTimeout) && (actOutputSample > outputSampleCount + triggerPos)) || // no trigger found, timeout and send anyway
				((triggerType == TRIGGER_NONE) && (actOutputSample > outputSampleCount)) // triggerDisabled
				)
			{
				// packet full, emit it
				if (incremental)
				{
					// fill buffer
					std::valarray<signed short> toSendBuffer(toSendIncremental * channelCount);
					for (size_t channel = 0; channel < channelCount; channel++)
						for (unsigned sample = 0; sample < toSendIncremental; sample ++)
						{
							unsigned actOutputSamplePos = (actOutputSample + sample - toSendIncremental + 1) % outputSampleCount;
							toSendBuffer[channel * toSendIncremental + sample] = outputSamples[channel * outputSampleCount + actOutputSamplePos];
						}
					// emit
					emit dataReady(toSendBuffer, outputSampleCount, outputTime, channelCount, 0, firstIncrementalSent ? DATA_FRAME_START_END : DATA_FRAME_END);
				}
				else
					emit dataReady(outputSamples, outputSampleCount, outputTime, channelCount, ((actOutputSamplePos + 1) % outputSampleCount), DATA_FRAME_START_END);
				
				// get new size and params
				unsigned oldOutputSampleCount = outputSampleCount;
				mutex.lock();
				outputSampleCount = _outputSampleCount;
				outputTime = _outputTime;
				triggerType = _triggerType;
				triggerTimeout = _triggerTimeout;
				triggerChannel = _triggerChannel;
				triggerValue = _triggerValue;
				triggerPos = _triggerPos;
				mutex.unlock();
				
				// reset output samples
				if (outputSampleCount != oldOutputSampleCount)
					outputSamples.resize(outputSampleCount * channelCount);
				Q_ASSERT(outputSamples.size() > channelCount);
				actOutputSample = 0;
				triggerLocked = false;
				incremental = outputSampleCount > sampleCountForIncremental;
				toSendIncremental = 0;
				firstIncrementalSent = true;
			}
		}
	}

	deleteActivePlugins(&plugins);
}

//! Delete all instances of active plugins (but not their description which do not lie in this object)
void DataConverter::deleteActivePlugins(ActivePlugins *toDelete)
{
	for (unsigned plugin = 0; plugin < toDelete->size(); plugin++)
	{
		Q_ASSERT((*toDelete)[plugin].plugin);
		// we use terminate so that if plugin is a QObject and has pending events it get them
		(*toDelete)[plugin].plugin->terminate();
		(*toDelete)[plugin].plugin = NULL;
	}
}
