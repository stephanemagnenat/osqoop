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

#ifndef __DATA_SOURCE_INTERFACE_H
#define __DATA_SOURCE_INTERFACE_H

#include <valarray>
#include <QPluginLoader>

class DataSourceDescription;

//! Interface to data sources.
/*!
	This interface is the base of data source classes.

	Such classes inherits from DataSource
	and reimplement getRawData to get the actual data.
	
	They must have protected constructors, and they must
	only be created by DataSourceDescription
	subclasses through the create() method.

	The data pointer passed to getRawData is a std::valarray of size
	description->inputCount() whose elements are std::valarray
	of 512 signed short each.

	Read the \ref DataSourceCookbook for more informations.
*/
class DataSource
{
protected:
	const DataSourceDescription *parent; //!< description of the plugin

	//! Construct the plugin from its description
	DataSource(const DataSourceDescription *description) { parent = description; }

public:
	//! Virtual destructor, do nothing
	virtual ~DataSource() { }
	//! Return this data source's description
	const DataSourceDescription *description() const { return parent; }	
	
	//! Try to initialize the source, return false if any problem arise
	virtual bool init() = 0;
	//! Read the raw data from source. Return the number of microsecond the data converter should sleep. If 0, do not sleep
	virtual unsigned getRawData(std::valarray<std::valarray<signed short> > *data) = 0;
	
	//! Return the number of inputs of the data source
	virtual unsigned inputCount() const = 0;
	//! Return the sample rate of the data source
	virtual unsigned samplingRate() const = 0;
    //! Return the number of unit per Volt of this source
    virtual unsigned unitPerVoltCount() const = 0;
};

//! Interface to data sources descriptions.
/*!
	This interface is the base of any data source plugin.

	Such plugin must inherits from this interface, and reimplement
	name(), descriptions() and create() methods.
	
	This interface only describes the plugin. Actual data acquisition
	is done in a subclass of DataSource, in the getRawData
	method. An instance of it must be returned by the
	create() method.
	
	Read the \ref DataSourceCookbook for more informations.
*/
class DataSourceDescription
{
public:
	//! Virtual destructor, do nothing
	virtual ~DataSourceDescription() { }
	
	//! Return the name of the data source
	virtual QString name() const = 0;
	//! Return a description of the data source
	virtual QString description() const = 0;
	
	//! Create an instance of the data source
	virtual DataSource *create() const = 0;
};

Q_DECLARE_INTERFACE(DataSourceDescription, "ch.eig.lsn.Oscilloscope.DataSourceDescription/1.0")

#endif
