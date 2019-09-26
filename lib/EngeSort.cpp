#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include "EngeSort.h"
#include "TV792Data.hxx"

char const* EngeSort::sayhello( ) {
    return "Hello! This is the Enge sort routine running in c++!!!";
}
char const* EngeSort::saygoodbye( ) {
  std::cout << "Peak 1 has " << ipeak1 << " counts" << std::endl;
  std::cout << "Peak 2 has " << ipeak2 << " counts" << std::endl;
  std::cout << "Gated peak has " << igated << " counts" << std::endl;
  
  return "Goodbye! I hope I served you well";
}

void EngeSort::Initialize(){

  // Set the names of the data
  DataNames.clear();
  DataNames.push_back("Pos1");
  is2D.push_back(false);
  hasGate.push_back(false);
  DataNames.push_back("DE");
  is2D.push_back(false);
  hasGate.push_back(true);
  DataNames.push_back("Pos1-cut");
  is2D.push_back(false);
  hasGate.push_back(false);
  
  // Fill some empty spectra
  std::vector<int> tempSpec;
  tempSpec.resize(4096,0);
  DataMatrix.clear();
  for(int i=0; i<DataNames.size(); i++)
    DataMatrix.push_back(tempSpec);

  // Then make 2D spectrum
  DataNames.push_back("DEvsPos1");
  is2D.push_back(true);
  hasGate.push_back(true);
  std::vector<int> row;
  row.resize(256,0);
  std::vector<std::vector<int>> tempSpec2D;
  for(int i=0; i<256; i++)
    tempSpec2D.push_back(row);
  //  tempSpec2D[0].resize(256,0);
  DataMatrix2D.push_back(tempSpec2D);

  // Make Gated spectra
  DataNames.push_back("DEvsPos1-gated");
  is2D.push_back(true);
  hasGate.push_back(false);
  tempSpec2D.clear();
  for(int i=0; i<256; i++)
    tempSpec2D.push_back(row);
  //  tempSpec2D[0].resize(256,0);
  DataMatrix2D.push_back(tempSpec2D);

  /*
  u_int n_t = DataMatrix2D.size();
  u_int n_rows = DataMatrix2D[0].size();
  u_int n_cols = DataMatrix2D[0][0].size();
  std::cout << n_t << " " << n_rows << " " << n_cols << std::endl;
  */
  
  std::cout << "Made " << DataNames.size() << " empty spectra" << std::endl;
  
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
void EngeSort::GenerateDataMatrix(int n)
//void EngeSort::GenerateDataMatrix(int n)
{
  int nbins = 4096;
  int nspec = 2;

  std::normal_distribution<double> distribution1(1500.0,200.0);
  std::normal_distribution<double> distribution2(2000.0,100.0);
  std::normal_distribution<double> distribution3(1700.0,200.0);
  std::normal_distribution<double> distribution4(3000.0,100.0);
  std::normal_distribution<double> crap(100,50); 


  Gate G1;
  if(GateCollection.size()>0){
    G1 = GateCollection[0];
  }
  
  // Fill the spectra
  std::vector<double> d1, d2;
  for(int i=0; i<n; i++){
    if(crap(generator)>120){
      ipeak1++;
      d1.push_back(distribution1(generator));
      d2.push_back(distribution2(generator));
    } else {
      ipeak2++;
      d1.push_back(distribution3(generator));
      d2.push_back(distribution4(generator));
    }
    DataMatrix[0][int(d1[i])]++;
    DataMatrix[1][int(d2[i])]++;
    DataMatrix2D[0][int(d1[i]/16.0)][int(d2[i]/16.0)]++;

    
    // The gated spectrum
    // Is the gate defined?
    if(G1.inGate(d1[i],d2[i])){
      igated++;
      DataMatrix2D[1][int(d1[i]/16.0)][int(d2[i]/16.0)]++;
      DataMatrix[2][int(d1[i])]++;
    }

  }
  /*
  // Test the gate
  G1.pnpoly(0,0);
  G1.pnpoly(1000,1000);
  G1.pnpoly(500,500);
  G1.pnpoly(1000,500);
  */
  /*  
  for(int i=0; i<50; i++){
    for(int j=0; j<50; j++){
      std::cout << DataMatrix2D[0][i][j] << " ";
    }
    std::cout << std::endl;
  }
  */

  /*
    Py_BEGIN_ALLOW_THREADS
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    Py_END_ALLOW_THREADS
  */
  
}

void EngeSort::putADC(uint32_t *dADC){

  int size = sizeof(dADC)/sizeof(dADC[0]);
  /*
  std::cout << "Channels: " << size << std::endl;
  for(int i=0; i<size; i++){
    std::cout << i << ": " << dADC[i] << "  " ;
  }
  std::cout << std::endl;
  */  
  int cPos1 = 0;
  int cDE = 1;

  if(dADC[cPos1] < 200 || dADC[cPos1] > 4095)dADC[cPos1] = 0;
  if(dADC[cDE] < 200 || dADC[cDE] > 4095)dADC[cDE] = 0;
  
  DataMatrix[0][int(dADC[cPos1])]++;
  DataMatrix[1][int(dADC[cDE])]++;

  /*  std::cout << dADC[cPos1] << " " << dADC[cDE] << std::endl;
  std::cout << int(dADC[cPos1]/16.0) << "  " << int(dADC[cDE]/16.0)
	    << std::endl;
  */
  
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
  std::cout << "number of spectra: " << eA->DataNames.size() << std::endl;
}
TARunObject* MidasAnalyzerModule::NewRunObject(TARunInfo* runinfo){
  printf("NewRunObject, run %d, file %s\n",runinfo->fRunNo, runinfo->fFileName.c_str());
  return new MidasAnalyzerRun(runinfo, this);
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
TAFlowEvent* MidasAnalyzerRun::Analyze(TARunInfo* runinfo, TMEvent* event,
				    TAFlags* flags, TAFlowEvent* flow){
  //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo,
  //	 event->serial_number, (int)event->event_id, event->data_size);

  if(event->event_id != 1)
    return flow;

  // Get the ADC Bank
  TMBank* bADC = event->FindBank("ADC1");

  uint32_t* dADC = (uint32_t*)event->GetBankData(bADC);
  //std::cout << dADC[0] << "  " << dADC[1] << "  " << dADC[2] << "  " << dADC[3] << std::endl;
  
  fRunEventCounter++;
  fModule->fTotalEventCounter++;
  fModule->eA->putADC(dADC);

  return flow;

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
    .def("GenerateDataMatrix", &EngeSort::GenerateDataMatrix) // void
    .def("getData", &EngeSort::getData)                // 1D histograms
    .def("getData2D", &EngeSort::getData2D)            // 2D histograms
    .def("getis2D", &EngeSort::getis2D)                // bool vector
    .def("gethasGate", &EngeSort::gethasGate)          // bool vector
    .def_readonly("SpectrumNames", &EngeSort::DataNames)
    .def("ClearData", &EngeSort::ClearData)        // void
    .def("putGate", &EngeSort::putGate)            // void
    .def("data", range(&EngeSort::begin, &EngeSort::end)) 
    ;

}
