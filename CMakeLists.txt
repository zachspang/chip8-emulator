# CMakeList.txt : CMake project for chip8-emulator, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
project ("chip8-emulator")

set(SDL2_DIR "${CMAKE_SOURCE_DIR}/lib/SDL2-2.32.10/cmake")
find_package(SDL2 REQUIRED)

# Add source to this project's executable.
add_executable (chip8-emulator "src/chip8-emulator.cpp" "src/cpu.cpp")

target_link_libraries(chip8-emulator SDL2::SDL2 SDL2::SDL2main)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET chip8-emulator PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
