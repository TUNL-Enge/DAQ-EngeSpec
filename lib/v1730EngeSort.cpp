#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
//#include <climits> // For INT_MAX
#include <cmath>

//#include "EngeSort.h"
#include "v1730EngeSort.h"
#include "TV792Data.hxx"

Messages messages;

std::string EngeSort::sayhello( ) {
  return messages.sayhello("EngeSort v1730");
}
std::string EngeSort::saygoodbye( ) {
  return messages.saygoodbye();
}
//std::string EngeSort::saysomething(std::string str){
//  return messages.saysomething(str);
//}

//---------------------------- Settings --------------------------------
// Number of channels in 1D and 2D histograms
// Ensure self.Nbins is syncronized with these in EngeSpec/modules/SpectrumHandlers.py in the SpectrumObject and SpectrumObject2D classes
int Channels1D = 8192; // TODO - Test this scale (was 4096, Nbins = 2^12)
int Channels2D = 1024; // TODO - Test this scale (was 512,  Nbins = 2^9)

// Threshold for EngeSpec Histograms
int Histogram_Threshold = 0;

// Define the channels (0-15)
int iFrontHE = 0; // Pos1 (Front) High-Energy
int iFrontLE = 1; // Pos1 (Front) Low-Energy
int iBackHE = 2;  // Pos2 (Back) High-Energy
int iBackLE = 3;  // Pos2 (Back) Low-Energy
int iE = 4;       // Scintillator (E)
int iDE = 5;      // Proportionality Counter (Delta E)
int iSiE = 6;     // Silicon Detector (E)
int iSiDE = 7;    // Silicon Detector (Delta E)

// Channels to trigger on for the Focal-Plane detector (FP) and Silicon detectors (Si)
int FP_trigger_ch = iFrontHE; // iE, iFrontHE, or iFrontLE
int Si_trigger_ch = iSiE; // iSiE or iSiDE

// Coincidence Windows for FP and Si detectors (in ns)
// If running the digitizer in Online (Coincidence) Mode, this should be twice the shaped trigger width used for the triggering channel
int window_FP_ns = 30000;
int window_Si_ns = 30000;

// EXTRAS Recording Enabled (True) or Disabled (False)
// bool extras = true; // Assumed to be true
// 32-bit word: bits[31:16] = Extended Time Stamp, bits[15:10] = Flags, bits[9:0] = Fine Time Stamp
//----------------------------------------------------------------------

//------------------ Global sort routine variables ---------------------

// Slope for SiTotalE derivation - read from parameters.dat 
//const double pSiSlope = 0.0;
double pSiSlope = 0.0;

// Converting window duration to timetag units (2 ns/unit - v1730)
uint64_t window_FP = (uint64_t) std::floor(window_FP_ns/2.0);
uint64_t window_Si = (uint64_t) std::floor(window_Si_ns/2.0);

// Half coincidence window width
uint64_t half_window_FP = (uint64_t) std::floor(window_FP/2.0);
uint64_t half_window_Si = (uint64_t) std::floor(window_Si/2.0);

// Maximum timetag value, at which point timetags reset - EXTRAS word extends rollover time
// - Default timetag for v1730 (EXTRAS disabled) is a 31-bit number. So max is 2^31 - 1 = 2147483647 or 0x7FFFFFFF or INT_MAX
//   Each timetag unit is 2 ns (v1730), so rollover time is 2 ns/unit * (2^31 - 1) units ~ 4.295 s
// - Timetag for v1730 with EXTRAS enabled and 0x010 (2) written to bits[10:8] of DPP Algorithm Control 2 is a 31+16=47 bit number
//   with an additional 10 bits reserved for the fine time stamp. So rollover is (2^47 - 1) units ~ 1.407x10^14, so 64-bit int needed
//   but realistically rollover is not necessary to consider. Time step is 2 ns / 1024 = 0.001953125 ns with fine time stamp.
// uint32_t timetag_rollover = (uint32_t) INT_MAX; // EXTRAS not enabled
// uint64_t timetag_rollover = 2^47 - 1; // EXTRAS enabled

// Compression for 2D histograms
double Compression = (double)Channels2D / (double)Channels1D; // 1/8

// vectors for event vs time histograms in FillEndOfRun()
std::vector<int> vPos1;
std::vector<int> vDE;
std::vector<int> vSiE;
std::vector<int> vSiDE;
std::vector<int> vSiTotalE;

// Counting variables
int nFrontHE = 0, nFrontLE = 0, nBackHE = 0, nBackLE = 0, nE = 0, nDE = 0, nSiE = 0, nSiDE = 0;
int inhibitedTriggersFP = 0, inhibitedTriggersSi = 0;
int Ecount = 0, DEcount = 0, SiEcount = 0, SiDEcount = 0, FrontHEcount = 0, FrontLEcount = 0, BackHEcount = 0, BackLEcount = 0;
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

Histogram *hPos1vsEvt;

