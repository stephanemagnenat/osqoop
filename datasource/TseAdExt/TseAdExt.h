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

#ifndef __TSEADEXT_H
#define __TSEADEXT_H

#include <DataSource.h>
#include <QString>
#include <QThread>
#include <QSemaphore>

class USBDevice;

//! Constants for TseAdExtDataSource
namespace TseAdExtDataSourceConstants
{
	const size_t DataChunkSize = 512*8; //!< the number of short int in a chunk
	const size_t ChunkBufferSize = 32; //!< the number of chunk in the circular buffer
}

//! TseAdExt, an USB 2 based, 8 channel, 14 bits, ~ 25 KHz data source.
/*!
	This data source takes data from a custom acquisition board developed
	at LSN for the TSE (Traitement de Signaux Electromyographiques) project.
	It is distributed with Osqoop as an example of external data source.

	The dataflow from the acquisition board to the plugin is illustrated below:
	\image html tseadext-timing.png

	Firmware is based on the Cypress test firmware by reimplementing the periph.c
	file. Sources are available in the fwsrc directory.
	Please keep in mind that they are not release quality and are only provided
	as an example.
	To compile the firmware, you'll need Cypress CY3681 EZ-USB FX2 Development Kit.
	Please search the Cypress web site for it (http://www.cypress.com). 

	Under Windows, this data source automatically loads the firmware.
	Under Linux, for security reason, you need to load the firmware while
	in root using the setupfx2 program. If after this, you still have permission
	problems, make sure that /dev/bus/usb/BUS/DEV is writable by the user executing
	Osqoop.
 */
class TseAdExtDataSource : public DataSource,  public QThread
{
public:
	TseAdExtDataSource(const DataSourceDescription *description, unsigned sliceLength);
	~TseAdExtDataSource();
	
	virtual unsigned getRawData(std::valarray<std::valarray<signed short> > *data);
	virtual bool init();
	
	virtual void run();
	
	virtual unsigned inputCount() const { return 8; }
	virtual unsigned samplingRate() const { return 24500 / sliceLength; }
	virtual unsigned unitPerVoltCount() const { return 1638; }

protected:
	//! An element of the circular buffer containing datas
	struct DataChunk
	{
		signed short data[TseAdExtDataSourceConstants::DataChunkSize]; //!< the datas
	};

	USBDevice *device; //!< USB device
	bool quit; //!< true if USB thread must stop
	unsigned chunk; //!< next chunk to be consumed by getRawData
	QSemaphore freeChunks; //!< Semaphore that counts the number of free chunks in the buffer
	QSemaphore usedChunks; //!< Semaphore that counts the number of eaten chunks in the buffer
	DataChunk chunkBuffer[TseAdExtDataSourceConstants::ChunkBufferSize]; //!< Circular buffer, protected by freeChunks and usedChunks semaphore, for data transmission between the USB thread and the converter thread for the TSEADX data source
	unsigned sliceLength; //!< every which number of source sample one is copied into destination. If 1, all sample are taken
};

//! Description of TseAdExt, an USB 2 based, 8 channel, 14 bits, ~ 25 KHz data source. 
class TseAdExtDataSourceDescription : public QObject, public DataSourceDescription
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "ch.eig.lsn.Oscilloscope.DataSourceDescription/1.0" FILE "tseadext.json")
	Q_INTERFACES(DataSourceDescription)

public:
	virtual QString name() const { return "TseAdExt acquisition board"; }
	virtual QString description() const { return "TseAdExt, an USB 2 based, 8 channel, 14 bits, ~ 25 KHz data source developed at LSN/EIG"; }
	virtual DataSource *create() const;
};


#endif
/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
