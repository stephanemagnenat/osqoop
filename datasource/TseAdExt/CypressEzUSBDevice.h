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

#ifndef __CYPRESS_EZUSB_DEVICE_H
#define __CYPRESS_EZUSB_DEVICE_H

#include <QtGlobal>

#include "USBDevice.h"

//! USB interface to the Cypress EzUSB FX/FX2 chip, using Cypres EzUSB driver
class CypressEzUSBDevice : public USBDevice
{
private:
	class Private;
	Private *p; //!< private data for this device

public:
	CypressEzUSBDevice();
	virtual ~CypressEzUSBDevice();
	virtual bool open(const QString &firmwareFilename);
	virtual bool close(void);
	virtual bool setInterface(unsigned number, unsigned alternateSetting);
	virtual unsigned bulkRead(unsigned pipeNum, char *buffer, size_t size);
	virtual unsigned bulkWrite(unsigned pipeNum, const char *buffer, size_t size);
};

#endif
