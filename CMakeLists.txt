cmake_minimum_required(VERSION 3.15.0)
project(opengl_tutorial VERSION 0.1.0)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -pedantic -static")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4")
endif()

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)

# Dependencies
find_package(glfw3 CONFIG REQUIRED)
find_package(glbinding CONFIG REQUIRED)

link_libraries(glfw)
link_libraries(glbinding::glbinding)

# Source code
include_directories(src/common)
file(GLOB_RECURSE common_src
          src/common/${child_dir}/*.h
          src/common/${child_dir}/*.cpp
          )

include(subdirlist.cmake)
SUBDIRLIST(children ${CMAKE_CURRENT_SOURCE_DIR}/src/executables)

# Create a target for each tutorial
foreach(child_dir ${children})
    file(GLOB_RECURSE source_files
        src/executables/${child_dir}/*.h
        src/executables/${child_dir}/*.cpp
        )
    add_executable(${child_dir} ${source_files} ${common_src})
    set(output_dir ${CMAKE_BINARY_DIR}/bin/${child_dir}/${CMAKE_BUILD_TYPE})
    set_target_properties(${child_dir}
                        PROPERTIES
                        RUNTIME_OUTPUT_DIRECTORY ${output_dir})

    # Create a symlink to the res directory next to the executable
    add_custom_command(TARGET ${child_dir} POST_BUILD
                    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/postbuild.py
                            ${CMAKE_CURRENT_SOURCE_DIR} ${output_dir})
endforeach()