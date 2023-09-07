#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include "v1730EngeSort.h"
#include "TV792Data.hxx"

Messages messages;

std::string EngeSort::sayhello( ) {
  return messages.sayhello("EngeSort v1730");
}
std::string EngeSort::saygoodbye( ) {
  return messages.saygoodbye();
}

int Channels1D = 8192;
int Channels2D = 512;

//---------------------------- Settings --------------------------------
// Coincidence Window for Focal Plane Detector Components (in ns)
int coinWindow = 5000;
// Automatically close the window when all appropriate coincidences have been recorded
bool autoCoinWindowClose = true;

// Coincidence Window for Silicon Detectors only (in ns)
int coinWindow_Si = 5000;
bool autoCoinWindowClose_Si = true;

// Scaling coin. time (for E vs t histograms) by this factor
// Scale is timeScale * 2 ns per bin [timeScale min = 1 (best resolution), max = 16]
int timeScale = 1;
//---------------------------------------------------------------------

//------------------ Global sort routine variables --------------------
int coinWindowStart;
int coinWindowStart_Si;
bool enableCoinWindow = false;
bool enableCoinWindow_Si = false;

// Flags to prevent more than one signal to be collected from a single detector during the coincidence window
bool flag_DE = false;
bool flag_FrontHE = false;
bool flag_FrontLE = false;
bool flag_BackHE = false;
bool flag_BackLE = false;
bool flag_Theta = false;

bool flag_SiE = false;
bool flag_SiDE = false;

// Data to save for each coincidence window (updated each window)
uint32_t save_FrontHE;
uint32_t save_FrontLE;
uint32_t save_BackHE;
uint32_t save_BackLE;
int save_Pos1;
int save_Pos2;
int save_DE;
int save_E;
int save_Theta;

int save_SiE;
int save_SiDE;
//----------------------------------------------------------------------

// 1D Spectra
Histogram *hPos1;
Histogram *hPos2;
Histogram *hDE;
Histogram *hE;
Histogram *hTheta;

Histogram *hSiE;
Histogram *hSiDE;

//Histogram *hTDC_E;
//Histogram *hTDC_DE;
//Histogram *hTDC_PosSum;

// 2D Spectra
Histogram *hDEvsPos1;
Histogram *hEvsPos1;
Histogram *hDEvsE;
Histogram *hPos2vsPos1;
Histogram *hThetavsPos1;

Histogram *hSiDEvsSiE;

// Gated Spectra
Histogram *hPos1_gDEvPos1_G1;
Histogram *hPos1_gDEvPos1_G2;

Histogram *hE_gE_G1;

// Counters
int totalCounter=0;
int gateCounter=0;

// Scalers
Scaler *sGates;
Scaler *sGatesLive;
Scaler *sClock;
Scaler *sClockLive;
Scaler *sFrontLE;
Scaler *sFrontHE;
Scaler *sBackLE;
Scaler *sBackHE;
Scaler *sE;
Scaler *sDE;
Scaler *BCI;

void EngeSort::Initialize(){

  //--------------------
  // 1D Histograms
  hPos1 = new Histogram("Position 1", Channels1D, 1);
  hPos2 = new Histogram("Position 2", Channels1D, 1);
  hDE = new Histogram("Delta E", Channels1D, 1);
  hE = new Histogram("E", Channels1D, 1);
  hTheta = new Histogram("Theta", Channels1D, 1);

  hSiE = new Histogram("Silicon E", Channels1D, 1);
  hSiDE = new Histogram("Silicon DE", Channels1D, 1);

  //hTDC_E = new Histogram("TDC E", Channels1D, 1);
  //hTDC_DE = new Histogram("TDC DE", Channels1D, 1);
  //hTDC_PosSum = new Histogram("TDC Position Sum", Channels1D, 1);
  
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
  hPos1_gDEvPos1_G1 = new Histogram("Pos 1; GDEvPos1-G1", Channels1D, 1);
  hPos1_gDEvPos1_G2 = new Histogram("Pos 1; GDEvPos1-G2", Channels1D, 1);

  hE_gE_G1 = new Histogram("E; GE-G1", Channels1D, 1);

  //--------------------
  // Gates
  //g2d_DEvsPos1_1 = new Gate("Gate 1");
  //g2d_DEvsPos1_2 = new Gate("Gate 2");
  //g2d_DEvsPos1_3 = new Gate("Gate 3");

  hE -> addGate("Energy Gate");
  
  hDEvsPos1 -> addGate("Protons");
  hDEvsPos1 -> addGate("Deuterons");
  //hDEvsPos1 -> addGate(g2d_DEvsPos1_2);
  //hDEvsPos1 -> addGate(g2d_DEvsPos1_3);


  // Build the scalers
  sGates = new Scaler("Total Gates", 0);    // Name, index
  sGatesLive = new Scaler("Total Gates Live", 1);    // Name, index
  
  sClock = new Scaler("Clock",2);
  sClockLive = new Scaler("Clock Live",3);
  sFrontLE = new Scaler("Front HE",4);
  sFrontHE = new Scaler("Front LE",5);
  sBackLE = new Scaler("Back HE",6);
  sBackHE = new Scaler("Back LE",7);
  sE = new Scaler("E",8);
  sDE = new Scaler("DE",9);
  BCI = new Scaler("BCI",15);

}

