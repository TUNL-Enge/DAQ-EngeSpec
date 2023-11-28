#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>		
#include <sstream>

#include "MDPPEventHandler.h"
#include "MDPPSort.h"
#include "TV792Data.hxx"

#include <fstream>

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
int Channels2D = 1000;


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

// Plastic Scintillators
Histogram *hADCPS1;
Histogram *hADCPS2;
Histogram *hADCPS3;
Histogram *hADCPS4;
Histogram *hADCPS5;
Histogram *hADCPS6;
Histogram *hADCPS7;
Histogram *hADCPS8;
Histogram *hADCPS9;

// Energy Calibrated Spectra

Histogram *hHPGe_E;


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

// random number generator for calibration values.
std::random_device rand_dev;
std::mt19937 generator(rand_dev());
std::uniform_int_distribution<int> uniform_sample(-1, 1);


// Calibrator setup
void Calibrator::load_file(std::string filename){
	// store the fit calibration parameters
	this->slope.resize(32);
	std::fill(this->slope.begin(), this->slope.end(), 0);

	this->intercept.resize(32);
	std::fill(this->intercept.begin(), this->intercept.end(), 0);
		
	// read in the calibration
	std::ifstream file(filename);
	std::string line;
	auto delim = ',';
	std::string temp_string;
	bool first_line = true;
	
	while (std::getline(file, line)){
		// just skipping the headers for now.
		if (first_line){
			first_line = false;
			continue; 
		}

		// convert so we can iterate over the delimiters
		auto ss = std::stringstream(line);
		// these are the line variables
		int channel;
		int slope;
		int intercept;
		int col = 0;
			
		while (std::getline(ss, temp_string, delim)){
			// temp string holds each column
			// col is incremented each time and keeps track of the column
			switch (col) {
			case 0 :
				// bank name case, it is not implemented right now
				break;

			case 1:
				channel = std::stoi(temp_string);
				break;

			case 2:
				this->slope[channel] = std::stod(temp_string);
				break;

			case 3:
				this->intercept[channel] = std::stod(temp_string);
				break;
				
			default:
				throw; 
	
			}
			col += 1; 	
		}			
	}
}

IntVector Calibrator::calibrate(vec_u32 &adc_values){

	IntVector result(adc_values.size(), 0); 

	for (int i=0; i < adc_values.size(); i++){
		// do the calibration
		auto temp = (double)adc_values[i];
		// check to make sure we have a adc value and that we have a calibration
		if ((temp > 0) && (this->slope[i] > 0)){
			result[i] = std::round((this->slope[i] * temp + this->intercept[i])
														 + uniform_sample(generator));
		}
	}
	
	return result;
}


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


	hADCPS1 = new Histogram("Plastic Scint. 1", Channels1D, 1);
	hADCPS2 = new Histogram("Plastic Scint. 2", Channels1D, 1);
	hADCPS3 = new Histogram("Plastic Scint. 3", Channels1D, 1);
	hADCPS4 = new Histogram("Plastic Scint. 4", Channels1D, 1);
	hADCPS5 = new Histogram("Plastic Scint. 5", Channels1D, 1);
	hADCPS6 = new Histogram("Plastic Scint. 6", Channels1D, 1);
	hADCPS7 = new Histogram("Plastic Scint. 7", Channels1D, 1);
	hADCPS8 = new Histogram("Plastic Scint. 8", Channels1D, 1);
	hADCPS9 = new Histogram("Plastic Scint. 9", Channels1D, 1);


	
	hNaIsum = new Histogram("NaI Sum", Channels1D, 1);
	hNaITDC = new Histogram("NaI Total TDC", Channels1D, 1);
	hMulti = new Histogram("NaI Multi", Channels1D, 1);

	hHPGe = new Histogram("HPGe", Channels1D, 1);

	hHPGe_E = new Histogram("HPGe_Cal", Channels1D, 1);

	
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

	// Initialize the calibrator
	this->calibrator_annulus_ps.load_file("nai_cal.csv");
	this->calibrator_hpge.load_file("hpge_cal.csv");
	
}


