#ifndef MAKEDATA_H
#define MAKEDATA_H

#include <vector>
#include <random>
#include <string>
#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "boost/multi_array.hpp"

namespace p = boost::python;
namespace np = boost::python::numpy;

typedef std::vector<double> vec;
typedef std::vector<vec> mat;
typedef std::vector<std::string> StringVector;

class DataMaker {
 public:

  DataMaker() {}

  char const* sayhello( );
  char const* saygoodbye( );
  void GenerateData(int n = 1000);
  void ClearData();
  void SimulateData();
  void PrintData();

  // New 2d matrix method
  np::ndarray GenerateDataMatrix(int n = 1000);

  // For returning the old data arrays (clean this up later)
  std::vector<int>::iterator begin(){
    return Dat[0].begin();
  }
  std::vector<int>::iterator end(){
    return Dat[0].end();
  }
  
  StringVector DataNames;

 private:
  bool isRunning = false;
  std::vector<int> Dat[2];
  std::vector<std::vector<int>> DataMatrix;
  std::default_random_engine generator;
};

#endif // MAKEDATA_H
