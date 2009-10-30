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

#include <QtCore>
#include <QSpinBox>
#include "Delay.h"
#include <Delay.moc>

QString ProcessingDelayDescription::systemName() const
{
	return QString("Delay");
}

QString ProcessingDelayDescription::name() const
{
	return QString("Delay");
}

QString ProcessingDelayDescription::description() const
{
	return QString("Echo the input to the output after a delay");
}

unsigned ProcessingDelayDescription::inputCount() const
{
	return 1;
}

unsigned ProcessingDelayDescription::outputCount() const
{
	return 1;
}

ProcessingPlugin *ProcessingDelayDescription::create(const DataSource *dataSource) const
{
	return new ProcessingDelay(this);
}


ProcessingDelay::ProcessingDelay(const ProcessingPluginDescription *description) :
	ProcessingPlugin(description)
{
	delay = 0;
    delayBufferPos = 0;
}

//! called through signal/slot system when GUI changes the Delay
void ProcessingDelay::delayChanged(int value)
{
	delay = (unsigned)value;
}

QWidget *ProcessingDelay::createGUI(void)
{
	QSpinBox *spinBox = new QSpinBox;
	spinBox->setRange(0, 1000000);
	spinBox->setValue(delay);
	connect(spinBox, SIGNAL(valueChanged(int)), SLOT(delayChanged(int)));
	return spinBox;
}

void ProcessingDelay::processData(const std::valarray<signed short *> &inputs, const std::valarray<signed short *> &outputs, unsigned sampleCount)
{
	signed short *srcPtr = inputs[0];
	signed short *destPtr = outputs[0];
    
    if (delay != delayBuffer.size())
    {
        delayBuffer.resize(delay, 0);
        delayBufferPos = 0;
    }
    
    if (delayBuffer.size() != 0)
        for (unsigned sample = 0; sample < sampleCount; sample++)
        {
            *destPtr++ = delayBuffer[delayBufferPos];
            delayBuffer[delayBufferPos] = *srcPtr++;
            delayBufferPos = (delayBufferPos + 1) % delayBuffer.size();
        }
    else
        for (unsigned sample = 0; sample < sampleCount; sample++)
            *destPtr++ = *srcPtr++;
}

void ProcessingDelay::load(QTextStream *stream)
{
	(*stream) >> delay;
}

void ProcessingDelay::save(QTextStream *stream)
{
	(*stream) << delay;
}

Q_EXPORT_PLUGIN(ProcessingDelayDescription)
