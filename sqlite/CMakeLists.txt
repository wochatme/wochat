project(libsqlite)

file(GLOB LIBSQLITE_SRC "sqlite3.c")

add_library(${PROJECT_NAME} ${LIBSQLITE_SRC})
set_property(TARGET ${PROJECT_NAME} PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})

