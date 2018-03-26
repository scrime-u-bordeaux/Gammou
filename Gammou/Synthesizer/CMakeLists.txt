
cmake_minimum_required (VERSION 3.1)

#######################################################
project(View LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
 
add_subdirectory("../Process" Process)
 
set(SYNTHESIZER_SRC
    synthesizer.h
	synthesizer.cpp
	synthesizer_circuits/master_circuit.h
	synthesizer_circuits/master_circuit.cpp
	synthesizer_circuits/polyphonic_circuit.h
	synthesizer_circuits/polyphonic_circuit.cpp
)

set(SYNTHESIZER_BUILTIN_COMPONENTS_SRC
	builtin_components.h
	builtin_components/builtin_components_ids.h
	builtin_components/sound_processing_helper.h
	builtin_components/sound_processing_helper.cpp
	builtin_components/adsr_env.h
	builtin_components/adsr_env.cpp
	builtin_components/lp2.h
	builtin_components/lp2.cpp
	builtin_components/product_component.cpp
	builtin_components/product_component.h
	builtin_components/saw.h
	#builtin_components/saw.cpp
	builtin_components/sin_component.h
	builtin_components/sin_component.cpp
	builtin_components/sum_component.h
	builtin_components/sum_component.cpp
)

set(PLUGIN_MANAGEMENT_SRC
	plugin_management/abstract_plugin_factory.h
	plugin_management/abstract_plugin_factory.cpp
	plugin_management/data_stream.h
	plugin_management/main_factory.h
	plugin_management/main_factory.cpp
	plugin_management/request_form.h
	plugin_management/request_form.cpp
)
 
set(SOUND_COMPONENT_SRC
	sound_component/multi_channel_data_container.h
	sound_component/multi_channel_data_container.cpp
	sound_component/sound_component.h
	sound_component/sound_component.cpp
	sound_component/sound_component_manager.h
	sound_component/sound_component_manager.cpp
)

add_library(synthesizer STATIC
	${SYNTHESIZER_SRC}
	${SYNTHESIZER_BUILTIN_COMPONENTS_SRC}
	${PLUGIN_MANAGEMENT_SRC}
	${SOUND_COMPONENT_SRC}
)

target_link_libraries(synthesizer PUBLIC process)
set_property(TARGET synthesizer PROPERTY POSITION_INDEPENDENT_CODE ON)  # Allow plugins to be built
           
if(UNIX)  # LINUX and APPLE OSX
	target_link_libraries(synthesizer PRIVATE m dl)	#	math.h and dlopen
endif()