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

#ifndef __PROCESSING_EMG_MOUSE_BACKPROP_NN_CLASSIFIER
#define __PROCESSING_EMG_MOUSE_BACKPROP_NN_CLASSIFIER

#include <ProcessingPlugin.h>
#include <QString>
#include <QPushButton>

const unsigned EMGMouseBackpropNNInputCount = 6;
const unsigned EMGMouseBackpropNNLearningStepCount = 64;

//! Description of the EMGMouse Neural network with back-propagation plugin
class ProcessingEMGMouseBackpropNNClassifierDescription : public QObject, public ProcessingPluginDescription
{
	Q_OBJECT
    Q_PLUGIN_METADATA(IID "ch.eig.lsn.Oscilloscope.ProcessingPluginDescription/1.0" FILE "emgmousebackpropnnclassifier.json")
    Q_INTERFACES(ProcessingPluginDescription)

public:
	QString systemName() const;
	QString name() const;
	QString description() const;
	unsigned inputCount() const;
	unsigned outputCount() const;
	ProcessingPlugin *create(const DataSource *dataSource) const;
};

namespace Teem
{
	class BackPropFeedForwardNeuralNetwork;
}

//! EMGMouse Neural network with back-propagation plugin.
/*!
	Learn the association of inputs to outputs suitable for mouse control.
	The three output channels correspond to x axis, y axis and mouse buttons
	(>0 left button, <0 right button, 0 = no button).

	This plugin has been developed for the EMGMouse
	(Traitement de Signaux Electromyographiques) project.
	It is distributed with Osqoop as an example of signal processing filter.
*/
class ProcessingEMGMouseBackpropNNClassifier : public QObject, public ProcessingPlugin
{
	Q_OBJECT

public:
	QWidget *createGUI(void);
	void processData(const std::valarray<signed short *> &inputs, const std::valarray<signed short *> &outputs, unsigned sampleCount);
	void terminate(void) { deleteLater(); }

protected:
	~ProcessingEMGMouseBackpropNNClassifier();

signals:
	void setButtonState(const QString &, bool);
	void setProgressValue(int);
	void setProgressVisible(bool);

private slots:
	void buttonClicked();

private:
	enum State
	{
		STATE_WAITING_CALIBRATION_IDLE = 0,
		STATE_CALIBRATION_IDLE,
		STATE_WAITING_CALIBRATION_XP,
		STATE_CALIBRATION_XP,
		STATE_WAITING_CALIBRATION_XN,
		STATE_CALIBRATION_XN,
		STATE_WAITING_CALIBRATION_YP,
		STATE_CALIBRATION_YP,
		STATE_WAITING_CALIBRATION_YN,
		STATE_CALIBRATION_YN,
		STATE_WAITING_CALIBRATION_LC,
		STATE_CALIBRATION_LC,
		STATE_WAITING_CALIBRATION_RC,
		STATE_CALIBRATION_RC,
		STATE_WAITING_CALIBRATION_IDLE2,
		STATE_CALIBRATION_IDLE2,
		STATE_RUNNING,
		STATE_COUNT
	};

private:
	inline double normalizeNNInput(short sample) { return (((double)sample) - mean) / stddev; }
	void buttonTextFromState(void);
	void estimateMeanAndStdVarOfInput(void);

private:
	State state;
	short calibrationDataIdle[EMGMouseBackpropNNLearningStepCount][EMGMouseBackpropNNInputCount];
	short calibrationDataIdle2[EMGMouseBackpropNNLearningStepCount][EMGMouseBackpropNNInputCount];
	short calibrationDataXP[EMGMouseBackpropNNLearningStepCount][EMGMouseBackpropNNInputCount];
	short calibrationDataXN[EMGMouseBackpropNNLearningStepCount][EMGMouseBackpropNNInputCount];
	short calibrationDataYP[EMGMouseBackpropNNLearningStepCount][EMGMouseBackpropNNInputCount];
	short calibrationDataYN[EMGMouseBackpropNNLearningStepCount][EMGMouseBackpropNNInputCount];
	short calibrationDataLC[EMGMouseBackpropNNLearningStepCount][EMGMouseBackpropNNInputCount];
	short calibrationDataRC[EMGMouseBackpropNNLearningStepCount][EMGMouseBackpropNNInputCount];
	double mean;
	double stddev;
	unsigned calibrationPos;
	Teem::BackPropFeedForwardNeuralNetwork *nn;

private:
	friend class ProcessingEMGMouseBackpropNNClassifierDescription;
	ProcessingEMGMouseBackpropNNClassifier(const ProcessingPluginDescription *description);
};

//! A QPushButton whose state can be set using a slot
class SettableQPushButton : public QPushButton
{
	Q_OBJECT
public:
	SettableQPushButton(const QString &s, QWidget *parent = NULL) : QPushButton(s, parent) {}

public slots:
	void setButtonState(const QString &, bool);
};

#endif
