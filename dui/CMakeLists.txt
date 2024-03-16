project(libdui)

file(GLOB LIBDUI_SRC 
	"dui.cpp" 
	"duiwin32.cpp" 
	"dui_render.c" 
	)

add_library(${PROJECT_NAME} ${LIBDUI_SRC})
set_property(TARGET ${PROJECT_NAME} PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_include_directories(${PROJECT_NAME} 
    PUBLIC ${PROJECT_SOURCE_DIR} 
           ${CMAKE_SOURCE_DIR}
           )

