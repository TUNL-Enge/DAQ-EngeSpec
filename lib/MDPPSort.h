//#ifndef EngeAnalyzer_H
//#define EngeAnalyzer_H
#include "EngeAnalyzerlib.h"
#include "MDPPEventHandler.h"

#include <vector>
#include <random>
#include <string>
#include <thread>
#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "boost/multi_array.hpp"
#include <fstream>

#include "manalyzer.h"
#include "midasio.h"

namespace p = boost::python;
namespace np = boost::python::numpy;

typedef std::vector<double> vec;
typedef std::vector<vec> mat;
typedef std::vector<mat> mat2d;
typedef std::vector<std::string> StringVector;
typedef std::vector<bool> BoolVector;
typedef std::vector<int> IntVector;

// This class holds the calibration for one module within the system.
class Calibrator {

public:

	Calibrator () {}

	void load_file(std::string filename);
	
	IntVector calibrate(vec_u32 &adc_values);

private:
	vec slope;
	vec intercept;
	
};

//class EngeAnalyzer: public TRootanaEventLoop {
class EngeSort {
 public:
  
  EngeSort() {}
  
  std::string sayhello( );
  std::string saygoodbye( );
	std::string saysomething(std::string);
  void Initialize();
  void ClearData();
  void SimulateData();

  // Connect the midas analyzer
  int connectMidasAnalyzer();
  int runMidasAnalyzer(boost::python::list file_list);
  
  // New 2d matrix method
  np::ndarray getData();
  np::ndarray getData2D();

  void sort(MDPPEvent& event_data);
  void incScalers(uint32_t *dSCAL);
  
  BoolVector getis2Ds();
  IntVector getNGates();
  StringVector getGateNames(std::string hname);
  StringVector getSpectrumNames();
  StringVector getScalerNames();
  IntVector getScalers();

  void setIsRunning(bool isr){isRunning = isr;}
  bool getIsRunning(){return isRunning;}

	Calibrator calibrator_annulus_ps;
	Calibrator calibrator_hpge;

  
  // Gate passing
  void putGate(std::string name, std::string gname, p::list x, p::list y);
  
  // For returning the old data arrays (clean this up later)
  std::vector<int>::iterator begin(){
    return Dat[0].begin();
  }
  std::vector<int>::iterator end(){
    return Dat[0].end();
  }
  
  
 private:

  // The analyzer module
  //MidasAnalyzerModule mAMod;

  
  bool isRunning = false;

  std::vector<int> Dat[2];
  std::vector<bool> is2D;
  std::vector<bool> hasGate;
  std::vector<std::vector<int>> DataMatrix;
  std::vector<std::vector<std::vector<int>>> DataMatrix2D;
  std::default_random_engine generator;

  //  std::vector<std::vector<std::vector<double>>> GateCollection;
  std::vector<Gate> GateCollection;

  // Scaler list
  //std::vector<int> Scalers;
  
  // Some counters
  int ipeak1 = 0;
  int ipeak2 = 0;
  int igated = 0;
};

/*
  Classes for manalyzer-type analyzer
*/
class MidasAnalyzerModule: public TAFactory{
 public:
  void Init(const std::vector<std::string> &args);
  void ConnectEngeAnalyzer(EngeSort *ea){eA=ea;}
  void Finish();
  TARunObject* NewRunObject(TARunInfo* runinfo);
    
  int fTotalEventCounter;
  EngeSort *eA;
};

MidasAnalyzerModule mAMod;

class MidasAnalyzerRun: public TARunObject{
 public:

 MidasAnalyzerRun(TARunInfo* runinfo, MidasAnalyzerModule *m)
   : TARunObject(runinfo){
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
	MDPPEvent mdpp_event;
	
};
