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

#include <QtCore>
#include <QCheckBox>
#include "VirtualMouse.h"
#include <QtGlobal>

#if defined(Q_WS_WIN32)

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#include <Windows.h>
#include <Winuser.h>

#endif

#if defined(Q_WS_X11)

#include <QX11Info>
#include <X11/extensions/XTest.h>

#endif

QString ProcessingVirtualMouseDescription::systemName() const
{
	return QString("VirtualMouse");
}

QString ProcessingVirtualMouseDescription::name() const
{
	return QString("Virtual mouse");
}

QString ProcessingVirtualMouseDescription::description() const
{
	return QString("Mouse the mouse on screen using dx and dy inputs");
}

unsigned ProcessingVirtualMouseDescription::inputCount() const
{
	return 3;
}

unsigned ProcessingVirtualMouseDescription::outputCount() const
{
	return 0;
}

ProcessingPlugin *ProcessingVirtualMouseDescription::create(const DataSource *dataSource) const
{
	return new ProcessingVirtualMouse(this);
}

//! Create a button to enable/disable plugin
QWidget *ProcessingVirtualMouse::createGUI(void)
{
	enabled = false;
	QCheckBox *button = new QCheckBox("Mouse motion"); 
	button->setShortcut(tr("Ctrl+Shift+m"));
	connect(button, SIGNAL(toggled(bool)), this, SLOT(enableDisable(bool)));
	return button;
}

//! called through signal/slot system when GUI enable/disable mouse motion
void ProcessingVirtualMouse::enableDisable(bool state)
{
	enabled = state;
}

void ProcessingVirtualMouse::processData(const std::valarray<signed short *> &inputs, const std::valarray<signed short *> &outputs, unsigned sampleCount)
{
	int buttonVal = inputs[2][0];
	unsigned buttons = 0;
	if (buttonVal > 0)
		buttons |= 1;
	if (buttonVal < 0)
		buttons |= 2;
	moveMouse(inputs[0][0], inputs[1][0], buttons);
}

//! Move mouse to a given position, change buttons state
void ProcessingVirtualMouse::moveMouse(int dx, int dy, unsigned buttons)
{
	if (enabled)
	{
		#if defined(Q_WS_WIN32)
		
		INPUT is;
		memset(&is, 0, sizeof(is));
		is.type = INPUT_MOUSE;

		// buttons
		if ((buttons&1) != (oldButtons&1))
			is.mi.dwFlags |= (buttons&1) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;

		if ((buttons&2) != (oldButtons&2))
			is.mi.dwFlags |= (buttons&2) ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;

		if ((buttons&4) != (oldButtons&4))
			is.mi.dwFlags |= (buttons&4) ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
		oldButtons = buttons;

		// axis
		if (dx || dy)
		{
			is.mi.dwFlags |= MOUSEEVENTF_MOVE;
			is.mi.dx = dx;
			is.mi.dy = dy;
		}
		int dz = 0;
		if (dz)
		{
			is.mi.dwFlags |= MOUSEEVENTF_WHEEL;
			is.mi.mouseData |= dz*WHEEL_DELTA;
		}

		// Send to win32 API
		SendInput(1, &is, sizeof(INPUT));
		
		#endif
		
		#if defined(Q_WS_X11)
		
		// buttons
		if ((buttons&1) != (oldButtons&1))
			XTestFakeButtonEvent(QX11Info::display(), 0, (buttons&1), CurrentTime);
		if ((buttons&2) != (oldButtons&2))
			XTestFakeButtonEvent(QX11Info::display(), 1, (buttons&1), CurrentTime);
		if ((buttons&4) != (oldButtons&4))
			XTestFakeButtonEvent(QX11Info::display(), 2, (buttons&1), CurrentTime);
		oldButtons = buttons;
		
		// axis
		if (dx || dy)
			XTestFakeRelativeMotionEvent(QX11Info::display(), dx, dy, CurrentTime);
		
		#endif
	}
}
