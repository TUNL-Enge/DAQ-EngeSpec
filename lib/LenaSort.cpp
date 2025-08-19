// ******************************************************************************
// GENERIC SORT ROUTINE FOR LENA GAMMA-GAMMA SPECTROMETER
// ******************************************************************************
//
// Authors: Caleb Marshall & Christian Iliadis
// Date: Feb. 2025
//
// ******************************************************************************
//
// Summary of setup procedure:
//
//  (1)  record noise level and set discriminator thresholds on all detectors
//  (2)  load and sort data
//  (3)  check NaI TDC spectra [60Co or 22Na]; there should be no structure in the
//       central peak, and no oscillatory pattern on either the left or right side
//  (4)  energy calibrate the NaI segments [using: 22Na (511 keV and 1275 keV); or
//       22Na (511 keV) and 60Co (2505 keV sum peak); or 14N(p,g) (763 keV and
//       2373 keV)
//  (5)  enter energy calibrations in external files
//  (6)  set gates in ADC and TDC spectra, both for annulus and scintillators
//  (7)  zero all histograms and sort data
//  (8)  set gate on multiplicity spectrum
//  (9)  set gates in the 2D spectra
//  (10) zero all histograms and sort data
//
// ******************************************************************************
//
// Comments:
//
// This sort routine is set up for a Ge detector plus a 16 segment NaI annulus
// plus several segments of plastic scintillator (veto). All events are analyzed
// that give rise to a Ge signal. Stored events are: Ge and individual NaI segment
// energies, time differences between NaI segment STOP and Ge (common) START, etc.
// Remember, NaI segment single spectra are not collected.
//
// Make sure that NaI discriminator thresholds are not set too high; you will see
// immediately if this is the case in the raw NaI [channel] spectra; if some of
// them do not show the 511 keV line because the threshold was set above this
// energy; so, the procedure for setting the thresholds is: high enough to cut
// out noise, but low enough to observe the 511 keV line.
//
// The energies of the same event in all NaI segments need to be added together
// in order to produce a NaI sum energy signal. That is done by energy calibrating
// each NaI segment first, then the signals from individual segments are added to
// display:
//   (i)  a 1D NaI sum energy spectrum [which is compressed by some factor]
//   (ii) 2D Ge vs. NaI sum energy spectra [both axes are compressed]; for both
//        the Ge and the NaI, the axes are in energy units
//
// Note 1: the energy in a NaI segment is only summed if the event is located both
//         in NaI ADC gate and in the Ge-NaI TDC coincidence gate.
// Note 2: after performing a NaI energy calibration, inspect the 1D NaI sum
//         spectrum to see if its shape makes sense; if one sees any double
//         peaks or similar, the energy calibrations on some segments may be off.
//
// For a given energy calibration it is important to choose compression factors
// and energy spectrum dimensions so that the energy calibrated spectra are
// compressed, otherwise energy channels with zero counts are produced.
//
// Gates are set on (i) the individual NaI segment ADC spectra (these should be
// "wide" gates, the lower limit set right above noise and the upper limit set close
// to the end of the spectrum; only events located in these gates are summed to
// produce a NaI sum energy signal); (ii) the individual TDC spectra (these should
// be centered on the true coincidence peak); (iii) the multiplicity spectrum
// (typically, you would set a gate to include all multiplicities); (iv) the 2D Ge
// (energy) vs. NaI sum (energy) spectrum; 4 gates can be set on the energy regions
// of interest; typically, in order to cut out room background events, one may set a
// gate E(Ge)+E(NaI)>4 MeV, or so.
//
// Gated Ge spectra (all in energy) are displayed for specific gating conditions.
//
// The sort routine includes several veto plastic scintillators. Gates can be set
// on each scintillator energy spectra (usually set on the muon peak) and on the
// true coincidence peak in the Ge-Sci timing spectrum. The vetoed Ge spectra that
// are gated on the Ge-NaI TDC's AND on the Ge-NaI 2D spectrum are not incremented
// if there is a signal in the Ge-Sci TAC gate or the Sci energy gate.
//
// Included are singles HPGe histograms that are vetoed on either scintillators
// or annulus. The latter is most useful for background suppression if the transition
// of interest is a ground state transition (i.e., multiplicity of 1). The scintillator
// veto (see above) will suppress muon events, but the NaI veto will also suppress
// background due to:
//     (i)  beam or cosmic-ray induced background cascades (i.e., one BG photon hits
//          the Ge, the other one the NaI), and
//     (ii) beam or cosmic-ray induced single BG photons that scatter from the Ge
//          into the NaI. For such gamma-ray events, the scintillators have only
//          small efficiencies.
//
// Included multiplicity spectrum that user can gate on; to include all multiplicities,
// set a wide gate over all channels; this spectrum shows actually 5 x multiplicity,
// for easier gating.
//
// ??check this statement??
// When incrementing the gated spectra:
// specify explicitly all conditions, otherwise confusion may arise; do not
// assume any implicit conditions; in other words, e.g., the sum NaI energy is just
// a variable; its value has no explicit information about logic it was obtained
// from.
//
//
// ToDo:
// (a) singles Ge spectrum that is vetoed both by the NaI AND by the scintillators.
// (b) beam pulsing; beam pulse on/off information can be found in histogram
//     hBeamPulse; setting a gate around peak in higher channels corresponds to
//     tagging events according to when beam was on or off; NOTE: "Ge BeamPulseOff"
//     is not really the spectrum with no beam, but it is the spectrum for all events
//     that are outside the "beam on gate".
//
// ******************************************************************************
// GENERAL INFORMATION
// ******************************************************************************

