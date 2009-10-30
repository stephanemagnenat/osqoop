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

#include "CypressEzUSBDevice.h"

#include <QtGlobal>
#include <QtDebug>
#include <QCoreApplication>

#if defined(Q_OS_WIN32)

#include <QFile>

#undef UNICODE
#include <Windows.h>
#include <Winuser.h>
#include <winioctl.h>
#include "fx2/usb100.h"
#include "fx2/ezusbsys.h"

//! Private data for CypressEzUSB device
class CypressEzUSBDevice::Private
{
public:
	HANDLE handle; //!< handle to Cypress driver

public:
	//! Reset device to state reset which can be either 0 or 1
	bool reset(unsigned char reset)
	{
		DWORD count;
		UCHAR buffer[1];
		VENDOR_OR_CLASS_REQUEST_CONTROL request_struct;

		request_struct.direction = 0;
		request_struct.requestType = 2;
		request_struct.recepient = 0;
		request_struct.requestTypeReservedBits = 0;
		request_struct.request = 0xA0;
		request_struct.value = 0xE600;
		request_struct.index = 0;

		buffer[0] = reset;

		return DeviceIoControl(
			handle,									// handle to device of interest
			IOCTL_EZUSB_VENDOR_OR_CLASS_REQUEST,	// control code of operation to perform
			&request_struct,						// pointer to buffer to supply input data
			sizeof(VENDOR_OR_CLASS_REQUEST_CONTROL),// size of input buffer
			buffer,									// pointer to buffer to receive output data
			sizeof(buffer),							// size of output buffer
			&count,									// pointer to variable to receive output byte count
			NULL									// pointer to overlapped structure for asynchronous operation
		);
	}
	
	//! Download firmware from filename. Can be a Qt ressource
	bool downloadFirmware(const QString &filename)
	{
		const size_t NBR_MAX_TO_READ_FROM_FILE = 66000;
		char buffer [NBR_MAX_TO_READ_FROM_FILE];
		
		// read file
		QFile file(filename);
		if (!file.open(QIODevice::ReadOnly))
		{
			qDebug() << "CypressEzUSBDevice::Private::downloadFirmware(" << filename << ") : can't open file";
			return false;
		}
		unsigned readCount = file.read(buffer, NBR_MAX_TO_READ_FROM_FILE);
		file.close();
		
		if (readCount == 0)
		{
			qDebug() << "CypressEzUSBDevice::Private::downloadFirmware(" << filename << ") : empty file";
			return false;
		}
	
		// enable reset
		reset(1);
	
		// download
		DWORD count;
	
		ANCHOR_DOWNLOAD_CONTROL download_struct;
		download_struct.Offset = 0;
	
		bool ioControl = DeviceIoControl(
			handle,							// handle to device of interest
			IOCTL_EZUSB_ANCHOR_DOWNLOAD,	// control code of operation to perform
			&download_struct,				// pointer to buffer to supply input data
			sizeof(ANCHOR_DOWNLOAD_CONTROL),// size of input buffer
			buffer,							// pointer to buffer to receive output data
			readCount,						// size of output buffer
			&count,							// pointer to variable to receive output byte count
			NULL							// pointer to overlapped structure for asynchronous operation
		);
		if (!ioControl)
		{
			qDebug() << "CypressEzUSBDevice::Private::downloadFirmware(" << filename << ") : ioControl failed";
			return false;
		}
	
		// disable reset
		reset(0);
		
		return true;
	}
};
	
#endif // Q_OS_WIN32

#if defined(Q_OS_UNIX)

#include <usb.h>

//! Private data for CypressEzUSB device
class CypressEzUSBDevice::Private
{
public:
	//! Init lib usb
	Private()
	{
		usb_init();
		dev = NULL;
	}
	
	//! Close device if opened
	~Private()
	{
		if (dev)
			usb_close(dev);
	}
	
	//! Handle to device
	usb_dev_handle *dev;
};

#endif // Q_OS_UNIX

//! Constructor, create p structure
CypressEzUSBDevice::CypressEzUSBDevice()
{
	p = new Private;
}

//! Desstructor, delete p structure
CypressEzUSBDevice::~CypressEzUSBDevice()
{
	delete p;
}

//! Open device.
//! On win32 Try names \\\\.\\ezusb-0 to \\\\.\\ezusb-9 and return true once one is found and opened. Returned false if none can be opened
//! On UNIX, locate all fx2 devices and load firmware on them
//! Firmware must be a .hex on Linux and a .ihx on Windows
bool CypressEzUSBDevice::open(const QString &firmwareFilename)
{
	#if defined(Q_OS_WIN32)
	
	UCHAR name [12] = "\\\\.\\ezusb-0";
	UCHAR i;

	// open device
	for (i=0;i<10;i++)
	{
		name[10] = 0x30 + i;
		p->handle = CreateFile(
				(const char*) name,
				GENERIC_WRITE,
				FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				NULL
			);
		if (p->handle != INVALID_HANDLE_VALUE)
			break;
	}
	if (p->handle == INVALID_HANDLE_VALUE)
	{
		qDebug() << "CypressEzUSBDevice::open(" << firmwareFilename << ") : no valid device found";
		return false;
	}
	
	// download firmware-
	return p->downloadFirmware(firmwareFilename);
	
	#endif // Q_OS_WIN32
	
	
	#if defined(Q_OS_UNIX)
	
	#ifndef TSEADEXT_NO_FX2_FW_LOAD
	// Load firmware using external setuid root program
	#ifndef FX2_FIRMWARE_DIR
	#define FX2_FIRMWARE_DIR QCoreApplication::applicationDirPath()
	#endif
	QString commandLine;
	commandLine += FX2_FIRMWARE_DIR;
	commandLine +=  "/osqoop-setupfx2 ";
	commandLine += FX2_FIRMWARE_DIR;
	commandLine += "/";
	commandLine += firmwareFilename;
	qDebug() << commandLine;
	system(commandLine.toLocal8Bit().data());
	#endif // TSEADEXT_NO_FX2_FW_LOAD
	
	// look at all devices
	struct usb_bus *busses;
	usb_find_busses();
	usb_find_devices();
	busses = usb_get_busses();
	
	// iterator on all buses
	for (struct usb_bus *bus = busses; bus; bus = bus->next)
	{
		// iterate on all devices
		for (struct usb_device *dev = bus->devices; dev; dev = dev->next)
		{
			if ((dev->descriptor.idVendor == 0x04b4) && (dev->descriptor.idProduct == 0x8613))
			{
				p->dev = usb_open(dev);
				//qDebug() << "CypressEzUSBDevice::open(" << firmwareFilename << ") : opened device " << bus->dirname << dev->filename;
				return true;
			}
		}
	}
	
	// print an error
	qDebug() << "CypressEzUSBDevice::open(" << firmwareFilename << ") : no valid device found";
	return false;
	
	#endif // Q_OS_UNIX
}


