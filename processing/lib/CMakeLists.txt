set(processing_SRCS
	BandPass2ndOrderFilter.cpp
	IIRFilter.cpp
	FeedForwardNeuralNetwork.cpp
)
add_library(processing ${processing_SRCS})