#include <boost/python/numpy/ndarray.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <sstream>
#include <string>
#include <cstdio>

#include "EngeAnalyzerlib.h"
#include "MDPPEventHandler.h"
#include "LenaSort.h"
#include "TV792Data.hxx"

#include <fstream>

Messages messages;

std::string EngeSort::sayhello()
{
	return messages.sayhello("LenaSort");
}
std::string EngeSort::saygoodbye()
{
	return messages.saygoodbye();
}
std::string EngeSort::saysomething(std::string str)
{
	return messages.saysomething(str);
}

// ******************************************************************************
// DEFINE HISTOGRAMS
// ******************************************************************************

// Binning to use in the histograms

// ADC channels:
int Channels1D = 65536;

// TDC channels; 781 ps per channel for common settings:
int ChannelsTDC = 10000;

// compressed channels in 2D histograms (energies):
int Channels2D = 1000;

// channels in multiplicity spectrum (stretched by factor 5):
int ChannelsMulti = 100;

// channels in energy spectra:
double Energies1D = 20000;

// Compression factor for 1D and 2D histograms
double compressE = 0.1;

// Scalers
Scaler *sPulser;
Scaler *sBCI;
Scaler *sTriggers;
Scaler *sLN2;

// -----------------------
// 1D HISTOGRAMS: UNGATED
// -----------------------
// 1D spectra have names h....; 2D spectra have names h2...

// HPGe singles (channels)
Histogram *hGe;

// HPGe singles (vetoed by scintillators)
Histogram *hGe_SV;

// HPGe singles (vetoed by annulus)
Histogram *hGe_NaIV;

// Pulser (HPGe preamp input)
Histogram *hPulser;

// HPGe singles (energy)
Histogram *hGeE;

// Annulus multiplicity
Histogram *hMulti;

// Annulus ADCs (in channel units)
Histogram *hNaIADC[16];

// Annulus TDCs (in channel units)
Histogram *hNaITDC[16];

// Energy sum in all segments (in energy units)
Histogram *hNaIsumE;

// Plastic scintillator ADCs and TDCs (channels)
Histogram *hSciADC[9];
Histogram *hSciTDC[9];

// -----------------------
// 2D HISTOGRAMS: GATED
// -----------------------

