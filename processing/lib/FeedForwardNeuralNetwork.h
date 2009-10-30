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

#ifndef _FEEDFORWARDNN_H
#define _FEEDFORWARDNN_H

#include <valarray>
#include <vector>
#include <string>
#include "Matrix.h"

/*!	\file FeedForwardNeuralNetwork.h
	\brief Interface of feed-forward only neural networks
*/

namespace Teem
{
	//! Class that implement a simple multilayer feedforward neural network
	//! without using the abstract NN architecture.
	//! \ingroup controllers
	class FeedForwardNeuralNetwork
	{
	public:
		//! Constructor, create a neural network with inputCount inputs and outputCount outputs
		FeedForwardNeuralNetwork(size_t inputCount, size_t outputCount, unsigned hiddenLayerCount = 0, const size_t *hidderLayerSizes = NULL, double biasValue = -1, const char *activationFunction = "tanh", double activationFunctionParameter = 1.0);
		//! Destructor
		virtual ~FeedForwardNeuralNetwork() { }
		
		//! Set the input values to the neural network.
		void setInput(unsigned index, double val);
		//! Return the input value.
		double getInput(unsigned index) const { assert(index < inputCount); return input[index]; }
		//! Returns the number of input.
		unsigned getInputCount() const { return inputCount; }
		//! Read the output values.
		double getOutput(unsigned index);
		//! Propagate the input values to the output through all the layers.
		void step();
		
		//! Return the number of layers.
		size_t layerNum() { return layerCount; }
		//! Return the size of the nth layer. Layer 0 is the fist layer after input.
		size_t layerSize(size_t layer) { return layerSizes[layer]; }
		//! Return the number of input.
		size_t inputNum() { return inputCount; }
		//! Return the number of output.
		size_t outputNum() { return outputCount; }
		
		//! Set the weight of a particular synapse to a particular layer. Using layer 0,
		//! synpases from input to layer 0 are set.
		void setWeight(size_t toLayer, size_t from, size_t to, double w) { weights[toLayer](from, to) = w; }
		//! Get the value for a particular weight.
		double getWeight(size_t toLayer, size_t from, size_t to) const { return weights[toLayer](from, to); }
		//! Return the width (number of columns) of the weights matrix for a given layer
		size_t getWeightsMatrixWidth(size_t layer) { return weights[layer].columnNum(); }
		//! Return the height (number of rows) of the weights matrix for a given layer
		size_t getWeightsMatrixHeight(size_t layer) { return weights[layer].rowNum(); }
		
		//! Set weight from bias to neuron.
		void setBiasWeight(size_t toLayer, size_t to, double w) { biasWeights[toLayer][to] = w; }
		//! Get weight from bias to neuron.
		double getBiasWeight(size_t toLayer, size_t to) const { return biasWeights[toLayer][to]; }
		
		//! Put random weights with uniform distribution.
		void randomize(double from, double to);
	
	protected:
		
		//! Activation function y = g(x)
		//! @param x activation
		//! @param b activation function parameter (slope)
		//! @return output
		typedef double (*ActivationFunction)(double x, double b);
	
		class ActivationFunctor;
		
		size_t inputCount; //!< number of input
		size_t outputCount; //!< number of output
		size_t layerCount; //!< number of layer
		unsigned hiddenLayerCount;  //!< number of hidden layer
		std::vector<size_t> layerSizes;  //!< size of layers
		double biasValue;  //!< value of the bias neuron
		std::string activationFunction;  //!< name of the activation function
		double activationFunctionParameter;  //!< parameter for the activation function
		
		std::vector<Matrix<double> > weights;  //!< weight matrix for each layer
		std::vector<std::valarray<double> > biasWeights;  //!< weight from the bias to each layer
		std::vector<std::valarray<double> > activations;  //!< activation of each neuron
		std::vector<std::valarray<double> > outputs;  //!< output of each neuron
		std::valarray<double> input;  //!< input vector
		ActivationFunction forwardActFunc;  //!< pointer to activation function
		
		//! Tanh activation function.
		static double TanhForwardActivationFunction(double x, double b)
		{
			return tanh(x * b);
		}
	};
	
	//! Functor to compute activation function on std::valarray<double>
	//! unsing std::for_each.
	class FeedForwardNeuralNetwork::ActivationFunctor
	{
	protected:
		ActivationFunction function;  //!< pointer to activation function
		double b;  //!< activation function parameter
		std::valarray<double>& result;  //!< array to hold result
		size_t pos;  //!< position in the result array
		
	public:
		//! Constructor, use function func and parameter b, and return result in res and 
		ActivationFunctor(ActivationFunction func, std::valarray<double> &res, double b)
			: result(res)
		{
			this->function = func;
			this->pos = 0;
			this->b = b;
		}
		
		//! Do the operation, store value in result[pos] and increment pos
		void operator()(double x)
		{
			result[pos++] = function(x, b);
		}
	};
	
	
	/**
	Feed forward neural network with online back-propagation. Use that way:
	
\code
for i in inputs
	nn.setInput(i, inputValue);
nn.step();
for i in outputs
	nn.setError(i, errorValue);
nn.stepBackward();
\endcode
	\ingroup controllers
	*/
	class BackPropFeedForwardNeuralNetwork : public FeedForwardNeuralNetwork
	{
	public:
		//! Constructor, create a feed forward neural network with online back-propagation with inputCount inputs and outputCount outputs
		BackPropFeedForwardNeuralNetwork(size_t inputCount, size_t outputCount, double learningRate = 0.1);
		//! Destructor
		virtual ~BackPropFeedForwardNeuralNetwork() { }
		
		//! Backpropagate the error on the weight.
		void stepBackward();
		
		//! Set the output error (desired value minus obtained value)
		void setError(size_t index, double val)
		{
			assert(index < error.size());
			error[index] = val;
		}
		
		//! Set the learning rate value.
		void setLearningRate(double rate) { learningRate = rate; }
		//! Get the learning rate value.
		double getLearningRate() { return learningRate; }
	
	protected:
		
		double learningRate;  //!< learning rate constant
		std::valarray<double> error;  //!< desired output
		ActivationFunction backwardActFunc;  //!< derivative of the activation function
		std::vector<std::valarray<double> > deltas;  //!< deltas (see backprop algorithm)
	
		//! Tanh backward activation function.
		static double TanhBackwardActivationFunction(double x, double b)
		{
			double t = tanh(x * b);
			return b * (1.0 - t * t);
		}
	};
}

#endif
