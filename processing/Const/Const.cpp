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
#include <QDoubleSpinBox>
#include <QLabel>
#include <QHBoxLayout>
#include <DataSource.h>
#include "Const.h"
#include <Const.moc>

QString ProcessingConstDescription::systemName() const
{
	return QString("Const");
}

QString ProcessingConstDescription::name() const
{
	return QString("Const");
}

QString ProcessingConstDescription::description() const
{
	return QString("Generate a constant output");
}

unsigned ProcessingConstDescription::inputCount() const
{
	return 0;
}

unsigned ProcessingConstDescription::outputCount() const
{
	return 1;
}

ProcessingPlugin *ProcessingConstDescription::create(const DataSource *dataSource) const
{
	return new ProcessingConst(this, dataSource);
}


ProcessingConst::ProcessingConst(const ProcessingPluginDescription *description, const DataSource *dataSource) :
	ProcessingPlugin(description),
    dataSource(dataSource)
{
	value = 0;
}

//! called through signal/slot system when GUI changes the value
void ProcessingConst::ConstRawChanged(int _value)
{
	value = _value;
    voltSpinBox->setValue(value / (double)dataSource->unitPerVoltCount());
}

void ProcessingConst::ConstVoltChanged(double _value)
{
    value = (int)(_value * (double)dataSource->unitPerVoltCount());
    rawSpinBox->setValue(value);
}

QWidget *ProcessingConst::createGUI(void)
{
    QWidget *guiBase = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(guiBase);

	rawSpinBox = new QSpinBox;
	rawSpinBox->setRange(-32768, 32767);
	rawSpinBox->setValue(value);
    
    voltSpinBox = new QDoubleSpinBox;
	voltSpinBox->setRange(-32768 / (double)dataSource->unitPerVoltCount(), 32767 / (double)dataSource->unitPerVoltCount());
	voltSpinBox->setValue(value / (double)dataSource->unitPerVoltCount());
    voltSpinBox->setSuffix(" V");
	
    connect(rawSpinBox, SIGNAL(valueChanged(int)), SLOT(ConstRawChanged(int)));
    connect(voltSpinBox, SIGNAL(valueChanged(double)), SLOT(ConstVoltChanged(double)));
	
    layout->addWidget(rawSpinBox);
    layout->addWidget(new QLabel(tr(" : ")));
    layout->addWidget(voltSpinBox);
    
    return guiBase;
}

void ProcessingConst::processData(const std::valarray<signed short *> &inputs, const std::valarray<signed short *> &outputs, unsigned sampleCount)
{
	signed short *destPtr = outputs[0];
	for (unsigned sample = 0; sample < sampleCount; sample++)
		*destPtr++ = value;
}

void ProcessingConst::load(QTextStream *stream)
{
	(*stream) >> value;
}

void ProcessingConst::save(QTextStream *stream)
{
	(*stream) << value;
}

Q_EXPORT_PLUGIN(ProcessingConstDescription)
