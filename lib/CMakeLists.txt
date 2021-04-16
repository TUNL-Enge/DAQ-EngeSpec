cmake_minimum_required( VERSION 2.8 )

set (CMAKE_CXX_STANDARD 11)
 
project( BoostPythonMakeData )
 
# Find necessary packages
#find_package( PythonInterp 3 REQUIRED )
find_package( PythonLibs 3 REQUIRED )
#set(PYTHON_LIBRARIES /opt/anaconda3/lib/libpython3.7m.so)
include_directories( ${PYTHON_INCLUDE_DIRS} )
MESSAGE(STATUS "Pythonlibs: " ${PYTHON_INCLUDE_DIRS})

find_package( Boost COMPONENTS numpy3 REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )
MESSAGE(STATUS "Boost: " ${Boost_INCLUDE_DIR})

#find_package( ROOT COMPONENTS Gui XMLParser XMLIO RHTTP REQUIRED )
#include_directories( ${ROOT_INCLUDE_DIRS} )
#MESSAGE(STATUS "ROOT: " ${ROOT_INCLUDE_DIRS})
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
add_library( MDPPSort SHARED MDPPSort.cpp)


target_link_libraries( EngeSort ${Boost_LIBRARIES} EngeAnalyzer 
  librootana.a -lz ${ROOT_LIBRARIES}  libmidas.a ) 
#-lrt )

target_link_libraries( v1730Sort ${Boost_LIBRARIES} EngeAnalyzer 
  librootana.a -lz ${ROOT_LIBRARIES}  libmidas.a  )

target_link_libraries( MDPPSort ${Boost_LIBRARIES} EngeAnalyzer 
  librootana.a -lz ${ROOT_LIBRARIES}  libmidas.a  )
 
