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
#include "CropBelowLevel.h"
#include <CropBelowLevel.moc>

QString ProcessingCropBelowLevelDescription::systemName() const
{
	return QString("CropBelowLevel");
}

QString ProcessingCropBelowLevelDescription::name() const
{
	return QString("CropBelowLevel");
}

QString ProcessingCropBelowLevelDescription::description() const
{
	return QString("Crop below a certain level around 0");
}

unsigned ProcessingCropBelowLevelDescription::inputCount() const
{
	return 1;
}

unsigned ProcessingCropBelowLevelDescription::outputCount() const
{
	return 1;
}

ProcessingPlugin *ProcessingCropBelowLevelDescription::create(const DataSource *dataSource) const
{
	return new ProcessingCropBelowLevel(this);
}


ProcessingCropBelowLevel::ProcessingCropBelowLevel(const ProcessingPluginDescription *description) :
	ProcessingPlugin(description)
{
	level = 0;
}

//! called through signal/slot system when GUI changes the gain
void ProcessingCropBelowLevel::levelChanged(int value)
{
	level = value;
}

QWidget *ProcessingCropBelowLevel::createGUI(void)
{
	QSpinBox *spinBox = new QSpinBox;
	spinBox->setRange(0, 32767);
	spinBox->setValue(level);
	connect(spinBox, SIGNAL(valueChanged(int)), SLOT(levelChanged(int)));
	return spinBox;
}

void ProcessingCropBelowLevel::processData(const std::valarray<signed short *> &inputs, const std::valarray<signed short *> &outputs, unsigned sampleCount)
{
	signed short *srcPtr = inputs[0];
	signed short *destPtr = outputs[0];
	for (unsigned sample = 0; sample < sampleCount; sample++)
	{
		signed short value = *srcPtr++;
		if (value > level)
			*destPtr++ = value - level;
		else if (value < -level)
			*destPtr++ = value + level;
		else
			*destPtr++ = 0;
	}
}

void ProcessingCropBelowLevel::load(QTextStream *stream)
{
	(*stream) >> level;
}

void ProcessingCropBelowLevel::save(QTextStream *stream)
{
	(*stream) << level;
}

Q_EXPORT_PLUGIN(ProcessingCropBelowLevelDescription)
