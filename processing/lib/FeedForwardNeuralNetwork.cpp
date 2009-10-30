/*
    this file is part of teem, an open source evolutionary robotics framework
	see http://lis.epfl.ch/resources/teem for more informations
    Copyright (C) 1999-2005 Stephane Magnenat <stephane at magnenat dot net>
    Copyright (c) 2004-2005 Antoine Beyeler <antoine dot beyeler at epfl dot ch>
    Copyright (C) 2005 Laboratory of Intelligent Systems, EPFL, Lausanne
    See AUTHORS for details

    This program is free software. You can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "Random.h"
#include <algorithm>

#include "FeedForwardNeuralNetwork.h"

/*!	\file FeedForwardNeuralNetwork.cpp
	\brief Implementation of feed-forward only neural networks
*/

namespace Teem
{
	FeedForwardNeuralNetwork::FeedForwardNeuralNetwork(size_t inputCount, size_t outputCount, unsigned hiddenLayerCount, const size_t *hidderLayerSizes, double biasValue, const char *activationFunction, double activationFunctionParameter) :
		hiddenLayerCount(hiddenLayerCount),
		biasValue(biasValue),
		activationFunction(activationFunction),
		activationFunctionParameter(activationFunctionParameter)
	{
		assert(inputCount > 0);
		
		this->inputCount = inputCount;
		this->outputCount = outputCount;
		
		// setup variable for layers
		if (hiddenLayerCount)
			assert(hidderLayerSizes);
		for(unsigned i = 0; i < hiddenLayerCount; i++)
			layerSizes.push_back(hidderLayerSizes[i]);
		layerSizes.push_back(outputCount);
		layerCount = hiddenLayerCount + 1;
		
		// init state variables
		for(unsigned i = 0; i < layerCount; i++)
		{
			unsigned layerInCount = (i == 0 ? inputCount : layerSizes[i-1]);
			unsigned layerOutCount = layerSizes[i];
			weights.push_back(Matrix<double>(layerInCount, layerOutCount));
			biasWeights.push_back(std::valarray<double>(layerOutCount));
			activations.push_back(std::valarray<double>(layerOutCount));
			outputs.push_back(std::valarray<double>(layerOutCount));
		}
		
		input.resize(inputCount);
		
		std::string s = activationFunction;
		if (s == "tanh")
			forwardActFunc = &TanhForwardActivationFunction;
		else
			assert(false);
	}
	
	void FeedForwardNeuralNetwork::setInput(unsigned index, double in)
	{
		assert(index < input.size());
		
		input[index] = in;
	}
	
	double FeedForwardNeuralNetwork::getOutput(unsigned index)
	{
		assert(index < outputs[outputs.size()-1].size());
		
		return outputs[outputs.size()-1][index];
	}

	void FeedForwardNeuralNetwork::step()
	{
		// for each layer...
		for (unsigned i = 0; i < layerCount; i++)
		{
			// output from previous layer
			std::valarray<double> &layerInput = (i == 0 ? input : outputs[i-1]);
			
			// compute activation for this layer
			activations[i] = weights[i] * layerInput;
			activations[i] += biasWeights[i] * ((double) biasValue);

			// compute output for this layer
			std::for_each(&activations[i][0], &activations[i][activations[i].size()], ActivationFunctor(forwardActFunc, outputs[i], activationFunctionParameter));
		}
	}
	
	void FeedForwardNeuralNetwork::randomize(double from, double to)
	{
		for(size_t i = 0; i < weights.size(); i++)
		{
			std::generate(&weights[i].flat()[0], &weights[i].flat()[weights[i].flat().size()], An::UniformRand(from, to));
			std::generate(&biasWeights[i][0], &biasWeights[i][biasWeights[i].size()], An::UniformRand(from, to));
		}
	}
	
	// ======================================================
	// ======================================================
	
	BackPropFeedForwardNeuralNetwork::BackPropFeedForwardNeuralNetwork(size_t inputCount, size_t outputCount, double learningRate) :
		FeedForwardNeuralNetwork(inputCount, outputCount),
		learningRate(learningRate)
	{
		// init state variables
		for(unsigned i = 0; i < layerCount; i++)
			deltas.push_back(std::valarray<double>( layerSizes[i]));
		
		error.resize(outputCount);
		
		std::string s = activationFunction;
		if(s == "tanh")
			backwardActFunc = &TanhBackwardActivationFunction;
		else
			assert(false);
	}
	
	void BackPropFeedForwardNeuralNetwork::stepBackward()
	{	
		// Compute deltas for output layer
		size_t idx = activations.size()-1;
		std::for_each(&activations[idx][0], &activations[idx][activations[idx].size()], ActivationFunctor(backwardActFunc, deltas[idx], activationFunctionParameter));
		//deltas[idx] += 0.1;	// suggested by Fahlman (1989), as mentionned in Herz, Krogh & Palmer, pg. 122
		deltas[idx] *= error;
		
		// Compute deltas for every other layers
		for(int i = idx-1; i >= 0; i--)
		{
			std::for_each(&activations[i][0], &activations[i][activations[i].size()], ActivationFunctor(backwardActFunc, deltas[i], activationFunctionParameter));
			deltas[i] *= deltas[i+1] * weights[i+1];
		}
		
		// Update all weights
		for(unsigned i = 0; i < weights.size(); i++)
		{
			Matrix<double> &W = weights[i];
			size_t fromCount = W.columnNum();
			size_t toCount = W.rowNum();
			std::valarray<double> &fromLayer = (i == 0 ? input : outputs[i-1]);
			
			// update weights coming from previous layer
			for(size_t to = 0; to < toCount; to++)
				for(size_t from = 0; from < fromCount; from++)
				{
					W(from, to) += learningRate * deltas[i][to] * fromLayer[from];
				}
			
			// update weights coming from bias.
			biasWeights[i] += learningRate * biasValue * deltas[i];
		}
	}
}
