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

#ifndef __DATA_CONVERTER_H
#define __DATA_CONVERTER_H

#include <QThread>
#include <QMutex>
#include <valarray>
#include <vector>

class ProcessingPlugin;
class DataSource;

//! Get signal from a DataSource object. Implement triggers and emit ready datas
class DataConverter : public QThread
{
	Q_OBJECT
	
public:
	//! Possible trigger types
	enum TriggerType
	{
		TRIGGER_NONE = 0, //!< do not trig
		TRIGGER_UP, //!< trig when signal goes up and cross the value
		TRIGGER_DOWN, //!< trig when signal goes down and cross the value
		TRIGGER_BOTH //!< trig whenever the signal cross the value
	};
	
	//! Flags to qualify data frame sent for viewing
	enum DataFrameFlags
	{
		DATA_FRAME_START = 0x1, //!< start of a new data frame, reread parameters
		DATA_FRAME_END = 0x2, //!< end of the data frame
		DATA_FRAME_START_END = 0x3 //!< both start and end
	};

	//! the state of an active plugin
	struct ActivePlugin
	{
		ProcessingPlugin *plugin; //!< which pluging
		std::vector<unsigned> inputs; //!< which channel goes to which input
		std::vector<unsigned> outputs; //!< which output goes to which channel
	};

	//! A vector of all plugins 
	typedef std::vector<ActivePlugin> ActivePlugins;


signals:
	//! Emit data which should be displayed
	void dataReady(const std::valarray<signed short> &, unsigned, unsigned, unsigned, unsigned, unsigned);
	
public:
	DataConverter(DataSource *dataSource, unsigned channelCount, unsigned timescale);
	virtual ~DataConverter();
	
	// Parameters changes
	void setTimeScale(unsigned ms);
	void setTrigger(TriggerType type, bool timeout, unsigned channel, unsigned pos, signed short value);

	// Plugin changes
	void getPluginMapping(ActivePlugins *configuration, unsigned *channelCount) const;
	void setPluginMapping(const ActivePlugins &configuration, unsigned channelCount);
	
	// Parameters getters
	TriggerType triggerType() const { return _triggerType; } //!< Return the trigger type
	bool triggerTimeout() const { return _triggerTimeout; } //!< Return wetehr trigger timesout or not (i.e. is auto mode)
	unsigned triggerChannel() const { return _triggerChannel; } //!< Return the trigger channel
	signed short triggerValue() const { return _triggerValue; } //!< Return the trigger value
	unsigned triggerPos() const { return _triggerPos; } //!< Return the trigger position
	unsigned outputSampleCount() const { return _outputSampleCount; } //!< Return the number of output sample
	
	void run();

protected:
	void deleteActivePlugins(ActivePlugins *toDelete);
	
protected:
	bool quit; //!< false by default, if set to true stop converter thread
	unsigned samplingRate; //!< the sampling rate of the source
	DataSource *dataSource; //!< the data source
	
	ActivePlugins _plugins; //!< processing plugins
	unsigned _channelCount; //!< the number of channel
	bool pluginConfigurationChanged; //!< has setPluginMapping been called
	
	unsigned _outputSampleCount; //!< number of sample to output
	unsigned _outputTime; //!< the time of output in ms

	TriggerType _triggerType; //!< type of trigger
	bool _triggerTimeout; //!< does trigger timeout and is data taken anyway (i.e. is auto mode)
	unsigned _triggerChannel; //!< channel of which to trigger
	signed short _triggerValue; //!< value of the trigger
	unsigned _triggerPos; //!< position of trigger within output buffer
	
	mutable QMutex mutex; //!< mutex for protecting access from GUI
};

#endif
