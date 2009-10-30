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

#include <QtCore>
#include <DataSource.h>
#include "EMGEnvelope.h"
#include <IntegerRealValuedFFT.h>
#include <BandPass2ndOrderFilter.h>
#include <IIRFilter.h>
#include <valarray>

void SettableQPushButton::setButtonState(const QString &text, bool enable)
{
	setText(text);
	setEnabled(enable);
}

QString ProcessingEMGEnvelopeDescription::systemName() const
{
	return QString("EMGEnvelope");
}

QString ProcessingEMGEnvelopeDescription::name() const
{
	return QString("EMG Envelope detector");
}

QString ProcessingEMGEnvelopeDescription::description() const
{
	return QString("Get the envelop of an electromyographic (EMG) signals.");
}

unsigned ProcessingEMGEnvelopeDescription::inputCount() const
{
	return 1;
}

unsigned ProcessingEMGEnvelopeDescription::outputCount() const
{
	return 1;
}

ProcessingPlugin *ProcessingEMGEnvelopeDescription::create(const DataSource *dataSource) const
{
	return new ProcessingEMGEnvelope(this, dataSource);
}

QWidget *ProcessingEMGEnvelope::createGUI(void)
{
	SettableQPushButton *button = new SettableQPushButton(tr("Calibrate 50% MVC"));

	connect(button, SIGNAL(clicked()), SLOT(buttonClicked()));
	connect(this, SIGNAL(setButtonState(const QString &, bool)), button, SLOT(setButtonState(const QString &, bool)));

	return button;
}

void ProcessingEMGEnvelope::processData(const std::valarray<signed short *> &inputs, const std::valarray<signed short *> &outputs, unsigned sampleCount)
{
	signed short *srcPtr = inputs[0];
	double rms = 0;
	for (unsigned sample = 0; sample < sampleCount;)
	{
		// decimation using mean on 8 samples
		int decimationMean = 0;
		for (unsigned i = 0; i < 8; i++)
			decimationMean += *srcPtr++;
		decimationMean >>= 3;
		sample += 8;

		if (state == STATE_CALIBRATION_HALF_MVC)
		{
			calibrationSlices[calibrationSlicePos][calibrationBufferPos++ * 2] = decimationMean;

			// next slice, do fft
			if (calibrationBufferPos == 256)
			{
				calibrationBufferPos = 0;
				fft.fft(calibrationSlices[calibrationSlicePos]);
				calibrationSlicePos++;
			}

			// end of calibration
			if (calibrationSlicePos == 16)
			{
				// reset values
				calibrationSlicePos = 0;

				// find max
				unsigned freq30Hz = (30 * 256 * 8) / (dataSource->samplingRate());
				unsigned freq400Hz = (400 * 256 * 8) / (dataSource->samplingRate());
				unsigned maxIntensityFreq = freq30Hz;
				int maxIntensityValue = 0;
				
				for (unsigned freq = freq30Hz; freq < freq400Hz; freq++)
				{
					// mean for this frequency
					int freqIntensityMean = 0;
					for (unsigned i = 0; i < 16; i++)
					{
						freqIntensityMean += (fft.module2(calibrationSlices[i], freq) >> 4); // shift of 4 bits to prevent overflow
					}

					// if bigger than max, replace max
					if (freqIntensityMean > maxIntensityValue)
					{
						maxIntensityValue = freqIntensityMean;
						maxIntensityFreq = freq;
					}
				}

				// set filter
				bpFilter->setBandWidth(10.0 * 8.0);
				bpFilter->setCutOffFreq(((double)maxIntensityFreq * (double)dataSource->samplingRate()) / ( 8.0 * 256.0));

				// change state
				state = STATE_RUNNING;
				buttonTextFromState(((double)maxIntensityFreq * (double)dataSource->samplingRate())/ ( 8.0 * 256.0));
			}
		}
		
		// do processing
		double value = decimationMean;

		value = hpFilter->getNext(value);

		if (state == STATE_RUNNING)
			value = bpFilter->getNext(value);
		rms += value * value;
	}

	// create output values
	rms /= ((double)sampleCount / 8.0);
	rms = sqrt(rms);
	rms = meanFilter->getNext(rms);
	rms = lpFilter->getNext(rms);
	signed short *destPtr = outputs[0];
	for (unsigned sample = 0; sample < sampleCount; sample++)
	{
		*destPtr++ = (short)rms;
	}
}

void ProcessingEMGEnvelope::buttonClicked()
{
	state = (State)((state + 1) % STATE_COUNT);
	buttonTextFromState();
}

void ProcessingEMGEnvelope::buttonTextFromState(double freq)
{
	switch (state)
	{
		case STATE_WAITING_CALIBRATION_HALF_MVC:
			emit setButtonState(tr("Idle, calibrate 50% MVC?"), true);
		break;
		case STATE_CALIBRATION_HALF_MVC:
			emit setButtonState(tr("Calibrating 50% MVC, please wait"), false);
		break;
		case STATE_RUNNING:
			emit setButtonState(tr("Running bp at %0 Hz, reset?").arg(freq), true);
		break;
		default:
		Q_ASSERT(false);
		break;
	}
}

ProcessingEMGEnvelope::ProcessingEMGEnvelope(const ProcessingPluginDescription *description, const DataSource *dataSource) :
	ProcessingPlugin(description),
    dataSource(dataSource)
{
	// init state
	state = STATE_WAITING_CALIBRATION_HALF_MVC;

	// init calibration
	memset(calibrationSlices, 0, sizeof(calibrationSlices));
	calibrationBufferPos = 0;
	calibrationSlicePos = 0;

	// create filters
	const double hpB[] = {0.9990, -0.9990};
	const double hpA[] = {1.0000, -0.9980};
	hpFilter = new IIRFilter(1, hpB, hpA);

	bpFilter = new BandPass2ndOrderFilter((double)dataSource->samplingRate(), 0, 0);

	std::valarray<double> meanB(1.0/10.0, 10);
	std::valarray<double> meanA(0.0, 10);
	meanA[0] = 1.0;
	meanFilter = new IIRFilter(9, &meanB[0], &meanA[0]);

	const double lpB[] = {0.0903, -0.0203, -0.0203, 0.0903};
	const double lpA[] = {1.0000, -2.0719, 1.7378, -0.5259 };
	lpFilter = new IIRFilter(3, lpB, lpA);
}

ProcessingEMGEnvelope::~ProcessingEMGEnvelope()
{
	delete hpFilter;
	delete bpFilter;
	delete meanFilter;
	delete lpFilter;
}

Q_EXPORT_PLUGIN(ProcessingEMGEnvelopeDescription)
