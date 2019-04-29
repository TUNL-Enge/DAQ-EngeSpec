#include "MakeData.h"
#include <boost/python.hpp>
 
BOOST_PYTHON_MODULE(libMakeData)
{
  using namespace boost::python;
  //    def( "MakeData", MakeData );

  class_<DataMaker>("DataMaker")
    .def("sayhello", &DataMaker::sayhello)          // string
    .def("GenerateData", &DataMaker::GenerateData)  // void
    .def("ClearData", &DataMaker::ClearData)        // void
    .def("PrintData", &DataMaker::PrintData)        // void
    .def("data", range(&DataMaker::begin, &DataMaker::end)) 
    ;
}
