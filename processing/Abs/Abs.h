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

#ifndef __PROCESSING_Negate
#define __PROCESSING_Negate

#include <ProcessingPlugin.h>

//! Description of the abs plugin
class ProcessingAbsDescription : public QObject, public ProcessingPluginDescription
{
	Q_OBJECT
    Q_PLUGIN_METADATA(IID "ch.eig.lsn.Oscilloscope.ProcessingPluginDescription/1.0" FILE "abs.json")
    Q_INTERFACES(ProcessingPluginDescription)

public:
	QString systemName() const;
	QString name() const;
	QString description() const;
	unsigned inputCount() const;
	unsigned outputCount() const;
	ProcessingPlugin *create(const DataSource *dataSource) const;
};

//! Abs plugin. Take the absolute value of the signal
class ProcessingAbs : public ProcessingPlugin
{
private:
	friend class ProcessingAbsDescription;
	ProcessingAbs(const ProcessingPluginDescription *description) : ProcessingPlugin(description) { }

public:
	void processData(const std::valarray<signed short *> &inputs, const std::valarray<signed short *> &outputs, unsigned sampleCount);
};

#endif
