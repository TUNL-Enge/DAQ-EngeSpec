#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include "EngeSort.h"
#include "TV792Data.hxx"

Messages messages;

std::string EngeSort::sayhello( ) {
  return messages.sayhello("EngeSort");
}
std::string EngeSort::saygoodbye( ) {
  return messages.saygoodbye();
}

int Channels1D = 4096;
int Channels2D = 256;

Histogram *hPos1;
Histogram *hDE;
Histogram *hPos1_gDEvPos1;

Histogram *hDEvsPos1;

void EngeSort::Initialize(){

  std::string hname;
  
  //--------------------
  // 1D Histograms
  hPos1 = new Histogram("Position 1", Channels1D, 1);
  //  hPos1 -> Print(0, 10);

  // TODO
  // http://www.cplusplus.com/forum/general/103269/
  // http://www.cplusplus.com/reference/set/set/
  // Make a struct that holds all of the histograms. This could then
  // be iterated over to collect data, etc.

  /*
  hname = "Delta E";
  hDE = new Histogram1D(hname, Channels1D);
  SpectrumNames.push_back(hname);
  is2D.push_back(false);
  hasGate.push_back(false);
  
  //--------------------
  // 2D Histograms
  hname = "DE vs Pos 1";
  hDEvsPos1 = new Histogram2D(hname, Channels2D);
  SpectrumNames.push_back(hname);
  is2D.push_back(true);
  hasGate.push_back(false);
  
  //--------------------
  // Gated Histograms
  hname = "Pos. 1; GPos1vDE";
  hPos1_gDEvPos1 = new Histogram1D(hname, Channels1D);
  SpectrumNames.push_back(hname);
  is2D.push_back(false);
  hasGate.push_back(false);
  */
}

//======================================================================
// This is the equivalent to the "sort" function in jam
void EngeSort::sort(uint32_t *dADC){

  int size = sizeof(dADC)/sizeof(dADC[0]);

  // Thresholds
  int Threshold = 200;
  
  // Define the channels
  int cPos1 = dADC[0];
  int cDE = dADC[1];

  // Apply software thresholds
  if(cPos1 < Threshold || cPos1 > Channels1D)cPos1 = 0;
  if(cDE < Threshold || cDE > Channels1D)cDE = 0;

  // Increment histograms
  hPos1 -> inc(cPos1);
  hDE -> inc(cDE);
  hDEvsPos1 -> inc(cPos1, cDE);

  /*
  DataMatrix[0][int(dADC[cPos1])]++;
  DataMatrix[1][int(dADC[cDE])]++;

  
  DataMatrix2D[0][int(dADC[cPos1]/16.0)][int(dADC[cDE]/16.0)]++;

  // The gated spectrum
  
  Gate G1;
  if(GateCollection.size()>0){
    G1 = GateCollection[0];
  }
  
  // Is the gate defined?
  if(G1.inGate(dADC[0]/16,dADC[1]/16)){
    igated++;
    DataMatrix2D[1][int(dADC[cPos1]/16.0)][int(dADC[cDE]/16.0)]++;
    DataMatrix[2][int(dADC[cPos1])]++;
  }

  */  
}


int EngeSort::connectMidasAnalyzer(){

  MidasAnalyzerModule mAMod;
  //TARegisterModule tarm(&mAMod);
  TARegister tar(&mAMod);

  mAMod.ConnectEngeAnalyzer(this);
  
  Py_BEGIN_ALLOW_THREADS
    manalyzer_main(0,0);
  Py_END_ALLOW_THREADS
    
  return 0;
}


np::ndarray EngeSort::getData(){
  
  // Create the matrix to return to python
  u_int n_rows = DataMatrix.size();
  u_int n_cols = DataMatrix[0].size();
  p::tuple shape = p::make_tuple(n_rows, n_cols);
  p::tuple stride = p::make_tuple(sizeof(int));
  np::dtype dtype = np::dtype::get_builtin<int>();
  p::object own;
  np::ndarray converted = np::zeros(shape, dtype);

  /*
    std::cout << "DataMatrix Size = " << n_rows << "x" << n_cols << std::endl;
    std::cout << "DataMatrix[0][0] = " << DataMatrix[0][0] << std::endl;
  */
  
  for (u_int i = 0; i < n_rows; i++)
    {
      shape = p::make_tuple(n_cols);
      converted[i] = np::from_data(DataMatrix[i].data(), dtype, shape, stride, own);
      int sum=0;
      /*
      for(int j=0; j<4096; j++){
	sum += DataMatrix[i][j];
      }
      std::cout << "Sum = " << sum << std::endl;
      */      
    }

  return converted;
  
}

