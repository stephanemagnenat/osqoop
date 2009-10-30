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
#include <QDoubleSpinBox>
#include "Pow.h"
#include <Pow.moc>

QString ProcessingPowDescription::systemName() const
{
	return QString("Pow");
}

QString ProcessingPowDescription::name() const
{
	return QString("Power");
}

QString ProcessingPowDescription::description() const
{
	return QString("Elevate the signal to a specific power");
}

unsigned ProcessingPowDescription::inputCount() const
{
	return 1;
}

unsigned ProcessingPowDescription::outputCount() const
{
	return 1;
}

ProcessingPlugin *ProcessingPowDescription::create(const DataSource *dataSource) const
{
	return new ProcessingPow(this);
}


ProcessingPow::ProcessingPow(const ProcessingPluginDescription *description) :
	ProcessingPlugin(description)
{
	exponent = 1;
}

//! called through signal/slot system when GUI changes the exponent
void ProcessingPow::exponentChanged(double value)
{
	exponent = (float)value;
}

QWidget *ProcessingPow::createGUI(void)
{
	QDoubleSpinBox *spinBox = new QDoubleSpinBox;
	spinBox->setRange(0.0, 10.0);
	spinBox->setValue(exponent);
	connect(spinBox, SIGNAL(valueChanged(double)), SLOT(exponentChanged(double)));
	return spinBox;
}

void ProcessingPow::processData(const std::valarray<signed short *> &inputs, const std::valarray<signed short *> &outputs, unsigned sampleCount)
{
	signed short *srcPtr = inputs[0];
	signed short *destPtr = outputs[0];
	for (unsigned sample = 0; sample < sampleCount; sample++)
    {
        float fVal = (*srcPtr++);
		*destPtr++ = (short)powf(fVal, exponent);
    }
}

void ProcessingPow::load(QTextStream *stream)
{
	(*stream) >> exponent;
}

void ProcessingPow::save(QTextStream *stream)
{
	(*stream) << exponent;
}

Q_EXPORT_PLUGIN(ProcessingPowDescription)
