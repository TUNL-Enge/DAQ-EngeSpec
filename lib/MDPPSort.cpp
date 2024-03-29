#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include "MDPPSort.h"
#include "TV792Data.hxx"

Messages messages;

std::string EngeSort::sayhello( ) {
  return messages.sayhello("MDPPSort");
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

// Scalers
Scaler *sPulser;
Scaler *sTriggers;

// 1D Spectra
Histogram *hDet1;
Histogram *hDet1_pu;
Histogram *hDet2;
Histogram *hDet3;
Histogram *hDet4;
Histogram *hDet5;
Histogram *hDet6;
Histogram *hDet7;
Histogram *hDet8;
Histogram *hDet9;
Histogram *hDet10;
Histogram *hDet11;
Histogram *hDet12;
Histogram *hDet13;
Histogram *hDet14;
Histogram *hDet15;
Histogram *hDet16;



Histogram *hTDC_Det0;
Histogram *hTDC_Det1;
Histogram *hTDC_Det2;

// 2D Spectra
Histogram *hDet2vsDet1;

// Gated Spectra
Histogram *hDet1_gDet2vsDet1_G1;

// 1D Gated Timing?
Histogram *hDet0_gTDC0;
Histogram *hDet1_gTDC1;
Histogram *hDet2_gTDC2;

// Summed Spectra
Histogram *hsumNaI;

// Counters
int totalCounter=0;

void EngeSort::Initialize(){

  std::string hname;
  
  //--------------------
  // 1D Histograms
  hDet1 = new Histogram("Det 1", Channels1D, 1);
  hDet1_pu = new Histogram("Det 1 - no pileup", Channels1D, 1);
  hDet2 = new Histogram("Det 2", Channels1D, 1);
	hDet3 = new Histogram("Det 3", Channels1D, 1);
	hDet4 = new Histogram("Det 4", Channels1D, 1);
	hDet5 = new Histogram("Det 5", Channels1D, 1);
	hDet6 = new Histogram("Det 6", Channels1D, 1);
	hDet7 = new Histogram("Det 7", Channels1D, 1);
	hDet8 = new Histogram("Det 8", Channels1D, 1);
	hDet9 = new Histogram("Det 9", Channels1D, 1);
	hDet10 = new Histogram("Det 10", Channels1D, 1);
	hDet11 = new Histogram("Det 11", Channels1D, 1);
	hDet12 = new Histogram("Det 12", Channels1D, 1);
	hDet13 = new Histogram("Det 13", Channels1D, 1);
	hDet14 = new Histogram("Det 14", Channels1D, 1);
	hDet15 = new Histogram("Det 15", Channels1D, 1);
	hDet16 = new Histogram("Det 16", Channels1D, 1);

  hTDC_Det0 = new Histogram("TDC Det0", Channels1D, 1);
  hTDC_Det1 = new Histogram("TDC Det1", Channels1D, 1);
  hTDC_Det2 = new Histogram("TDC Det2", Channels1D, 1);
  //--------------------

	//--------------------
  sPulser = new Scaler("Pulser", 0);    // Name, index
  sTriggers = new Scaler("Triggers", 1);    // Name, index
  //--------------------

	
  //--------------------
  // 2D Histograms
  hDet2vsDet1 = new Histogram("DE vs Pos1", Channels2D, 2);
  //--------------------

	//--------------------
  // Gated Histograms
  hDet1_gDet2vsDet1_G1 = new Histogram("Det 1; GDet2vsDet1-G1", Channels1D, 1);

	hDet0_gTDC0 = new Histogram("Det 0; GDet0vsTDC0_G0",Channels1D, 1);
	hDet1_gTDC1 = new Histogram("Det 1; GDet1vsTDC1_G1",Channels1D, 1);
	hDet2_gTDC2 = new Histogram("Det 2; GDet2vsTDC2_G2",Channels1D, 1);
  //--------------------

	//--------------------
	//Summed Spectra
	hsumNaI = new Histogram("Summed NaI Spectra",Channels1D, 1);

	//--------------------


	// Gates
  //  hE -> addGate("Energy Gate");
  
  hDet2vsDet1 -> addGate("Gate");

	hTDC_Det0 -> addGate("Gate");
	hTDC_Det1 -> addGate("Gate");
	hTDC_Det2 -> addGate("Gate");
	
	
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


  // Build the scalers
  // Not currently used

 
}

//======================================================================
// This is the equivalent to the "sort" function in jam
// Arguments are the raw events
void EngeSort::sort(uint32_t *dMDPP, int nMDPP){

  totalCounter++;
  //  double ADCsize = sizeof(&dADC)/sizeof(&dADC[0]);
  //std::cout << nMDPP << std::endl;
  
  // Thresholds
  /*
  int Threshold = 10;
  for(int i=0; i<ADCsize; i++)
    if(dADC[i] < Threshold || dADC[i] > Channels1D)dADC[i]=0;
  for(int i=0; i<TDCsize; i++)
    if(dTDC[i] < Threshold || dTDC[i] > Channels1D)dTDC[i]=0;
  */

  // Cade: here's an example
  //  std::cout << "nMDPP = " << nMDPP << "\n";
  
  /* 
     Here is where the raw data gets split into voltage and timing information
  */
  int dADC[16] = {0};   // stores energies
  int dADC_pu[16] = {0}; // Energies removing pileup flagged pulses
  int dTDC[16] = {0};   // stores times
  
  for(int i = 0; i < nMDPP; i++){
		// if (((dMDPP[i] >> 30) & 0x3) == 3){
		// 	int temp = dMDPP[i] & 0x3FFFFFFF;
		// 	if (temp != 1073741823){
		// 	printf("Event: %d\n", temp);
		// 	}
		// 	continue;
		// }
    if ((dMDPP[i] & 0xF0000000) != 0x10000000){
      continue;
    }
    int signal = dMDPP[i] & 0xFFFF;    // either time or energy
    int chn = (dMDPP[i] >> 16) & 0x1F;
    // ERROR: Channels 1-16 are energy readings
    //        Channels 17-32 are time readings (chn = chn-16) Manual P. 25
    int pu = (dMDPP[i] >> 23) & 0x1;  // pile-up
    int ov = (dMDPP[i] >> 24) & 0x1;  // overflow
    // std::cout << "i: " << i << " chn = " << chn << " signal = " << signal
		// 					 << "\n" ;
   
    if(chn <= 15){
      dADC[chn] = signal;
      if((!pu ) & (!ov))dADC_pu[chn] = signal;
    }
    else if(chn > 15){
      dTDC[chn-16] = signal;
    }
  }

	int cSumNaI = 0; //sum of spectra 
	for (int i = 0; i < 16; i++){
		cSumNaI += dADC[i];
	}

	//  std::cout << "Assigned data to channels\n";

  // Define the channels
  int cDet1 = (int)std::round(dADC[0]/comp1d);
  int cDet1_pu = (int)std::round(dADC_pu[0]/comp1d);
  int cDet2 = (int)std::round(dADC[1]/comp1d);
  //  int cDet1 = dADC[0];
  //  int cDet2 = dADC[1];
  int cDet3 = dADC[2];
  int cDet4 = dADC[3];
  int cDet5 = dADC[4];
  int cDet6 = dADC[5];
  int cDet7 = dADC[6];
  int cDet8 = dADC[7]; 
  int cDet9 = dADC[8];
  int cDet10 = dADC[9];
  int cDet11 = dADC[10];
  int cDet12 = dADC[11];
  int cDet13 = dADC[12];
  int cDet14 = dADC[13];
  int cDet15 = dADC[14];
  int cDet16 = dADC[15];
  


  int cTDC_Det0 = dTDC[0]; //(int)std::round(dTDC[0]/comp1d);
  int cTDC_Det1 = dTDC[1]; //(int)std::round(dTDC[1]/comp1d);
	int cTDC_Det2 = dTDC[2];
  int cTDC_Det3 = dTDC[3];
  int cTDC_Det4 = dTDC[4];
  int cTDC_Det5 = dTDC[5];
  int cTDC_Det6 = dTDC[6];
  int cTDC_Det7 = dTDC[7]; 
  int cTDC_Det8 = dTDC[8];
  int cTDC_Det9 = dTDC[9];
  int cTDC_Det10 = dTDC[10];
  int cTDC_Det11 = dTDC[11];
  int cTDC_Det12 = dTDC[12];
  int cTDC_Det13 = dTDC[13];
  int cTDC_Det14 = dTDC[14];
  int cTDC_Det15 = dTDC[15];

  // Below this point only deal in 'c' values, which are the
  // compressed values

  // ------------------------------------------------------------
  // Compressed versions for 2D spectra
  // ------------------------------------------------------------
  double compression = 10.0; //(double)Channels1D/ (double)Channels2D;
  int cDet1comp = (int) std::floor(cDet7 / compression);
  int cDet2comp = (int) std::floor(cDet15 / 20.0);
  
  // Increment 1D histograms
  hDet1 -> inc(cDet1);
  hDet1_pu -> inc(cDet1_pu);
  hDet2 -> inc(cDet2);
	hDet3 -> inc(cDet3);
	hDet4 -> inc(cDet4);
	hDet5 -> inc(cDet5);
	hDet6 -> inc(cDet6);
	hDet7 -> inc(cDet7);
	hDet8 -> inc(cDet8);
	hDet9 -> inc(cDet9);
	hDet10 -> inc(cDet10);
	hDet11 -> inc(cDet11);
	hDet12 -> inc(cDet12);
	hDet13 -> inc(cDet13);
	hDet14 -> inc(cDet14);
	hDet15 -> inc(cDet15);
	hDet16 -> inc(cDet16);
	
  //std::cout << "Incremented 1d spec" << std::endl;
  
  hTDC_Det0 -> inc(cTDC_Det0);
  hTDC_Det1 -> inc(cTDC_Det1);
  hTDC_Det2 -> inc(cTDC_Det2);

	//std::cout << "Incremented TDC" << std::endl;

	hsumNaI -> inc(cSumNaI);

	
  // Increment 2D histograms
  hDet2vsDet1 -> inc(cDet1comp, cDet2comp);

  //std::cout << "2D spect" << std::endl;
  
  // The gated spectrum
  //std::cout << "gated spec" << std::endl; 

  Gate &G1 = hDet2vsDet1 -> getGate(0);
	Gate &TG0 = hTDC_Det0 -> getGate(0);
	Gate &TG1 = hTDC_Det1 -> getGate(0);
	Gate &TG2 = hTDC_Det2 -> getGate(0);
	
	
	//	G1.Print();
  if(G1.inGate(cDet1comp,cDet2comp)){
    hDet1_gDet2vsDet1_G1->inc(cDet7);
  }
	
	if(TG0.inGate(cTDC_Det0)){
		hDet0_gTDC0->inc(cDet1);
	}

	if(TG1.inGate(cTDC_Det1)){
		hDet1_gTDC1->inc(cDet2);
	}

	if(TG2.inGate(cTDC_Det2)){
		hDet2_gTDC2->inc(cDet3);
	}
}


// Increment the scalers
// TODO: make this automatic. If the scaler is
// defined we should assume that the user wants to increment it
void EngeSort::incScalers(uint32_t *dSCAL){
	sPulser -> inc(dSCAL);
	sTriggers -> inc(dSCAL);
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
    
    // Get the ADC Bank
    TMBank* bMDPP = event->FindBank("MDPP");
    uint32_t* dMDPP = (uint32_t*)event->GetBankData(bMDPP);

    // Get the size
    
    //  int nMDPP = (bMDPP->data_size - 2)/4;
    int nMDPP = (bMDPP->data_size)/4;
    //  std::cout << "ADC Size: " << bMDPP->data_size << std::endl;
  
    fRunEventCounter++;
    fModule->fTotalEventCounter++;
    //  std::cout << "Calling sort" << std::endl;
    fModule->eA->sort(dMDPP, nMDPP);

  } else if(event->event_id == 2){

    // Get the Scaler Bank
    // Get the Scaler Bank
    TMBank* bSCAL = event->FindBank("SCLR");
    uint32_t *dSCAL = (uint32_t*)event->GetBankData(bSCAL);

    fModule->eA->incScalers(dSCAL);
  }

  //  STD::cout << bSCAL << "  " << dSCAL << std::endl;

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
