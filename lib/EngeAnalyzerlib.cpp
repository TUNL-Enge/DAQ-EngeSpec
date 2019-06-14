#include "EngeAnalyzer.h"
#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

BOOST_PYTHON_MODULE(libEngeAnalyzer)
{
  using namespace boost::python;
  // Initialize numpy
  Py_Initialize();
  boost::python::numpy::initialize();
  //    def( "MakeData", MakeData );

  class_<vec>("double_vec")
    .def(vector_indexing_suite<vec>());
  class_<mat>("double_mat")
    .def(vector_indexing_suite<mat>());
  class_<mat2d>("double_mat2d")
    .def(vector_indexing_suite<mat2d>());
  class_<StringVector>("StringVector")
    .def(vector_indexing_suite<StringVector>());
  class_<BoolVector>("BoolVector")
    .def(vector_indexing_suite<BoolVector>());
    
  class_<EngeAnalyzer>("EngeAnalyzer")
    .def("sayhello", &EngeAnalyzer::sayhello)          // string
    .def("saygoodbye", &EngeAnalyzer::saygoodbye)          // string
    .def("Initialize", &EngeAnalyzer::Initialize)          // void
    .def("GenerateDataMatrix", &EngeAnalyzer::GenerateDataMatrix) // void
    .def("getData", &EngeAnalyzer::getData)                // 1D histograms
    .def("getData2D", &EngeAnalyzer::getData2D)            // 2D histograms
    .def("getis2D", &EngeAnalyzer::getis2D)                // bool vector
    .def("gethasGate", &EngeAnalyzer::gethasGate)          // bool vector
    .def_readonly("SpectrumNames", &EngeAnalyzer::DataNames)
    .def("ClearData", &EngeAnalyzer::ClearData)        // void
    .def("putGate", &EngeAnalyzer::putGate)            // void
    .def("data", range(&EngeAnalyzer::begin, &EngeAnalyzer::end)) 
    ;

  class_<MidasAnalyzer>("MidasAnalyzer")
    .def("connectMidasAnalyzer", &connectMidasAnalyzer) // int
    ;

}
