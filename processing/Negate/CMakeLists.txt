set(Negate_SRCS Negate.cpp)
qt4_automoc(${Negate_SRCS})
include_directories (${CMAKE_BINARY_DIR}/processing/Negate)
add_library(Negate MODULE ${Negate_SRCS})
install(TARGETS Negate DESTINATION share/osqoop/processing)