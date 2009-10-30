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

#include <algorithm>
#include <QtCore>
#include <QString>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialog>
#include "TseAdExt.h"
#include <TseAdExt.moc>
#include "CypressEzUSBDevice.h"

//! Dialog box for choosing datasource frequency
class FrequencyDialog : public QDialog
{
public:
	QListWidget *frequenciesList; //!< the list holding available frequencies

	//! Creates the widgets, especially the list of available frequencies
	FrequencyDialog()
	{
		QVBoxLayout *layout = new QVBoxLayout(this);

		layout->addWidget(new QLabel(tr("Please select the desired frequency in Hz")));

		frequenciesList = new QListWidget();
		for (unsigned i = 1; i <= 10; i++)
			frequenciesList->addItem(QString("%1").arg(24500 / i));
		connect(frequenciesList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), SLOT(accept()));
		layout->addWidget(frequenciesList);
		
		setWindowTitle("TseAdExt data source");
	}
};


DataSource *TseAdExtDataSourceDescription::create() const
{
	unsigned sliceLength = 1;
	
	FrequencyDialog frequenciesDialog;
	int ret = frequenciesDialog.exec();
	if (ret != QDialog::Rejected)
		sliceLength = frequenciesDialog.frequenciesList->currentRow() + 1;
	return new TseAdExtDataSource(this, sliceLength);
}


TseAdExtDataSource::TseAdExtDataSource(const DataSourceDescription *description, unsigned sliceLength) :
	DataSource(description),
	freeChunks(TseAdExtDataSourceConstants::ChunkBufferSize),
	usedChunks(0),
	sliceLength(sliceLength)
{
	chunk = 0;
	quit = false;
	device = NULL;
}

TseAdExtDataSource::~TseAdExtDataSource()
{
	if (device)
	{
		quit = true;
		wait();
		device->close();
		delete device;
	}
}

unsigned TseAdExtDataSource::getRawData(std::valarray<std::valarray<signed short> > *data)
{
	Q_ASSERT(data->size() >= 8);
	size_t destSample = 0;

	for (unsigned block = 0; block < sliceLength; block++)
	{
		// get data
		usedChunks.acquire();
	
		// extend sign and linearise datas
		for (size_t sample = 0; sample < 512;)
		{
			for (size_t channel = 0; channel < 8; channel++)
			{
				unsigned short raw = (unsigned short)chunkBuffer[chunk % TseAdExtDataSourceConstants::ChunkBufferSize].data[(sample * 8 + channel)];
	
				// extend sign from 14th bit to 16th bit: this is because our converter is only 14 bits
				if (raw & 0x2000)
					raw |= 0xc000;
	
				signed short value = (signed short)raw;
				(*data)[channel][destSample] = value;
			}
			sample += sliceLength;
			destSample++;
		}
	
		// next chunk
		chunk++;
		
		// release data
		freeChunks.release();
	}

	return 0;
}

bool TseAdExtDataSource::init()
{
	#if defined(Q_OS_UNIX)
	const unsigned COMMAND_PIPE = 1;
	#endif
	// for unknown reason, win32 cypress drivers want different ID that do not correspond to the reality
	#if defined(Q_OS_WIN32)
	const unsigned COMMAND_PIPE = 0;
	#endif
	
	bool result;

	// create device
	device = new CypressEzUSBDevice();
	Q_ASSERT(device);

	// open device and download firmware
	CypressEzUSBDevice usbDevice;
	#if defined(Q_OS_WIN32)
	result = device->open(":fw.bix");
	#endif
	#if defined(Q_OS_UNIX)
	result = device->open("tseadextfw.hex");
	#endif
	Q_ASSERT(result);
	if (!result)
	{
		qDebug() << "TseAdExtDataSource::init() : can't open device";
		return false;
	}

	// select interface
	result = device->setInterface(0, 1);
	Q_ASSERT(result);
	if (!result)
	{
		qDebug() << "TseAdExtDataSource::init() : can't set interface";
		return false;
	}

	// send command for continuous reading
	char command = 0;
	result = device->bulkWrite(COMMAND_PIPE, &command, 1);
	Q_ASSERT(result);
	if (!result)
	{
		qDebug() << "TseAdExtDataSource::init() : can't send command";
		return false;
	}

	start(QThread::TimeCriticalPriority);
	//emit(statusUpdated(tr("TSEADEXT acquisition board operational")));
	return true;
}

//! USB thread: continuously poll data from USB and fill them into circular buffer
void TseAdExtDataSource::run()
{
	#if defined(Q_OS_UNIX)
	const unsigned READ_PIPE = 6;
	#endif
	// for unknown reason, win32 cypress drivers want different ID that do not correspond to the reality
	#if defined(Q_OS_WIN32)
	const unsigned READ_PIPE = 4;
	#endif

	size_t sourceChunk = 0;
	while (!quit)
	{
		if (!freeChunks.tryAcquire())
		{
			//emit(statusUpdated(tr("Acquisition warning : no more free chunk, risk of loosing input data")));
			freeChunks.acquire();
		}
		
		bool result = device->bulkRead(READ_PIPE, (char *)chunkBuffer[sourceChunk % TseAdExtDataSourceConstants::ChunkBufferSize].data, 512 * 8 * 2);
		Q_ASSERT(result);
		if (!result)
			quit = true;

		sourceChunk++;

		usedChunks.release();
	}
}

Q_EXPORT_PLUGIN(TseAdExtDataSourceDescription)
