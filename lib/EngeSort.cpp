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

// 1D Spectra
Histogram *hPos1;
Histogram *hPos2;
Histogram *hDE;
Histogram *hE;
Histogram *hTheta;

Histogram *hSiE;
Histogram *hSiDE;

Histogram *hTDC_E;
Histogram *hTDC_DE;
Histogram *hTDC_PosSum;

// 2D Spectra
Histogram *hDEvsPos1;
Histogram *hEvsPos1;
Histogram *hDEvsE;
Histogram *hPos2vsPos1;
Histogram *hThetavsPos1;

Histogram *hSiDEvsSiE;

// Gated Spectra
Histogram *hPos1_gDEvPos1;

// 2D Gate on the DE vs Pos1 spectrum
Gate *g2d_DEvsPos1;

// Counters
int totalCounter=0;
int gateCounter=0;

void EngeSort::Initialize(){

  std::string hname;
  
  //--------------------
  // 1D Histograms
  hPos1 = new Histogram("Position 1", Channels1D, 1);
  //  hPos1 -> Print(0, 10);
  hPos2 = new Histogram("Position 2", Channels1D, 1);
  hDE = new Histogram("Delta E", Channels1D, 1);
  hE = new Histogram("E", Channels1D, 1);
  hTheta = new Histogram("Theta", Channels1D, 1);

  hSiE = new Histogram("Silicon E", Channels1D, 1);
  hSiDE = new Histogram("Silicon DE", Channels1D, 1);

  hTDC_E = new Histogram("TDC E", Channels1D, 1);
  hTDC_DE = new Histogram("TDC DE", Channels1D, 1);
  hTDC_PosSum = new Histogram("TDC Position Sum", Channels1D, 1);
  
  //--------------------
  // 2D Histograms
  hDEvsPos1 = new Histogram("DE vs Pos1", Channels2D, 2);
  hEvsPos1 = new Histogram("E vs Pos1", Channels2D, 2);
  hDEvsE = new Histogram("DE vs E", Channels2D, 2);
  hPos2vsPos1 = new Histogram("Pos 2 vs Pos 1", Channels2D, 2);
  hThetavsPos1 = new Histogram("Theta vs Pos 1", Channels2D, 2);

  hSiDEvsSiE = new Histogram("Si DE vs Si E", Channels2D, 2);
  
  //--------------------
  // Gated Histograms
  hPos1_gDEvPos1 = new Histogram("Pos 1; GDEvPos1", Channels1D, 1);


  //--------------------
  // Gates
  hDEvsPos1 -> addGate("DE vs Pos1 Gate");
  
  /*
  // Loop through and print all histograms
  for(auto h: Histograms){
    if(h->getnDims() == 1) {
      std::cout << "Found a 1D histogram!" << std::endl;
      h->Print(0,10);
    } else if(h->getnDims() == 2) {
      std::cout << "Found a 2D histogram!" << std::endl;
      h->Print(0,10,0,10);
    }
    std::cout << std::endl;
  }
  */
  
}

