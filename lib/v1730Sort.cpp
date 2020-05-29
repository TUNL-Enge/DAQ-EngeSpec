#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include "v1730Sort.h"
#include "TV792Data.hxx"

Messages messages;

std::string EngeSort::sayhello( ) {
  return messages.sayhello("v1730Sort");
}
std::string EngeSort::saygoodbye( ) {
  return messages.saygoodbye();
}

int Channels1D = 4096;
int Channels2D = 256;

// 1D Spectra
Histogram *hDet1;

// Counters
int totalCounter=0;
int gateCounter=0;

void EngeSort::Initialize(){

  //--------------------
  // 1D Histograms
  hDet1 = new Histogram("Detector 1", Channels1D, 1);
  
}

//======================================================================
// This is the equivalent to the "sort" function in jam
void EngeSort::sort(uint32_t *dADC, int nADC, uint32_t *dTDC, int nTDC){

  totalCounter++;

  //  double ADCsize = sizeof(dADC)/sizeof(dADC[0]);
  //  double TDCsize = sizeof(dTDC)/sizeof(dTDC[0]);

  // std::cout << ADCsize << "  " << TDCsize << std::endl;
  
  // Thresholds
  /*
  int Threshold = 10;
  for(int i=0; i<32; i++)
    if(dADC[i] < Threshold || dADC[i] > Channels1D)dADC[i]=0;
  for(int i=0; i<32; i++)
    if(dTDC[i] < Threshold || dTDC[i] > Channels1D)dTDC[i]=0;
  */

  for(int i = 0; i<nADC; i++){
    // Define the channels
    uint32_t dat = dADC[i] & 0xFFFF;
    int cDet1 = (int)std::floor(dat/8.0);
    
    f_qlong << dat << std::endl;
    //std::cout << i << " " << cDet1 << std::endl;
  
    // Increment 1D histograms
    if(cDet1 > 20 && cDet1 < 4096)
      hDet1 -> inc(cDet1);
  }
}


// Connect the analyzer to midas
int EngeSort::connectMidasAnalyzer(){

  TARegister tar(&mAMod);

  mAMod.ConnectEngeAnalyzer(this);
  return 0;
}

// Run the midas analyzer
int EngeSort::runMidasAnalyzer(boost::python::list file_list){
  std::cout << "runMidasAnalyzer " << len(file_list) << std::endl;;
  // We need to send a dummy argument to manalyzer, which gets ignored
  std::string filename = "dummy ";
  for(int i=0; i<len(file_list); i++){
    std::string file = boost::python::extract<std::string>(file_list[i]);
    std::cout << " " << file;
    filename += file + " ";
  }
  std::cout << std::endl;
  
  std::cout << filename << std::endl;
  
  enum { kMaxArgs = 64 };
  int ac=0;
  char *av[kMaxArgs];

  char *dup = strdup(filename.c_str());
  char *p2 = strtok(dup, " ");
  while (p2 && ac < kMaxArgs-1)
    {
      av[ac++] = p2;
      p2=strtok(0, " ");
    }
  av[ac]=0;
  
  Py_BEGIN_ALLOW_THREADS
    manalyzer_main(ac,av);
    //    manalyzer_main(0,0);
  Py_END_ALLOW_THREADS
    
  return 0;
}

// Return a vector of spectrum names
StringVector EngeSort::getSpectrumNames(){

  StringVector s;
  for(auto h: Histograms){
    s.push_back(h -> getName());
  }

  return s;
}

// Return a vector of spectrum names
StringVector EngeSort::getScalerNames(){

  StringVector s;
  for(auto Sclr: Scalers){
    s.push_back(Sclr -> getName());
  }

  return s;
}

// Return a bool vector of whether the spectra are 2D
BoolVector EngeSort::getis2Ds(){

  BoolVector is2d;
  for(auto h: Histograms){
    bool b = (h -> getnDims() == 2) ? true : false;
    is2D.push_back(b);
  }

  return is2D;
}
// Return a bool vector of whether the spectra have gates
BoolVector EngeSort::gethasGates(){

  BoolVector hasgate;
  for(auto h: Histograms){
    hasgate.push_back(h -> gethasGate());
  }

  return hasgate;
}

// Return a vector of scalers
IntVector EngeSort::getScalers(){

  IntVector sclr;
  for(auto sc: Scalers){
    sclr.push_back(sc -> getValue());
  }

  return sclr;
}


np::ndarray EngeSort::getData(){

  // Create the matrix to return to python
  u_int n_rows = nHist1D;//Histograms.size();
  u_int n_cols = Channels1D; //Histograms[0].getnChannels();
  p::tuple shape = p::make_tuple(n_rows, n_cols);
  p::tuple stride = p::make_tuple(sizeof(int));
  np::dtype dtype = np::dtype::get_builtin<int>();
  p::object own;
  np::ndarray converted = np::zeros(shape, dtype);
  
  
  // Loop through all histograms and pack the 1D histograms into a numpy matrix
  int i=0;
  for(auto h: Histograms)
    {
      if(h -> getnDims()==1){
	//h.Print(1000,2000);
	shape = p::make_tuple(h -> getnChannels());
	converted[i] = np::from_data(h -> getData1D().data(), dtype, shape, stride, own);
	i++;
      }
    }

  return converted;
  
}

