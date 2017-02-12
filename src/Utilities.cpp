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

#include "Utilities.h"
#include <cmath>
#include <map>
#include <QSettings>
#include <cassert>
#include <valarray>
#include "Settings.h"

//! Produce a string containing timescale information in ms per division. duration is in ms per width. Result is per division
QString timeScaleToStringWidthToDiv(unsigned duration)
{
	QString timeScaleText;
	// 100 us to 1 ms per division
    if (qAbs(duration) < 10)
	{
		/* we use argument 1 to 4 instead of 1 to 2 because %10
		is wrongly assumed as being argument 10 instead of argument
		1 and a zero. This behaviour is not the one the doc says */
		timeScaleText = QString("%1%3%4 %2s").arg(duration).arg(QChar(0xB5, 0)).arg(0).arg(0);
	}
	// 1 ms to 1000 ms per division
    else if (qAbs(duration) < 10000)
	{
		timeScaleText = QString("%1 ms").arg(duration /10);
	}
	// 1 s per division
	else
	{
		timeScaleText = QString("%1 s").arg(duration /10000);
	}
	return timeScaleText;
}

//! Produce a string containing timescale information. duration is in ms
QString timeScaleToString(double duration)
{
	QString timeScaleText;
	// 100 us to 1 ms per division
	if (fabs(duration) < 1)
	{
		/* we use argument 1 to 4 instead of 1 to 2 because %10
		is wrongly assumed as being argument 10 instead of argument
		1 and a zero. This behaviour is not the one the doc says */
		timeScaleText = QString("%1 %2s").arg(round(duration * 100000) / 100).arg(QChar(0xB5, 0));
	}
	// 1 ms to 1000 ms per division
	else if (fabs(duration) < 1000)
	{
		timeScaleText = QString("%1 ms").arg(round(duration * 100) / 100);
	}
	// 1 s per division
	else
	{
		timeScaleText = QString("%1 s").arg(round(duration / 10) / 100);
	}
	return timeScaleText;
}

//! Produce a string containing y scale information. yDivisionFactor is in mV per height
QString YScaleToString(unsigned yDivisionFactor)
{
	QString yScaleText;
	// 100 uV to 1 mV per division
	if (yDivisionFactor < 10)
	{
		/* we use argument 1 to 4 instead of 1 to 2 because %10
		is wrongly assumed as being argument 10 instead of argument
		1 and a zero. This behaviour is not the one the doc says */
		yScaleText = QString("%1%3%4 %2V").arg(yDivisionFactor).arg(QChar(0xB5, 0)).arg(0).arg(0);
	}
	// 1 mV to 1000 mV per division
	else if (yDivisionFactor < 10000)
	{
		yScaleText = QString("%1 mV").arg(yDivisionFactor /10);
	}
	// 1 V per division
	else
	{
		yScaleText = QString("%1 V").arg(yDivisionFactor /10000);
	}
	return yScaleText;
}

//! Map of unsigned to QString
typedef std::map<unsigned, QString> UnsignedQStringMap;
//! Custom channel names
UnsignedQStringMap customChannelNames;
//! Number of channel in data source
unsigned dataSourceChannelCount;
//! Colors for the input channels 
std::valarray<QColor> colors;

//! Tell the channel renaming system about the number of channel in data source
void setDataSourceChannelCount(unsigned number)
{
	dataSourceChannelCount = number;
	colors.resize(number);
	
	for (unsigned i = 0; i < dataSourceChannelCount; i++)
		colors[i].setHsv((i * 360) / dataSourceChannelCount, 255, 200);
}

//! Transform a logic channel id (stored value, signed, extended channels are negative) into a physic channel id (index to memory, unsigned)
unsigned logicChannelIdToPhysic(int logic, bool *ok)
{
	if (logic >= (int)dataSourceChannelCount)
	{
		if (ok)
			*ok = false;
		return 0;
	}
	
	if (logic < 0)
		logic = ((int)dataSourceChannelCount) - logic - 1;
	
	if (ok)
		*ok = true;
	
	return (unsigned)logic;
}