bool CypressEzUSBDevice::close(void)
{
	#if defined(Q_OS_WIN32)
	// TODO
	return false;
	
	#endif // Q_OS_WIN32
	
	#if defined(Q_OS_UNIX)
	
	int res = usb_close(p->dev);
	p->dev = NULL;
	if (res != 0)
	{
		qDebug() << usb_strerror();
		return false;
	}
	
	return true;
	
	#endif // Q_OS_UNIX
}

	
bool CypressEzUSBDevice::setInterface(unsigned number, unsigned alternateSetting)
{
	#if defined(Q_OS_WIN32)
	
	DWORD count;
	SET_INTERFACE_IN interface_struct;

	interface_struct.interfaceNum = number;
	interface_struct.alternateSetting =  alternateSetting;

	return DeviceIoControl(
		p->handle,			// handle to device of interest
		IOCTL_Ezusb_SETINTERFACE,	// control code of operation to perform
		&interface_struct,			// pointer to buffer to supply input data
		sizeof(SET_INTERFACE_IN),	// size of input buffer
		NULL,						// pointer to buffer to receive output data
		0,							// size of output buffer
		&count,						// pointer to variable to receive output byte count
		NULL						// pointer to overlapped structure for asynchronous operation
	);
	
	#endif // Q_OS_WIN32
	
	#if defined(Q_OS_UNIX)
	
	if (usb_claim_interface(p->dev, number) != 0)
	{
		qDebug() << usb_strerror();
		return false;
	}
	if (usb_set_altinterface(p->dev, alternateSetting) != 0)
	{
		qDebug() << usb_strerror();
		return false;
	}
	return true;
	
	#endif // Q_OS_UNIX
}


unsigned CypressEzUSBDevice::bulkRead(unsigned pipeNum, char *buffer, size_t size)
{
	#if defined(Q_OS_WIN32)
	
	BULK_TRANSFER_CONTROL bulk_struct;
	DWORD count;

	bulk_struct.pipeNum = pipeNum;

	DeviceIoControl(
		p->handle,				// handle to device of interest
		IOCTL_EZUSB_BULK_READ,			// control code of operation to perform
		&bulk_struct,					// pointer to buffer to supply input data
		sizeof(BULK_TRANSFER_CONTROL),	// size of input buffer
		buffer,							// pointer to buffer to receive output data
		size, 							// size of output buffer
		&count,							// pointer to variable to receive output byte count
		NULL							// pointer to overlapped structure for asynchronous operation
	);

	return count;
	
	
	#endif // Q_OS_WIN32
	
	#if defined(Q_OS_UNIX)
	
	int count = usb_bulk_read(p->dev, pipeNum, buffer, size, 1000);
	if (count < 0)
	{
		qDebug() << "CypressEzUSBDevice::bulkRead(" << pipeNum << "," << (size_t)buffer << "," << size << ")";
		qDebug() << usb_strerror();
		return 0;
	}
	return (unsigned)count;
	
	#endif // Q_OS_UNIX
}


unsigned CypressEzUSBDevice::bulkWrite(unsigned pipeNum, const char *buffer, size_t size)
{
	#if defined(Q_OS_WIN32)
	
	BULK_TRANSFER_CONTROL bulk_struct;
	DWORD count;
	
	bulk_struct.pipeNum = pipeNum;

	return DeviceIoControl(
		p->handle,						// handle to device of interest
		IOCTL_EZUSB_BULK_WRITE,			// control code of operation to perform
		&bulk_struct,					// pointer to buffer to supply input data
		sizeof(BULK_TRANSFER_CONTROL),	// size of input buffer
		const_cast<char *>(buffer),		// pointer to buffer to receive output data
		size,							// size of output buffer
		&count,							// pointer to variable to receive output byte count
		NULL							// pointer to overlapped structure for asynchronous operation
	);

	return count;
	
	#endif // Q_OS_WIN32
	
	#if defined(Q_OS_UNIX)
	
	int count = usb_bulk_write(p->dev, pipeNum, (char *)buffer, size, 1000);
	if (count < 0)
	{
		qDebug() << "CypressEzUSBDevice::bulkWrite(" << pipeNum << "," << (size_t)buffer << "," << size << ")";
		qDebug() << usb_strerror();
		return 0;
	}
	return (unsigned)count;
	
	#endif // Q_OS_UNIX
}
