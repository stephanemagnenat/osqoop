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
#include <QProgressBar>
#include <QBoxLayout>
#include <DataSource.h>
#include "EMGMouseBackpropNNClassifier.h"
#include <EMGMouseBackpropNNClassifier.moc>
#include <FeedForwardNeuralNetwork.h>
#include <fstream>

void SettableQPushButton::setButtonState(const QString &text, bool enable)
{
	setText(text);
	setEnabled(enable);
}

QString ProcessingEMGMouseBackpropNNClassifierDescription::systemName() const
{
	return QString("EMGMouseBackpropNNClassifier");
}

QString ProcessingEMGMouseBackpropNNClassifierDescription::name() const
{
	return QString("EMG neural network for mouse control");
}

QString ProcessingEMGMouseBackpropNNClassifierDescription::description() const
{
	return QString("Classify electromyographics (EMG) inputs for control of computer mouse using back-propagation of error in a feed-forward neural network.");
}

unsigned ProcessingEMGMouseBackpropNNClassifierDescription::inputCount() const
{
	return EMGMouseBackpropNNInputCount;
}

unsigned ProcessingEMGMouseBackpropNNClassifierDescription::outputCount() const
{
	return 3;
}

ProcessingPlugin *ProcessingEMGMouseBackpropNNClassifierDescription::create(const DataSource *dataSource) const
{
	return new ProcessingEMGMouseBackpropNNClassifier(this);
}

QWidget *ProcessingEMGMouseBackpropNNClassifier::createGUI(void)
{
	QWidget *w = new QWidget();
	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, w);

	SettableQPushButton *button = new SettableQPushButton(tr("Waiting. Click calibrates idle"), w);
	layout->addWidget(button);
	connect(button, SIGNAL(clicked()), SLOT(buttonClicked()));
	connect(this, SIGNAL(setButtonState(const QString &, bool)), button, SLOT(setButtonState(const QString &, bool)));
	
	QProgressBar *progressBar = new QProgressBar(w);
	layout->addWidget(progressBar);
	progressBar->setRange(0, EMGMouseBackpropNNLearningStepCount - 1);
	progressBar->setVisible(false);
	connect(this, SIGNAL(setProgressValue(int)), progressBar, SLOT(setValue(int)));
	connect(this, SIGNAL(setProgressVisible(bool)), progressBar, SLOT(setVisible(bool)));

	return w;
}

