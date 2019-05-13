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
  class_<StringVector>("StringVector")
    .def(vector_indexing_suite<StringVector>());
    
  class_<DataMaker>("DataMaker")
    .def("sayhello", &DataMaker::sayhello)          // string
    .def("saygoodbye", &DataMaker::saygoodbye)          // string
    .def("GenerateDataMatrix", &DataMaker::GenerateDataMatrix)
    .def_readonly("SpectrumNames", &DataMaker::DataNames)
    .def("GenerateData", &DataMaker::GenerateData)  // void
    .def("ClearData", &DataMaker::ClearData)        // void
    .def("PrintData", &DataMaker::PrintData)        // void
    .def("data", range(&DataMaker::begin, &DataMaker::end)) 
    ;
}
