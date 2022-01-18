cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(onepointninegdpsmod VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED src/dllmain.cpp src/explorer.cpp src/state.cpp src/menu.cpp)

add_subdirectory(libraries/minhook)
add_subdirectory(libraries/imgui-hook)
add_subdirectory(libraries/cocos-headers)

target_include_directories(${PROJECT_NAME} PRIVATE
  libraries/minhook/include
  libraries/mat-dash/include
  libraries/imgui-hook
  libraries/imgui-hook/imgui
)

target_link_libraries(${PROJECT_NAME}
  minhook imgui-hook cocos2d ${CMAKE_SOURCE_DIR}/libraries/glew32.lib
)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})