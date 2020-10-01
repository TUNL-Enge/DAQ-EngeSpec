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

int Channels1D = 8192;

int Channels2D = 1024;

//------ Settings ------
double long_gate = 700; // in ns

// List v1730 channels used (0-15)
// Eventually enable histogams based directly on which channels are enabled from fev1730-DPP.cxx
IntVector channelsUsed = {0,1}; //,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
IntVector channelsCoin = {0,1}; // channels used for coincidence
bool coincidences = true;
//----------------------

// 1D Spectra
Histogram *hDet1, *hDet2, *hDet3, *hDet4, *hDet5, *hDet6, *hDet7, *hDet8, *hDet9, *hDet10, *hDet11, *hDet12, *hDet13, *hDet14, *hDet15, *hDet16, *hDetCoin;
// 2D Spectra
Histogram *hDetEvsE, *hDetEvsT0, *hDetEvsT1;
// 1D Gated Spectra
Histogram *hDet_gEvsT0_G1, *hDet_gEvsT0_G2, *hDet_gEvsT0_G3, *hDet_gEvsT1_G1, *hDet_gEvsT1_G2, *hDet_gEvsT1_G3;
// 2D Gated Spectra
Histogram *hDet_gEvsE_G1, *hDet_gEvsE_G2, *hDet_gEvsE_G3;

// Counters
int totalCounter=0;
int gateCounter=0;