Histogram *hSiDEvsSiE;

// Gated Spectra
Histogram *hPos1_gDEvsPos1_G1;
Histogram *hPos1_gDEvsPos1_G2;
Histogram *hPos1_gDEvsE_G1;
Histogram *hPos1_gDEvsE_G2;

Histogram *hPos1_gPos1vsEvt;
Histogram *hPos1_gDEvPos1_gPos1vsEvt;
Histogram *hDEvsPos1_G_Pos1vsEvt;

Histogram *hE_gE_G1;

//Histogram *hSiE_G_SiDEvsSiE;
Histogram *hSiTotalE_G_SiDEvsSiE;
Histogram *hSiDEvsSiTotalE_G_SiDEvsSiE;

Histogram *hSiTotalE_G_Pos1vsEvt;
Histogram *hSiTotalE_G_SiDEvsSiE_G_Pos1vsEvt;
Histogram *hSiTotalEvsEvt_G_SiDEvsSiE;

// Counters
int totalCounter=0;
int gateCounter=0;
int EventNo=0;

/* // TODO - How do scalers change between v785 and v1730?
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
*/

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

  hPos1vsEvt = new Histogram("Pos 1 vs Event", Channels2D, 2);

  hSiDEvsSiE = new Histogram("SiDE vs SiE", Channels2D, 2);

  //--------------------
  // Gated Histograms
  hPos1_gDEvsPos1_G1 = new Histogram("Pos 1; G-DEvsPos1-G1", Channels1D, 1);
  hPos1_gDEvsPos1_G2 = new Histogram("Pos 1; G-DEvsPos1-G2", Channels1D, 1);

  hPos1_gDEvsE_G1 = new Histogram("Pos 1; G-DEvsE-G1", Channels1D, 1);
  hPos1_gDEvsE_G2 = new Histogram("Pos 1; G-DEvsE-G2", Channels1D, 1);

  hPos1_gPos1vsEvt = new Histogram("Pos 1; G-Pos1vsEvent", Channels1D, 1); // Pos1vsEvent gates incremented in EngeSort::FillEndofRun(), not EngeSort::sort()
  hPos1_gDEvPos1_gPos1vsEvt = new Histogram("Pos 1; G-DEvPos1-G2 + G-Pos1vsEvent", Channels1D, 1);
  hDEvsPos1_G_Pos1vsEvt = new Histogram("DE vs Pos1; G-Pos1vsEvent", Channels2D, 2);

  hE_gE_G1 = new Histogram("E; GE-G1", Channels1D, 1);

  //hSiE_G_SiDEvsSiE = new Histogram("SiE; G-SiDEvsSiE", Channels1D, 1);
  hSiTotalE_G_SiDEvsSiE = new Histogram("SiTotE; G-SiDEvsSiE", Channels1D, 1);
  hSiDEvsSiTotalE_G_SiDEvsSiE = new Histogram("SiDE vs SiTotE; G-SiDEvsSiE", Channels2D, 2);

  hSiTotalE_G_Pos1vsEvt = new Histogram("SiTotE; G-Pos1vsEvent", Channels1D, 1);
  hSiTotalE_G_SiDEvsSiE_G_Pos1vsEvt = new Histogram("SiTotE; G-SiDEvsSiE + G-Pos1vsEvent", Channels1D, 1);

  hSiTotalEvsEvt_G_SiDEvsSiE = new Histogram("SiTotE vs Event; G-SiDEvsSiE", Channels2D, 2);

  //--------------------
  // Gates
  hE -> addGate("Energy Gate");
  
  hDEvsPos1 -> addGate("DE vs Pos1 Gate 1");
  hDEvsPos1 -> addGate("DE vs Pos1 Gate 2");

  hDEvsE -> addGate("DE vs E Gate 1");
  hDEvsE -> addGate("DE vs E Gate 2");
  
  hPos1vsEvt -> addGate("Event Gate");

  hSiDEvsSiE -> addGate("SiDE vs SiE Gate");

  /*
  // Loop through and print all histograms
  for(auto h: Histograms){
    if(h->getnDims() == 1) {
      std::cout << "Found a 1D histogram!" << std::endl;
      h->Print(0,10);
    }
    else if(h->getnDims() == 2) {
      std::cout << "Found a 2D histogram!" << std::endl;
      h->Print(0,10,0,10);
    }
    std::cout << std::endl;
  }
  */

  /* // TODO - How do scalers change between v785 and v1730?
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
  */

}

