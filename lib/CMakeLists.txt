cmake_minimum_required( VERSION 3.2...3.31 )

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "-O3")

## Only add -lrt if it's not Mac
if(NOT APPLE)
link_libraries(rt)
endif()
 
project( BoostPythonMakeData )

# Default to release build
if (NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
  if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
  endif()
endif()

# Find necessary packages
find_package( Python3 REQUIRED COMPONENTS Development )
include_directories( ${Python3_INCLUDE_DIRS} )
MESSAGE(STATUS "Python: " ${Python3_INCLUDE_DIRS})

find_package( Boost COMPONENTS numpy REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )
MESSAGE(STATUS "Boost: " ${Boost_INCLUDE_DIR})

find_package( ROOT COMPONENTS Gui XMLParser XMLIO RHTTP )
include_directories( ${ROOT_INCLUDE_DIRS} )
MESSAGE(STATUS "ROOT: " ${ROOT_INCLUDE_DIRS})

set(MIDASSYS "$ENV{MIDASSYS}")
include_directories( ${MIDASSYS}/include )
link_directories( ${MIDASSYS}/lib )
MESSAGE(STATUS "MIDAS: " ${MIDASSYS}/include)

set(ROOTANASYS "$ENV{ROOTANASYS}")
include_directories( ${ROOTANASYS}/include )
link_directories( ${ROOTANASYS}/lib )
MESSAGE(STATUS "ROOTANA: " ${ROOTANASYS}/include)

LINK_LIBRARIES(${Boost_LIBRARIES} ${PYTHON_LIBRARIES}) # Deprecated but so convenient!
## Still unable to statically link these on Linux.
# Define the general Enge analyzer library
add_library( EngeAnalyzer SHARED EngeAnalyzerlib.cpp )

# Build the standard Enge sort routine
add_library( EngeSort SHARED EngeSort.cpp)
target_link_libraries( EngeSort ${Boost_LIBRARIES} EngeAnalyzer 
  librootana.a -lz ${ROOT_LIBRARIES} libmidas.a ) 

# Build the standard LENA sort routine
# Multi Card Crap
add_library( MDPPevt SHARED MDPPEventHandler.cpp )
add_library( tomlread SHARED mdpp_module_config.cpp )
add_library( LenaSort SHARED LenaSort.cpp)
# Ensure .so extension, which is needed by python when it imports the module.
set_target_properties(LenaSort tomlread MDPPevt EngeAnalyzer PROPERTIES SUFFIX ".so")
# The extra target is just so CMake does not screw up the dependencies.
target_link_libraries( MDPPevt ${Boost_LIBRARIES} EngeAnalyzer tomlread
  librootana.a -lz ${ROOT_LIBRARIES}  libmidas.a )
target_link_libraries( LenaSort ${Boost_LIBRARIES} EngeAnalyzer MDPPevt tomlread
  librootana.a -lz ${ROOT_LIBRARIES}  libmidas.a )

## Include all of the user files
#include( CMakeUserLists.txt )
