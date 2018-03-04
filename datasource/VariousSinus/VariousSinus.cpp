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
#include "VariousSinus.h"

QString VariousSinusDataSourceDescription::name() const
{
	return "Various sinus generator";
}

QString VariousSinusDataSourceDescription::description() const
{
	return "A simple sinus generator of various frequencies";
}

DataSource *VariousSinusDataSourceDescription::create() const
{
	return new VariousSinusDataSource(this);
}


VariousSinusDataSource::VariousSinusDataSource(const DataSourceDescription *description) :
	DataSource(description)
{
	t = 0;
}

unsigned VariousSinusDataSource::getRawData(std::valarray<std::valarray<signed short> > *data)
{
	Q_ASSERT(data->size() >= 8);

	for (size_t sample = 0; sample < 512; sample++)
	{
		for (size_t channel = 0; channel < 8; channel++)
		{
			(*data)[channel][sample] = (signed short)(1000.0 * sin( (t * 2 * M_PI) / (100 * (double)(channel + 1)) ));
		}
		t++;
	}
	return 1000000 / (10000 / 512);
}

unsigned VariousSinusDataSource::inputCount() const
{
	return 8;
}

unsigned VariousSinusDataSource::samplingRate() const
{
	return 10000;
}

unsigned VariousSinusDataSource::unitPerVoltCount() const
{
    return 1000;
}
