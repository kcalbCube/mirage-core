cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(SDLImageStub C)
file(GLOB SDL_IMAGE_SOURCES "${CMAKE_SOURCE_DIR}/include/SDL_image/IMG*.c")

add_library(SDLImageStub ${SDL_IMAGE_SOURCES})
target_include_directories(SDLImageStub PUBLIC "${CMAKE_SOURCE_DIR}/include/SDL/include")

project(MirageCore CXX)
set(PROJECT_NAME mirage-core)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "*.h" "*.cpp")
add_library(${PROJECT_NAME} ${SOURCE_FILES})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include/magic_enum/include")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include/fmt/include")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include/entt/src")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include/SDL/include")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include/SDL_image")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include/imgui")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include/boost")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include/cpp-typelist")

target_compile_options(${PROJECT_NAME} PUBLIC
	-std=c++20 
	-Wpedantic 
	-Wall
	-Wfloat-conversion 
	-Wno-unused-but-set-variable
	-march=native
	-fno-strict-aliasing
)

target_link_libraries(${PROJECT_NAME} PUBLIC boost_serialization fmt-header-only fmtlog-static SDLImageStub)
