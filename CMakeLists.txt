# CMakeLists.txt for libyui-mga-gtk
#
# Usage:
#
#   mkdir build
#   cd build
#   cmake ..
#
#   make
#   sudo make install
#
# Restart with a clean build environment:
#   rm -rf build
#
# Show the complete compiler commands with all arguments:
#   make VERBOSE=1

cmake_minimum_required( VERSION 3.17 )
project( libyui-mga-gtk )

# Options usage:
#
#   cmake -DBUILD_DOC=on -DBUILD_EXAMPLES=off ..

option( BUILD_SRC         "Build in src/ subdirectory"                on )
option( BUILD_DOC         "Build class documentation"                 off )
option( WERROR            "Treat all compiler warnings as errors"     off )

# Non-boolean options
set( DOC_DESTDIR "" CACHE STRING "Destination directory prefix for installing docs" )

#----------------------------------------------------------------------


set( CMAKE_INSTALL_MESSAGE LAZY ) # Suppress "up-to-date" messages during "make install"

add_compile_options( "-Wall" )
IF (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  # Initialize compiler flags for all targets in all subdirectories
  add_compile_options( "-Os" )    # Optimize for size (overrides CMake's -O3 in RELEASE builds)
endif()

if ( WERROR )
  add_compile_options( "-Werror" )
endif()


#
# Descend into subdirectories
#

if ( BUILD_SRC )
  add_subdirectory( src )
endif()

# TODO
#if ( BUILD_DOC )
#  add_subdirectory( doc )
#endif()