//======================================================================
// This is the equivalent to the "sort" function in jam
void EngeSort::sort(uint32_t *dADC, int nADC, uint32_t *dTDC, int nTDC){

  totalCounter++;
  EventNo++;

  //std::cout << "Sort # " << totalCounter << std::endl;
  //std::cout << "nADC:  " << nADC << std::endl;
  //std::cout << "nTDC:  " << nTDC << std::endl;

  // vectors for event loop
  std::vector<int16_t> energies;
  std::vector<uint64_t> course_timestamps; // With EXTRAS enabled and 0x010 (2) format, 47-bit max | T = EXTRAS[31:16] + 31-bit Trigger Timetag | Timetag units
  std::vector<double> fine_timestamps; // With EXTRAS enabled and 0x010 (2) format, 10-bit max floating point number | T = EXTRAS[9:0] / 1024 | Fraction of 1 timetag unit
  std::vector<int> digitizer_chs;
  std::vector<int> trigger_indices;

  // Extract qlong, channel, course and fine timestamps, and extract index if ch is a trigger
  //std::cout << "nADC = " << nADC << std::endl;
  for(int i=0; i<nADC; i+=3){

    int16_t qlong = (int16_t) (dADC[i] & 0xFFFF);
    energies.push_back((int16_t) std::floor(qlong/4.0)); // TODO - Maxes out at Channels1D (8,192).
    int digitizer_ch = (int) (dADC[i] & 0xFFFF0000) >> 16;
    digitizer_chs.push_back(digitizer_ch);
    uint32_t timetag = dADC[i+1]; // 31-bit trigger time tag (TTT)
    uint64_t extended_timetag = (uint64_t) ((dADC[i+2] & 0xFFFF0000) >> 16); // 16-bit timetag rollover extension
    uint64_t course_timestamp = (uint64_t) ((extended_timetag & 0xFFFF) << 31) + (uint64_t) timetag;
    course_timestamps.push_back(course_timestamp);
    uint32_t fine_timetag = (uint32_t) (dADC[i+2] & 0x3FF); // 10-bit fine time tag (from CFD ZC interpolation)
    double fine_timestamp = (double) fine_timetag / 1024.0; // Fraction with 2^10 = 1024 steps
    fine_timestamps.push_back(fine_timestamp);

    if (digitizer_ch == FP_trigger_ch || digitizer_ch == Si_trigger_ch){
      trigger_indices.push_back((int) i/3.0);
      //std::cout << "Trigger Index: " << (int) i/3.0 << std::endl;
    }

    if (digitizer_ch == iFrontHE){nFrontHE++;} // std::cout << "FrontHE course timestamp: " << course_timestamp << std::endl;} // std::cout << "FrontHE energy: " << qlong << std::endl;}
    if (digitizer_ch == iFrontLE){nFrontLE++;} // std::cout << "FrontLE course timestamp: " << course_timestamp << std::endl;} // std::cout << "FrontLE energy: " << qlong << std::endl;}
    if (digitizer_ch == iBackHE){nBackHE++;}
    if (digitizer_ch == iBackLE){nBackLE++;}
    if (digitizer_ch == iE){nE++;} // std::cout << "E course timestamp: " << course_timestamp << std::endl;} // std::cout << "E energy: " << qlong << std::endl;}
    if (digitizer_ch == iDE){nDE++;} // std::cout << "DE course timestamp: " << course_timestamp << std::endl;} //std::cout << "DE energy: " << qlong << std::endl;}
    if (digitizer_ch == iSiDE){nSiDE++;} // << std::cout << "SiDE energy: " << (int16_t) std::floor(qlong/4.0) << std::endl;}
    if (digitizer_ch == iSiE){nSiE++;} //std::cout << "SiE energy: " << (int16_t) std::floor(qlong/4.0) << std::endl;}
  }

  // Previous trigger window end timetag (initial value ensures 1st trigger is not ignored)
  uint64_t timetag_window_stop_previous_FP = 0;
  uint64_t timetag_window_stop_previous_Si = 0;

  // Collect coincidence events for each FP or Si trigger
  for (int trigger_index : trigger_indices){

    //std::cout << "Trigger Index: " << trigger_index << std::endl;

    int trigger_ch = digitizer_chs[trigger_index];
    uint64_t half_window;
    // Get window size based on the kind of trigger
    if (trigger_ch == FP_trigger_ch){
      half_window = half_window_FP;
    }
    else if (trigger_ch == Si_trigger_ch){
      half_window = half_window_Si;
    }

    // Trigger timestamp and its coincidence window start/stop timestamps.
    uint64_t course_timestamp_trigger = course_timestamps[trigger_index];
    // double fine_timestamp_trigger = fine_timestamps[trigger_index]; // Windows not accounting for fine timestamp
    uint64_t timetag_window_start;
    uint64_t timetag_window_stop;
    
    // Find window start and stop timetags. Not considering fine timestamp.
    if (course_timestamp_trigger >= half_window){
      timetag_window_start = course_timestamp_trigger - half_window;
    }
    // ... if window extends below 0
    else if (course_timestamp_trigger < half_window){
      timetag_window_start = 0;
    }

    timetag_window_stop = course_timestamp_trigger + half_window;

    // Inhibit trigger if current window overlaps with the previous window.
    if (trigger_ch == FP_trigger_ch){ // FP window
      if (timetag_window_start < timetag_window_stop_previous_FP){
        inhibitedTriggersFP++;
        continue;
      }
    }
    else if (trigger_ch == Si_trigger_ch){ // Si window
      if (timetag_window_start < timetag_window_stop_previous_Si){
        inhibitedTriggersSi++;
        continue;
      }
    }
    // Note: If timetag_window_start == timetag_window_stop_previous and an event happens to occur at that timetag, it is collected by the later event
    // Note: If a trigger is inhibited, the next trigger window is compared with the last uninhibited trigger window

    // Update trigger info for next trigger to check (must come after overlapping triggers are inhibited)
    if (trigger_ch == FP_trigger_ch){
      timetag_window_stop_previous_FP = timetag_window_stop;
    }
    else if (trigger_ch == Si_trigger_ch){
      timetag_window_stop_previous_Si = timetag_window_stop;
    }

    // ***********************************************************************************************
    // Event loop variables for each coincidence window

    bool in_window = true;
    bool increment_backwards = true;
    int event_increment = 0;

    // Data to save for coincidences. Stays 0 if no corresponding event in window
    uint64_t FrontHE_course = 0, FrontLE_course = 0, BackHE_course = 0, BackLE_course = 0;
    double FrontHE_fine = 0.0, FrontLE_fine = 0.0, BackHE_fine = 0.0, BackLE_fine = 0.0;
    int E = 0, DE = 0, SiE = 0, SiDE = 0;

    // Flags to prevent more than one signal to be collected from a single component during the coincidence window.
    // True when signal is detected during window.
    bool fFrontHE = false, fFrontLE = false, fBackHE = false, fBackLE = false,
         fE = false, fDE = false, fSiE = false, fSiDE = false;
    
    // ***********************************************************************************************

    // Collect each event in the trigger coincidence window
    while(in_window){

      int event_index;
      uint64_t course_timestamp;
      double fine_timestamp;
      int event_ch;
      int energy;

      // Look for events before trigger first (between trigger and start of window)
      if (increment_backwards){
        event_index = trigger_index - event_increment;
      }
      // Then look for events after trigger (between trigger and end of window)
      else{
        event_index = trigger_index + event_increment;
      }
      event_increment++;

      // Ensure we have not indexed below the first event or above the last event in the board aggregate
      if (event_index >= 0 && event_index <= ((int)(nADC/3.0) - 1)){
        course_timestamp = course_timestamps[event_index];
        fine_timestamp = fine_timestamps[event_index];
        event_ch = digitizer_chs[event_index];
        energy = (int) energies[event_index];

        //if (event_ch == iFrontHE){std::cout << "At FrontHE event in window" << std::endl;}
        //else if (event_ch == iFrontLE){std::cout << "At FrontLE event in window" << std::endl;}

        //if (event_ch == iFrontHE && !fFrontHE){std::cout << "At FrontHE event in window (Flag false)" << std::endl;}
        //else if (event_ch == iFrontLE && !fFrontLE){std::cout << "At FrontLE event in window (Flag false)" << std::endl;}

        // Skip if Si event in FP window
        if ((event_ch == iSiE || event_ch == iSiDE) && trigger_ch == FP_trigger_ch){
          continue;
        }
        // Skip if FP event in Si window
        else if ((event_ch == iE || event_ch == iDE || event_ch == iFrontHE || event_ch == iFrontLE || 
                  event_ch == iBackHE || event_ch == iBackLE) && trigger_ch == Si_trigger_ch){
          continue;
        }

        // Check if the next event is in the window
        if (course_timestamp >= timetag_window_start && course_timestamp < timetag_window_stop){

          // Bring energy to 0 if signal is below our Histogram threshold or above Channels1D (Pos1 threshold handled separately below)
          if ((event_ch == iE || event_ch == iDE || event_ch == iSiE || event_ch == iSiDE) && 
              (energy < Histogram_Threshold || energy >= Channels1D)){
            energy = 0;
          }
                    
          // ***********************************************************************************************
          // Event is within window. Save each unique event to its correspnding variable(s)

          //std::cout << "Energy: " << energy << std::endl;

          if (event_ch == iE && !fE){fE = true; E = energy; Ecount++;}
          else if (event_ch == iDE && !fDE){fDE = true; DE = energy; DEcount++;}
          else if (event_ch == iSiE && !fSiE){fSiE = true; SiE = energy; SiEcount++;}
          else if (event_ch == iSiDE && !fSiDE){fSiDE = true; SiDE = energy; SiDEcount++;}
          else if (event_ch == iFrontHE && !fFrontHE){fFrontHE = true; FrontHE_course = course_timestamp; FrontHE_fine = fine_timestamp; FrontHEcount++;} // std::cout << "FrontHE course timestamp (coin): " << course_timestamp << std::endl;}
          else if (event_ch == iFrontLE && !fFrontLE){fFrontLE = true; FrontLE_course = course_timestamp; FrontLE_fine = fine_timestamp; FrontLEcount++;} // std::cout << "FrontLE course timestamp (coin): " << course_timestamp << std::endl;}
          else if (event_ch == iBackHE && !fBackHE){fBackHE = true; BackHE_course = course_timestamp; BackHE_fine = fine_timestamp; BackHEcount++;}
          else if (event_ch == iBackLE && !fBackLE){fBackLE = true; BackLE_course = course_timestamp; BackLE_fine = fine_timestamp; BackLEcount++;}          
          // ***********************************************************************************************
        }
        else{
          // Event not within window
          if (increment_backwards){
            // Reset and start searching forwards after trigger
            increment_backwards = false;
            event_increment = 1;
          }
          else{
            // Collected all events in the window
            in_window = false;
          }
        }
      }
      else{
        // Reached either the first or last event in the board aggregate
        if (increment_backwards){
          // Reset and start searching forwards
          increment_backwards = false;
          event_increment = 1;
        }
        else{
          // Collected all events in the window
          in_window = false;
        }
      } 
    }
    // ***********************************************************************************************
    // Derive info from saved energy/timetag data during the coincidence window, or 0 if nonexistent

    int Ecomp = (int) std::floor(E * Compression);
    int DEcomp = (int) std::floor(DE * Compression);
    int SiEcomp = (int) std::floor(SiE * Compression);
    int SiDEcomp = (int) std::floor(SiDE * Compression);

    if(fDE){vDE.push_back(DE);}
    else{vDE.push_back(0);}

    if (fSiE){vSiE.push_back(SiE);}
    else{vSiE.push_back(0);}
    if (fSiDE){vSiDE.push_back(SiDE);}
    else{vSiDE.push_back(0);}

    int SiTotalE = 0;
    if (fSiE && fSiDE){
      SiTotalE = (int) std::floor((SiE + pSiSlope*SiDE) / (1.0 + pSiSlope));
      vSiTotalE.push_back(SiTotalE);
    }
    else{vSiTotalE.push_back(0);}
    int SiTotalEcomp = (int) std::floor(SiTotalE * Compression);

    int Pos1 = 0;
    int Pos2 = 0;
    int Pos1comp = 0;
    int Pos2comp = 0;
    int Theta = 0;

    if (fFrontHE && fFrontLE){
      // Pos = HE - LE, scaled so Max Ch = 1 microsecond and Min Ch = -1 microsecond, offset to center (4,096)
      int Pos1_course = (int) (FrontHE_course - FrontLE_course); // from -1 microsecond to +1 microsecond i.e. -500 to +500 timetag units
      double Pos1_fine = FrontHE_fine - FrontLE_fine; // from -1023/1024 to +1023/1024 timetag units
      double Pos1_interpolated = ((double) Pos1_course) + Pos1_fine;
      Pos1 = (int) std::floor((Channels1D / 1000.0) * Pos1_interpolated) + (Channels1D/2); // scaled to [0, Channels1D] with each bin unique
      //std::cout << "Pos1 course + fine: " << Pos1_interpolated << std::endl;
      if (Pos1 < Histogram_Threshold || Pos1 >= Channels1D){Pos1=0;}
      vPos1.push_back(Pos1);
    }
    else{vPos1.push_back(0);}
    Pos1comp = (int) std::floor(Pos1 * Compression);

    if (fBackHE && fBackLE){
      int Pos2_course = (int) (BackHE_course - BackLE_course);
      double Pos2_fine = BackHE_fine - BackLE_fine;
      double Pos2_interpolated = ((double) Pos2_course) + Pos2_fine;
      Pos2 = (int) std::floor((Channels1D / 1000.0) * Pos2_interpolated) + (Channels1D/2);
      if (Pos2 < Histogram_Threshold || Pos2 >= Channels1D){Pos2=0;}
    }
    Pos2comp = (int) std::floor(Pos2 * Compression);

    if (fFrontHE && fFrontLE && fBackHE && fBackLE){
      Theta = (int) std::round(10000.0*atan((Pos2-Pos1)/100.)/3.1415 - 4000.);
      Theta = std::max(0,Theta);
    }
    int Thetacomp = (int) std::floor(Theta * Compression);

    // ***********************************************************************************************
    // Increment Histograms for all events in the window

    // 1D Histograms
    hPos1 -> inc(Pos1);
    hPos2 -> inc(Pos2);
    hE -> inc(E);
    hDE -> inc(DE);
    hSiE -> inc(SiE);
    hSiDE -> inc(SiDE);
    hTheta -> inc(Theta);

    // 2D Histograms
    hDEvsPos1 -> inc(Pos1comp, DEcomp);
    hEvsPos1 -> inc(Pos1comp, Ecomp);
    hDEvsE -> inc(Ecomp, DEcomp);
    hPos2vsPos1 -> inc(Pos1comp, Pos2comp);
    hThetavsPos1 -> inc(Pos1comp, Thetacomp);
    hSiDEvsSiE -> inc(SiEcomp, SiDEcomp);

    // Gated 1D Histograms

    // E Gate
    Gate &G = hE -> getGate(0);
    //G.Print();
    if (G.inGate(E)){
      hE_gE_G1 -> inc(E);
    }

    // Pos1 G DE vs Pos1
    Gate &G1 = hDEvsPos1 -> getGate(0);
    //G1.Print();
    if (G1.inGate(Pos1comp,DEcomp)){
      gateCounter++;
      hPos1_gDEvsPos1_G1 -> inc(Pos1);
    }
    Gate &G2 = hDEvsPos1 -> getGate(1);
    //G2.Print();
    if (G2.inGate(Pos1comp,DEcomp)){
      gateCounter++;
      hPos1_gDEvsPos1_G2 -> inc(Pos1);
    }

    // Si Gates
    Gate &G3 = hSiDEvsSiE -> getGate(0);
    //G3.Print();
    if (G3.inGate(SiEcomp,SiDEcomp)){
      gateCounter++;
      //hSiE_G_SiDEvsSiE -> inc(SiE);
      hSiTotalE_G_SiDEvsSiE -> inc(SiTotalE);
      hSiDEvsSiTotalE_G_SiDEvsSiE -> inc(SiTotalEcomp,SiDEcomp);
    }

    // Pos1 G DE vs E Gates
    Gate &G4 = hDEvsE -> getGate(0);
    //G4.Print();
    if (G4.inGate(Ecomp,DEcomp)){
      gateCounter++;
      hPos1_gDEvsE_G1 -> inc(Pos1);
    }
    Gate &G5 = hDEvsE -> getGate(1);
    //G5.Print();
    if (G5.inGate(Ecomp,DEcomp)){
      gateCounter++;
      hPos1_gDEvsE_G2 -> inc(Pos1);
    }
    // ***********************************************************************************************
  }
}
//======================================================================

