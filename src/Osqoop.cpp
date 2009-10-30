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

#include "OscilloscopeWindow.h"
#include <QApplication>
#include <QTextCodec>
#include <QTranslator>
#include <QString>
#include <QLocale>


/*! \mainpage Osqoop
	\author Stephane Magnenat et al. - Laboratory of Digital Systems - Geneva Engineering School
	\version 1.0.0

	\section Introduction

	Osqoop is an open source software oscilloscope. This is the developer documentation. It provides:
	- Full cross-linked source documentation (use top links)
	- A \ref ProcessingPluginsCookbook to introduce you into plugins writing
	- A \ref DataSourceCookbook to introduce you into data source writing
	- Informations about compilation (see below)

	\section Compilation

	\subsection Requirements

	To compile Osqoop, you need:
	- Qt 4.0 (minimum) or Qt 4.1 (for all features), including
	qmake, gcc (or mingw32 on Windows) and a working shell.
	Please note that the Windows open source Qt 4.x can install mingw32
	for you.

	In addition, to compile and use the TseAdExt data source, you need:
	- On Unix: libusb
	To run it, you need:
	- On Windows: Cypress EzUSB driver
	- On Unix: fxload

	In addition, to compile the virtual mouse, you need:
	- On Unix: libxtest

	\subsection ChoosingYourPlugins Choosing your plugins

	The distribution of Osqoop comes with all plugins enabled and thus requires
	all aforementioned libraries. If you want to skip compilation of some plugins
	or do not have the required libraries, you can select the ones you want to compile
	by editing the SUBDIRS variable in datasource/DataSource.pro (for data source)
	and processing/Plugins.pro (for plugins).
	
	\subsection Compiling

	The actual compilation is very easy. Execute, in Osqoop base directory:
	- ./configure (or configure.bat in Windows): to recreate the makefiles from the .pro
	- make: to compile the execute from the sources using the makefiles
	On Unix, you also need to call the fixhelperperm script as root (or in sudo)
	after compilation in order to setuid it.

	You can regenerate this documentation using doxygen.

	\subsection Cleaning Cleaning your sources

	Osqoop ships with a some cleaning scripts:
	- makeclean: executing it will clean your sources. It will not remove the generated makefiles
	- makedistclean: call makeclean but also removes generated makefiles
	- makesourcedist: make a .tar.gz for distribution
	
	\section AuthorsAndContributors Authors and contributors

	\verbinclude authors

	\section License

	\verbinclude license

*/

#include <QLabel>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	QTranslator translator;
	translator.load(QString("osqoop_") + QLocale::system().name(), ":");
	app.installTranslator(&translator);
	
	OscilloscopeWindow oscilloscopeWindow;

	// only run if the main window is valid (i.e. the data source is ok)
	if (oscilloscopeWindow.isValid())
	{
		oscilloscopeWindow.show();
		return app.exec();
	}
	else
		return 0;
}

