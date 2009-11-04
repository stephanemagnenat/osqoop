/*

Osqoop, an open source software oscilloscope.
Copyright (C) 2006 Lucas Tamarit <lucas dot tamarit at gmail dot com>
Laboratory of Signal Processing http://eig.unige.ch/~kocher/
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

#include "BandPass2ndOrderFilter.h"
#include <BandPass2ndOrderFilter.moc>
#include <cmath>

//! Constructor. Build a filter with a given initial cut-off frequency and bandwidth
BandPass2ndOrderFilter::BandPass2ndOrderFilter(double samplingRate, double cutOffFreq, double bandWidth) :
	IIRFilter(2),
    _samplingRate(samplingRate),
	_cutOffFreq(cutOffFreq),
	_bandWidth(bandWidth)
{
	updateCoeffs();
}

//! Update filter coefficient from cut-off frequency and bandwidth
void BandPass2ndOrderFilter::updateCoeffs()
{
	double c = ( tan(M_PI*_bandWidth)-1 ) / ( tan(M_PI*_bandWidth)+1 );
	double d = -cos( 2*M_PI*_cutOffFreq );

	// coefficients for inputs
	double b [3] = {(1.0+c)/2.0, 0.0, (-c-1.0)/2.0};
	// coefficients for recursive filtered value
	double a [3] = {1.0, d*(1.0-c),	-c};

	setCoeffs(b, a);
}

//! Set the cut-off frequency in Hz
void BandPass2ndOrderFilter::setCutOffFreq(double freq)
{
	_cutOffFreq = freq / _samplingRate;
	updateCoeffs();
}

//! Set the bandwidth in Hz
void BandPass2ndOrderFilter::setBandWidth(double freq)
{
	_bandWidth = freq / _samplingRate;
	updateCoeffs();
}
