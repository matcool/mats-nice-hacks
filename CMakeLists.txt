cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(matsnicehacks VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED
  src/dllmain.cpp
  src/explorer.cpp
  src/state.cpp
  src/menu.cpp
  src/preview-mode.cpp
  src/save-file.cpp
  src/lvl-share.cpp
  libraries/nfd/src/nfd_common.c
  libraries/nfd/src/nfd_win.cpp
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX UNICODE _UNICODE)

# this is so stupid
set(CMAKE_SIZEOF_VOID_P 4)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
  # ensure 32 bit on clang
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -target i386-pc-windows-msvc ")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -target i386-pc-windows-msvc ")
endif()

add_subdirectory(libraries/minhook)
add_subdirectory(libraries/imgui-hook)
add_subdirectory(libraries/cocos-headers)

target_include_directories(${PROJECT_NAME} PRIVATE
  libraries/minhook/include
  libraries/mat-dash/include
  libraries/imgui-hook
  libraries/imgui-hook/imgui
  libraries
  libraries/nfd/src/include
)

target_link_libraries(${PROJECT_NAME}
  minhook imgui-hook cocos2d ${CMAKE_SOURCE_DIR}/libraries/glew32.lib
)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