//======================================================================
// This is the equivalent to the "sort" function in jam
void EngeSort::sort(uint32_t *dADC, int nADC, uint32_t *dTDC, int nTDC){

  totalCounter++;

  // double ADCsize = sizeof(dADC)/sizeof(dADC[0]);
  // double TDCsize = sizeof(dTDC)/sizeof(dTDC[0]);
  // std::cout << ADCsize << "  " << TDCsize << std::endl;

  // v1730 Channels: 0 = Front HE, 1 = Front LE, 2 = Back HE, 3 = Back LE, 4 = E, 5 = DE, 6 = SiE, 7 = SiDE

  uint32_t dat, ch, timetag;
  int cDet;

  // Energies (+ Ch #) are even nADC counts, timetags are odd counts (see ReadQLong in v1730DPP.c)
  for(int i = 0; i<nADC; i+=2){
    // Define the channels
    dat = dADC[i] & 0xFFFF; // dADC[i] includes channel # and qlong
    ch = (dADC[i] & 0xFFFF0000) >> 16;
    cDet = (int) std::floor(dat/4.0);
    timetag = dADC[i+1];
    
    // Silicon detectors separate from focal plane trigger window
    // Triggering on either SiE or SiDE and closing window automatically after a coincidence
    if (enableCoinWindow_Si == false){
      if (cDet > 20 && cDet < Channels1D){
        if (ch == 6){ // SiE
          enableCoinWindow_Si = true;
          coinWindowStart_Si = (int) timetag;
          flag_SiE = true;
          save_SiE = cDet;
          hSiE -> inc(cDet);
        }
        else if (ch == 7){ // SiDE
          enableCoinWindow_Si = true;
          coinWindowStart_Si = (int) timetag;
          flag_SiDE = true;
          save_SiDE = cDet;
          hSiDE -> inc(cDet);
        }
      }
    }
    else{
      // Check if timetag is outside of Si coincidence window (taking into account timetag rollback to zero)
      int int_timetag = (int) timetag;
      if ((int_timetag > coinWindowStart_Si + coinWindow_Si) || (coinWindowStart_Si > 4294967296 - coinWindow_Si && int_timetag > coinWindowStart_Si + coinWindow_Si - 4294967296)){
        enableCoinWindow_Si = false;
        flag_SiE = false;
        flag_SiDE = false;
      }
      else{
        if (cDet > 20 && cDet < Channels1D){
          if (ch == 6 && flag_SiE == false){ // SiE
            flag_SiE = true;
            save_SiE = cDet;
            hSiE -> inc(cDet);

            // 2D SiDE vs SiE
            int scaled_SiE = (int) std::floor(save_SiE/4.0);
            int scaled_SiDE = (int) std::floor(save_SiDE/4.0);
            hSiDEvsSiE -> inc(scaled_SiE, scaled_SiDE);

            // Automatically close window if both SiE and SiDE signals have been recorded
            if (autoCoinWindowClose_Si == true){
              enableCoinWindow_Si = false;
              flag_SiE = false;
              flag_SiDE = false;
            }
          }
          else if (ch == 7 && flag_SiDE == false){ // SiDE
            flag_SiDE = true;
            save_SiDE = cDet;
            hSiDE -> inc(cDet);

            // 2D SiDE vs SiE
            int scaled_SiE = (int) std::floor(save_SiE/4.0);
            int scaled_SiDE = (int) std::floor(save_SiDE/4.0);
            hSiDEvsSiE -> inc(scaled_SiE, scaled_SiDE);

            // Automatically close window if both SiE and SiDE signals have been recorded
            if (autoCoinWindowClose_Si == true){
              enableCoinWindow_Si = false;
              flag_SiE = false;
              flag_SiDE = false;
            }
          }
        }
      }
    }

    // Triggering on scintillator (E)
    if (enableCoinWindow == false){
      if (cDet > 20 && cDet < Channels1D){
        if (ch == 4){ // E
          enableCoinWindow = true;
          coinWindowStart = (int) timetag;
          save_E = cDet;
          hE -> inc(cDet);
          // E Gate
          Gate &G = hE -> getGate(0);
          if (G.inGate(cDet)){
            hE_gE_G1 -> inc(cDet);
          }
        }
      }
    }
    else{
      // Check if timetag is outside of scintillator trigger window (taking into account timetag rollback to zero)
      int int_timetag = (int) timetag;
      if ((int_timetag > coinWindowStart + coinWindow) || (coinWindowStart > 4294967296 - coinWindow && int_timetag > coinWindowStart + coinWindow - 4294967296)){
        enableCoinWindow = false;
        flag_DE = false;
        flag_FrontHE = false;
        flag_FrontLE = false;
        flag_BackHE = false;
        flag_BackLE = false;
        flag_Theta = false;
      }
      else{
        if (cDet > 20 && cDet < Channels1D){
          if (ch == 5 && flag_DE == false){ // DE
            flag_DE = true;
            save_DE = cDet;
            hDE -> inc(save_DE);

            // 2D DE vs E
            int scaled_E = (int) std::floor(save_E/4.0);
            int scaled_DE = (int) std::floor(save_DE/4.0);
            hDEvsE -> inc(scaled_E, scaled_DE);

            // 2D DE vs Pos1
            if (flag_FrontHE == true && flag_FrontLE == true){
              // Cut off the outer-edges of the time range with center at 256. Adjust scaling by timeScale setting
              int scaled_Pos1 = ((save_Pos1 - (Channels1D / 2)) / timeScale) + (Channels2D / 2);
              hDEvsPos1 -> inc(scaled_Pos1, scaled_DE);

              // DE vs Pos1 Gates
              Gate &G1 = hDEvsPos1 -> getGate(0);
              //G1.Print();
              if(G1.inGate(scaled_Pos1,scaled_DE)){
                gateCounter++;
                hPos1_gDEvPos1_G1 -> inc(save_Pos1);
              }

              Gate &G2 = hDEvsPos1 -> getGate(1);
              //G2.Print();
              if(G2.inGate(scaled_Pos1,scaled_DE)){
                gateCounter++;
                hPos1_gDEvPos1_G2 -> inc(save_Pos1);
              }
            }
          }
        }
        if (ch == 0 && flag_FrontHE == false){
          flag_FrontHE = true;
          save_FrontHE = timetag;
          // Coincidence Pos1
          if (flag_FrontLE == true){
            int int_FrontHE = (int) save_FrontHE;
            int int_FrontLE = (int) save_FrontLE; 
            int diff_Pos1 = int_FrontHE - int_FrontLE; // HE - LE is the referrence
            save_Pos1 = diff_Pos1 + (Channels1D / 2); // offset to center
            hPos1 -> inc(save_Pos1);

            // 2D E vs Pos1
            int scaled_E = (int) std::floor(save_E/4.0);
            int scaled_Pos1 = ((save_Pos1 - (Channels1D / 2)) / timeScale) + (Channels2D / 2);
            hEvsPos1 -> inc(scaled_Pos1, scaled_E);

            // 2D DE vs Pos1
            if (flag_DE == true){
              int scaled_DE = (int) std::floor(save_DE/4.0);
              hDEvsPos1 -> inc(scaled_Pos1, scaled_DE);

              // DE vs Pos1 Gates
              Gate &G1 = hDEvsPos1 -> getGate(0);
              //G1.Print();
              if(G1.inGate(scaled_Pos1,scaled_DE)){
                gateCounter++;
                hPos1_gDEvPos1_G1 -> inc(save_Pos1);
              }

              Gate &G2 = hDEvsPos1 -> getGate(1);
              //G2.Print();
              if(G2.inGate(scaled_Pos1,scaled_DE)){
                gateCounter++;
                hPos1_gDEvPos1_G2 -> inc(save_Pos1);
              }
            }
          }
        }
        else if (ch == 1 && flag_FrontLE == false){
          flag_FrontLE = true;
          save_FrontLE = timetag;
          // Coincidence Pos1
          if (flag_FrontHE == true){
            int int_FrontHE = (int) save_FrontHE;
            int int_FrontLE = (int) save_FrontLE;
            int diff_Pos1 = int_FrontHE - int_FrontLE; // HE - LE is the referrence
            save_Pos1 = diff_Pos1 + (Channels1D / 2); // offset to center
            hPos1 -> inc(save_Pos1);

            // 2D E vs Pos1
            int scaled_E = (int) std::floor(save_E/4.0);
            int scaled_Pos1 = ((save_Pos1 - (Channels1D / 2)) / timeScale) + (Channels2D / 2);
            hEvsPos1 -> inc(scaled_Pos1, scaled_E);

            // 2D DE vs Pos1
            if (flag_DE == true){
              int scaled_DE = (int) std::floor(save_DE/4.0);
              hDEvsPos1 -> inc(scaled_Pos1, scaled_DE);

              // DE vs Pos1 Gates
              Gate &G1 = hDEvsPos1 -> getGate(0);
              //G1.Print();
              if(G1.inGate(scaled_Pos1,scaled_DE)){
                gateCounter++;
                hPos1_gDEvPos1_G1 -> inc(save_Pos1);
              }

              Gate &G2 = hDEvsPos1 -> getGate(1);
              //G2.Print();
              if(G2.inGate(scaled_Pos1,scaled_DE)){
                gateCounter++;
                hPos1_gDEvPos1_G2 -> inc(save_Pos1);
              }
            }
          }
        }
        else if (ch == 2 && flag_BackHE == false){
          flag_BackHE = true;
          save_BackHE = timetag;
          // Coincidence Pos2
          if (flag_BackLE == true){
            int int_BackHE = (int) save_BackHE;
            int int_BackLE = (int) save_BackLE;
            int diff_Pos2 = int_BackHE - int_BackLE; // HE - LE is the referrence
            save_Pos2 = diff_Pos2 + (Channels1D / 2); // offset to center
            hPos2 -> inc(save_Pos2);
          }
        }
        else if (ch == 3 && flag_BackLE == false){
          flag_BackLE = true;
          save_BackLE = timetag;
          // Coincidence Pos2
          if (flag_BackHE == true){
            int int_BackHE = (int) save_BackHE;
            int int_BackLE = (int) save_BackLE;
            int diff_Pos2 = int_BackHE - int_BackLE; // HE - LE is the referrence
            save_Pos2 = diff_Pos2 + (Channels1D / 2); // offset to center
            hPos2 -> inc(save_Pos2);
          }
        }
        // Theta and Pos2 vs Pos1
        if (flag_FrontHE == true && flag_FrontLE == true && flag_BackHE == true && flag_BackLE == true && flag_Theta == false){
          flag_Theta == true;
          int Theta = (int) std::round(10000.0*atan((save_Pos2 - save_Pos1)/100.)/3.1415 - 4000.);
          save_Theta = std::max(0,Theta);
          hTheta -> inc(save_Theta);
          // Theta vs Pos1
          int scaled_Pos1 = ((save_Pos1 - (Channels1D / 2)) / timeScale) + (Channels2D / 2);
          hThetavsPos1 -> inc(scaled_Pos1, save_Theta); // Should theta be scaled? It is in EngeSort. But should this be treated like a coin. time or energy or something else?
          // Pos2 vs Pos1
          int scaled_Pos2 = ((save_Pos2 - (Channels1D / 2)) / timeScale) + (Channels2D / 2);
          hPos2vsPos1 -> inc(scaled_Pos1, scaled_Pos2);
        }
        // Automatically close window if all coincidences have been recorded
        if (flag_FrontHE == true && flag_FrontLE == true && flag_BackHE == true && flag_BackLE == true && flag_Theta == true && flag_DE == true){
          if (autoCoinWindowClose == true){
            enableCoinWindow = false;
            flag_DE = false;
            flag_FrontHE = false;
            flag_FrontLE = false;
            flag_BackHE = false;
            flag_BackLE = false;
            flag_Theta = false;
          }
        }
      }
    }
  }
}
//======================================================================

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
    //std::cout << event->BankListToString() << std::endl;

    // Get the ADC Bank
    TMBank* bADC = event->FindBank("V730");
    uint32_t* dADC = (uint32_t*)event->GetBankData(bADC);
    TMBank* bTDC = event->FindBank("TDC1");
    uint32_t* dTDC = (uint32_t*)event->GetBankData(bTDC);

    //    printf("V1730 Bank: Name = %s, Type = %d, Size = %d\n",&bADC->name[0],
    //	   bADC->type,bADC->data_size); 

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

    //std::cout << "nADC = " << nADC << " nTDC = " << nTDC << std::endl;
    
  
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
  time_t run_start_time = 0; //runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
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
    .def("getNGates", &EngeSort::getNGates)          // bool vector
    .def("getSpectrumNames", &EngeSort::getSpectrumNames)
    .def("getIsRunning", &EngeSort::getIsRunning)        // bool value
    .def("getScalerNames", &EngeSort::getScalerNames)     // string vector
    .def("getScalers", &EngeSort::getScalers)             // IntVector of scaler values
    .def("ClearData", &EngeSort::ClearData)        // void
    .def("getGateNames", &EngeSort::getGateNames)             // string vector of gate names
    .def("putGate", &EngeSort::putGate)            // void
    .def("data", range(&EngeSort::begin, &EngeSort::end))
    //    .def("Histogram1D", &Histogram1D::Histogram1D)
    ;

}