np::ndarray EngeSort::getData2D(){

  // Create the 3D matrix to return to python
  u_int n_t = nHist2D; //DataMatrix2D.size();
  u_int n_rows = Channels2D; //DataMatrix2D[0].size();
  u_int n_cols = Channels2D; //DataMatrix2D[0][0].size();

  //  std::cout << n_t << " " << n_rows << " " << n_cols << std::endl;

  p::tuple shape = p::make_tuple(n_t, n_rows, n_cols);
  p::tuple stride = p::make_tuple(sizeof(int));
  np::dtype dtype = np::dtype::get_builtin<int>();
  p::object own;
  np::ndarray converted = np::zeros(shape, dtype);

  //  for(u_int t = 0; t<n_t; t++){
  int t=0;
  for(auto h: Histograms){
    if(h -> getnDims()==2){
      for (int i = 0; i < h -> getnChannels(); i++)
	{
	  shape = p::make_tuple(h -> getnChannels());
	  converted[t][i] = np::from_data(h -> getData2D()[i].data(), dtype, shape, stride, own);
	}
      t++;
    }
  }
  return converted;
  
}


void EngeSort::putGate(std::string name, p::list x, p::list y){

  // First find the spectrum that corresponds to the name
  for(auto h:Histograms){
    if(h -> getName() == name){
      //std::cout << "Found the histogram! With name: " << h->getName() << " " << name << std::endl;

      // Make sure this histogram has gates defined
      if(h -> gethasGate()){
	//std::cout << "Yes, this histogram has gates!" << std::endl;
	
	p::ssize_t len = p::len(x);
	Gate *G1 = h->getGates(0);
	
	// Make a vector for the gate
	for(int i=0; i<len; i++){
	  std::vector<double> tmp;
	  tmp.push_back(p::extract<double>(x[i]));
	  tmp.push_back(p::extract<double>(y[i]));
	  G1->addVertex(tmp);
	}
      }
    }
  }
  
}

void EngeSort::ClearData(){

  for(auto h:Histograms){
    h->Clear();
  }
  
  totalCounter=0;
  gateCounter=0;
  
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
  eA->setIsRunning(false);
}
TARunObject* MidasAnalyzerModule::NewRunObject(TARunInfo* runinfo){
  printf("NewRunObject, run %d, file %s\n",runinfo->fRunNo, runinfo->fFileName.c_str());
  eA->setIsRunning(true);
  return new MidasAnalyzerRun(runinfo, this);
}
TAFlowEvent* MidasAnalyzerRun::Analyze(TARunInfo* runinfo, TMEvent* event,
				    TAFlags* flags, TAFlowEvent* flow){

  if(event->event_id == 1){

    event->FindAllBanks();
    std::cout << event->BankListToString() << std::endl;

    // Get the ADC Bank
    TMBank* bADC = event->FindBank("V730");
    uint32_t* dADC = (uint32_t*)event->GetBankData(bADC);
    TMBank* bTDC = event->FindBank("TDC1");
    uint32_t* dTDC = (uint32_t*)event->GetBankData(bTDC);

    printf("V1730 Bank: Name = %s, Type = %d, Size = %d\n",&bADC->name[0],
	   bADC->type,bADC->data_size); 

    uint64_t dat;
    dat = dADC[0] & 0xFFFF;
    //  printf("dADC[0] = 0x%x\n",dat);
    //  printf("dADC[0] = %d\n",dat);

    int singleADCSize = 0;
    int singleTDCSize = 0;
    if(bADC->type == 4)singleADCSize = 2;
    if(bADC->type == 6)singleADCSize = 4;
    
    // Find the size of the data
    int nADC = 0;
    int nTDC = 0;
    if(bADC)nADC=(bADC->data_size - 2)/singleADCSize;
    if(bTDC)nTDC=(bTDC->data_size - 2)/singleTDCSize;

    std::cout << "nADC = " << nADC << " nTDC = " << nTDC << std::endl;
    
  
    fRunEventCounter++;
    fModule->fTotalEventCounter++;
    fModule->eA->sort(dADC, nADC, dTDC, nTDC);

  } else if(event->event_id == 2){

    std::cout << "This is a scaler event. It should never happen!" << std::endl;

  }
    
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
  
  f_qlong.open("qlong_engespec.txt");
}
void MidasAnalyzerRun::EndRun(TARunInfo* runinfo){
  printf("End run %d\n",runinfo->fRunNo);
  printf("Counted %d events\n",fRunEventCounter);
  std::cout << "Total counts: " << totalCounter << "   Gated counts: " << gateCounter << std::endl;
  
  f_qlong.close();
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
  class_<IntVector>("IntVector")
    .def(vector_indexing_suite<IntVector>());
    
  class_<EngeSort>("EngeSort")
    .def("sayhello", &EngeSort::sayhello)          // string
    .def("saygoodbye", &EngeSort::saygoodbye)          // string
    .def("Initialize", &EngeSort::Initialize)          // void
    .def("connectMidasAnalyzer", &EngeSort::connectMidasAnalyzer) // int
    .def("runMidasAnalyzer", &EngeSort::runMidasAnalyzer) // int
    .def("getData", &EngeSort::getData)                // 1D histograms
    .def("getData2D", &EngeSort::getData2D)            // 2D histograms
    .def("getis2Ds", &EngeSort::getis2Ds)                // bool vector
    .def("gethasGates", &EngeSort::gethasGates)          // bool vector
    .def("getSpectrumNames", &EngeSort::getSpectrumNames)
    .def("getIsRunning", &EngeSort::getIsRunning)        // bool value
    .def("getScalerNames", &EngeSort::getScalerNames)     // string vector
    .def("getScalers", &EngeSort::getScalers)             // IntVector of scaler values
    .def("ClearData", &EngeSort::ClearData)        // void
    .def("putGate", &EngeSort::putGate)            // void
    .def("data", range(&EngeSort::begin, &EngeSort::end))
    //    .def("Histogram1D", &Histogram1D::Histogram1D)
    ;

}
