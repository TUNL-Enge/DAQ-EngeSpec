cmake_minimum_required( VERSION 2.8 )
 
project( BoostPythonMakeData )
 
# Find necessary packages
find_package( PythonLibs 3 REQUIRED )
include_directories( ${PYTHON_INCLUDE_DIRS} )
MESSAGE(STATUS "Pythonlibs: " ${PYTHON_INCLUDE_DIRS})

find_package( Boost COMPONENTS numpy3 REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )
MESSAGE(STATUS "Boost: " ${Boost_INCLUDE_DIR})

find_package( ROOT COMPONENTS Gui XMLParser XMLIO RHTTP REQUIRED )
include_directories( ${ROOT_INCLUDE_DIRS} )
MESSAGE(STATUS "ROOT: " ${ROOT_INCLUDE_DIRS})

set(MIDASSYS "$ENV{MIDASSYS}")
include_directories( ${MIDASSYS}/include )
link_directories( ${MIDASSYS}/linux/lib )
MESSAGE(STATUS "MIDAS: " ${MIDASSYS}/include)

set(ROOTANASYS "$ENV{ROOTANASYS}")
include_directories( ${ROOTANASYS}/include )
link_directories( ${ROOTANASYS}/lib )
MESSAGE(STATUS "ROOTANA: " ${ROOTANASYS}/include)

##find_package(Boost REQUIRED COMPONENTS numpy)
 
# Build our library
add_library( MakeData SHARED MakeData.cpp )
add_library( EngeAnalyzer SHARED EngeAnalyzer.cpp)
 
# Define the wrapper library that wraps our library
add_library( libMakeData SHARED MakeDatalib.cpp )
add_library( libEngeAnalyzer SHARED EngeAnalyzerlib.cpp )
target_link_libraries( libMakeData ${Boost_LIBRARIES} MakeData )
target_link_libraries( libEngeAnalyzer ${Boost_LIBRARIES} EngeAnalyzer
  librootana.a ${ROOT_LIBRARIES}  libmidas.a )
# don't prepend wrapper library name with lib
set_target_properties( libMakeData PROPERTIES PREFIX "../" )
set_target_properties( libEngeAnalyzer PROPERTIES PREFIX "../" )
