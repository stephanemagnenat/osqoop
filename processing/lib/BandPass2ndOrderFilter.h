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

#ifndef __BANDPASS_2ND_ORDER_H
#define __BANDPASS_2ND_ORDER_H

#include <QObject>
#include "IIRFilter.h"

//! 2nd order band-pass filter, allowing direct control on the cut-off frequency and bandwidth;
class BandPass2ndOrderFilter : public QObject, public IIRFilter
{
    Q_OBJECT
    
public:
	BandPass2ndOrderFilter(double samplingRate, double cutOffFreq, double bandWidth);

	double cutOffFreq() { return _cutOffFreq; } //!< Return the cut-off frequency of this filter
	double bandWidth() { return _bandWidth; } //!< Return the bandwidth of this filter
    
public slots:
    void setCutOffFreq(double);
	void setBandWidth(double);
	
private:
    double _samplingRate; //< sampling rate
	double _cutOffFreq; //!< cut-off frequency of this filter
	double _bandWidth; //!< bandwidth of this filter

	void updateCoeffs();
};

#endif
