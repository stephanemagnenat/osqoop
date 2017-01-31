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

#ifndef __PROCESSING_INTERFACE_H
#define __PROCESSING_INTERFACE_H

#include <valarray>
#include <QPluginLoader>

class ProcessingPluginDescription;
class DataSource;
class QWidget;
class QTextStream;

//! Interface to processing plugins
/*!
	This interface is the base of data processing classes.

	Such classes inherits from ProcessingPlugin
	and reimplement processData to do the actual processing.
	
	They must have protected constructors, and they must
	only be created by ProcessingPluginDescription
	subclasses through the create() method.

	Read the \ref ProcessingPluginsCookbook for more informations.
*/
class ProcessingPlugin
{
protected:
	friend class OscilloscopeWindow;
	const ProcessingPluginDescription *parent; //!< description of the plugin

	//! Construct the plugin from its description
	ProcessingPlugin(const ProcessingPluginDescription *description) { parent = description; }
	//! Virtual destructor, do nothing
	virtual ~ProcessingPlugin() { }

public:
	//! Return the description of the plugin
	const ProcessingPluginDescription *description() { return parent; }
	//! Return a GUI for this plugin
	virtual QWidget *createGUI(void) { return NULL; }
	//! Apply plugin to datas
	virtual void processData(const std::valarray<signed short *> &inputs, const std::valarray<signed short *> &outputs, unsigned sampleCount) = 0;
	//! Self delete, useful for reimplementation using QObject that would do a deleteLater instead of delete this
	virtual void terminate(void) { delete this; }
	//! Load instance specific data from text stream
	virtual void load(QTextStream *) { }
	//! Save instance specific data to text stream
	virtual void save(QTextStream *) { }
};


//! Interface to processing plugins descriptions
/*!
	This interface is the base of any processing plugin.

	Such plugin must inherits from this interface, and reimplement
	name(), inputCount() and outputCount() methods.
	
	This interface only describes the plugin. Actual data processing
	is done in a subclass of ProcessingPlugin, in the processData
	method. An instance of it must be returned by the
	create() method.
	
	Read the \ref ProcessingPluginsCookbook for more informations.
*/
class ProcessingPluginDescription
{
public:
	//! Virtual destructor, do nothing
	virtual ~ProcessingPluginDescription() { }
	//! Return the system name (without whitespace) of the plugin
	virtual QString systemName() const = 0;
	//! Return the human readable name of the plugin
	virtual QString name() const = 0;
	//! Return a description of the plugin
	virtual QString description() const = 0;
	//! Return the number of inputs of the plugin
	virtual unsigned inputCount() const = 0;
	//! Return the number of outputs of the plugin
	virtual unsigned outputCount() const = 0;
	//! Create an instance of the plugin
	virtual ProcessingPlugin *create(const DataSource *dataSource) const = 0;
};


Q_DECLARE_INTERFACE(ProcessingPluginDescription, "ch.eig.lsn.Oscilloscope.ProcessingPluginDescription/1.0")

#endif
