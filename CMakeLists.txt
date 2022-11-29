cmake_minimum_required(VERSION 3.7)

project(NewtonFractal)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} graphics/graphics.h graphics/graphics.cpp main.cpp)
file(COPY resources/ DESTINATION resources/)
target_link_libraries(${PROJECT_NAME}  ${SDL2_LIBRARIES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)
