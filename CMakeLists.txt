cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
cmake_policy(SET CMP0075 NEW)
cmake_policy(SET CMP0076 NEW)
if (CMAKE_VERSION VERSION_GREATER "3.27.0")
  cmake_policy(SET CMP0144 NEW)
endif()

option(Xyce_USE_SUPERBUILD "Use superbuild to install Trilinos?" OFF)
if(Xyce_USE_SUPERBUILD)
  include("XyceSuperBuild.cmake")
  return()
endif()

# Set a default build type if none was specified (using the fact that
# CMAKE_CONFIGURATION_TYPES is non-zero for mutli-configuration generators).
#
# This has to be before "project" is called, because, for compilers targeting
# the MSVC ABI on Windows, "project" sets the build type.  This behavior was
# changed in CMake version 3.15, but was not made the *default* behavior.
# Since we're compatible with CMake 3.13, we can't use the new behavior. When
# the minimum CMake version requirement is increased, see CMake policy CMP0091:
#    <https://cmake.org/cmake/help/latest/policy/CMP0091.html>
# (Originally, this block was placed right before the
# "include(cmake/config.cmake)" call, below.)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  message("Setting the build type to \"Release\", since it was not explicitly set.")
  # Set the possible values of build type for ccmake and cmake-gui.
  # If a new build type is ever added to Xyce, then this list should be updated.
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "None" "Debug" "Release"
     "RelWithDebInfo" "MinSizeRel")
endif()

project(Xyce VERSION 7.11.0 LANGUAGES CXX C)

# Note: setting C++11 standard here can still be overridden by Trilinos later.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE)
  list(APPEND CMAKE_INSTALL_RPATH "@loader_path/../lib")

  # set the OSX toolkit version to be consistent with trilinos build
  #set(CMAKE_OSX_DEPLOYMENT_TARGET "12.5" CACHE STRING "" FORCE)
else()
  list(APPEND CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()
if(NOT DEFINED CMAKE_INSTALL_RPATH_USE_LINK_PATH)
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

option(BUILD_SHARED_LIBS "Build shared libraries?" OFF)
option(Xyce_GTEST_UNIT_TESTS "Build google test based unit tests" OFF)

include(cmake/config.cmake)
include(cmake/utilities.cmake)

# print out Xyce-related variables
if (CMAKE_VERSION VERSION_GREATER "3.24.0")
  cmake_language(GET_MESSAGE_LOG_LEVEL LOG_LEVEL)
  if(LOG_LEVEL STREQUAL "DEBUG")
    dump_cmake_variables("Xyce.*")
  endif()
endif()

if(Xyce_PLUGIN_SUPPORT AND (NOT BUILD_SHARED_LIBS))
  message(WARNING "Forcing BUILD_SHARED_LIBS to ON for plugin support.")
  set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries?" FORCE)
endif()

# add any compiler-specific flags all things Xyce here. at present these are:
#    * IntelLLVM C++ - requires "-fp-model=precise" (or "/fp:precise" on Windows)
#                      for passage of some xyce tests
if(WIN32)
  add_compile_options("$<IF:$<CXX_COMPILER_ID:IntelLLVM>,/fp:precise,>")
else()
  add_compile_options("$<IF:$<CXX_COMPILER_ID:IntelLLVM>,-fp-model=precise,>")
endif()

if(BUILD_SHARED_LIBS AND CMAKE_HOST_WIN32 )
  # if building a shared library under windows automatically export all
  # symbols in the resulting "dll"  This is not optimal but will work
  # until a detailed code markup is done.
  set( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON )
endif()

add_subdirectory(src)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
if(Xyce_TEST_BINARIES)
  add_subdirectory(src/test)
endif()

add_subdirectory(utils)

# add in unit tests subdirectory if it exists
if ( EXISTS ${Xyce_SOURCE_DIR}/Xyce_UnitTests)
  add_subdirectory (Xyce_UnitTests)
endif ( EXISTS ${Xyce_SOURCE_DIR}/Xyce_UnitTests)

# Enable Xyce plugin capability
if(Xyce_PLUGIN_SUPPORT)
  find_program(ADMS_XML admsXml)

  if(ADMS_XML)
    configure_file(
      utils/buildxyceplugin.cmake.in
      buildxyceplugin.sh
      @ONLY)
    configure_file(
      utils/XycePluginProject.cmake.in
      XycePluginProject.cmake
      @ONLY)

    install(
      PROGRAMS
        ${ADMS_XML}
        ${CMAKE_BINARY_DIR}/buildxyceplugin.sh
      DESTINATION bin)

    install(
      FILES
        utils/ADMS/adms.implicit.xml
        utils/ADMS/xyceVersion_nosac.xml
        utils/ADMS/xyceBasicTemplates_nosac.xml
        utils/ADMS/xyceAnalogFunction_nosac.xml
        utils/ADMS/xyceHeaderFile_nosac.xml
        utils/ADMS/xyceImplementationFile_nosac.xml
        ${CMAKE_BINARY_DIR}/XycePluginProject.cmake
      DESTINATION share)

    install(
      DIRECTORY utils/ADMS/examples/toys
      DESTINATION share/examples)

    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/utils/ADMS)
    configure_file(${CMAKE_SOURCE_DIR}/utils/ADMS/CMakeLists.txt ${CMAKE_BINARY_DIR}/utils/ADMS/CMakeLists.txt @ONLY )
    get_target_property(XYCE_INCLUDE_DIRECTORIES XyceLib INCLUDE_DIRECTORIES)
    set(XYCE_CXX_COMPILER ${CMAKE_CXX_COMPILER})
    configure_file(${CMAKE_SOURCE_DIR}/utils/ADMS/InPlaceTesting/CMakeLists.txt ${CMAKE_BINARY_DIR}/utils/ADMS/InPlaceTesting/CMakeLists.txt @ONLY )
    install(
      FILES
        ${CMAKE_BINARY_DIR}/utils/ADMS/CMakeLists.txt
      DESTINATION share)

    message(STATUS "Plugin compatibility enabled")
  else()
    message(WARNING "ADMS not found.  Disabling the plugin capability.")
    set(Xyce_PLUGIN_SUPPORT OFF CACHE BOOL "Install Xyce with plugin compatibility" FORCE)
  endif()
else()
  message(STATUS "Plugin compatibility not enabled")
endif()

# if no generator type is specified, use RPM
set(GEN_TYPE "RPM" CACHE STRING "What generator to use.")

# generate the package name for cpack
string(REPLACE ":" ";" verList ${Xyce_XyceVERSION})
list(GET verList 0 develOrRelease)
list(GET verList 1 develPhase)
list(GET verList 2 versionNum)

if(develOrRelease STREQUAL "D")
  set(develOrRelease "DEVELOPMENT")
else()
  set(develOrRelease "Release")
endif()
if(develPhase STREQUAL "")
  set(develPhaseStr "")
else()
  set(develPhaseStr "-${develPhase}")
endif()

# if the user didn't specify it set the install directory according to
# the CPACK_PACKAGE_NAME
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr/local/${CPACK_PACKAGE_NAME}" CACHE PATH "Installation Directory" FORCE)
endif()
message(DEBUG "[DBG]: CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")

# this causes the creation of CPackConfig.cmake and CPackSourceConfig.cmake 
# logic for setting package options is set based on 
# cmake/feature_modes.cmake
# cmake/CPackConfig.cmake
include(CPack)
#See the CPackConfig.cmake in Xyce/cmake for settings
