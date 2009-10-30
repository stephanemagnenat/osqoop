/*

Oscilloscope, a Qt based Oscilloscope with signal processing plugins
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
#include "Greater.h"
#include <Greater.moc>

QString ProcessingGreaterDescription::systemName() const
{
	return QString("Greater");
}

QString ProcessingGreaterDescription::name() const
{
	return QString("Greater");
}

QString ProcessingGreaterDescription::description() const
{
	return QString("If A > B return C else return D");
}

unsigned ProcessingGreaterDescription::inputCount() const
{
	return 4;
}

unsigned ProcessingGreaterDescription::outputCount() const
{
	return 1;
}

ProcessingPlugin *ProcessingGreaterDescription::create(const DataSource *dataSource) const
{
	return new ProcessingGreater(this);
}

void ProcessingGreater::processData(const std::valarray<signed short *> &inputs, const std::valarray<signed short *> &outputs, unsigned sampleCount)
{
	signed short *srcPtr0 = inputs[0];
	signed short *srcPtr1 = inputs[1];
    signed short *srcPtr2 = inputs[2];
    signed short *srcPtr3 = inputs[3];
	signed short *destPtr = outputs[0];
	for (unsigned sample = 0; sample < sampleCount; sample++)
    {
        if ((*srcPtr0++) > (*srcPtr1++))
		    *destPtr++ = *srcPtr2;
        else
            *destPtr++ = *srcPtr3;
        srcPtr2++;
        srcPtr3++;
    }
}

Q_EXPORT_PLUGIN(ProcessingGreaterDescription)