// Ge v NaIsumE (energies); gated on TDCs
Histogram *h2dGevsNaIsumE[4];

// -----------------------
// 1D HISTOGRAMS: GATED
// -----------------------

// HPGe gated on: NaI ADCs, NaI TDCs, 2D Ge v NaI, scint vetoed
Histogram *hGeTE2dSV[4];

// NaI gated on: NaI ADCs, NaI TDCs, 2D Ge v NaI, scint vetoed
Histogram *hNaITE2dSV[4];

// ******************************************************************************
// OTHER: UTILITIES, ENERGY CALIBRATION
// ******************************************************************************

// Random number generator setup for Gaussian distribution
std::random_device rand_dev;
std::mt19937 generator(rand_dev());
// Mean = 0.0, Standard deviation = 0.5 (energy units)
std::normal_distribution<double> gaussian_sample(0.0, 0.5);

// Utility to add integers to strings [used later]
std::string name_with_index(std::string name, int index)
{
	return name + std::to_string(index);
}

// Utility for transforming TDC values
int shift_tdc(int orig_val, int new_ref, int offset)
{
	// The logic here is that if new_ref did not fire, treat it like
	// the orig channel did not fire at all.
	if ((new_ref > 0) && (orig_val > 0)) {
		return (orig_val - new_ref) + offset;
	} else {
		return 0;
	}
}

