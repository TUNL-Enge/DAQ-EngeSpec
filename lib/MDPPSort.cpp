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


/* ------------- 
Global Variables 
----------------*/

// binning to use in the histograms
int Channels1D = 65536;
int Channels2D = 512;


// Scalers
Scaler *sPulser;
Scaler *sTriggers;

// 1D Spectra

// Annulus Energy Histograms
Histogram *hADCNaI0;
Histogram *hADCNaI1;
Histogram *hADCNaI2;
Histogram *hADCNaI3;
Histogram *hADCNaI4;
Histogram *hADCNaI5;
Histogram *hADCNaI6;
Histogram *hADCNaI7;
Histogram *hADCNaI8;
Histogram *hADCNaI9;
Histogram *hADCNaI10;
Histogram *hADCNaI11;
Histogram *hADCNaI12;
Histogram *hADCNaI13;
Histogram *hADCNaI14;
Histogram *hADCNaI15;

// Annulus Timing Histograms
Histogram *hTDCNaI0;
Histogram *hTDCNaI1;
Histogram *hTDCNaI2;
Histogram *hTDCNaI3;
Histogram *hTDCNaI4;
Histogram *hTDCNaI5;
Histogram *hTDCNaI6;
Histogram *hTDCNaI7;
Histogram *hTDCNaI8;
Histogram *hTDCNaI9;
Histogram *hTDCNaI10;
Histogram *hTDCNaI11;
Histogram *hTDCNaI12;
Histogram *hTDCNaI13;
Histogram *hTDCNaI14;
Histogram *hTDCNaI15;

// Annulus Group Variables
Histogram *hNaIsum;
Histogram *hNaITDC;
Histogram *hMulti;

// HPGe Singles
Histogram *hHPGe;

// Pulser
Histogram *hPulser;

// 2D Spectra
Histogram *hHPGevNaIsum;
Histogram *hHPGevNaITDC;


// Gated Spectra
Histogram *ghHPGeE;
Histogram *ghHPGeT;

// Gated Timing Spectra
Histogram *ghNaI_0TDC_0;
Histogram *ghNaI_1TDC_1;
Histogram *ghNaI_2TDC_2;
Histogram *ghNaI_3TDC_3;
Histogram *ghNaI_4TDC_4;
Histogram *ghNaI_5TDC_5;
Histogram *ghNaI_6TDC_6;
Histogram *ghNaI_7TDC_7;
Histogram *ghNaI_8TDC_8;
Histogram *ghNaI_9TDC_9;
Histogram *ghNaI_10TDC_10;
Histogram *ghNaI_11TDC_11;
Histogram *ghNaI_12TDC_12;
Histogram *ghNaI_13TDC_13;
Histogram *ghNaI_14TDC_14;
Histogram *ghNaI_15TDC_15;

// Counters
int totalCounter=0;

