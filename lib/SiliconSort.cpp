#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include "SiliconSort.h"
#include "TV792Data.hxx"

Messages messages;

std::string EngeSort::sayhello( ) {
  return messages.sayhello("SiliconSort");
}
std::string EngeSort::saygoodbye( ) {
  return messages.saygoodbye();
}
std::string EngeSort::saysomething(std::string str) {
  return messages.saysomething(str);
}

int Channels1D = 4096;
int Channels2D = 512;

int comp1d = 1;   // The amount of compression to apply to
// measurements so they fit in the spectra
int comp2d = Channels1D/Channels2D;

// 1D Spectra
Histogram *hEFront;
Histogram *hEBack;
Histogram *hETotal;
Histogram *hRing;
Histogram *hSegment;

// 2D Spectra
Histogram *hETotalvsRing;

// Gated Spectra
Histogram *hETotal_gETotalvsRing;

// Counters
int totalCounter=0;

void EngeSort::Initialize(){

  std::string hname;
  
  //--------------------
  // 1D Histograms
  hEFront = new Histogram("E Front", Channels1D, 1);
  hETotal = new Histogram("E Total", Channels1D, 1);

	
  //--------------------
  // 2D Histograms
  hETotalvsRing = new Histogram("ETotal vs ring", Channels2D, 2);
  //--------------------

	//--------------------
  // Gated Histograms
  hETotal_gETotalvsRing = new Histogram("E Total; gETotalvsRing", Channels1D, 1);
	//--------------------


	// Gates
  //  hE -> addGate("Energy Gate");
  hETotalvsRing -> addGate("Gate");
 
}

//======================================================================
// This is the equivalent to the "sort" function in jam
// Arguments are the raw events
void EngeSort::sort(uint32_t *dMDP1, int nMDP1, uint32_t *dMDP2, int nMDP2){

  totalCounter++;
  
  // Loop through all signals in event (should only be two, one for
  // front and one for back + headers and footers)
  for(int i = 0; i < nMDP1; i++){

		// Ignore header and footer events
    if (((dMDP1[i] & 0xF0000000) != 0x10000000) &
				((dMDP2[i] & 0xF0000000) != 0x10000000)){
      continue;
    }
    int EFront = dMDP1[i] & 0xFFFF;    // either time or energy
    int ring = (dMDP1[i] >> 16) & 0x1F;
		int EBack = dMDP2[i] & 0xFFFF;
		int segment = (dMDP2[i] >> 16) & 0x1F;
		int ETotal = EFront+EBack;

		// Increment 1D Histograms
		hEFront->inc(EFront);
		hETotal->inc(ETotal);

		// Increment 2D Histograms
		int ETotalcomp = (int) std::floor(ETotal/comp2d);
		hETotalvsRing->inc(ETotalcomp,ring);

		// Do gating
		Gate &G1 = hETotalvsRing -> getGate(0);
		if(G1.inGate(ETotalcomp,ring)){
			hETotal_gETotalvsRing->inc(ETotal);
		}

	}

}


// Increment the scalers
// TODO: make this automatic. If the scaler is
// defined we should assume that the user wants to increment it
void EngeSort::incScalers(uint32_t *dSCAL){
	// Nothing happens
	
}

// Connect the analyzer to midas
int EngeSort::connectMidasAnalyzer(){

  TARegister tar(&mAMod);

  mAMod.ConnectEngeAnalyzer(this);
  return 0;
}

