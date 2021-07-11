# CMakeLists.txt for libyui-qt/src

include( ../VERSION.cmake )
include( GNUInstallDirs )       # set CMAKE_INSTALL_INCLUDEDIR, ..._LIBDIR

# Use the package PkgConfig to detect GTK+ headers/library files
FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)

PKG_CHECK_MODULES(YUI REQUIRED libyui)
pkg_get_variable(YUI_SO_VERSION libyui soversion)
pkg_get_variable(YUI_SO_MAJOR libyui soversion_major)
#pkg_get_variable(YUI_SO_MINOR libyui soversion_minor)
#pkg_get_variable(YUI_SO_PATCH libyui soversion_patch)

message (STATUS "Using ${YUI_LIBRARY_DIRS}/libyui.so.${YUI_SO_VERSION}")
find_package(Boost COMPONENTS system filesystem REQUIRED)

##### This is needed to be set for the libyui core
SET( SONAME_MAJOR ${YUI_SO_MAJOR} )
SET( SONAME ${YUI_SO_VERSION} )

PKG_CHECK_MODULES(YUIGTK REQUIRED libyui-gtk)
PKG_CHECK_MODULES(YUIMGA REQUIRED libyui-mga)

#
# libyui plugin specific
#

set( TARGETLIB          libyui-mga-gtk )
set( TARGETLIB_BASE     yui-mga-gtk    )

set( HEADERS_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/yui/mga/gtk )
set( PLUGIN_DIR          ${CMAKE_INSTALL_LIBDIR}/yui ) # /usr/lib64/yui

# if DESTDIR is set, CMAKE_INSTALL_INCLUDEDIR already contains it
# during "make install" (but not for other make targets!):
#
#    sudo make install DESTDIR=/work/foo
# or
#    DESTDIR=/work/foo sudo make install
#
# -> the include files are installed to /work/foo/usr/include/...
# We need that for RPM builds to install everything to $RPM_BUILD_ROOT.


set( SOURCES
  YGWE.cc
  YMGAGMenuBar.cc
  YMGA_GCBTable.cc
  YMGAGWidgetFactory.cc
  )


set( HEADERS
  YGWE.h
  YMGAGMenuBar.h
  YMGA_GCBTable.h
  YMGAGWidgetFactory.h
  )


# Add shared lib to be built
add_library( ${TARGETLIB} SHARED
  ${SOURCES}
  ${HEADERS}
  )


# Include directories and compile options
#

# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS} ${YUI_INCLUDE_DIRS} ${YUIMGA_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})


# Make the version from ../../VERSION.cmake available as a #define
target_compile_definitions( ${TARGETLIB} PUBLIC VERSION="${VERSION}" )


#
# Linking
#

# https://cmake.org/cmake/help/latest/command/link_directories.html suggests to use target_link_libraries
# and anyway LINK_DIRECTORIES command will apply only to targets created after it is called, so must be set
# before add_library in the case.
target_link_directories( ${TARGETLIB}
  PUBLIC ${YUI_LIBRARY_DIRS}
  PUBLIC ${YUIGTK_LIBRARY_DIRS}
  PUBLIC ${YUIMGA_LIBRARY_DIRS}
  PUBLIC ${GTK3_LIBRARY_DIRS}
)


# Libraries that are needed to build this shared lib
#
# If in doubt what is really needed, check with "ldd -u" which libs are unused.
target_link_libraries( ${TARGETLIB}
  ${YUI_LIBRARIES}
  ${YUIGTK_LIBRARIES}
  ${YUIMGA_LIBRARIES}
  ${GTK3_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  )


# https://cmake.org/cmake/help/latest/manual/cmake-properties.7.html#target-properties
set_target_properties( ${TARGETLIB} PROPERTIES
  VERSION       ${SONAME}           # From ../../VERSION.cmake
  SOVERSION     ${SONAME_MAJOR}     # From ../../VERSION.cmake
  OUTPUT_NAME   ${TARGETLIB_BASE}
  )


#
# Install
#

# Install the headers first so the message about the lib does not scroll away
install( FILES   ${HEADERS}   DESTINATION ${HEADERS_INSTALL_DIR} )
install( TARGETS ${TARGETLIB} LIBRARY DESTINATION ${PLUGIN_DIR} )