//======================================================================
// This is the equivalent to the "sort" function in jam
// Arguments are the raw events
void EngeSort::sort(MDPPEvent& event_data){
  totalCounter++;


	// get the data to make it easier for later
	vec_u32 scp_adc = event_data.get_data("scp1").adc;
	vec_u32 scp_tdc = event_data.get_data("scp1").tdc;
	
	vec_u32 qdc_adc = event_data.get_data("qdc1").adc;
	vec_u32 qdc_tdc = event_data.get_data("qdc1").tdc;

	IntVector cal_values = this->calibrator_annulus_ps.calibrate(qdc_adc);
	IntVector hpge_cal_values = this->calibrator_hpge.calibrate(scp_adc);
	
  
  // // Increment 1D histograms
	hADCNaI0  -> inc(qdc_adc[0]);
	hADCNaI1  -> inc(qdc_adc[1]);
	hADCNaI2  -> inc(qdc_adc[2]);
	hADCNaI3  -> inc(qdc_adc[3]);  
	hADCNaI4  -> inc(qdc_adc[4]);
	hADCNaI5  -> inc(qdc_adc[5]);
	hADCNaI6  -> inc(qdc_adc[6]);
	hADCNaI7  -> inc(qdc_adc[7]);
	hADCNaI8  -> inc(qdc_adc[8]);
	hADCNaI9 	-> inc(qdc_adc[9]);
	hADCNaI10	-> inc(qdc_adc[10]);
	hADCNaI11	-> inc(qdc_adc[11]);
	hADCNaI12	-> inc(qdc_adc[12]);
	hADCNaI13	-> inc(qdc_adc[13]);
	hADCNaI14	-> inc(qdc_adc[14]);
	hADCNaI15	-> inc(qdc_adc[15]);

	hTDCNaI0  -> inc(qdc_tdc[0]);
	hTDCNaI1  -> inc(qdc_tdc[1]);
	hTDCNaI2  -> inc(qdc_tdc[2]);
	hTDCNaI3  -> inc(qdc_tdc[3]);  
	hTDCNaI4  -> inc(qdc_tdc[4]);
	hTDCNaI5  -> inc(qdc_tdc[5]);
	hTDCNaI6  -> inc(qdc_tdc[6]);
	hTDCNaI7  -> inc(qdc_tdc[7]);
	hTDCNaI8  -> inc(qdc_tdc[8]);
	hTDCNaI9 	-> inc(qdc_tdc[9]);
	hTDCNaI10	-> inc(qdc_tdc[10]);
	hTDCNaI11	-> inc(qdc_tdc[11]);
	hTDCNaI12	-> inc(qdc_tdc[12]);
	hTDCNaI13	-> inc(qdc_tdc[13]);
	hTDCNaI14	-> inc(qdc_tdc[14]);
	hTDCNaI15	-> inc(qdc_tdc[15]);

	hADCPS1 -> inc(qdc_adc[17]);
	hADCPS2 -> inc(qdc_adc[18]);
	hADCPS3 -> inc(qdc_adc[19]);
	hADCPS4 -> inc(qdc_adc[20]);
	hADCPS5 -> inc(qdc_adc[21]);
	hADCPS6 -> inc(qdc_adc[22]);
	hADCPS7 -> inc(qdc_adc[23]);
	hADCPS8 -> inc(qdc_adc[24]);
	hADCPS9 -> inc(qdc_adc[25]);

	
	double SumNaI = 0;
	int multi = 0;
	bool first_hit = true;
	int NaITDC;
	// sum up the calibrated annulus
	for(int i=0; i < 16; i++){
	  SumNaI += (double) cal_values[i];
	  if (qdc_adc[i] > 0) {
	    multi += 1;
	    if (first_hit) {
	      // This spectra will be the timing difference between the
	      // HPGe timing signal and first NaI segment that fired.
	      NaITDC = (scp_tdc[2] - qdc_tdc[i]);
	      first_hit = false;
	    }
	  }
	}

	
	
	hNaIsum -> inc(SumNaI);
	hMulti -> inc(multi);
	hNaITDC -> inc(NaITDC);

	hHPGe -> inc(scp_adc[0]); 
	hHPGe_E -> inc(hpge_cal_values[0]);
 	
  // ------------------------------------------------------------
  // Compressed versions for 2D spectra
  // ------------------------------------------------------------
  double compressionE = Channels2D/10000.0; //(double)Channels1D/ (double)Channels2D;
  double compressionT = Channels2D/10000.0; //(double)Channels1D/ (double)Channels2D;
  int cSum = (int) std::floor(SumNaI * compressionE);
  int cHPGe = (int) std::floor((double) hpge_cal_values[0] * compressionE);
  int cTDC = (int) std::floor(NaITDC / compressionT);

	

  // hPulser -> inc(qdc_adc[16]);
	
  // // Increment 2D histograms

  hHPGevNaIsum -> inc(cHPGe, cSum);
	
  hHPGevNaITDC -> inc(cHPGe, cTDC);
  
  // // The gated spectrum
  // //TG : Timing Gate
	
  Gate &G1 = hHPGevNaIsum -> getGate(0);
  Gate &G2 = hHPGevNaITDC -> getGate(0);
	
  Gate &TG0 = hTDCNaI0 -> getGate(0);
  Gate &TG1 = hTDCNaI1 -> getGate(0);
  Gate &TG2 = hTDCNaI2 -> getGate(0);
  Gate &TG3 = hTDCNaI3 -> getGate(0);
  Gate &TG4 = hTDCNaI4 -> getGate(0);
  Gate &TG5 = hTDCNaI5 -> getGate(0);
  Gate &TG6 = hTDCNaI6 -> getGate(0);
  Gate &TG7 = hTDCNaI7 -> getGate(0);
  Gate &TG8 = hTDCNaI8 -> getGate(0);
  Gate &TG9 = hTDCNaI9 -> getGate(0);
  Gate &TG10 = hTDCNaI10 -> getGate(0);
  Gate &TG11 = hTDCNaI11 -> getGate(0);
  Gate &TG12 = hTDCNaI12 -> getGate(0);
  Gate &TG13 = hTDCNaI13 -> getGate(0);
  Gate &TG14 = hTDCNaI14 -> getGate(0);
  Gate &TG15 = hTDCNaI15 -> getGate(0);


	
  if(G1.inGate(cHPGe, cSum)){
    ghHPGeE->inc(scp_adc[0]);
  }


  if(G2.inGate(cHPGe, cTDC)){
    ghHPGeT->inc(scp_adc[0]);
  }


  if(TG0.inGate(qdc_tdc[0])){
    ghNaI_0TDC_0 -> inc(qdc_adc[0]);
  }
	
  if(TG1.inGate(qdc_tdc[1])){
    ghNaI_1TDC_1 -> inc(qdc_adc[1]);
		
  }


  if(TG2.inGate(qdc_tdc[2])){
    ghNaI_2TDC_2 -> inc(qdc_adc[2]);
		
  }


  if(TG3.inGate(qdc_tdc[3])){
    ghNaI_3TDC_3 -> inc(qdc_adc[3]);
		
  }

  if(TG4.inGate(qdc_tdc[4])){
    ghNaI_4TDC_4 -> inc(qdc_adc[4]);
		
  }

  if(TG5.inGate(qdc_tdc[5])){
    ghNaI_5TDC_5 -> inc(qdc_adc[5]);
		
  }

  if(TG6.inGate(qdc_tdc[6])){
    ghNaI_6TDC_6 -> inc(qdc_adc[6]);
		
  }

  if(TG7.inGate(qdc_tdc[7])){
    ghNaI_7TDC_7 -> inc(qdc_adc[7]);
		
  }
  if(TG8.inGate(qdc_tdc[8])){
    ghNaI_8TDC_8 -> inc(qdc_adc[8]);
		
  }
  if(TG9.inGate(qdc_tdc[9])){
    ghNaI_9TDC_9 -> inc(qdc_adc[9]);
		
  }
  if(TG10.inGate(qdc_tdc[10])){
    ghNaI_10TDC_10 -> inc(qdc_adc[10]);
		
  }
  if(TG11.inGate(qdc_tdc[11])){
    ghNaI_11TDC_11 -> inc(qdc_adc[11]);
		
  }

  if(TG12.inGate(qdc_tdc[12])){
    ghNaI_12TDC_12 -> inc(qdc_adc[12]);
		
  }
  if(TG13.inGate(qdc_tdc[13])){
    ghNaI_13TDC_13 -> inc(qdc_adc[13]);
		
  }
  if(TG14.inGate(qdc_tdc[14])){
    ghNaI_14TDC_14 -> inc(qdc_adc[14]);
		
  }
  if(TG15.inGate(qdc_tdc[15])){
    ghNaI_15TDC_15 -> inc(qdc_adc[15]);
		
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


		/* This is where the changes from the single card version start.
			 
			 The first order attempt will be to assume that the event contains
			 at most a single bank from each card, and that banks in a single
			 event are true coincidence data (i.e no event building)
			 
		 */

		this->mdpp_event.read_banks(event);

		fRunEventCounter++;
		fModule->fTotalEventCounter++;
		//  std::cout << "Calling sort" << std::endl;
		fModule->eA->sort(this->mdpp_event);
		// ready the banks for the next iteration
		this->mdpp_event.clear_data();
  } else if(event->event_id == 2){

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