void EngeSort::FillEndOfRun(int nEvents){

  // TODO - Verify nEvents is correct. Should be nADC/3? (with EXTRAS enabled)
  //        But the v785 version of fTotalEventCounter increments every sort routine call

  int nEventsInGate_P1 = 0;
  int nEventsInGate_Si = 0;
  int nEvents_P1 = 0;
  int nEvents_Si = 0;
  double fracEventsInGate_P1 = 0.0;
  double fracEventsInGate_Si = 0.0;
  double fracBCI = 0.0;
  bool bPos1vsEventGate = false;

  double EvtCompression = (double)Channels2D/(double)nEvents;

  std::cout << "To convert x to event in Pos 1 vs Event spectrum: Event # = x * " <<
    1/EvtCompression << std::endl;

  std::cout << "Length of vPos1 = " << vPos1.size() << std::endl;
  //std::cout << "Length of vDE = " << vDE.size() << std::endl;
  //std::cout << "Length of vSiE = " << vSiE.size() << std::endl;
  //std::cout << "Length of vSiDE = " << vSiDE.size() << std::endl;
  //std::cout << "Length of vSiTotalE = " << vSiTotalE.size() << std::endl;

  std::cout << "Total FrontHE = " << nFrontHE << std::endl;
  std::cout << "Total FrontLE = " << nFrontLE << std::endl;
  std::cout << "Total BackHE  = " << nBackHE << std::endl;
  std::cout << "Total BackLE  = " << nBackLE << std::endl;
  std::cout << "Total E       = " << nE << std::endl;
  std::cout << "Total DE      = " << nDE << std::endl;

  std::cout << "Total SiE     = " << nSiE << std::endl;
  std::cout << "Total SiDE    = " << nSiDE << std::endl;

  std::cout << "\nInhibited FP Triggers = " << inhibitedTriggersFP << std::endl;
  std::cout << "Inhibited Si Triggers = " << inhibitedTriggersSi << std::endl;
  std::cout << "\nE in window count = " << Ecount << std::endl;
  std::cout << "DE in window count = " << DEcount << std::endl;
  std::cout << "FrontHE in window count = " << FrontHEcount << std::endl;
  std::cout << "FrontLE in window count = " << FrontLEcount << std::endl;
  std::cout << "BackHE in window count = " << BackHEcount << std::endl;
  std::cout << "BackLE in window count = " << BackLEcount << std::endl;

  std::cout << "SiE in window count = " << SiEcount << std::endl;
  std::cout << "SiDE in window count = " << SiDEcount << std::endl;
  std::cout << std::endl;

  //for(int i=0; i<nEvents; i++){
  for(int i=0; i<vPos1.size(); i++){
    int e = (int) std::floor(EvtCompression * i);
    int p = (int) std::floor(Compression * vPos1[i]);
    int de = (int) std::floor(Compression * vDE[i]);
    int sie = (int) std::floor(Compression * vSiE[i]);
    int side = (int) std::floor(Compression * vSiDE[i]);
    int sitotale = (int) std::floor(Compression * vSiTotalE[i]);

    //std::cout << "FillEndOfRun Event #" << i << std::endl;
    hPos1vsEvt -> inc(e, p);

    Gate &G6 = hPos1vsEvt->getGate(0);
    if(G6.inGate(e, p)){
      gateCounter++;
      hPos1_gPos1vsEvt->inc(vPos1[i]); // Pos1
      hDEvsPos1_G_Pos1vsEvt->inc(p,de); // DEcomp vs Pos1comp
      nEventsInGate_P1++;
      bPos1vsEventGate = true;
    }

    // Cutting the same events from the SiTotalE spectrum as are cut from the Pos1 spectrum, so the P1 peak / Si peak ratio is consistent.
    // Trick: Using Pos1vsEvt gate, but checking if (e,sitotale) is in the gate. The y-axis value doesn't matter since the gate spans the entire y-axis.
    // An alternative solution would be to make a separate histogram of SiTotalE vs Event and gate on this too, 
    // but this would require the Pos1vsEvent and SiTotalEvsEvent histograms to have identical gates to get the identical events.
    if(G6.inGate(e, sitotale)){
      hSiTotalE_G_Pos1vsEvt->inc(vSiTotalE[i]); // SiTotalE
      nEventsInGate_Si++;
    }

    Gate &G7 = hDEvsPos1->getGate(1); // Same as G2, which is the default DEvsPos1 for some reason
    if(G6.inGate(e, p) && G7.inGate(p, de)){ // Double gate - DEvsPos1 and Pos1vsEvent
      hPos1_gDEvPos1_gPos1vsEvt->inc(vPos1[i]); // Pos1
    }

    Gate &G8 = hSiDEvsSiE->getGate(0);
    // Double Gate - SiDEvsSiE and Pos1vsEvent (same trick as above to get Si events in the same gate as Pos1 vs Event)
    if(G6.inGate(e, sitotale) && G8.inGate(sie, side)){
      hSiTotalE_G_SiDEvsSiE_G_Pos1vsEvt->inc(vSiTotalE[i]); // SiTotalE
    }

    if(G8.inGate(sie,side)){
      hSiTotalEvsEvt_G_SiDEvsSiE->inc(e, sitotale);
    }

    if(p>0)
      nEvents_P1++;
    if(sitotale>0)
      nEvents_Si++;

  }
  // Clear vectors
  vPos1.clear();
  vDE.clear();
  vSiE.clear();
  vSiDE.clear();
  vSiTotalE.clear();

  Gate &G6 = hPos1vsEvt->getGate(0);
  if(bPos1vsEventGate){
    // Be careful not to interpret the min/max event gate channels as the total rectangular gate. 
    // It's possible (and often necessary) to create two rectangles with a single gate, to cut out more than one region.
    // This is why getting the fraction of events in the gate is a better strategy to calculate the new BCI, clock, and clocklive,
    // but the Pos1 and SiTotalE fractions are not identical (they are close). Take the average of these fractions as the new BCI, clock, and clocklive fraction to use.
    std::cout << "Pos1vsEvent Gate Min/Max Event Chs: " << std::round(G6.getMinx()) << " to " << std::round(G6.getMaxx()) << std::endl;
    fracEventsInGate_P1 = (double) nEventsInGate_P1 / (double) nEvents_P1;
    fracEventsInGate_Si = (double) nEventsInGate_Si / (double) nEvents_Si;
    std::cout << "Number of events in Pos1vsEvent Gate: " << nEventsInGate_P1 << std::endl;
    std::cout << "Number of events in SiTotalEvsEvent Gate: " << nEventsInGate_Si << std::endl;
    std::cout << "Total Number of Pos1 Events: " << nEvents_P1 << std::endl;
    std::cout << "Total Number of SiTotalE Events: " << nEvents_Si << std::endl;
    std::cout << "Fraction of Events in Pos1vsEvent Gate: " << fracEventsInGate_P1 << std::endl;
    std::cout << "Fraction of Events in SiTotalEvsEvent Gate: " << fracEventsInGate_Si << std::endl;
    fracBCI = (double) ((fracEventsInGate_P1 + fracEventsInGate_Si) / 2.0);
    std::cout << "New BCI, Clock, and Clocklive fraction = " << fracBCI << std::endl;
  }
}