//======================================================================
// This is the equivalent to the "sort" function in jam
void EngeSort::sort(uint32_t *dADC, uint32_t *dTDC){

  totalCounter++;

  double ADCsize = sizeof(dADC)/sizeof(dADC[0]);
  double TDCsize = sizeof(dTDC)/sizeof(dTDC[0]);

  std::cout << ADCsize << "  " << TDCsize << std::endl;
  
  // Thresholds
  int Threshold = 10;
  for(int i=0; i<ADCsize; i++)
    if(dADC[i] < Threshold || dADC[i] > Channels1D)dADC[i]=0;
  for(int i=0; i<TDCsize; i++)
    if(dTDC[i] < Threshold || dTDC[i] > Channels1D)dTDC[i]=0;

  std::cout << "Done thresholding" << std::endl;
  
  // Define the channels
  int cE = dADC[0];
  int cDE = dADC[1];
  int cPos1 = dADC[2];
  int cPos2 = dADC[3];
  int cSiE = dADC[5];
  int cSiDE = dADC[7];

  int cTDC_E = dTDC[0];
  int cTDC_DE = dTDC[1];
  int cTDC_Pos1 = dTDC[2];
  int cTDC_Pos2 = dTDC[3];

  std::cout << cPos1 << std::endl;
  
  // Calculate some things
  int cTheta = (int)std::round(10000.0*atan((cPos2-cPos1)/100.)/3.1415 - 4000.);
  cTheta = std::max(0,cTheta);
  int cTDC_PosSum = (int)std::round((cTDC_Pos1 + cTDC_Pos2)/2.);

  // ------------------------------------------------------------
  // Compressed versions for 2D spectra
  // ------------------------------------------------------------
  double compression = (double)Channels1D/ (double)Channels2D;
  int cEcomp = (int) std::floor(cE / compression);
  int cDEcomp = (int) std::floor(cDE / compression);
  int cPos1comp = (int) std::floor(cPos1 / compression);
  int cPos2comp = (int) std::floor(cPos2 / compression);
  int cThetacomp = (int) std::floor(cTheta / compression);
  
  int cSiEcomp = (int) std::floor(cSiE / compression);
  int cSiDEcomp = (int) std::floor(cSiDE / compression);
	
  // std::cout << "Compressed data" << std::endl;

  
  // Increment 1D histograms
  hPos1 -> inc(cPos1);
  hPos2 -> inc(cPos2);
  hDE -> inc(cDE);
  hE -> inc(cE);
  hTheta -> inc(cTheta);

  hSiE -> inc(cSiE);
  hSiDE -> inc(cSiDE);

  //std::cout << "Incremented 1d spec" << std::endl;
  
  hTDC_E -> inc(cTDC_E);
  hTDC_DE -> inc(cTDC_DE);
  hTDC_PosSum -> inc(cTDC_PosSum);

  //std::cout << "Incremented TDC" << std::endl;
  
  // Increment 2D histograms
  //std::cout << cPos1comp << " " << cDEcomp <<
  //  " " << cEcomp << " " << cPos2comp << " " << cThetacomp << std::endl;
  hDEvsPos1 -> inc(cPos1comp, cDEcomp);
  hEvsPos1 -> inc(cPos1comp, cEcomp);
  hDEvsE -> inc(cEcomp, cDEcomp);
  hPos2vsPos1 -> inc(cPos1comp, cPos2comp);
  hThetavsPos1 -> inc(cPos1comp, cThetacomp);

  hSiDEvsSiE -> inc(cSiEcomp, cSiDEcomp);

  //std::cout << "2D spect" << std::endl;
  
  // The gated spectrum
  Gate *G1 = hDEvsPos1->getGates(0);
  //G1->Print();
  if(G1->inGate(cPos1comp,cDEcomp)){
    gateCounter++;;
    hPos1_gDEvPos1->inc(cPos1);
  }

  //std::cout << "gated spec" << std::endl; 
  
}

//#include <cstring>
int EngeSort::connectMidasAnalyzer(std::string filename){
//int EngeSort::connectMidasAnalyzer(){

  MidasAnalyzerModule mAMod;
  //TARegisterModule tarm(&mAMod);
  TARegister tar(&mAMod);

  mAMod.ConnectEngeAnalyzer(this);

  std::cout << "connectMidasAnalyzer " << filename.size() << " " << filename << std::endl;

  // We need to send a dummy argument to manalyzer, which gets ignored
  filename = "dummy " + filename;
  
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
}
TARunObject* MidasAnalyzerModule::NewRunObject(TARunInfo* runinfo){
  printf("NewRunObject, run %d, file %s\n",runinfo->fRunNo, runinfo->fFileName.c_str());
  return new MidasAnalyzerRun(runinfo, this);
}
TAFlowEvent* MidasAnalyzerRun::Analyze(TARunInfo* runinfo, TMEvent* event,
				    TAFlags* flags, TAFlowEvent* flow){

  if(event->event_id != 1)
    return flow;

  std::cout << "Analyzing" << std::endl;
  
  // Get the ADC Bank
  TMBank* bADC = event->FindBank("ADC1");
  uint32_t* dADC = (uint32_t*)event->GetBankData(bADC);
  TMBank* bTDC = event->FindBank("TDC1");
  uint32_t* dTDC = (uint32_t*)event->GetBankData(bTDC);
  
  fRunEventCounter++;
  fModule->fTotalEventCounter++;
  std::cout << "Calling sort" << std::endl;
  fModule->eA->sort(dADC, dTDC);

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
  std::cout << "Total counts: " << totalCounter << "   Gated counts: " << gateCounter << std::endl;
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
    .def("getis2Ds", &EngeSort::getis2Ds)                // bool vector
    .def("gethasGates", &EngeSort::gethasGates)          // bool vector
    .def("getSpectrumNames", &EngeSort::getSpectrumNames)
    .def("ClearData", &EngeSort::ClearData)        // void
    .def("putGate", &EngeSort::putGate)            // void
    .def("data", range(&EngeSort::begin, &EngeSort::end))
    //    .def("Histogram1D", &Histogram1D::Histogram1D)
    ;

}
