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

#include "IIRFilter.h"
#include <QDebug>
#include <QMutexLocker>
#include <cassert>

//! Constructor, for a given order and coefficients
IIRFilter::IIRFilter(unsigned order, const double *coeffB, const double *coeffA) :
		b(order+1),
		a(order+1),
		x(order+1),
		y(order+1)
{
	if (coeffA)
		std::copy(coeffA, coeffA+order+1, &a[0]);
	else
		std::fill(&a[0], &a[a.size()], 0);
	if (coeffB)
		std::copy(coeffB, coeffB+order+1, &b[0]);
	else
		std::fill(&b[0], &b[b.size()], 0);
	std::fill(&y[0], &y[y.size()], 0);
	std::fill(&x[0], &x[x.size()], 0);
	this->order = order;
}

//! Set the coefficients of the filter. Note that <coeffB> and <coeffA> must point to array of length <order>+1
void IIRFilter::setCoeffs(const double *coeffB, const double *coeffA)
{
	QMutexLocker locker(&mutex);
	std::copy(coeffA, coeffA+order+1, &a[0]);
	std::copy(coeffB, coeffB+order+1, &b[0]);
	std::fill(&y[0], &y[y.size()], 0);
	std::fill(&x[0], &x[x.size()], 0);
}

//! Make the dot product of a and b
double IIRFilter::dotProduct(const std::valarray<double> &a, const std::valarray<double> &b)
{
	double result = 0;
	assert(a.size() == b.size());
	for (size_t i = 0; i < a.size(); i++)
	{
		result += a[i]*b[i];
	}
	return result;
}	

//! Return the next filtered value given the new raw value
double IIRFilter::getNext(double value)
{
	// shift x and y
	for (unsigned i = order; i>0; i--)
	{
		x[i] = x[i-1];
		y[i] = y[i-1];
	}
/*
	qDebug() << "Bs " << b[0] << " " << b[1] << " " << b[2];
	qDebug() << "As " << a[0] << " " << a[1] << " " << a[2];
*/
	x[0] = value;
	y[0] = 0;
	double result = dotProduct(b, x) - dotProduct(a, y);
	y[0] = result;

	return result;
}

//! Lock the filter parameter changes
void IIRFilter::lock(void)
{
	mutex.lock();
}

//! Unlock the filter parameter changes
void IIRFilter::unlock(void)
{
	mutex.unlock();
}
