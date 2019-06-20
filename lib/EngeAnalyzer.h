//#ifndef EngeAnalyzer_H
//#define EngeAnalyzer_H

#include <vector>
#include <random>
#include <string>
#include <thread>
#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "boost/multi_array.hpp"

#include "TRootanaEventLoop.hxx"
#include "THistogramArrayBase.h"

#include "manalyzer.h"
#include "midasio.h"

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
  int pnpoly(double testx, double testy);
  int inGate(double x, double y);
  
 private:
  
  std::vector<std::vector<double>> Points;
  double minx, maxx, miny, maxy;
  
};



//class EngeAnalyzer: public TRootanaEventLoop {
class EngeAnalyzer {
 public:
  
  EngeAnalyzer() {}
  
  char const* sayhello( );
  char const* saygoodbye( );
  void Initialize();
  void ClearData();
  void SimulateData();

  // Connect the midas analyzer
  int connectMidasAnalyzer();
  
  // New 2d matrix method
  void GenerateDataMatrix(int n = 1000);
  np::ndarray getData();
  np::ndarray getData2D();

  void putADC(uint32_t *dADC);
  
  BoolVector getis2D(){return is2D;}
  BoolVector gethasGate(){return hasGate;}

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
  std::vector<bool> hasGate;
  std::vector<std::vector<int>> DataMatrix;
  std::vector<std::vector<std::vector<int>>> DataMatrix2D;
  std::default_random_engine generator;

  //  std::vector<std::vector<std::vector<double>>> GateCollection;
  std::vector<Gate> GateCollection;

  // Some counters
  int ipeak1 = 0;
  int ipeak2 = 0;
  int igated = 0;
};



/*
  Classes for manalyzer-type analyzer
*/
class MidasAnalyzerModule: public TAModuleInterface{
 public:
  void Init(const std::vector<std::string> &args);
  void ConnectEngeAnalyzer(EngeAnalyzer *ea){eA=*ea;}
  void Finish();
  TARunInterface* NewRun(TARunInfo* runinfo);
    
  int fTotalEventCounter;
  EngeAnalyzer eA;
};
  
class MidasAnalyzerRun: public TARunInterface{
 public:
 MidasAnalyzerRun(TARunInfo* runinfo, MidasAnalyzerModule *m)
   : TARunInterface(runinfo){
    fModule = m;
    fRunEventCounter = 0;
  }
  ~MidasAnalyzerRun(){}
    
  void BeginRun(TARunInfo* runinfo);
  void EndRun(TARunInfo* runinfo);
  void PauseRun(TARunInfo* runinfo){}
  void ResumeRun(TARunInfo* runinfo){}
    
  TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow);
  void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event){}
    
  int fRunEventCounter;
  MidasAnalyzerModule* fModule;
};