// Run the midas analyzer
int EngeSort::runMidasAnalyzer(boost::python::list file_list){
  std::cout << "runMidasAnalyzer " << len(file_list) << std::endl;
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
IntVector EngeSort::getNGates(){

  IntVector ngates;
  for(auto h: Histograms){
    ngates.push_back(h -> getNGates());
  }

  return ngates;
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

StringVector EngeSort::getGateNames(std::string hname){

  StringVector gname;
  
  // First find the spectrum that corresponds to the hname
  for(auto h:Histograms){
    if(h -> getName() == hname){
      //std::cout << "Hist: " << hname << " has " << h -> getNGates() << " gates" << std::endl;
      // Make sure this histogram has gates defined
      for(int i=0; i< (h -> getNGates()); i++){
	Gate G1 = h->getGate(i);
	gname.push_back(G1.getName());
      }
    }
  }

  return gname;
}

void EngeSort::putGate(std::string name, std::string gname, p::list x, p::list y){

  //std::cout << "Putting gate: " << gname << " into histogram " << name << std::endl;
  
  // First find the spectrum that corresponds to the name
  for(auto h:Histograms){
    if(h -> getName() == name){
      //std::cout << "Found the histogram! With name: " << h->getName() << std::endl;

      // Make sure this histogram has gates defined
      //if(h -> getNGates() > 0){
      //std::cout << "Yes, this histogram has gates!" << std::endl;
      for(int ig = 0; ig < (h -> getNGates()); ig++){
	Gate &G1 = h->getGate(ig);
	if(G1.getName() == gname){

	  G1.Clear();
	  //G1.Print();

	  p::ssize_t len = p::len(x);
	  // Make a vector for the gate
	  for(int i=0; i<len; i++){
	    std::vector<double> tmp;
	    tmp.push_back(p::extract<double>(x[i]));
	    tmp.push_back(p::extract<double>(y[i]));
	    G1.addVertex(tmp);
	  }
	  //G1.Print();
	}
      }
    }
  }
  
}

void EngeSort::ClearData(){

  for(auto h:Histograms){
    h->Clear();
  }

  for(auto Sclr: Scalers){
    Sclr -> Clear();
  }
  totalCounter=0;
  
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
  printf("Finish!\n");
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

  //  std::cout << "Analyzing" << std::endl;

  if(event->event_id == 1){

    event->FindAllBanks();
    //std::cout << event->BankListToString() << std::endl;
    
    // Get the Front bank
    TMBank* bMDP1 = event->FindBank("MDP1");
    uint32_t* dMDP1 = (uint32_t*)event->GetBankData(bMDP1);
    // Get the size
    int nMDP1 = (bMDP1->data_size)/4;

		// Get the Back bank
    TMBank* bMDP2 = event->FindBank("MDP2");
    uint32_t* dMDP2 = (uint32_t*)event->GetBankData(bMDP2);
    // Get the size
    int nMDP2 = (bMDP2->data_size)/4;

    fRunEventCounter++;
    fModule->fTotalEventCounter++;
    //  std::cout << "Calling sort" << std::endl;
    fModule->eA->sort(dMDP1, nMDP1, dMDP2, nMDP2);

  } else if(event->event_id == 2){
		// Shouldn't land here until we define scalers in the frontend
		
  }

  return flow;

}

/* 
   manalyzer run
*/
void MidasAnalyzerRun::BeginRun(TARunInfo* runinfo){
  printf("Begin run %d\n",runinfo->fRunNo);
  uint32_t run_start_time_binary = 0;
  //runinfo->fOdb->RU32("/Runinfo/Start time binary", &run_start_time_binary);
  time_t run_start_time = run_start_time_binary;
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
  class_<IntVector>("IntVector")
    .def(vector_indexing_suite<IntVector>());
    
  class_<EngeSort>("EngeSort")
    .def("sayhello", &EngeSort::sayhello)          // string
    .def("saygoodbye", &EngeSort::saygoodbye)          // string
		.def("saysomething", &EngeSort::saysomething)      // string
    .def("Initialize", &EngeSort::Initialize)          // void
    .def("connectMidasAnalyzer", &EngeSort::connectMidasAnalyzer) // int
    .def("runMidasAnalyzer", &EngeSort::runMidasAnalyzer) // int
    .def("getData", &EngeSort::getData)                // 1D histograms
    .def("getData2D", &EngeSort::getData2D)            // 2D histograms
    .def("getis2Ds", &EngeSort::getis2Ds)                // bool vector
    .def("getNGates", &EngeSort::getNGates)            // int vector
    .def("getSpectrumNames", &EngeSort::getSpectrumNames) // string vector
    .def("getIsRunning", &EngeSort::getIsRunning)        // bool value
    .def("getScalerNames", &EngeSort::getScalerNames)     // string vector
    .def("getScalers", &EngeSort::getScalers)             // IntVector of scaler values
    .def("getGateNames", &EngeSort::getGateNames)             // string vector of gate names
    .def("ClearData", &EngeSort::ClearData)        // void
    .def("putGate", &EngeSort::putGate)            // void
    .def("data", range(&EngeSort::begin, &EngeSort::end))
    //    .def("Histogram1D", &Histogram1D::Histogram1D)
    ;

}
