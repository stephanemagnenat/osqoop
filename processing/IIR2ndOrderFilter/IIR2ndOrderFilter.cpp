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

#include <IIRFilter.h>
#include "IIR2ndOrderFilter.h"
#include <IIR2ndOrderFilter.moc>
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

QString IIR2ndOrderFilterDescription::systemName() const
{
	return QString("IIR2ndOrderFilter");
}

QString IIR2ndOrderFilterDescription::name() const
{
	return QString("IIR 2nd order filter");
}

QString IIR2ndOrderFilterDescription::description() const
{
	return QString("IIR 2nd order filter with user defined coefficient");
}

unsigned IIR2ndOrderFilterDescription::inputCount() const
{
	return 1;
}

unsigned IIR2ndOrderFilterDescription::outputCount() const
{
	return 1;
}

ProcessingPlugin *IIR2ndOrderFilterDescription::create(const DataSource *dataSource) const
{
	return new IIR2ndOrderFilter(this);
}


// ------------------------------------------------------------------------------------------------------------------------------

IIR2ndOrderFilter::IIR2ndOrderFilter(const ProcessingPluginDescription *description) :
	ProcessingPlugin(description)
{
	filter = new IIRFilter(2);
	std::fill(b, b+3, 0);
    std::fill(a, a+3, 0);
}

QWidget *IIR2ndOrderFilter::createGUI(void)
{
	QWidget *guiBase = new QWidget;
	QGridLayout *layout = new QGridLayout(guiBase);
    
    QDoubleSpinBox *bSpinBox[3];
    QDoubleSpinBox *aSpinBox[3];
    
    for (unsigned i = 0; i < 3; i++)
    {
        bSpinBox[i] = new QDoubleSpinBox;
        bSpinBox[i]->setRange(-1000, 1000);
        bSpinBox[i]->setSingleStep(0.01);
        bSpinBox[i]->setValue(b[i]);
    }
    connect(bSpinBox[0], SIGNAL(valueChanged(double)), SLOT(b0Changed(double)));
    connect(bSpinBox[1], SIGNAL(valueChanged(double)), SLOT(b1Changed(double)));
    connect(bSpinBox[2], SIGNAL(valueChanged(double)), SLOT(b2Changed(double)));
    
    for (unsigned i = 0; i < 2; i++)
    {
        aSpinBox[i] = new QDoubleSpinBox;
        aSpinBox[i]->setRange(-1000, 1000);
        aSpinBox[i]->setSingleStep(0.01);
        aSpinBox[i]->setValue(a[i+1]);
    }
    connect(aSpinBox[0], SIGNAL(valueChanged(double)), SLOT(a1Changed(double)));
    connect(aSpinBox[1], SIGNAL(valueChanged(double)), SLOT(a2Changed(double)));

    layout->addWidget(new QLabel("0"), 0, 1);
    layout->addWidget(new QLabel("1"), 0, 2);
    layout->addWidget(new QLabel("2"), 0, 3);
    layout->addWidget(new QLabel("b"), 1, 0);
    layout->addWidget(bSpinBox[0], 1, 1);
    layout->addWidget(bSpinBox[1], 1, 2);
    layout->addWidget(bSpinBox[2], 1, 3);
    layout->addWidget(new QLabel("a"), 2, 0);
    layout->addWidget(aSpinBox[0], 2, 2);
    layout->addWidget(aSpinBox[1], 2, 3);
	
	return guiBase;
}


IIR2ndOrderFilter::~IIR2ndOrderFilter()
{
	delete filter;
}

void IIR2ndOrderFilter::b0Changed(double value)
{
    b[0] = value;
    filter->setCoeffs(b, a);    
}

void IIR2ndOrderFilter::b1Changed(double value)
{
    b[1] = value;
    filter->setCoeffs(b, a);
}

void IIR2ndOrderFilter::b2Changed(double value)
{
    b[2] = value;
    filter->setCoeffs(b, a);    
}

void IIR2ndOrderFilter::a1Changed(double value)
{
    a[1] = value;
    filter->setCoeffs(b, a);
}

void IIR2ndOrderFilter::a2Changed(double value)
{
    a[2] = value;
    filter->setCoeffs(b, a);
}

void IIR2ndOrderFilter::processData(const std::valarray<signed short *> &inputs, const std::valarray<signed short *> &outputs, unsigned sampleCount)
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

void IIR2ndOrderFilter::load(QTextStream *stream)
{
    for (unsigned i = 0; i < 3; i++)
        (*stream) >> b[i];
	for (unsigned i = 0; i < 3; i++)
	    (*stream) >> a[i];
	filter->setCoeffs(b, a);
}

void IIR2ndOrderFilter::save(QTextStream *stream)
{
    for (unsigned i = 0; i < 3; i++)
        (*stream) << b[i] << " ";
    for (unsigned i = 0; i < 3; i++)
        (*stream) << a[i] << " ";
    filter->setCoeffs(b, a);
}

Q_EXPORT_PLUGIN(IIR2ndOrderFilterDescription)