/* // TODO - How do scalers change between v785 and v1730?
// Increment the scalers
// TODO: make this automatic. If the scaler is
// defined we should assume that the user wants to increment it
void EngeSort::incScalers(uint32_t *dSCAL){

  //std::cout << "Incrementing scalers" << std::endl;
  
  sGates -> inc(dSCAL);
  sGatesLive -> inc(dSCAL);
  sClock-> inc(dSCAL);
  sClockLive -> inc(dSCAL);
  sFrontLE -> inc(dSCAL);
  sFrontHE -> inc(dSCAL);
  sBackLE -> inc(dSCAL);
  sBackHE -> inc(dSCAL);
  sE -> inc(dSCAL);
  sDE -> inc(dSCAL);
  BCI -> inc(dSCAL);
}
*/

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

  while (p2 && ac < kMaxArgs-1){
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

  //IntVector sclr;
  IntVector sclr = {0};
  //for(auto sc: Scalers){
  //  sclr.push_back(sc -> getValue());
  //}

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
  for(auto h: Histograms){
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
      for (int i = 0; i < h -> getnChannels(); i++){
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
      //if(h -> getNGates() > 0){}
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

  /*
  for(auto Sclr: Scalers){
    Sclr -> Clear();
  }
  */
  
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
  printf("Finish!\n");
  eA->FillEndOfRun(fTotalEventCounter);
  //printf("Counted %d events\n",fTotalEventCounter);
  //std::cout << "number of spectra: " << eA->getSpectrumNames().size() << std::endl;
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

    // printf("V1730 Bank: Name = %s, Type = %d, Size = %d\n",&bADC->name[0],
    // bADC->type,bADC->data_size); 

    // uint64_t dat;
    // dat = dADC[0] & 0xFFFF;
    // printf("dADC[0] = 0x%x\n",dat);
    // printf("dADC[0] = %d\n",dat);

    int singleADCSize = 0;
    int singleTDCSize = 0;
    if(bADC->type == 4)singleADCSize = 2;
    if(bADC->type == 6)singleADCSize = 4;
    
    // Find the size of the data
    int nADC = 0;
    int nTDC = 0;
    //if(bADC)nADC=(bADC->data_size - 2)/singleADCSize; // TODO - Why data_size - 2 bytes (16 bits)? Ch and Board Agg headers are already removed
    //if(bTDC)nTDC=(bTDC->data_size - 2)/singleTDCSize;
    if(bADC)nADC=(bADC->data_size)/singleADCSize;
    if(bTDC)nTDC=(bTDC->data_size)/singleTDCSize;

    //std::cout << "nADC = " << nADC << " nTDC = " << nTDC << std::endl;
  
    fRunEventCounter++;
    //fModule->fTotalEventCounter++;
    //fModule->fTotalEventCounter += (int) nADC/2; // TODO - 2 memory locations (Timetag and qlong) per event?
    fModule->fTotalEventCounter += (int) nADC/3; // TODO - 3 memory locations (Timetag, qlong, and EXTRAS) per event?

    fModule->eA->sort(dADC, nADC, dTDC, nTDC);

  } else if(event->event_id == 2){

    // TODO - How do scalers change between v785 and v1730?
    std::cout << "This is a scaler event. It should never happen!" << std::endl;

    /*
    // Get the Scaler Bank
    TMBank* bSCAL = event->FindBank("SCLR");
    uint32_t *dSCAL = (uint32_t*)event->GetBankData(bSCAL);

    fModule->eA->incScalers(dSCAL);
    */
  }
    
  return flow;

}

/* 
   manalyzer run
*/

void MidasAnalyzerRun::BeginRun(TARunInfo* runinfo){
  printf("Begin run %d\n",runinfo->fRunNo);
  //time_t run_start_time = 0; // runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
  //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));

  auto start = std::chrono::system_clock::now();
  std::time_t start_time = std::chrono::system_clock::to_time_t(start);

  std::cout << "Start time = " << std::ctime(&start_time) << "\n";
  //  time_t run_start_time = run_start_time_binary;
  //  printf("ODB Run start time: %s\n",  run_start_time);

  // Read the parameters
  std::ifstream infile;
  infile.open("Parameters.dat",std::ifstream::in);
  
  infile >> pSiSlope;
  infile.close();

  printf("Silicon slope = %f\n",pSiSlope);

  fRunEventCounter = 0;
}

void MidasAnalyzerRun::EndRun(TARunInfo* runinfo){
  printf("End run %d\n",runinfo->fRunNo);
  printf("Counted %d events\n",fRunEventCounter);
  //std::cout << "Total counts: " << totalCounter << "   Gated counts: " << gateCounter << std::endl;

  //time_t run_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
  //printf("ODB Run stop time: %d: %s", (int)run_stop_time, ctime(&run_stop_time));

  auto stop = std::chrono::system_clock::now();
  std::time_t stop_time = std::chrono::system_clock::to_time_t(stop);
  std::cout << "Stop time = " << std::ctime(&stop_time) << "\n";

  //printf("BCI was %d\n",fModule->eA->getScalers()[10]);
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
    //.def("saysomething", &EngeSort::saysomething)     // string
    .def("Initialize", &EngeSort::Initialize)          // void
    .def("connectMidasAnalyzer", &EngeSort::connectMidasAnalyzer) // int
    .def("runMidasAnalyzer", &EngeSort::runMidasAnalyzer) // int
    .def("getData", &EngeSort::getData)                // 1D histograms
    .def("getData2D", &EngeSort::getData2D)            // 2D histograms
    .def("getis2Ds", &EngeSort::getis2Ds)                // bool vector
    .def("getNGates", &EngeSort::getNGates)          // int vector
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
