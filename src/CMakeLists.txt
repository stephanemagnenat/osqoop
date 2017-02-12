set(osqoop_SRCS
	SignalDisplayData.cpp
	SignalViewWidget.cpp
	DataConverter.cpp
	OscilloscopeWindow.cpp
	Osqoop.cpp
	Utilities.cpp
	ProcessingPluginDialog.cpp
)
qt5_add_resources(osqoop_RESS Osqoop.qrc)
add_executable(osqoop ${osqoop_SRCS} ${osqoop_RESS})
include_directories (${CMAKE_SOURCE_DIR}/processing/lib)
target_link_libraries(osqoop Qt5::Core Qt5::Gui Qt5::Widgets Qt5::PrintSupport ${QWT_LIBRARY})
install(TARGETS osqoop RUNTIME DESTINATION bin)
