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

#include "SignalDisplayData.h"
#include "Utilities.h"

//! Return the number of sample per channel
unsigned SignalDisplayData::sampleCount(void) const
{
	return data.size() / channelCount;
}

//! Compute the mean, maximum amplitude in DC, maximum amplitude in AC (mean substracted). NULL can be passed if value has to be ignored
void SignalDisplayData::channelAmplitude(unsigned channel, int *mean, int *maxAmplitudeDC, int *maxAmplitudeAC) const
{
	Q_ASSERT(channel < channelCount);

	const signed short *channelSamples = &data[channel * sampleCount()];
	int minValue, maxValue;
	int meanValue = 0;

	minValue = maxValue = *channelSamples++;
	for (unsigned i = 1; i < sampleCount(); i++)
	{
		int v = *channelSamples++;
		minValue = std::min(v, minValue);
		maxValue = std::max(v, maxValue);
		meanValue += v;
	}
	meanValue /= sampleCount();
	if (mean)
		*mean = meanValue;
	if (maxAmplitudeDC)
		*maxAmplitudeDC = std::max(maxValue, abs(minValue));
	if (maxAmplitudeAC)
		*maxAmplitudeAC = std::max(maxValue - meanValue, abs(minValue - meanValue));
}

//! Clip a sample position against the number of sample
int SignalDisplayData::clipSamplePos(int pos) const
{
	int s = (int)sampleCount();
	if (pos < 0)
		return 0;
	else if (pos >= s)
		return s - 1;
	else
		return pos;
}

//! Return a duration in sample for the given time in ms. Integer version
unsigned SignalDisplayData::timeToSample(unsigned time) const
{
	return (time * sampleCount()) / duration;
}

//! Return a duration in sample for the given time in ms. Floating point version
double SignalDisplayData::timeToSample(double time) const
{
	return (time * (double)sampleCount()) / (double)duration;
}

//! Return a duration in ms for a given sample value. Integer version
unsigned SignalDisplayData::sampleToTime(unsigned sample) const
{
	return (sample * duration) / sampleCount();
}

//! Return a duration in ms for a given sample value. Floating point version
double SignalDisplayData::sampleToTime(double sample) const
{
	return (sample * (double)duration) / (double)sampleCount();
}

//! Return a string indicating the duration of sample. Integer version
QString SignalDisplayData::sampleToTimeString(int sample) const
{
	return timeScaleToString(sampleToTime((unsigned)abs(sample)));
}

//! Return a string indicating the duration of sample. Floating point version
QString SignalDisplayData::sampleToTimeString(double sample) const
{
	return timeScaleToString(sampleToTime(fabs(sample)));
}

//! Return a string indicating the frequency in Hz
QString SignalDisplayData::sampleToFreqString(int sample) const
{
	double duration_ms = sampleToTime((unsigned)abs(sample)); //((double)abs(sample) * (double)duration) / (double)sampleCount();
	double f = 1000 / (duration_ms);
	if (f > 1000)
		return QString("%0 kHz").arg(round(f / 10) / 100);
	else
		return QString("%0 Hz").arg(round(f * 100) / 100);
}