//! Transform a physic channel id (index to memory, unsigned) into a logic channel id (stored value, signed, extended channels are negative)
int physicChannelIdToLogic(unsigned physic)
{
	int logic = (int)physic;
	if (physic >= dataSourceChannelCount)
		logic = -(int)(physic - dataSourceChannelCount) - 1;
	return logic;
}

//! Produce a string which is the user representation of a channel
QString channelNumberToString(unsigned channel)
{
	// try custom channel names first
	UnsignedQStringMap::const_iterator it = customChannelNames.find(channel);
	if (it != customChannelNames.end())
		return it->second;
	
	// then standard names
	if (channel < dataSourceChannelCount)
		return QString("S%1").arg(channel + 1);
	else
		return QString("E%1").arg(channel + 1 - dataSourceChannelCount);
}

//! Return the integer channel number from its string representation
unsigned channelNumberFromString(const QString &channel)
{
	// try custom channel names first
	for (UnsignedQStringMap::const_iterator it = customChannelNames.begin(); it != customChannelNames.end(); ++it)
	{
		if (it->second == channel)
			return it->first;
	}
	
	// then standard names
    if (channel.toLatin1().constData()[0] == 'S')
		return channel.mid(1).toUInt() - 1;
	else
		return channel.mid(1).toUInt() + dataSourceChannelCount - 1;
}

//! Set a custom name for a given channel
void setCustomChannelName(unsigned channel, const QString &name)
{
	// if already in, clear old name
	for (UnsignedQStringMap::iterator it = customChannelNames.begin(); it != customChannelNames.end(); ++it)
	{
		if (it->second == name)
			customChannelNames.erase(it);
	}
	// set new name or erase custom if null
	if (name == "")
		customChannelNames.erase(channel);
	else
		customChannelNames[channel] = name;
}

//! Clear the list of custom channel names
void clearCustomChannelNames(void)
{
	customChannelNames.clear();
}

//! Restore custom channel names from settings
void loadCustomChannelNames(void)
{
	QSettings settings(ORGANISATION_NAME, APPLICATION_NAME);
	int size = settings.beginReadArray("customChannelNames");
	for (int i = 0; i < size; i++)
	{
		settings.setArrayIndex(i);
		bool ok;
		unsigned id = logicChannelIdToPhysic(settings.value("id").toInt(), &ok);
		if (ok)
			customChannelNames[id] = settings.value("name").toString();
	}
	settings.endArray();
}

//! Save custom channel names to settings
void saveCustomChannelNames(void)
{
	QSettings settings(ORGANISATION_NAME, APPLICATION_NAME);
	settings.beginWriteArray("customChannelNames", customChannelNames.size());
	size_t i = 0;
	for (UnsignedQStringMap::const_iterator it = customChannelNames.begin(); it != customChannelNames.end(); ++it)
	{
		settings.setArrayIndex(i++);
		// source channels are stored positive and extended channels are stored negative minus 1
		settings.setValue("id", physicChannelIdToLogic(it->first));
		settings.setValue("name", it->second);
	}
	settings.endArray();
}

//! Return color of channel
QColor getChannelColor(unsigned channel)
{
	// we have predefined colors only for the input channels
	if (channel < dataSourceChannelCount)
		return colors[channel];
	else
		return Qt::darkGray;
}

unsigned scaleFactorsTable[] = {
		1,
		2,
		5,
		10,
		20,
		50,
		100,
		200,
		500,
		1000,
		2000,
		5000,
		10000,
		20000,
		50000
	}; // 15 values from 1 to 10000

//! Return a scale factor for a given index. 0 => 1, 1 => 2, 2 => 5, 3 => 10, ...
unsigned getScaleFactor(unsigned index)
{
	assert(index < ScaleFactorCount);
	return scaleFactorsTable[index];
}

//! Return the nearest index of a given scale factor. 0 => 1, 1 => 2, 2 => 5, 3 => 10, ...
unsigned getScaleFactorInvert(unsigned factor)
{
	unsigned nearestValue = 50000;
	unsigned nearestIndex = 0;
	for (size_t i = 0; i < ScaleFactorCount; i++)
	{
		unsigned dist = abs((int)factor - (int)scaleFactorsTable[i]);
		if (dist < nearestValue)
		{
			nearestValue = dist;
			nearestIndex = i;
		}
	}
	return nearestIndex;
}