np::ndarray EngeSort::getData2D(){
  
  // Create the 3D matrix to return to python
  u_int n_t = DataMatrix2D.size();
  u_int n_rows = DataMatrix2D[0].size();
  u_int n_cols = DataMatrix2D[0][0].size();
  //std::cout << n_t << " " << n_rows << " " << n_cols << std::endl;
  p::tuple shape = p::make_tuple(n_t, n_rows, n_cols);
  p::tuple stride = p::make_tuple(sizeof(int));
  np::dtype dtype = np::dtype::get_builtin<int>();
  p::object own;
  np::ndarray converted = np::zeros(shape, dtype);

  for(u_int t = 0; t<n_t; t++){
    for (u_int i = 0; i < n_rows; i++)
      {
	shape = p::make_tuple(n_cols);
	converted[t][i] = np::from_data(DataMatrix2D[t][i].data(), dtype, shape, stride, own);
	/*      int sum=0;
		for(int j=0; j<4096; j++){
		sum += DataMatrix[i][j];
		}
		std::cout << "Sum = " << sum << std::endl;
      */
      }
  }

  return converted;
  
}


void EngeSort::putGate(char* name, p::list x, p::list y){

  //std::cout << "The name is " << name << std::endl;
  p::ssize_t len = p::len(x);
  // Make a vector for the gate
  Gate G1;//std::vector<std::vector<double>> Gate;
  for(int i=0; i<len; i++){
    std::vector<double> tmp;
    tmp.push_back(p::extract<double>(x[i]));
    tmp.push_back(p::extract<double>(y[i]));
    G1.addVertex(tmp);
  }

  GateCollection.push_back(G1);

  /*
  for(int i=0; i<Gate.size(); i++){
    std::cout << Gate[i][0] << " " << Gate[i][1] << std::endl;
  }
  */
  
}

void EngeSort::ClearData(){

  // Clear the 1D data
  std::vector<int> row;
  row.resize(4096,0);
  for(int k=0; k<DataMatrix.size(); k++)
    DataMatrix[k] = row;
  
  // Clear the 2D data
  row.resize(256,0);
  for(int k=0; k<DataMatrix2D.size(); k++){
    for(int i=0; i<256; i++)
      DataMatrix2D[k][i] = row;
  }

  ipeak1 = ipeak2 = igated = 0;
  
}

//----------------------------------------------------------------------

/* 
   manalyzer module
*/
void MidasAnalyzerModule::Init(const std::vector<std::string> &args){

  printf("Initializing Midas Analyzer Module\n");
  printf("Arguments:\n");
  for (unsigned i=0; i<args.size(); i++)
    printf("arg[%d]: [%s]\n", i, args[i].c_str());

  fTotalEventCounter = 0;
}
void MidasAnalyzerModule::Finish(){
  printf("Finish!");
  printf("Counted %d events\n",fTotalEventCounter);
  std::cout << "number of spectra: " << eA->getSpectrumNames().size() << std::endl;
}
TARunObject* MidasAnalyzerModule::NewRunObject(TARunInfo* runinfo){
  printf("NewRunObject, run %d, file %s\n",runinfo->fRunNo, runinfo->fFileName.c_str());
  return new MidasAnalyzerRun(runinfo, this);
}
TAFlowEvent* MidasAnalyzerRun::Analyze(TARunInfo* runinfo, TMEvent* event,
				    TAFlags* flags, TAFlowEvent* flow){

  if(event->event_id != 1)
    return flow;

  // Get the ADC Bank
  TMBank* bADC = event->FindBank("ADC1");
  uint32_t* dADC = (uint32_t*)event->GetBankData(bADC);

  fRunEventCounter++;
  fModule->fTotalEventCounter++;
  fModule->eA->sort(dADC);

  return flow;

}

/* 
   manalyzer run
*/
void MidasAnalyzerRun::BeginRun(TARunInfo* runinfo){
  printf("Begin run %d\n",runinfo->fRunNo);
  time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
  printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));

  fRunEventCounter = 0;
}
void MidasAnalyzerRun::EndRun(TARunInfo* runinfo){
  printf("End run %d\n",runinfo->fRunNo);
  printf("Counted %d events\n",fRunEventCounter);
}

BOOST_PYTHON_MODULE(EngeSort)
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
    
  class_<EngeSort>("EngeSort")
    .def("sayhello", &EngeSort::sayhello)          // string
    .def("saygoodbye", &EngeSort::saygoodbye)          // string
    .def("Initialize", &EngeSort::Initialize)          // void
    .def("connectMidasAnalyzer", &EngeSort::connectMidasAnalyzer) // int
    .def("getData", &EngeSort::getData)                // 1D histograms
    .def("getData2D", &EngeSort::getData2D)            // 2D histograms
    .def("getis2D", &EngeSort::getis2D)                // bool vector
    .def("gethasGate", &EngeSort::gethasGate)          // bool vector
    .def("getSpectrumNames", &EngeSort::getSpectrumNames)
    .def("ClearData", &EngeSort::ClearData)        // void
    .def("putGate", &EngeSort::putGate)            // void
    .def("data", range(&EngeSort::begin, &EngeSort::end))
    //    .def("Histogram1D", &Histogram1D::Histogram1D)
    ;

}
