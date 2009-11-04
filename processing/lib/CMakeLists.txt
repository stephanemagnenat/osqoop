set(processing_SRCS
	BandPass2ndOrderFilter.cpp
	IIRFilter.cpp
	FeedForwardNeuralNetwork.cpp
)
qt4_automoc(${processing_SRCS})
include_directories (${CMAKE_BINARY_DIR}/processing/lib)
add_library(processing ${processing_SRCS})
if (CMAKE_COMPILER_IS_GNUCC)
	set_target_properties(processing PROPERTIES COMPILE_FLAGS "-fPIC")
endif (CMAKE_COMPILER_IS_GNUCC)
