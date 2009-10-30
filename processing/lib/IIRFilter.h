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

#ifndef __IIR_FILTER_H
#define __IIR_FILTER_H

#include <valarray>
#include <QMutex>

//! Implement a iir filter with a given kernel. This filter is thread-safe
class IIRFilter
{
private:
	std::valarray<double> b; //!< coefficients for x
	std::valarray<double> a; //!< coefficients for y
	std::valarray<double> x; //!< memory of input samples
	std::valarray<double> y; //!< memory of filter output
	unsigned order; //!< order of the filter
	QMutex mutex; //!< mutex for changing filter's parameter while running

private:
	double dotProduct(const std::valarray<double> &a, const std::valarray<double> &b);

public:
	IIRFilter(unsigned order, const double *coeffB = NULL, const double *coeffA = NULL);
	void setCoeffs(const double *coeffB, const double *coeffA);
	double getNext(double value);
	void lock(void);
	void unlock(void);
};

#endif
