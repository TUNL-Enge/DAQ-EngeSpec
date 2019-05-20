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
typedef std::vector<mat> mat2d;
typedef std::vector<std::string> StringVector;
typedef std::vector<bool> BoolVector;

class Gate{
  
 public:

  Gate() {};

  void addVertex(std::vector<double> v);
  std::vector<std::vector<double>> getPoints(){return Points;}
  bool inBound(double x, double y);
  bool inGate(double x, double y);
  
 private:

  std::vector<std::vector<double>> Points;
  
};


class DataMaker {
 public:

  DataMaker() {}

  char const* sayhello( );
  char const* saygoodbye( );
  void Initialize();
  void ClearData();
  void SimulateData();

  // New 2d matrix method
  void GenerateDataMatrix(int n = 1000);
  np::ndarray getData();
  np::ndarray getData2D();

  BoolVector getis2D(){return is2D;}

  // Gate passing
  void putGate(char* name, p::list x, p::list y);
  
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
  std::vector<bool> is2D;
  std::vector<std::vector<int>> DataMatrix;
  std::vector<std::vector<std::vector<int>>> DataMatrix2D;
  std::default_random_engine generator;

  //  std::vector<std::vector<std::vector<double>>> GateCollection;
  std::vector<Gate> GateCollection;
};




#endif // MAKEDATA_H
