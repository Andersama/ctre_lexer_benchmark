# CMakeList.txt : CMake project for ctre_lexer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
find_package(ctre REQUIRED)

message(STATUS "ctre: ${ctre_VERSION}")
message(STATUS "ctre_DIR: ${ctre_DIR}")

include_directories(
	/
	"${ctre_DIR}/../../../include"
)

# Add source to this project's executable.
add_executable (ctre_lexer "ctre_lexer.cpp" "ctre_lexer.h" "nanobench.h" "nanobench.cpp")
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 20)

# TODO: Add tests and install targets if needed.