void ProcessingEMGMouseBackpropNNClassifier::processData(const std::valarray<signed short *> &inputs, const std::valarray<signed short *> &outputs, unsigned sampleCount)
{
	// for each calibration state

	// idle
	if (state == STATE_CALIBRATION_IDLE)
	{
		for (unsigned i = 0; i < EMGMouseBackpropNNInputCount; i++)
			calibrationDataIdle[calibrationPos][i] = inputs[i][0];
		emit setProgressValue(calibrationPos);
		if (++calibrationPos == EMGMouseBackpropNNLearningStepCount)
		{
			calibrationPos = 0;
			state = STATE_WAITING_CALIBRATION_XP;
			buttonTextFromState();
		}
	}
	
	// X+
	if (state == STATE_CALIBRATION_XP)
	{
		for (unsigned i = 0; i < EMGMouseBackpropNNInputCount; i++)
			calibrationDataXP[calibrationPos][i] = inputs[i][0];
		emit setProgressValue(calibrationPos);
		if (++calibrationPos == EMGMouseBackpropNNLearningStepCount)
		{
			calibrationPos = 0;
			state = STATE_WAITING_CALIBRATION_XN;
			buttonTextFromState();
		}
	}

	// X-
	if (state == STATE_CALIBRATION_XN)
	{
		for (unsigned i = 0; i < EMGMouseBackpropNNInputCount; i++)
			calibrationDataXN[calibrationPos][i] = inputs[i][0];
		emit setProgressValue(calibrationPos);
		if (++calibrationPos == EMGMouseBackpropNNLearningStepCount)
		{
			calibrationPos = 0;
			state = STATE_WAITING_CALIBRATION_YP;
			buttonTextFromState();
		}
	}

	// Y+
	if (state == STATE_CALIBRATION_YP)
	{
		for (unsigned i = 0; i < EMGMouseBackpropNNInputCount; i++)
			calibrationDataYP[calibrationPos][i] = inputs[i][0];
		emit setProgressValue(calibrationPos);
		if (++calibrationPos == EMGMouseBackpropNNLearningStepCount)
		{
			calibrationPos = 0;
			state = STATE_WAITING_CALIBRATION_YN;
			buttonTextFromState();
		}
	}

	// Y-
	if (state == STATE_CALIBRATION_YN)
	{
		for (unsigned i = 0; i < EMGMouseBackpropNNInputCount; i++)
			calibrationDataYN[calibrationPos][i] = inputs[i][0];
		emit setProgressValue(calibrationPos);
		if (++calibrationPos == EMGMouseBackpropNNLearningStepCount)
		{
			calibrationPos = 0;
			state = STATE_WAITING_CALIBRATION_LC;
			buttonTextFromState();
		}
	}

	// Left click
	if (state == STATE_CALIBRATION_LC)
	{
		for (unsigned i = 0; i < EMGMouseBackpropNNInputCount; i++)
			calibrationDataLC[calibrationPos][i] = inputs[i][0];
		emit setProgressValue(calibrationPos);
		if (++calibrationPos == EMGMouseBackpropNNLearningStepCount)
		{
			calibrationPos = 0;
			state = STATE_WAITING_CALIBRATION_RC;
			buttonTextFromState();
		}
	}

	// Right click
	if (state == STATE_CALIBRATION_RC)
	{
		for (unsigned i = 0; i < EMGMouseBackpropNNInputCount; i++)
			calibrationDataRC[calibrationPos][i] = inputs[i][0];
		emit setProgressValue(calibrationPos);
		if (++calibrationPos == EMGMouseBackpropNNLearningStepCount)
		{
			calibrationPos = 0;
			state = STATE_WAITING_CALIBRATION_IDLE2;
			buttonTextFromState();
		}
	}

	// idle 2
	if (state == STATE_CALIBRATION_IDLE2)
	{
		for (unsigned i = 0; i < EMGMouseBackpropNNInputCount; i++)
			calibrationDataIdle2[calibrationPos][i] = inputs[i][0];
		emit setProgressValue(calibrationPos);
		if (++calibrationPos == EMGMouseBackpropNNLearningStepCount)
		{
			calibrationPos = 0;

			// calibrate normalization
			estimateMeanAndStdVarOfInput();
			
			// randomize NN
			double randomRange = 1.0 / sqrt((double)nn->inputNum()); // 1 on sqrt of fan-in
			nn->randomize(-randomRange, randomRange);

			// shuffle learning samples
			for (unsigned i = 0; i < EMGMouseBackpropNNLearningStepCount; i++)
			{
				unsigned exchange = rand() % EMGMouseBackpropNNLearningStepCount;
				for (unsigned j = 0; j < EMGMouseBackpropNNInputCount; j++)
				{
					std::swap(calibrationDataIdle[i][j], calibrationDataIdle[exchange][j]);
					std::swap(calibrationDataIdle2[i][j], calibrationDataIdle2[exchange][j]);
					std::swap(calibrationDataXP[i][j], calibrationDataXP[exchange][j]);
					std::swap(calibrationDataXN[i][j], calibrationDataXN[exchange][j]);
					std::swap(calibrationDataYP[i][j], calibrationDataYP[exchange][j]);
					std::swap(calibrationDataYN[i][j], calibrationDataYN[exchange][j]);
					std::swap(calibrationDataLC[i][j], calibrationDataLC[exchange][j]);
					std::swap(calibrationDataRC[i][j], calibrationDataRC[exchange][j]);
				}
			}

			// do the learning !
			std::ofstream errorLog("error.log.txt");
			for (unsigned learningStep = 0; learningStep < EMGMouseBackpropNNLearningStepCount; learningStep++)
			{
				const double learningTargetValue = 0.6;
				
				// decrease learning rate with time
				nn->setLearningRate(((double)(EMGMouseBackpropNNLearningStepCount - learningStep)) / (double)(EMGMouseBackpropNNLearningStepCount * 10));

				// Idle
				for (unsigned i = 0; i < EMGMouseBackpropNNInputCount; i++)
					nn->setInput(i, normalizeNNInput(calibrationDataIdle[learningStep][i]));
				nn->step();
				nn->setError(0, 0 - nn->getOutput(0));
				nn->setError(1, 0 - nn->getOutput(1));
				nn->setError(2, 0 - nn->getOutput(2));
				errorLog << (0 - nn->getOutput(0)) << " " << (0 - nn->getOutput(1)) << " " << (0 - nn->getOutput(2)) << std::endl;
				nn->stepBackward();

				// XP
				for (unsigned i = 0; i < EMGMouseBackpropNNInputCount; i++)
					nn->setInput(i, normalizeNNInput(calibrationDataXP[learningStep][i]));
				nn->step();
				nn->setError(0, learningTargetValue - nn->getOutput(0));
				nn->setError(1, 0 - nn->getOutput(1));
				nn->setError(2, 0 - nn->getOutput(2));
				errorLog << (learningTargetValue - nn->getOutput(0)) << " " << (0 - nn->getOutput(1)) << " " << (0 - nn->getOutput(2)) << std::endl;
				nn->stepBackward();
				
				// XN
				for (unsigned i = 0; i < EMGMouseBackpropNNInputCount; i++)
					nn->setInput(i, normalizeNNInput(calibrationDataXN[learningStep][i]));
				nn->step();
				nn->setError(0, (-learningTargetValue) - nn->getOutput(0));
				nn->setError(1, 0 - nn->getOutput(1));
				nn->setError(2, 0 - nn->getOutput(2));
				errorLog << (-learningTargetValue - nn->getOutput(0)) << " " << (0 - nn->getOutput(1)) << " " << (0 - nn->getOutput(2)) << std::endl;
				nn->stepBackward();

				// Idle 2
				for (unsigned i = 0; i < EMGMouseBackpropNNInputCount; i++)
					nn->setInput(i, normalizeNNInput(calibrationDataIdle2[learningStep][i]));
				nn->step();
				nn->setError(0, 0 - nn->getOutput(0));
				nn->setError(1, 0 - nn->getOutput(1));
				nn->setError(2, 0 - nn->getOutput(2));
				errorLog << (0 - nn->getOutput(0)) << " " << (0 - nn->getOutput(1)) << " " << (0 - nn->getOutput(2)) << std::endl;
				nn->stepBackward();

				// YP
				for (unsigned i = 0; i < EMGMouseBackpropNNInputCount; i++)
					nn->setInput(i, normalizeNNInput(calibrationDataYP[learningStep][i]));
				nn->step();
				nn->setError(0, 0 - nn->getOutput(0));
				nn->setError(1, learningTargetValue - nn->getOutput(1));
				nn->setError(2, 0 - nn->getOutput(2));
				errorLog << (0 - nn->getOutput(0)) << " " << (learningTargetValue - nn->getOutput(1)) << " " << (0 - nn->getOutput(2)) << std::endl;
				nn->stepBackward();
				
				// YN
				for (unsigned i = 0; i < EMGMouseBackpropNNInputCount; i++)
					nn->setInput(i, normalizeNNInput(calibrationDataYN[learningStep][i]));
				nn->step();
				nn->setError(0, 0 - nn->getOutput(0));
				nn->setError(1, (-learningTargetValue) - nn->getOutput(1));
				nn->setError(2, 0 - nn->getOutput(2));
				errorLog << (0 - nn->getOutput(0)) << " " << ((-learningTargetValue) - nn->getOutput(1)) << " " << (0 - nn->getOutput(2)) << std::endl;
				nn->stepBackward();

				// Left click
				for (unsigned i = 0; i < EMGMouseBackpropNNInputCount; i++)
					nn->setInput(i, normalizeNNInput(calibrationDataLC[learningStep][i]));
				nn->step();
				nn->setError(0, 0 - nn->getOutput(0));
				nn->setError(1, 0 - nn->getOutput(1));
				nn->setError(2, learningTargetValue - nn->getOutput(2));
				errorLog << (0 - nn->getOutput(0)) << " " << (0 - nn->getOutput(1)) << " " << (learningTargetValue - nn->getOutput(2)) << std::endl;
				nn->stepBackward();

				// Right click
				for (unsigned i = 0; i < EMGMouseBackpropNNInputCount; i++)
					nn->setInput(i, normalizeNNInput(calibrationDataRC[learningStep][i]));
				nn->step();
				nn->setError(0, 0 - nn->getOutput(0));
				nn->setError(1, 0 - nn->getOutput(1));
				nn->setError(2, (-learningTargetValue) - nn->getOutput(2));
				errorLog << (0 - nn->getOutput(0)) << " " << (0 - nn->getOutput(1)) << " " << ((-learningTargetValue) - nn->getOutput(2)) << std::endl;
				nn->stepBackward();
			}

			// dump the weights
			std::ofstream nnLog("nn.log.txt");
			// for each layer
			for (size_t layer = 0; layer < nn->layerNum(); layer++)
			{
				nnLog << "Layer " << layer << std::endl;
				// for each neurone
				for (size_t y = 0; y < nn->getWeightsMatrixHeight(layer); y++)
				{
					// for each connection
					for (size_t x = 0; x < nn->getWeightsMatrixWidth(layer); x++)
						nnLog << nn->getWeight(layer, x, y) << " ";
					nnLog << nn->getBiasWeight(layer, y) << " ";
					nnLog << "\n" << std::endl;
				}
			}

			// change the state
			state = STATE_RUNNING;
			buttonTextFromState();
		}
	}
	
	// write output if running
	if (state == STATE_RUNNING)
	{
		for (unsigned i = 0; i < EMGMouseBackpropNNInputCount; i++)
			nn->setInput(i, normalizeNNInput(inputs[i][0]));
		nn->step();
		for (unsigned i = 0; i < 3; i++)
		{
			short *destPtr = outputs[i];
			short outputVal = (short)(10.0 * nn->getOutput(i));
			for (unsigned sample = 0; sample < sampleCount; sample++)
				*destPtr++ = outputVal;
		}
	}
}

