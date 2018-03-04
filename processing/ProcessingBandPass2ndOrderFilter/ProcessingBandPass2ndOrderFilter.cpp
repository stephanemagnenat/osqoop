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

#include <BandPass2ndOrderFilter.h>
#include "ProcessingBandPass2ndOrderFilter.h"
#include <DataSource.h>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QTextStream>
#include <QtPlugin>

/*
//! Return the string describing frequency f
QString freqToString(double f)
{
	if (f > 1000)
		return QString("%0 kHz").arg(round(f / 10) / 100);
	else
		return QString("%0 Hz").arg(round(f * 100) / 100);
}*/

QString ProcessingBandPass2ndOrderFilterDescription::systemName() const
{
	return QString("BandPass2ndOrderFilter");
}

QString ProcessingBandPass2ndOrderFilterDescription::name() const
{
	return QString("Band-pass 2nd order filter");
}

QString ProcessingBandPass2ndOrderFilterDescription::description() const
{
	return QString("Band-pass 2nd order filter with user defined cutoff frequency and bandwidth");
}

unsigned ProcessingBandPass2ndOrderFilterDescription::inputCount() const
{
	return 1;
}

unsigned ProcessingBandPass2ndOrderFilterDescription::outputCount() const
{
	return 1;
}

ProcessingPlugin *ProcessingBandPass2ndOrderFilterDescription::create(const DataSource *dataSource) const
{
	return new ProcessingBandPass2ndOrderFilter(this, dataSource);
}


// ------------------------------------------------------------------------------------------------------------------------------

ProcessingBandPass2ndOrderFilter::ProcessingBandPass2ndOrderFilter(const ProcessingPluginDescription *description, const DataSource *dataSource) :
	ProcessingPlugin(description),
    dataSource(dataSource)
{
	filter = new BandPass2ndOrderFilter((double)dataSource->samplingRate(), 0.1, 0.1);
}

QWidget *ProcessingBandPass2ndOrderFilter::createGUI(void)
{
	QWidget *guiBase = new QWidget;
	QGridLayout *layout = new QGridLayout(guiBase);

    QDoubleSpinBox *sbCenterFreq = new QDoubleSpinBox;
    sbCenterFreq->setMinimum(0);
    sbCenterFreq->setMaximum((double)dataSource->samplingRate() / 2);
    sbCenterFreq->setValue(filter->cutOffFreq());
    sbCenterFreq->setSuffix(" Hz");
    connect(sbCenterFreq, SIGNAL(valueChanged(double)), filter, SLOT(setCutOffFreq(double)));

	QDoubleSpinBox *sbBandWidth = new QDoubleSpinBox;
	sbBandWidth->setMinimum(0);
	sbBandWidth->setMaximum((double)dataSource->samplingRate() / 2);
	sbBandWidth->setValue(filter->bandWidth());
    sbBandWidth->setSuffix(" Hz");
    connect(sbBandWidth, SIGNAL(valueChanged(double)), filter, SLOT(setBandWidth(double)));

    layout->addWidget(new QLabel(tr("Center Frequency")), 0, 0);
	layout->addWidget(sbCenterFreq, 0, 1);
	layout->addWidget(new QLabel(tr("Band Width")), 1, 0);
	layout->addWidget(sbBandWidth, 1, 1);
	
	return guiBase;
}


ProcessingBandPass2ndOrderFilter::~ProcessingBandPass2ndOrderFilter()
{
	delete filter;
}

void ProcessingBandPass2ndOrderFilter::processData(const std::valarray<signed short *> &inputs, const std::valarray<signed short *> &outputs, unsigned sampleCount)
{
	signed short *srcPtr0 = inputs[0];
	signed short *destPtr0 = outputs[0];

	filter->lock();
	for (unsigned sample = 0; sample < sampleCount; sample++)
	{
		double sourceValue = (double)(*srcPtr0);

		double filteredValue = filter->getNext(sourceValue);
		*destPtr0 = (signed short)(filteredValue);

		srcPtr0++;
		destPtr0++;
	}
	filter->unlock();
}

void ProcessingBandPass2ndOrderFilter::load(QTextStream *stream)
{
	double value;

	(*stream) >> value;
	filter->setCutOffFreq(value);
	(*stream) >> value;
	filter->setBandWidth(value);
}

void ProcessingBandPass2ndOrderFilter::save(QTextStream *stream)
{
	(*stream) << filter->cutOffFreq() << " " << filter->bandWidth();
}

