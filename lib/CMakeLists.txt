cmake_minimum_required( VERSION 3.2 )

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "-O3")

## Only add -lrt if it's not Mac
if(NOT APPLE)
link_libraries(rt)
endif()
 
project( BoostPythonMakeData )
 
# Find necessary packages
#find_package( PythonInterp 3 REQUIRED )
find_package( PythonLibs 3 REQUIRED )
#set(PYTHON_LIBRARIES /opt/anaconda3/lib/libpython3.7m.so)
include_directories( ${PYTHON_INCLUDE_DIRS} )
MESSAGE(STATUS "Pythonlibs: " ${PYTHON_INCLUDE_DIRS})

find_package( Boost COMPONENTS numpy REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )
MESSAGE(STATUS "Boost: " ${Boost_INCLUDE_DIR})

find_package( ROOT COMPONENTS Gui XMLParser XMLIO RHTTP )
include_directories( ${ROOT_INCLUDE_DIRS} )
MESSAGE(STATUS "ROOT: " ${ROOT_INCLUDE_DIRS})
#set(NO_ROOT "1")

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

#include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/lib )

# Define the general Enge analyzer library
add_library( EngeAnalyzer SHARED EngeAnalyzerlib.cpp )

# Build our Sort Routine
add_library( EngeSort SHARED EngeSort.cpp)

# Build our Sort Routine
add_library( v1730Sort SHARED v1730Sort.cpp)

# Build our Sort Routine
add_library( MDPPSort_CI SHARED MDPPSort_CI.cpp)

# Build our Sort Routine
add_library( MDPPSort SHARED MDPPSort.cpp)

# Build our Sort Routine
add_library( SiliconSort SHARED SiliconSort.cpp)

# Build our Sort Routine
add_library( RBSSort SHARED RBSSort.cpp)


target_link_libraries( EngeSort ${Boost_LIBRARIES} EngeAnalyzer 
  librootana.a -lz ${ROOT_LIBRARIES} libmidas.a ) 

target_link_libraries( v1730Sort ${Boost_LIBRARIES} EngeAnalyzer 
  librootana.a -lz ${ROOT_LIBRARIES} libmidas.a )

target_link_libraries( MDPPSort ${Boost_LIBRARIES} EngeAnalyzer 
  librootana.a -lz ${ROOT_LIBRARIES} libmidas.a )

target_link_libraries( SiliconSort ${Boost_LIBRARIES} EngeAnalyzer 
  librootana.a -lz ${ROOT_LIBRARIES} libmidas.a )

target_link_libraries( RBSSort ${Boost_LIBRARIES} EngeAnalyzer 
  librootana.a -lz ${ROOT_LIBRARIES} libmidas.a )