void EngeSort::Initialize(){

  std::string hname;

	// We initialize the histograms defined above.

	
  //--------------------
  // 1D Histograms

	hADCNaI0 = new Histogram("NaI 0", Channels1D, 1); 
	hADCNaI1 = new Histogram("NaI 1", Channels1D, 1); 
	hADCNaI2 = new Histogram("NaI 2", Channels1D, 1); 
	hADCNaI3 = new Histogram("NaI 3", Channels1D, 1); 
	hADCNaI4 = new Histogram("NaI 4", Channels1D, 1); 
	hADCNaI5 = new Histogram("NaI 5", Channels1D, 1); 
	hADCNaI6 = new Histogram("NaI 6", Channels1D, 1); 
	hADCNaI7 = new Histogram("NaI 7", Channels1D, 1); 
	hADCNaI8 = new Histogram("NaI 8", Channels1D, 1); 
	hADCNaI9 = new Histogram("NaI 9", Channels1D, 1); 
	hADCNaI10 = new Histogram("NaI 10", Channels1D, 1);
	hADCNaI11 = new Histogram("NaI 11", Channels1D, 1);
	hADCNaI12 = new Histogram("NaI 12", Channels1D, 1);
	hADCNaI13 = new Histogram("NaI 13", Channels1D, 1);
	hADCNaI14 = new Histogram("NaI 14", Channels1D, 1);
	hADCNaI15 = new Histogram("NaI 15", Channels1D, 1);


	hTDCNaI0 = new Histogram("TDC 0", Channels1D, 1); 
	hTDCNaI1 = new Histogram("TDC 1", Channels1D, 1); 
	hTDCNaI2 = new Histogram("TDC 2", Channels1D, 1); 
	hTDCNaI3 = new Histogram("TDC 3", Channels1D, 1); 
	hTDCNaI4 = new Histogram("TDC 4", Channels1D, 1); 
	hTDCNaI5 = new Histogram("TDC 5", Channels1D, 1); 
	hTDCNaI6 = new Histogram("TDC 6", Channels1D, 1); 
	hTDCNaI7 = new Histogram("TDC 7", Channels1D, 1); 
	hTDCNaI8 = new Histogram("TDC 8", Channels1D, 1); 
	hTDCNaI9 = new Histogram("TDC 9", Channels1D, 1); 
	hTDCNaI10 = new Histogram("TDC 10", Channels1D, 1);
	hTDCNaI11 = new Histogram("TDC 11", Channels1D, 1);
	hTDCNaI12 = new Histogram("TDC 12", Channels1D, 1);
	hTDCNaI13 = new Histogram("TDC 13", Channels1D, 1);
	hTDCNaI14 = new Histogram("TDC 14", Channels1D, 1);
	hTDCNaI15 = new Histogram("TDC 15", Channels1D, 1);



	
	hNaIsum = new Histogram("NaI Sum", Channels1D, 1);
	hNaITDC = new Histogram("NaI Total TDC", Channels1D, 1);
	hMulti = new Histogram("NaI Multi", Channels1D, 1);

	hHPGe = new Histogram("HPGe", Channels1D, 1);

	hPulser = new Histogram("Pulser", Channels1D, 1);


	//--------------------

	//--------------------
	// Scalers
  sPulser = new Scaler("Pulser", 0);    // Name, index
  sTriggers = new Scaler("Triggers", 1);    // Name, index
  //--------------------

	
  //--------------------
  // 2D Histograms
  //--------------------

	hHPGevNaIsum = new Histogram("HPGe v NaI", Channels2D, 2);
	hHPGevNaITDC = new Histogram("HPGe v TDC NaI", Channels2D, 2);

	
	//--------------------
  // Gated Histograms
  //--------------------

	ghHPGeE = new Histogram("Gated E", Channels1D, 1);
	ghHPGeT = new Histogram("Gated T", Channels1D, 1);

	//Gated Timing Histogram
	ghNaI_0TDC_0 = new Histogram("gated NaI_0 vs TDC 0", Channels1D,1);
	ghNaI_1TDC_1 = new Histogram("gated NaI 1 vs TDC 1", Channels1D,1);
	ghNaI_2TDC_2 = new Histogram("gated NaI 2 vs TDC 2", Channels1D,1);
	ghNaI_3TDC_3 = new Histogram("gated NaI 3 vs TDC 3", Channels1D,1);
	ghNaI_4TDC_4 = new Histogram("gated NaI 4 vs TDC 4", Channels1D,1);
	ghNaI_5TDC_5 = new Histogram("gated NaI 5 vs TDC 5", Channels1D,1);
	ghNaI_6TDC_6 = new Histogram("gated NaI 6 vs TDC 6", Channels1D,1);
	ghNaI_7TDC_7 = new Histogram("gated NaI 7 vs TDC 7", Channels1D,1);
	ghNaI_8TDC_8 = new Histogram("gated NaI 8 vs TDC 8", Channels1D,1);
	ghNaI_9TDC_9 = new Histogram("gated NaI 9 vs TDC 9", Channels1D,1);
	ghNaI_10TDC_10 = new Histogram("gated NaI 10 vs TDC 10", Channels1D,1);
	ghNaI_11TDC_11 = new Histogram("gated NaI 11 vs TDC 11", Channels1D,1);
	ghNaI_12TDC_12 = new Histogram("gated NaI 12 vs TDC 12", Channels1D,1);
	ghNaI_13TDC_13 = new Histogram("gated NaI 13 vs TDC 13", Channels1D,1);
	ghNaI_14TDC_14 = new Histogram("gated NaI 14 vs TDC 14", Channels1D,1);
	ghNaI_15TDC_15 = new Histogram("gated NaI 15 vs TDC 15", Channels1D,1);
	
	// Gates
  
	hHPGevNaIsum -> addGate("Energy Gate");
	hHPGevNaITDC -> addGate("Time Gate");
	hTDCNaI0 -> addGate("Time Gate");
  hTDCNaI1 -> addGate("Time Gate");
	hTDCNaI2 -> addGate("Time Gate");
	hTDCNaI3 -> addGate("Time Gate");
  hTDCNaI4 -> addGate("Time Gate");
	hTDCNaI5 -> addGate("Time Gate");
	hTDCNaI6 -> addGate("Time Gate");
  hTDCNaI7 -> addGate("Time Gate");
	hTDCNaI8 -> addGate("Time Gate");
	hTDCNaI9 -> addGate("Time Gate");
  hTDCNaI10 -> addGate("Time Gate");
	hTDCNaI11 -> addGate("Time Gate");
	hTDCNaI12 -> addGate("Time Gate");
  hTDCNaI13 -> addGate("Time Gate");
	hTDCNaI14 -> addGate("Time Gate");
	hTDCNaI15 -> addGate("Timing Gate");
}

