cmake_minimum_required( VERSION 3.2 )

set (CMAKE_CXX_STANDARD 17)
 
project( BoostPythonMakeData )

# Default to release build
if (NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
  if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
  endif()
endif()

# Find necessary packages
find_package( PythonLibs 3 REQUIRED )
include_directories( ${PYTHON_INCLUDE_DIRS} )
MESSAGE(STATUS "Pythonlibs: " ${PYTHON_INCLUDE_DIRS})

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

##find_package(Boost REQUIRED COMPONENTS numpy)

LINK_LIBRARIES(${Boost_LIBRARIES} ${PYTHON_LIBRARIES}) # Deprecated but so convenient!

# Define the general Enge analyzer library
add_library( EngeAnalyzer SHARED EngeAnalyzerlib.cpp )

# Multi Card Crap
add_library( MDPPevt SHARED MDPPEventHandler.cpp )
add_library( tomlread SHARED mdpp_module_config.cpp )

# Build our Sort Routine
add_library( MDPPSort SHARED MDPPSort.cpp)

set_target_properties(MDPPSort tomlread MDPPevt EngeAnalyzer PROPERTIES SUFFIX ".so")

if(APPLE)
target_link_libraries( MDPPSort MDPPevt tomlread ${Boost_LIBRARIES} EngeAnalyzer
  librootana.a -lz ${ROOT_LIBRARIES}  libmidas.a )
else()
target_link_libraries( MDPPSort ${Boost_LIBRARIES} EngeAnalyzer MDPPevt tomlread
  librootana.a -lz ${ROOT_LIBRARIES}  libmidas.a -lrt )    
endif()
