/*

Osqoop, an open source software oscilloscope.
Copyright (C) 2006 Lucas Tamarit <lucas dot tamarit at gmail dot com>
Laboratory of Signal Processing http://eig.unige.ch/~kocher/
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

#ifndef __PROCESSING_TSE_LUCAS
#define __PROCESSING_TSE_LUCAS

#include <ProcessingPlugin.h>
#include <QString>
#include <QPushButton>
#include <IntegerRealValuedFFT.h>

//! Description of the TSE Lucas filter plugin
class ProcessingEMGEnvelopeDescription : public QObject, public ProcessingPluginDescription
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "ch.eig.lsn.Oscilloscope.ProcessingPluginDescription/1.0" FILE "emgenvelope.json")
	Q_INTERFACES(ProcessingPluginDescription)

public:
	QString systemName() const;
	QString name() const;
	QString description() const;
	unsigned inputCount() const;
	unsigned outputCount() const;
	ProcessingPlugin *create(const DataSource *dataSource) const;
};

class IIRFilter;
class BandPass2ndOrderFilter;

//! Get the envelop of electromyographic signals
/*!
	This plugin computes the envelope of electromyographic signals. It has been
	developed for the TSE (Traitement de Signaux Electromyographiques) project.
	It is distributed with Osqoop as an example of signal processing filter.
*/
class ProcessingEMGEnvelope : public QObject, public ProcessingPlugin
{
	Q_OBJECT

public:
	QWidget *createGUI(void);
	void processData(const std::valarray<signed short *> &inputs, const std::valarray<signed short *> &outputs, unsigned sampleCount);
	void terminate(void) { deleteLater(); }

protected:
	~ProcessingEMGEnvelope();

signals:
	void setButtonState(const QString &, bool);

private slots:
	void buttonClicked();

private:
	enum State
	{
		STATE_WAITING_CALIBRATION_HALF_MVC = 0,
		STATE_CALIBRATION_HALF_MVC,
		STATE_RUNNING,
		STATE_COUNT
	};

private:
	void buttonTextFromState(double freq = 0.0);

private:
	State state;
	short calibrationSlices[16][512];
	unsigned calibrationBufferPos;
	unsigned calibrationSlicePos;
	IntegerRealValuedFFT<256> fft;
	IIRFilter *hpFilter; //!< filter out DC component
	BandPass2ndOrderFilter *bpFilter; //!< select a frequency range
	IIRFilter *meanFilter; //!< get envelope by mean
	IIRFilter *lpFilter; //!< filter out high frequency of envelope
	const DataSource *dataSource; //!< data source, to get sampling rate

private:
	friend class ProcessingEMGEnvelopeDescription;
	ProcessingEMGEnvelope(const ProcessingPluginDescription *description, const DataSource *dataSource);
};

//! A QPushButton whose state can be set using a slot
class SettableQPushButton : public QPushButton
{
	Q_OBJECT
public:
	SettableQPushButton(const QString &s) : QPushButton(s) {}

public slots:
	void setButtonState(const QString &, bool);
};

#endif
/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