void ProcessingEMGMouseBackpropNNClassifier::estimateMeanAndStdVarOfInput(void)
{
	// compute mean from calibration data
	mean = 0;
	for (unsigned i = 0; i < EMGMouseBackpropNNLearningStepCount; i++)
		for (unsigned j = 0; j < EMGMouseBackpropNNInputCount; j++)
		{
			mean += (double)calibrationDataXP[i][j];
			mean += (double)calibrationDataXN[i][j];
			mean += (double)calibrationDataYP[i][j];
			mean += (double)calibrationDataYN[i][j];
			mean += (double)calibrationDataLC[i][j];
			mean += (double)calibrationDataRC[i][j];
		}
	mean /= (EMGMouseBackpropNNLearningStepCount*6*EMGMouseBackpropNNInputCount);

	// compute stddev from calibration data
	stddev = 0;
	for (unsigned i = 0; i < EMGMouseBackpropNNLearningStepCount; i++)
		for (unsigned j = 0; j < EMGMouseBackpropNNInputCount; j++)
		{
			stddev += ((double)calibrationDataXP[i][j] - mean) * ((double)calibrationDataXP[i][j] - mean);
			stddev += ((double)calibrationDataXN[i][j] - mean) * ((double)calibrationDataXN[i][j] - mean);
			stddev += ((double)calibrationDataYP[i][j] - mean) * ((double)calibrationDataYP[i][j] - mean);
			stddev += ((double)calibrationDataYN[i][j] - mean) * ((double)calibrationDataYN[i][j] - mean);
			stddev += ((double)calibrationDataLC[i][j] - mean) * ((double)calibrationDataLC[i][j] - mean);
			stddev += ((double)calibrationDataRC[i][j] - mean) * ((double)calibrationDataRC[i][j] - mean);
		}
	stddev /= (EMGMouseBackpropNNLearningStepCount*6*EMGMouseBackpropNNInputCount);
	stddev = sqrt(stddev);

	// log
	std::ofstream calibrationLog("calibration.log.txt");
	calibrationLog << "mean " << mean << " stddev " << stddev << std::endl;
}