void EngeSort::Initialize(){

  //--------------------
  // 1D Histograms
  
  if (std::find(channelsUsed.begin(), channelsUsed.end(), 0) != channelsUsed.end())
    hDet1 = new Histogram("Channel 0", Channels1D, 1);
  if (std::find(channelsUsed.begin(), channelsUsed.end(), 1) != channelsUsed.end())
    hDet2 = new Histogram("Channel 1", Channels1D, 1);
  if (std::find(channelsUsed.begin(), channelsUsed.end(), 2) != channelsUsed.end())
    hDet3 = new Histogram("Channel 2", Channels1D, 1);
  if (std::find(channelsUsed.begin(), channelsUsed.end(), 3) != channelsUsed.end())
    hDet4 = new Histogram("Channel 3", Channels1D, 1);
  if (std::find(channelsUsed.begin(), channelsUsed.end(), 4) != channelsUsed.end())
    hDet5 = new Histogram("Channel 4", Channels1D, 1);
  if (std::find(channelsUsed.begin(), channelsUsed.end(), 5) != channelsUsed.end())
    hDet6 = new Histogram("Channel 5", Channels1D, 1);
  if (std::find(channelsUsed.begin(), channelsUsed.end(), 6) != channelsUsed.end())
    hDet7 = new Histogram("Channel 6", Channels1D, 1);
  if (std::find(channelsUsed.begin(), channelsUsed.end(), 7) != channelsUsed.end())
    hDet8 = new Histogram("Channel 7", Channels1D, 1);
  if (std::find(channelsUsed.begin(), channelsUsed.end(), 8) != channelsUsed.end())
    hDet9 = new Histogram("Channel 8", Channels1D, 1);
  if (std::find(channelsUsed.begin(), channelsUsed.end(), 9) != channelsUsed.end())
    hDet10 = new Histogram("Channel 9", Channels1D, 1);
  if (std::find(channelsUsed.begin(), channelsUsed.end(), 10) != channelsUsed.end())
    hDet11 = new Histogram("Channel 10", Channels1D, 1);
  if (std::find(channelsUsed.begin(), channelsUsed.end(), 11) != channelsUsed.end())
    hDet12 = new Histogram("Channel 11", Channels1D, 1);
  if (std::find(channelsUsed.begin(), channelsUsed.end(), 12) != channelsUsed.end())
    hDet13 = new Histogram("Channel 12", Channels1D, 1);
  if (std::find(channelsUsed.begin(), channelsUsed.end(), 13) != channelsUsed.end())
    hDet14 = new Histogram("Channel 13", Channels1D, 1);
  if (std::find(channelsUsed.begin(), channelsUsed.end(), 14) != channelsUsed.end())
    hDet15 = new Histogram("Channel 14", Channels1D, 1);
  if (std::find(channelsUsed.begin(), channelsUsed.end(), 15) != channelsUsed.end())
    hDet16 = new Histogram("Channel 15", Channels1D, 1);
  
  if (coincidences){
    hDetCoin = new Histogram("Coincidences (Ch " + std::to_string(channelsCoin[0]) + ", Ch " + std::to_string(channelsCoin[1]) + ")", Channels1D, 1);
    hDetEvsE = new Histogram("E (Ch " + std::to_string(channelsCoin[0]) + ") vs E (Ch " + std::to_string(channelsCoin[1]) + ")", Channels2D, 2);
    hDetEvsT0 = new Histogram("E (Ch " + std::to_string(channelsCoin[0]) + ") vs Coin. Time", Channels2D, 2);
    hDetEvsT1 = new Histogram("E (Ch " + std::to_string(channelsCoin[1]) + ") vs Coin. Time", Channels2D, 2);
    
    // Gated Histograms
    hDet_gEvsE_G1 = new Histogram("E (Ch " + std::to_string(channelsCoin[0]) + ") vs E (Ch " + std::to_string(channelsCoin[1]) + ") Gated on Coin. #1", Channels2D, 2);
    hDet_gEvsE_G2 = new Histogram("E (Ch " + std::to_string(channelsCoin[0]) + ") vs E (Ch " + std::to_string(channelsCoin[1]) + ") Gated on Coin. #2", Channels2D, 2);
    hDet_gEvsE_G3 = new Histogram("E (Ch " + std::to_string(channelsCoin[0]) + ") vs E (Ch " + std::to_string(channelsCoin[1]) + ") Gated on Coin. #3", Channels2D, 2);
    
    hDet_gEvsT0_G1 = new Histogram("E (Ch " + std::to_string(channelsCoin[0]) + ") vs Coin. Time Gated on Coin. #1", Channels2D, 2);
    hDet_gEvsT0_G2 = new Histogram("E (Ch " + std::to_string(channelsCoin[0]) + ") vs Coin. Time Gated on Coin. #2", Channels2D, 2);
    hDet_gEvsT0_G3 = new Histogram("E (Ch " + std::to_string(channelsCoin[0]) + ") vs Coin. Time Gated on Coin. #3", Channels2D, 2);
    
    hDet_gEvsT1_G1 = new Histogram("E (Ch " + std::to_string(channelsCoin[1]) + ") vs Coin. Time Gated on Coin. #1", Channels2D, 2);
    hDet_gEvsT1_G2 = new Histogram("E (Ch " + std::to_string(channelsCoin[1]) + ") vs Coin. Time Gated on Coin. #2", Channels2D, 2);
    hDet_gEvsT1_G3 = new Histogram("E (Ch " + std::to_string(channelsCoin[1]) + ") vs Coin. Time Gated on Coin. #3", Channels2D, 2);
    
    // Gates    
    hDetCoin -> addGate("Coincidence #1");
    hDetCoin -> addGate("Coincidence #2");
    hDetCoin -> addGate("Coincidence #3");
  }
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

  if (coincidences){
    uint32_t dat, ch, timetag, prev_timetag;
    int cDet, prev_energy, energy0, energy1, timetag0, timetag1;
    uint32_t prev_ch = (dADC[0] & 0xFFFF0000) >> 16;
    for(int i = 0; i<nADC; i++){
      // Energies are even midas counts
      if (i % 2 == 0){
        // Define the channels
        dat = dADC[i] & 0xFFFF; // dADC[i] includes channel # and qlong
        ch = (dADC[i] & 0xFFFF0000) >> 16;
      
        /*
        if (ch == channelsCoin[0]){
          f_qlong0 << dat << std::endl;
        }
        else if (ch == channelsCoin[1]){
          f_qlong1 << dat << std::endl;    
        }
        */
        
        cDet = (int)std::floor(dat/8.0);
        
        sortChannels(ch, cDet, dat);
      
      }
      // Timetags are odd midas counts
      else{
        timetag = dADC[i];
        
        /*
        if (ch == channelsCoin[0]){
          f_timetag0 << timetag << std::endl;
        }
        else if (ch == channelsCoin[1]){
          f_timetag1 << timetag << std::endl;
        }
        */
        
        // Coincidence must be between two consecutive, different channels
        if (prev_ch != ch){
          if (ch == channelsCoin[0]){
            timetag0 = (int) timetag;
            timetag1 = (int) prev_timetag;
            energy0 = cDet;
            energy1 = prev_energy;
          }
          else if (ch == channelsCoin[1]){
            timetag0 = (int) prev_timetag;
            timetag1 = (int) timetag;
            energy0 = prev_energy;
            energy1 = cDet;
          }
          int diff = 2 * (timetag0 - timetag1); // in timetag units [1 unit = 2 ns]
          if (std::abs(diff) < long_gate){
            int diff_scaled = ((int) std::floor(diff * 4096 / long_gate)) + 4096; // offset by 4096, scaled by long gate in timetag units
            hDetCoin -> inc(diff_scaled);
            //std::cout << "Coincidence!" << std::endl;

            if (energy0 > 20 && energy0 < Channels1D && energy1 > 20 && energy1 < Channels1D){
              int energy0_scaled = (int) std::floor(energy0/8.0);
              int energy1_scaled = (int) std::floor(energy1/8.0);
              int time_scaled = (int) std::floor(diff_scaled / 8.0);
              hDetEvsE -> inc(energy1_scaled, energy0_scaled); // Energy of Detector 0 vs Detector 1 (y vs x)
              hDetEvsT0 -> inc(time_scaled, energy0_scaled); // Energy of Detector 0 vs Coincidence Time
              hDetEvsT1 -> inc(time_scaled, energy1_scaled); // Energy of Detector 1 vs Coincidence Time
              
              // Gates
              Gate &G1 = hDetCoin -> getGate(0);
              if(G1.inGate(diff_scaled)){
                std::cout << "In Gate 1" << std::endl;
                gateCounter++;
                hDet_gEvsE_G1 -> inc(energy1_scaled, energy0_scaled);
                hDet_gEvsT0_G1 -> inc(time_scaled, energy0_scaled);
                hDet_gEvsT1_G1 -> inc(time_scaled, energy1_scaled);
                std::cout << "Passed Gate 1 Code" << std::endl;
              }
              Gate &G2 = hDetCoin -> getGate(1);
              if(G2.inGate(diff_scaled)){
                std::cout << "In Gate 2" << std::endl;
                gateCounter++;
                hDet_gEvsE_G2 -> inc(energy1_scaled, energy0_scaled);
                hDet_gEvsT0_G2 -> inc(time_scaled, energy0_scaled);
                hDet_gEvsT1_G2 -> inc(time_scaled, energy1_scaled);
                std::cout << "Passed Gate 2 Code" << std::endl;
              }
              Gate &G3 = hDetCoin -> getGate(2);
              if(G3.inGate(diff_scaled)){
                std::cout << "In Gate 3" << std::endl;
                gateCounter++;
                // Segmentation fault occurs when incrementing the gated histograms
                hDet_gEvsE_G3 -> inc(energy1_scaled, energy0_scaled);
                std::cout << "Test 1" << std::endl;
                hDet_gEvsT0_G3 -> inc(time_scaled, energy0_scaled);
                std::cout << "Test 2" << std::endl;
                hDet_gEvsT1_G3 -> inc(time_scaled, energy1_scaled);
                std::cout << "Passed Gate 3 Code" << std::endl;
              }
            }
          }
        }
        // Current ch, timetag, and energy are the prev ones next time
        prev_ch = ch;
        prev_timetag = timetag;
        prev_energy = cDet;
      }
    }
  }
  else{ // if not doing coincidences
    
    for(int i = 0; i<nADC; i++){
      // Define the channels
      uint32_t dat = dADC[i] & 0xFFFF; // dADC[i] includes channel # and qlong
      uint32_t ch = (dADC[i] & 0xFFFF0000) >> 16;
      
      int cDet = (int)std::floor(dat/8.0);
        
      sortChannels(ch, cDet, dat);
    }
  }
}
//======================================================================
void EngeSort::sortChannels(int ch, int cDet, uint32_t dat){
    
  // Increment 1D histograms
  if (cDet > 20 && cDet < Channels1D){
    if (ch == 0){
      hDet1 -> inc(cDet);
      //f_qlong0 << dat << std::endl;
    }
    else if (ch == 1){
      hDet2 -> inc(cDet);
      //f_qlong1 << dat << std::endl;
    }
    else if (ch == 2)
      hDet3 -> inc(cDet);
    else if (ch == 3)
      hDet4 -> inc(cDet);
    else if (ch == 4)
      hDet5 -> inc(cDet);
    else if (ch == 5)
      hDet6 -> inc(cDet);
    else if (ch == 6)
      hDet7 -> inc(cDet);
    else if (ch == 7)
      hDet8 -> inc(cDet);
    else if (ch == 8)
      hDet9 -> inc(cDet);
    else if (ch == 9)
      hDet10 -> inc(cDet);
    else if (ch == 10)
      hDet11 -> inc(cDet);
    else if (ch == 11)
      hDet12 -> inc(cDet);
    else if (ch == 12)
      hDet13 -> inc(cDet);
    else if (ch == 13)
      hDet14 -> inc(cDet);
    else if (ch == 14)
      hDet15 -> inc(cDet);
    else if (ch == 15)
      hDet16 -> inc(cDet);
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
  time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
  printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));

  fRunEventCounter = 0;
  
  //f_qlong0.open("/home/daq/midas/online/src/v1730/Coincidences/2NaIDetectors/qlong0_Eu152_10min.txt");
  //f_qlong1.open("/home/daq/midas/online/src/v1730/Coincidences/2NaIDetectors/qlong1_Eu152_10min.txt");
  //f_timetag0.open("/home/daq/midas/online/src/v1730/Coincidences/2NaIDetectors/timetag0_Eu152_10min.txt");
  //f_timetag1.open("/home/daq/midas/online/src/v1730/Coincidences/2NaIDetectors/timetag1_Eu152_10min.txt");
  
}
void MidasAnalyzerRun::EndRun(TARunInfo* runinfo){
  printf("End run %d\n",runinfo->fRunNo);
  printf("Counted %d events\n",fRunEventCounter);
  std::cout << "Total counts: " << totalCounter << "   Gated counts: " << gateCounter << std::endl;
  
  //f_qlong0.close();
  //f_qlong1.close();
  //f_timetag0.close();
  //f_timetag1.close();
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
