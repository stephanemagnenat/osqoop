set(processing_SRCS
	BandPass2ndOrderFilter.cpp
	IIRFilter.cpp
	FeedForwardNeuralNetwork.cpp
)
qt4_automoc(${processing_SRCS})
include_directories (${CMAKE_BINARY_DIR}/processing/lib)
add_library(processing ${processing_SRCS})