void ProcessingEMGMouseBackpropNNClassifier::buttonClicked()
{
	state = (State)((state + 1) % STATE_COUNT);
	buttonTextFromState();
}

void ProcessingEMGMouseBackpropNNClassifier::buttonTextFromState(void)
{
	switch (state)
	{
		case STATE_WAITING_CALIBRATION_IDLE:
			emit setButtonState(tr("Waiting. Click calibrates idle"), true);
			emit setProgressVisible(false);
		break;
		case STATE_CALIBRATION_IDLE:
			emit setButtonState(tr("Calibrating idle, please wait"), false);
			emit setProgressVisible(true);
			emit setProgressValue(0);
		break;
		case STATE_WAITING_CALIBRATION_XP:
			emit setButtonState(tr("Waiting. Click calibrates X+"), true);
			emit setProgressVisible(false);
		break;
		case STATE_CALIBRATION_XP:
			emit setButtonState(tr("Calibrating X+, please wait"), false);
			emit setProgressVisible(true);
			emit setProgressValue(0);
		break;
		case STATE_WAITING_CALIBRATION_XN:
			emit setButtonState(tr("Waiting. Click calibrates X-"), true);
			emit setProgressVisible(false);
		break;
		case STATE_CALIBRATION_XN:
			emit setButtonState(tr("Calibrating X-, please wait"), false);
			emit setProgressVisible(true);
			emit setProgressValue(0);
		break;
		case STATE_WAITING_CALIBRATION_YP:
			emit setButtonState(tr("Waiting. Click calibrates Y+"), true);
			emit setProgressVisible(false);
		break;
		case STATE_CALIBRATION_YP:
			emit setButtonState(tr("Calibrating Y+, please wait"), false);
			emit setProgressVisible(true);
			emit setProgressValue(0);
		break;
		case STATE_WAITING_CALIBRATION_YN:
			emit setButtonState(tr("Waiting. Click calibrates Y-"), true);
			emit setProgressVisible(false);
		break;
		case STATE_CALIBRATION_YN:
			emit setButtonState(tr("Calibrating Y-, please wait"), false);
			emit setProgressVisible(true);
			emit setProgressValue(0);
		break;
		case STATE_WAITING_CALIBRATION_LC:
			emit setButtonState(tr("Waiting. Click calibrates left click"), true);
			emit setProgressVisible(false);
		break;
		case STATE_CALIBRATION_LC:
			emit setButtonState(tr("Calibrating left click, please wait"), false);
			emit setProgressVisible(true);
			emit setProgressValue(0);
		break;
		case STATE_WAITING_CALIBRATION_RC:
			emit setButtonState(tr("Waiting. Click calibrates right click"), true);
			emit setProgressVisible(false);
		break;
		case STATE_CALIBRATION_RC:
			emit setButtonState(tr("Calibrating right click, please wait"), false);
			emit setProgressVisible(true);
			emit setProgressValue(0);
		break;
		case STATE_WAITING_CALIBRATION_IDLE2:
			emit setButtonState(tr("Waiting. Click calibrates idle (2)"), true);
			emit setProgressVisible(false);
		break;
		case STATE_CALIBRATION_IDLE2:
			emit setButtonState(tr("Calibrating idle (2), please wait"), false);
			emit setProgressVisible(true);
			emit setProgressValue(0);
		break;
		case STATE_RUNNING:
			emit setButtonState(tr("Running. Click resets"), true);
			emit setProgressVisible(false);
		break;
		default:
		Q_ASSERT(false);
		break;
	}
}

ProcessingEMGMouseBackpropNNClassifier::ProcessingEMGMouseBackpropNNClassifier(const ProcessingPluginDescription *description) :
	ProcessingPlugin(description)
{
	// init state
	state = STATE_WAITING_CALIBRATION_IDLE;

	// init calibration
	calibrationPos = 0;

	// create NN
	nn = new Teem::BackPropFeedForwardNeuralNetwork(EMGMouseBackpropNNInputCount, 3);
}

ProcessingEMGMouseBackpropNNClassifier::~ProcessingEMGMouseBackpropNNClassifier()
{
	delete nn;
}

Q_EXPORT_PLUGIN(ProcessingEMGMouseBackpropNNClassifierDescription)
