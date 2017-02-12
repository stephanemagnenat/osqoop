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

#ifndef __VARIOUS_SINUS_H
#define __VARIOUS_SINUS_H

#include <DataSource.h>

//! Description of VariousSinus, a simple sinus generator of various frequencies
class VariousSinusDataSourceDescription : public QObject, public DataSourceDescription
{
	Q_OBJECT
    Q_PLUGIN_METADATA(IID "ch.eig.lsn.Oscilloscope.DataSourceDescription/1.0" FILE "varioussinus.json")
    Q_INTERFACES(DataSourceDescription)

public:
	virtual QString name() const;
	virtual QString description() const;
	
	virtual DataSource *create() const;
};

//! VariousSinus, a simple sinus generator of various frequencies
class VariousSinusDataSource : public DataSource
{
private:
	friend class VariousSinusDataSourceDescription;
	VariousSinusDataSource(const DataSourceDescription *description);
	double t; //!< time

public:
	virtual unsigned getRawData(std::valarray<std::valarray<signed short> > *data);
	virtual bool init(void) { return true; }
	
	virtual unsigned inputCount() const;
	virtual unsigned samplingRate() const;
    virtual unsigned unitPerVoltCount() const;
};


#endif
