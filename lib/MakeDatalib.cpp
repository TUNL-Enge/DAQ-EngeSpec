#include "MakeData.h"
#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

BOOST_PYTHON_MODULE(libMakeData)
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
    
  class_<DataMaker>("DataMaker")
    .def("sayhello", &DataMaker::sayhello)          // string
    .def("saygoodbye", &DataMaker::saygoodbye)          // string
    .def("Initialize", &DataMaker::Initialize)          // void
    .def("GenerateDataMatrix", &DataMaker::GenerateDataMatrix) // void
    .def("getData", &DataMaker::getData)                // 1D histograms
    .def("getData2D", &DataMaker::getData2D)            // 2D histograms
    .def("getis2D", &DataMaker::getis2D)                // bool vector
    .def_readonly("SpectrumNames", &DataMaker::DataNames)
    .def("ClearData", &DataMaker::ClearData)        // void
    .def("putGate", &DataMaker::putGate)            // void
    .def("data", range(&DataMaker::begin, &DataMaker::end)) 
    ;
}
