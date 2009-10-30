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

#ifndef __SIGNAL_DISPLAY_DATA
#define __SIGNAL_DISPLAY_DATA

#include <QString>
#include <valarray>

class QMenu;
class DataConverter;

//! Information struct for signal display.
/*! Contains the data themselves and some other informations (such as
	the number of channel,...) as well as shared GUI objects (such as
	the timeScaleMenu).
	Used to share signal between multiple widgets.
	Helper methods also do timescale-related computations.
*/
struct SignalDisplayData
{
	unsigned duration; //!< duration of data in millisecond
	unsigned channelCount; //!< number of channel
	unsigned incrementalPos; //!< position of new data in case of incremental acquisition
	unsigned samplePerChannelCount; //!< number of sample per channel
	DataConverter *dataConverter; //!< data converter, to get trigger information
	std::valarray<signed short> data; //!< the data (local copy is required when resizing)
	
	unsigned sampleCount(void) const;
	void channelAmplitude(unsigned channel, int *mean, int *maxAmplitudeDC, int *maxAmplitudeAC) const;
	int clipSamplePos(int pos) const;
	unsigned timeToSample(unsigned time) const;
	double timeToSample(double time) const;
	unsigned sampleToTime(unsigned sample) const;
	double sampleToTime(double sample) const;
	QString sampleToTimeString(int sample) const;
	QString sampleToTimeString(double sample) const;
	QString sampleToFreqString(int sample) const;
};
#endif