// Calibrator setup
void Calibrator::load_file(std::string filename)
{
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

	while (std::getline(file, line)) {
		// just skipping the headers for now.
		if (first_line) {
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

		while (std::getline(ss, temp_string, delim)) {
			// temp string holds each column
			// col is incremented each time and keeps track of the column
			switch (col) {
			case 0:
				// bank name case, it is not implemented right now
				break;

			case 1:
				channel = std::stoi(temp_string);
				break;

			case 2:
				this->slope[channel] = std::stod(temp_string);
				break;

			case 3:
				this->intercept[channel] =
					std::stod(temp_string);
				break;

			default:
				throw;
			}
			col += 1;
		}
	}
}

// Modify the calibrate function to use the Gaussian distribution
IntVector Calibrator::calibrate(vec_u32 &adc_values)
{
	IntVector result(adc_values.size(), 0);

	for (int i = 0; i < adc_values.size(); i++) {
		auto temp = static_cast<double>(adc_values[i]);
		if ((temp > 0) && (this->slope[i] > 0)) {
			result[i] = std::round(
				(this->slope[i] * temp + this->intercept[i]) +
				gaussian_sample(generator));
		}
	}
	return result;
}

// Counters
int totalCounter = 0;

// ******************************************************************************
// INITIALIZATIONS [SCALERS, HISTOGRAMS, GATES]
// ******************************************************************************

void EngeSort::Initialize()
{
	std::string hname;

	//------------------------------
	// Scalers
	//------------------------------

	sBCI = new Scaler("BCI", 0); // Name, index
	sPulser = new Scaler("Pulser", 1);
	sTriggers = new Scaler("Trigger", 2);
	sLN2 = new Scaler("LN2", 3);

	//------------------------------
	// 1D Histograms: ungated/vetoed
	//------------------------------

	// HPGe singles (channels)
	hGe = new Histogram("Ge Singles", Channels1D, 1);

	// HPGe singles (channels) vetoed by scintillators
	hGe_SV = new Histogram("Ge Singles Scint veto", Channels1D, 1);

	// HPGe singles (channels) vetoed by annulus
	hGe_NaIV = new Histogram("Ge Singles NaI veto", Channels1D, 1);

	// Pulser (HPGe preamp input)
	hPulser = new Histogram("Pulser", Channels1D, 1);

	// HPGe singles (energy)
	hGeE = new Histogram("Ge Singles E", Energies1D, 1);

	// NaI ADCs (channels)
	for (int i = 0; i < 16; i++) {
		hNaIADC[i] = new Histogram(name_with_index("NaI ADC ", i),
					   Channels1D, 1);
	}

	// NaI TDCs (channels)
	for (int i = 0; i < 16; i++) {
		hNaITDC[i] = new Histogram(name_with_index("NaI TDC ", i),
					   ChannelsTDC, 1);
	}

	// NaI energy sum (energies)
	hNaIsumE = new Histogram("NaI Sum E", Energies1D, 1);

	// NaI multiplicity
	hMulti = new Histogram("NaI Multi", ChannelsMulti, 1);

	// Plastic scintillator ADCs
	for (int i = 0; i < 9; i++) {
		hSciADC[i] = new Histogram(name_with_index("Sci ADC ", i),
					   Channels1D, 1);
	}

	// Plastic scintillator TDCs
	for (int i = 0; i < 9; i++) {
		hSciTDC[i] = new Histogram(name_with_index("Sci TDC ", i),
					   Channels1D, 1);
	}

	//------------------------------
	// 2D Histograms: gated
	//------------------------------

	// Ge v NaIsumE (energies)
	for (int i = 0; i < 4; i++) {
		h2dGevsNaIsumE[i] = new Histogram(
			name_with_index("NaI v Ge ", i), Channels2D, 2);
	}

	//------------------------------
	// 1D Histograms: gated
	//------------------------------

	// HPGe gated on: NaI ADCs, NaI TDCs, 2D Ge v NaI, scint vetoed
	for (int i = 0; i < 4; i++) {
		hGeTE2dSV[i] = new Histogram(
			name_with_index("Ge T E SV 2D_", i), Energies1D, 1);
	}

	// NaI gated on: NaI ADCs, NaI TDCs, 2D Ge v NaI, scint vetoed
	for (int i = 0; i < 4; i++) {
		hNaITE2dSV[i] = new Histogram(
			name_with_index("NaI T E SV 2D_", i), Energies1D, 1);
	}

	//------------------------------
	// Define gates in histograms
	//------------------------------

	// on 1D histograms of NaI ADCs and TDCs
	for (int i = 0; i < 16; i++) {
		hNaIADC[i]->addGate(name_with_index("NaI ADC Gate ", i));
		hNaITDC[i]->addGate(name_with_index("NaI TDC Gate ", i));
	}

	// on 1D histograms of plastic scintillator ADCs and TDCs
	for (int i = 0; i < 9; i++) {
		hSciADC[i]->addGate(name_with_index("Sci ADC Gate", i));
		hSciTDC[i]->addGate(name_with_index("Sci TDC Gate", i));
	}

	// annulus multiplicity gate
	hMulti->addGate("NaI Multiplicity");

	// on 2D histograms of HPGe v NaI (energies)
	for (int i = 0; i < 4; i++) {
		h2dGevsNaIsumE[i]->addGate(name_with_index("2D_", i));
	}

	//----------------------------------
	// Initialize energy calibrators
	//----------------------------------

	// read from external files for HPGe and NaI segments
	this->calibrator_annulus_ps.load_file("nai_cal.csv");
	this->calibrator_hpge.load_file("hpge_cal.csv");
}

// ******************************************************************************
// DATA SORT [PROCESS EACH EVENT]
// ******************************************************************************

// Increment scalers
void EngeSort::incScalers(uint32_t *dSCAL)
{
	sPulser->inc(dSCAL);
	sBCI->inc(dSCAL);
	sTriggers->inc(dSCAL);
	sLN2->inc(dSCAL);
}

void EngeSort::sort(MDPPEvent &event_data)
{
	totalCounter++;

	//-----------------------------
	// Assign data variables
	//-----------------------------

	vec_u32 scp_adc = event_data.get_data("scp1").adc;
	vec_u32 scp_tdc = event_data.get_data("scp1").tdc;

	vec_u32 qdc_adc = event_data.get_data("qdc1").adc;
	vec_u32 qdc_tdc = event_data.get_data("qdc1").tdc;

	IntVector hpge_cal = this->calibrator_hpge.calibrate(scp_adc);
	IntVector nai_cal = this->calibrator_annulus_ps.calibrate(qdc_adc);

	// ref time is the self trigger time stamp of the hpge energy channel
	// it will be added to time differences latter on to ensure
	// all timing histograms are positive.
	int ref_time = scp_tdc[0];

	// Now see if the hpge timing optimized channel fired
	int cGe_t = scp_tdc[2];

	//----------------------------------
	// Define pulser events
	//----------------------------------

	// the pulser histogram contains the counts from the HPGe
	int cpulser = scp_adc[4];

	bool pulser_event = false;
	if (cpulser > 0) {
		hPulser->inc(scp_adc[0]);
		pulser_event = true;
	}

	//----------------------------------
	// Increment 1D histograms: ungated
	//----------------------------------

	// HPGe
	if (!pulser_event) {
		hGe->inc(scp_adc[0]);
		hGeE->inc(hpge_cal[0]);
	}

	// NaI
	for (int i = 0; i < 16; i++) {
		hNaIADC[i]->inc(qdc_adc[i]);
		hNaITDC[i]->inc(shift_tdc(qdc_tdc[i], cGe_t, ref_time));
	}

	// Plastic Scintillator
	for (int i = 0; i < 9; i++) {
		hSciADC[i]->inc(qdc_adc[i + 17]);
		hSciTDC[i]->inc(shift_tdc(qdc_tdc[i + 17], cGe_t, ref_time));
	}

	//----------------------------------------------------------
	// Sum energies of NaI segments; determine multiplicity and
	// if annulus fired
	//----------------------------------------------------------

	double SumNaIE = 0.0;
	int multi = 0;
	bool nai_fire = false;

	// Determine if NaI fired; sum NaI energies
	for (int i = 0; i < 16; i++) {
		Gate &gNaIADC = hNaIADC[i]->getGate(0);
		Gate &gNaITDC = hNaITDC[i]->getGate(0);

		// Check if both ADC and TDC signals are in gate
		if (gNaIADC.inGate(qdc_adc[i]) &&
		    gNaITDC.inGate(shift_tdc(qdc_tdc[i], cGe_t, ref_time))) {
			// Sum the energy and count multiplicity
			SumNaIE += static_cast<double>(nai_cal[i]);
			multi += 5;

			// Flag that the annulus fired
			nai_fire = true;
		}
	}

	//------------------------------------
	// Determine if scintillators fired
	//------------------------------------

	bool sci_fire = false;
	for (int i = 0; i < 9; i++) {
		Gate &gSciADC = hSciADC[i]->getGate(0);
		Gate &gSciTDC = hSciTDC[i]->getGate(0);
		// scintillator fired if there are a signal in the energy OR
		// timing spectrum
		sci_fire = sci_fire ||
			   (gSciADC.inGate(qdc_adc[i + 17]) ||
			    gSciTDC.inGate(shift_tdc(qdc_tdc[i + 17], cGe_t,
						     ref_time)));
	}

	//----------------------------------------------------------------
	// Increment 1D HPGe histograms vetoed by scintillators or annulus
	//----------------------------------------------------------------

	if (!pulser_event && !sci_fire) {
		hGe_SV->inc(scp_adc[0]);
	}

	if (!pulser_event && !nai_fire) {
		hGe_NaIV->inc(scp_adc[0]);
	}

	//------------------------------------
	// Increment 1D histograms: gated
	//------------------------------------

	// Increment NaI energy sum and multiplicity histograms;
	// they are gated on NaI ADCs and TDCs
	if (nai_fire) {
		hNaIsumE->inc(SumNaIE);
		hMulti->inc(multi);
	}

	//-----------------------------------------------------
	// Compress variables for 2D histograms in energy units and
	// increment compressed 2D histogram (gated)
	//-----------------------------------------------------

	// compressed variables (energies)
	int cGeE = (int)std::floor((double)hpge_cal[0] * compressE);
	int cSumNaIE = (int)std::floor(SumNaIE * compressE);

	if (nai_fire) {
		for (int i = 0; i < 4; i++) {
			h2dGevsNaIsumE[i]->inc(cGeE, cSumNaIE);
		}
	}

	//---------------------------------------------
	// Increment 1D HPGe and NaI histograms: gated
	//---------------------------------------------

	for (int i = 0; i < 4; i++) {
		Gate &g2d = h2dGevsNaIsumE[i]->getGate(0);
		Gate &gMulti = hMulti->getGate(0);
		// Check if:
		//   - No scintillator veto (!sci_fire)
		//   - NaI fired (nai_fire)
		//   - The event falls within the 2D gate (g2d.inGate(cGeE, cSumNaIE))
		//   - The event passes the NaI multiplicity gate (gMulti.inGate(multi))
		if (!sci_fire && nai_fire && g2d.inGate(cGeE, cSumNaIE) &&
		    gMulti.inGate(multi)) {
			hGeTE2dSV[i]->inc(
				hpge_cal[0]); // Increment HPGe histogram
			hNaITE2dSV[i]->inc(SumNaIE); // Increment NaI histogram
		}
	}
}

// ******************************************************************************
// CONNECT TO AND RUN MIDAS
// ******************************************************************************

int EngeSort::connectMidasAnalyzer()
{
	TARegister tar(&mAMod);

	mAMod.ConnectEngeAnalyzer(this);
	return 0;
}

// Run the midas analyzer
int EngeSort::runMidasAnalyzer(boost::python::list file_list)
{
	std::cout << "runMidasAnalyzer " << len(file_list) << std::endl;
	// We need to send a dummy argument to manalyzer, which gets ignored
	std::string filename = "dummy ";
	for (int i = 0; i < len(file_list); i++) {
		std::string file =
			boost::python::extract<std::string>(file_list[i]);
		std::cout << " " << file;
		filename += file + " ";
	}
	filename += " --no-profiler";

	std::cout << std::endl;
	std::cout << filename << std::endl;

	enum { kMaxArgs = 64 };
	int ac = 0;
	char *av[kMaxArgs];

	char *dup = strdup(filename.c_str());
	char *p2 = strtok(dup, " ");

	while (p2 && ac < kMaxArgs - 1) {
		av[ac++] = p2;
		p2 = strtok(0, " ");
	}
	av[ac] = 0;

	Py_BEGIN_ALLOW_THREADS manalyzer_main(ac, av);
	//    manalyzer_main(0,0);
	Py_END_ALLOW_THREADS return 0;
}

// Return a vector of spectrum names
StringVector EngeSort::getSpectrumNames()
{
	StringVector s;
	for (auto h : Histograms) {
		s.push_back(h->getName());
	}
	return s;
}
// Return a vector of spectrum names
StringVector EngeSort::getScalerNames()
{
	StringVector s;
	for (auto Sclr : Scalers) {
		s.push_back(Sclr->getName());
	}
	return s;
}
// Return a bool vector of whether the spectra are 2D
BoolVector EngeSort::getis2Ds()
{
	BoolVector is2D;
	for (auto h : Histograms) {
		bool b = (h->getnDims() == 2) ? true : false;
		is2D.push_back(b);
	}
	return is2D;
}
// Return a bool vector of whether the spectra have gates
IntVector EngeSort::getNGates()
{
	IntVector ngates;
	for (auto h : Histograms) {
		ngates.push_back(h->getNGates());
	}
	return ngates;
}

// Return the number of channels in each Histogram
IntVector EngeSort::getNChannels()
{
	IntVector nchannels;
	for (auto h : Histograms) {
		nchannels.push_back(h->getnChannels());
	}
	return nchannels;
}

// Return a vector of scalers
IntVector EngeSort::getScalers()
{
	IntVector sclr;
	for (auto sc : Scalers) {
		sclr.push_back(sc->getValue());
	}
	return sclr;
}

np::ndarray EngeSort::getData()
{
	// Create the matrix to return to python
	u_int n_rows = nHist1D; //Histograms.size();
	u_int n_cols = Channels1D; //Histograms[0].getnChannels();
	p::tuple shape = p::make_tuple(n_rows, n_cols);
	p::tuple stride = p::make_tuple(sizeof(int));
	np::dtype dtype = np::dtype::get_builtin<int>();
	p::object own;
	np::ndarray converted = np::zeros(shape, dtype);

	// Loop through all histograms and pack the 1D histograms into a numpy matrix
	int i = 0;
	for (auto const &h : Histograms) {
		if (h->getnDims() == 1) {
			IntVector &temp = h->getData1D();
			for (int j = 0; j < h->getnChannels(); j++) {
				converted[i][j] = temp[j];
			}

			i++;
		}
	}
	return converted;
}

np::ndarray EngeSort::getData2D()
{
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
	int t = 0;
	for (auto const &h : Histograms) {
		if (h->getnDims() == 2) {
			std::vector<IntVector> &temp = h->getData2D();
			for (int i = 0; i < h->getnChannels(); i++) {
				for (int j = 0; j < h->getnChannels(); j++) {
					converted[t][i][j] = temp[i][j];
				}
			}
			t++;
		}
	}
	return converted;
}

StringVector EngeSort::getGateNames(std::string hname)
{
	StringVector gname;

	// First find the spectrum that corresponds to the hname
	for (auto h : Histograms) {
		if (h->getName() == hname) {
			//std::cout << "Hist: " << hname << " has " << h -> getNGates() << " gates" << std::endl;
			// Make sure this histogram has gates defined
			for (int i = 0; i < (h->getNGates()); i++) {
				Gate G1 = h->getGate(i);
				gname.push_back(G1.getName());
			}
		}
	}

	return gname;
}

void EngeSort::putGate(std::string name, std::string gname, p::list x,
		       p::list y)
{
	//std::cout << "Putting gate: " << gname << " into histogram " << name << std::endl;

	// First find the spectrum that corresponds to the name
	for (auto h : Histograms) {
		if (h->getName() == name) {
			//std::cout << "Found the histogram! With name: " << h->getName() << std::endl;

			// Make sure this histogram has gates defined
			//if(h -> getNGates() > 0){
			//std::cout << "Yes, this histogram has gates!" << std::endl;
			for (int ig = 0; ig < (h->getNGates()); ig++) {
				Gate &G1 = h->getGate(ig);
				if (G1.getName() == gname) {
					G1.Clear();
					p::ssize_t len = p::len(x);
					// Make a vector for the gate
					for (int i = 0; i < len; i++) {
						std::vector<double> tmp;
						tmp.push_back(
							p::extract<double>(
								x[i]));
						tmp.push_back(
							p::extract<double>(
								y[i]));
						G1.addVertex(tmp);
					}
				}
			}
		}
	}
}

void EngeSort::ClearData()
{
	for (auto h : Histograms) {
		h->Clear();
	}

	for (auto Sclr : Scalers) {
		Sclr->Clear();
	}
	totalCounter = 0;
}

//------------------------------------
// manalyzer module
//------------------------------------

void MidasAnalyzerModule::Init(const std::vector<std::string> &args)
{
	printf("Initializing Midas Analyzer Module\n");
	printf("Arguments:\n");
	for (unsigned i = 0; i < args.size(); i++)
		printf("arg[%d]: [%s]\n", i, args[i].c_str());

	fTotalEventCounter = 0;
}

void MidasAnalyzerModule::Finish()
{
	printf("Finish!\n");
	eA->setIsRunning(false);
}

TARunObject *MidasAnalyzerModule::NewRunObject(TARunInfo *runinfo)
{
	printf("\n\n-------------------------------------------------\n");
	printf("Loading run file, run %d, file %s\n", runinfo->fRunNo,
	       runinfo->fFileName.c_str());
	eA->setIsRunning(true);
	return new MidasAnalyzerRun(runinfo, this);
}

TAFlowEvent *MidasAnalyzerRun::Analyze(TARunInfo *runinfo, TMEvent *event,
				       TAFlags *flags, TAFlowEvent *flow)
{
	//  std::cout << "Analyzing" << std::endl;

	if (event->event_id == 1) {
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
	} else if (event->event_id == 2) {
		// Get the Scaler Bank
		TMBank *bSCAL = event->FindBank("SCLR");
		uint32_t *dSCAL = (uint32_t *)event->GetBankData(bSCAL);

		fModule->eA->incScalers(dSCAL);
	}

	//  STD::cout << bSCAL << "  " << dSCAL << std::endl;

	return flow;
}

//------------------------------------
//   manalyzer run
//------------------------------------

void MidasAnalyzerRun::BeginRun(TARunInfo *runinfo)
{
	printf("Begin run %d\n", runinfo->fRunNo);
	uint32_t run_start_time_binary = 0;
	//runinfo->fOdb->RU32("/Runinfo/Start time binary", &run_start_time_binary);
	time_t run_start_time = run_start_time_binary;
	//printf("ODB Run start time: %d: %s", (int)run_start_time,
	//       ctime(&run_start_time));
	fModule->eA->setIsRunning(true);
	fRunEventCounter = 0;
}

void MidasAnalyzerRun::EndRun(TARunInfo *runinfo)
{
	printf("End run %d\n", runinfo->fRunNo);
	printf("Counted %d events\n", fRunEventCounter);
	fModule->eA->setIsRunning(false);
}

BOOST_PYTHON_MODULE(EngeSort)
{
	using namespace boost::python;
	// Initialize numpy
	Py_Initialize();
	boost::python::numpy::initialize();
	//    def( "MakeData", MakeData );

	class_<vec>("double_vec").def(vector_indexing_suite<vec>());
	class_<mat>("double_mat").def(vector_indexing_suite<mat>());
	class_<mat2d>("double_mat2d").def(vector_indexing_suite<mat2d>());
	class_<StringVector>("StringVector")
		.def(vector_indexing_suite<StringVector>());
	class_<BoolVector>("BoolVector")
		.def(vector_indexing_suite<BoolVector>());
	class_<IntVector>("IntVector").def(vector_indexing_suite<IntVector>());

	class_<EngeSort>("EngeSort")
		.def("sayhello", &EngeSort::sayhello) // string
		.def("saygoodbye", &EngeSort::saygoodbye) // string
		.def("saysomething", &EngeSort::saysomething) // string
		.def("Initialize", &EngeSort::Initialize) // void
		.def("connectMidasAnalyzer",
		     &EngeSort::connectMidasAnalyzer) // int
		.def("runMidasAnalyzer", &EngeSort::runMidasAnalyzer) // int
		.def("getData", &EngeSort::getData) // 1D histograms
		.def("getData2D", &EngeSort::getData2D) // 2D histograms
		.def("getis2Ds", &EngeSort::getis2Ds) // bool vector
		.def("getNGates", &EngeSort::getNGates) // int vector
		.def("getNChannels", &EngeSort::getNChannels) // int vector
		.def("getSpectrumNames",
		     &EngeSort::getSpectrumNames) // string vector
		.def("getIsRunning", &EngeSort::getIsRunning) // bool value
		.def("getScalerNames",
		     &EngeSort::getScalerNames) // string vector
		.def("getScalers",
		     &EngeSort::getScalers) // IntVector of scaler values
		.def("getGateNames",
		     &EngeSort::getGateNames) // string vector of gate names
		.def("ClearData", &EngeSort::ClearData) // void
		.def("putGate", &EngeSort::putGate) // void
		.def("data", range(&EngeSort::begin, &EngeSort::end));
}
