cmake_minimum_required( VERSION 3.2 )

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

LINK_LIBRARIES(${Boost_LIBRARIES} ${PYTHON_LIBRARIES}) # Deprecated but so convenient!
## Still unable to statically link these on Linux.
# Define the general Enge analyzer library
add_library( EngeAnalyzer SHARED EngeAnalyzerlib.cpp )

# Multi Card Crap
add_library( MDPPevt SHARED MDPPEventHandler.cpp )
add_library( tomlread SHARED mdpp_module_config.cpp )

# Build our Sort Routine
add_library( MDPPSort_CI SHARED MDPPSort_CI.cpp)

add_library( v1730EngeSort SHARED v1730EngeSort.cpp)

# Build our Sort Routine
add_library( MDPPSort SHARED MDPPSort.cpp)

# Build our Sort Routine
add_library( SiliconSort SHARED SiliconSort.cpp)

# Build our Sort Routine
add_library( RBSSort SHARED RBSSort.cpp)

# Build our Sort Routine
add_library( 88SrSort SHARED 88SrSort.cpp)


target_link_libraries( EngeSort ${Boost_LIBRARIES} EngeAnalyzer 
  librootana.a -lz ${ROOT_LIBRARIES} libmidas.a ) 

target_link_libraries( v1730Sort ${Boost_LIBRARIES} EngeAnalyzer 
  librootana.a -lz ${ROOT_LIBRARIES} libmidas.a )

target_link_libraries( v1730EngeSort ${Boost_LIBRARIES} EngeAnalyzer 
  librootana.a -lz ${ROOT_LIBRARIES}  libmidas.a -lrt )

target_link_libraries( MDPPSort ${Boost_LIBRARIES} EngeAnalyzer 
  librootana.a -lz ${ROOT_LIBRARIES} libmidas.a )

target_link_libraries( SiliconSort ${Boost_LIBRARIES} EngeAnalyzer 
  librootana.a -lz ${ROOT_LIBRARIES} libmidas.a )

target_link_libraries( RBSSort ${Boost_LIBRARIES} EngeAnalyzer 
  librootana.a -lz ${ROOT_LIBRARIES}  libmidas.a -lrt )

target_link_libraries( 88SrSort ${Boost_LIBRARIES} EngeAnalyzer 
  librootana.a -lz ${ROOT_LIBRARIES}  libmidas.a -lrt )

# Ensure .so extension, which is needed by python when it imports the module.
set_target_properties(MDPPSort tomlread MDPPevt EngeAnalyzer PROPERTIES SUFFIX ".so")

# rt is needed by midas on linux, not sure if it is actually required to build the library
if(NOT APPLE)
  link_libraries(rt)
endif()
# The extra target is just so CMake does not screw up the dependencies.
target_link_libraries( MDPPevt ${Boost_LIBRARIES} EngeAnalyzer tomlread
  librootana.a -lz ${ROOT_LIBRARIES}  libmidas.a )
target_link_libraries( MDPPSort ${Boost_LIBRARIES} EngeAnalyzer MDPPevt tomlread
  librootana.a -lz ${ROOT_LIBRARIES}  libmidas.a )

