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

#ifndef __UTILITIES_H
#define __UTILITIES_H

#include <QString>
#include <QColor>

const unsigned ScaleFactorCount = 15;

QString timeScaleToStringWidthToDiv(unsigned duration);

QString timeScaleToString(double duration);

QString YScaleToString(unsigned yDivisionFactor);

void setDataSourceChannelCount(unsigned number);

unsigned logicChannelIdToPhysic(int logic, bool *ok = NULL);

int physicChannelIdToLogic(unsigned physic);

QString channelNumberToString(unsigned channel);

unsigned channelNumberFromString(const QString &channel);

void setCustomChannelName(unsigned channel, const QString &name);

void clearCustomChannelNames(void);

void loadCustomChannelNames(void);

void saveCustomChannelNames(void);

QColor getChannelColor(unsigned channel);

unsigned getScaleFactor(unsigned index);

unsigned getScaleFactorInvert(unsigned factor);

#endif