enum EventType {long_integral, tdc, none};

struct QDCEvent {
	int channel;
	EventType event_type;
};

// Convert the raw channel number from the bank to the appropriate channel 
QDCEvent qdc_channel_assign(int nchannels, int raw_channel){

	// gets rounded down to give the case statement
	int chan_case = raw_channel/nchannels; 

	QDCEvent evt;
	
	switch(chan_case){
		
	case 0 :
		// This is the case of long integration value.
		evt.channel = raw_channel;
		evt.event_type = long_integral;
		break;
		
	case 1 :
		// TDC value
		evt.channel = (raw_channel - nchannels);
		evt.event_type = tdc;
		break;
		
	default :
		evt.channel = -1;
		evt.event_type = none;
		// I don't care about the rest for now.
	}

	return evt;
}

//======================================================================
// This is the equivalent to the "sort" function in jam
// Arguments are the raw events
void EngeSort::sort(uint32_t *dMDPP, int nMDPP){
  totalCounter++;

  int dADC[32] = {0};   // stores energies
  int dTDC[32] = {0};   // stores times
  
  for(int i = 0; i < nMDPP; i++){

		// skip non-data events
    if ((dMDPP[i] & 0xF0000000) != 0x10000000){
      continue;
    }
    int signal = dMDPP[i] & 0xFFFF;    // either time or energy
		
		int chn = (dMDPP[i] >> 16) & 0xFF; // 32 channels is 7 bit address
		//std::cout << "Channel: " << chn << " Energy: " << signal <<std::endl;

    int ov = (dMDPP[i] >> 24) & 0x1;  // overflow

		// CAM: Trying to make this a bit more flexible, but it might be needlessly complicated.
		QDCEvent evt = qdc_channel_assign(32, chn);

		switch(evt.event_type){

		case long_integral :
			dADC[evt.channel] = signal;
			break;
			
		case tdc :
			dTDC[evt.channel] = signal;
			break;
			
		case none :
			break;
						
		}
		
  }

	int SumNaI = 0; //sum of spectra
	int multi = 0; // multiplicity
	int NaITDC = 0; // combined timing spectra
	for (int i = 0; i < 16; i++){
		if (i == 2){
			// plug is the HPGe for now
			continue;
		}
		SumNaI += dADC[i];
		if (dADC[i] > 0){
			multi += 1;
		}
		// take the first hit
		if (dTDC[i] > 0) {
			if (((NaITDC != 0) && (dTDC[i] < NaITDC)) | (NaITDC == 0)) {
					NaITDC = dTDC[i];
			}
			
		}
	}


	

  // Below this point only deal in 'c' values, which are the
  // compressed values

  // ------------------------------------------------------------
  // Compressed versions for 2D spectra
  // ------------------------------------------------------------
  double compressionE = 5000.0/Channels2D; //(double)Channels1D/ (double)Channels2D;
	double compressionT = 1000.0/Channels2D; //(double)Channels1D/ (double)Channels2D;
  int cSum = (int) std::floor(SumNaI / compressionE);
  int cHPGe = (int) std::floor(dADC[2] / compressionE);
  int cTDC = (int) std::floor(dADC[2] / compressionT);
	
  
  // Increment 1D histograms
	hADCNaI0  -> inc(dADC[0]);
	hADCNaI1  -> inc(dADC[1]);
	hADCNaI2  -> inc(0); // not implemented right now.
	hADCNaI3  -> inc(dADC[3]);  
	hADCNaI4  -> inc(dADC[4]);
	hADCNaI5  -> inc(dADC[5]);
	hADCNaI6  -> inc(dADC[6]);
	hADCNaI7  -> inc(dADC[7]);
	hADCNaI8  -> inc(dADC[8]);
	hADCNaI9 	-> inc(dADC[9]);
	hADCNaI10	-> inc(dADC[10]);
	hADCNaI11	-> inc(dADC[11]);
	hADCNaI12	-> inc(dADC[12]);
	hADCNaI13	-> inc(dADC[13]);
	hADCNaI14	-> inc(dADC[14]);
	hADCNaI15	-> inc(dADC[15]);

	hTDCNaI0  -> inc(dTDC[0]);
	hTDCNaI1  -> inc(dTDC[1]);
	hTDCNaI2  -> inc(0); // not implemented right now.
	hTDCNaI3  -> inc(dTDC[3]);  
	hTDCNaI4  -> inc(dTDC[4]);
	hTDCNaI5  -> inc(dTDC[5]);
	hTDCNaI6  -> inc(dTDC[6]);
	hTDCNaI7  -> inc(dTDC[7]);
	hTDCNaI8  -> inc(dTDC[8]);
	hTDCNaI9 	-> inc(dTDC[9]);
	hTDCNaI10	-> inc(dTDC[10]);
	hTDCNaI11	-> inc(dTDC[11]);
	hTDCNaI12	-> inc(dTDC[12]);
	hTDCNaI13	-> inc(dTDC[13]);
	hTDCNaI14	-> inc(dTDC[14]);
	hTDCNaI15	-> inc(dTDC[15]);
	
  hNaIsum -> inc(SumNaI);
	hMulti -> inc(multi);
	hNaITDC -> inc(NaITDC);

	hHPGe -> inc(dADC[2]);

	hPulser -> inc(dADC[16]);
	
  // Increment 2D histograms
  hHPGevNaIsum -> inc(cHPGe, cSum);
  hHPGevNaITDC -> inc(cHPGe, cTDC);

  
  // The gated spectrum
	//TG : Timing Gate
	
  Gate &G1 = hHPGevNaIsum -> getGate(0);
	Gate &G2 = hHPGevNaITDC -> getGate(0);
	Gate &TG0 = ghNaI_0TDC_0 -> getGate(0);
	Gate &TG1 = ghNaI_1TDC_1 -> getGate(0);
	Gate &TG2 = ghNaI_2TDC_2 -> getGate(0);
	Gate &TG3 = ghNaI_3TDC_3 -> getGate(0);
	Gate &TG4 = ghNaI_4TDC_4 -> getGate(0);
	Gate &TG5 = ghNaI_5TDC_5 -> getGate(0);
	Gate &TG6 = ghNaI_6TDC_6 -> getGate(0);
	Gate &TG7 = ghNaI_7TDC_7 -> getGate(0);
	Gate &TG8 = ghNaI_8TDC_8 -> getGate(0);
	Gate &TG9 = ghNaI_9TDC_9 -> getGate(0);
	Gate &TG10 = ghNaI_10TDC_10 -> getGate(0);
	Gate &TG11 = ghNaI_11TDC_11 -> getGate(0);
	Gate &TG12 = ghNaI_12TDC_12 -> getGate(0);
	Gate &TG13 = ghNaI_13TDC_13 -> getGate(0);
	Gate &TG14 = ghNaI_14TDC_14 -> getGate(0);
	Gate &TG15 = ghNaI_15TDC_15 -> getGate(0);

	
  if(G1.inGate(cHPGe, cSum)){
    ghHPGeE->inc(dADC[2]);
  }

	if(G2.inGate(cHPGe, cTDC)){
    ghHPGeT->inc(dADC[2]);
  }

	if(TG0.inGate(dTDC[0])){
		ghNaI_0TDC_0 -> inc(dADC[0]);
		
	}
	if(TG1.inGate(dTDC[1])){
		ghNaI_1TDC_1 -> inc(dADC[1]);
		
	}
	if(TG2.inGate(dTDC[2])){
		ghNaI_2TDC_2 -> inc(dADC[2]);
		
	}

	if(TG3.inGate(dTDC[3])){
		ghNaI_3TDC_3 -> inc(dADC[3]);
		
	}

	if(TG4.inGate(dTDC[4])){
		ghNaI_4TDC_4 -> inc(dADC[4]);
		
	}
	if(TG5.inGate(dTDC[5])){
		ghNaI_5TDC_5 -> inc(dADC[5]);
		
	}
	if(TG6.inGate(dTDC[6])){
		ghNaI_6TDC_6 -> inc(dADC[6]);
		
	}
	if(TG7.inGate(dTDC[7])){
		ghNaI_7TDC_7 -> inc(dADC[7]);
		
	}
	if(TG8.inGate(dTDC[8])){
		ghNaI_8TDC_8 -> inc(dADC[8]);
		
	}
	if(TG9.inGate(dTDC[9])){
		ghNaI_9TDC_9 -> inc(dADC[9]);
		
	}
	if(TG10.inGate(dTDC[10])){
		ghNaI_10TDC_10 -> inc(dADC[10]);
		
	}
	if(TG11.inGate(dTDC[11])){
		ghNaI_11TDC_11 -> inc(dADC[11]);
		
	}

	if(TG12.inGate(dTDC[12])){
		ghNaI_12TDC_12 -> inc(dADC[12]);
		
	}
	if(TG13.inGate(dTDC[13])){
		ghNaI_13TDC_13 -> inc(dADC[13]);
		
	}
	if(TG14.inGate(dTDC[14])){
		ghNaI_14TDC_14 -> inc(dADC[14]);
		
	}
	if(TG15.inGate(dTDC[15])){
		ghNaI_15TDC_15 -> inc(dADC[15]);
		
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
    std::cout << event->BankListToString() << std::endl;
    
    // Get the Bank
    TMBank* bMDPP = event->FindBank("MDPP");
    uint32_t* dMDPP = (uint32_t*)event->GetBankData(bMDPP);

    // Get the size
    
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
