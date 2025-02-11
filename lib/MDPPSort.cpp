#include <boost/python/numpy/ndarray.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <sstream>
#include <string>

#include "EngeAnalyzerlib.h"
#include "MDPPEventHandler.h"
#include "MDPPSort.h"
#include "TV792Data.hxx"

#include <fstream>

Messages messages;

std::string EngeSort::sayhello()
{
	return messages.sayhello("MDPPSort");
}
std::string EngeSort::saygoodbye()
{
	return messages.saygoodbye();
}
std::string EngeSort::saysomething(std::string str)
{
	return messages.saysomething(str);
}

/* -------------
Global Variables

This is where histograms need to be defined.
----------------*/

// binning to use in the histograms
int Channels1D = 65536;
int ChannelsTDC = 10000;
int Channels2D = 500;

// Scalers
Scaler *sPulser;
Scaler *sBCI;
Scaler *sTriggers;
Scaler *sLN2;

////////////////
/* 1D Spectra */
////////////////

// Annulus Variables
// multiplicity
Histogram *hMulti;

// Annulus ADC Histograms (i.e. no calibration)
Histogram *hNaIADC[16];
// And TDC
Histogram *hNaITDC[16];

// This will be the energy calibrated sum.
Histogram *hNaIsumE;

// Plastic Scintillators ADC and TDC
Histogram *hSciADC[9];
Histogram *hSciTDC[9];

// HPGe Histograms

// HPGe Singles
Histogram *hGe;
// Energy Calibrated
Histogram *hGeE;

// CeBr if you have it
Histogram *hCeBr;
Histogram *hTDCCeBr;

// Pulser
Histogram *hPulser;

// TAC
Histogram *hBeam;

////////////////
/* 2D Spectra */
////////////////
Histogram *h2dGevsNaIsumE_no_gates;
Histogram *h2dGevsNaIsumE;

///////////////////
/* Gated Spectra */
///////////////////

// HPGe spectra gated on 2D gates.
// Implicit is that these will also have the effects of the NaI ADC & TDC gates.
Histogram *hGeNaITE2d[4];

// These are the same, but they will also include the scintillator veto.
Histogram *hGeNaITE2d_SV[4];

// random number generator for calibration values.
std::random_device rand_dev;
std::mt19937 generator(rand_dev());
std::uniform_real_distribution<double> uniform_sample(-0.1, 0.1);

std::string name_with_index(std::string name, int index)
{
	return name + std::to_string(index);
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

IntVector Calibrator::calibrate(vec_u32 &adc_values)
{
	IntVector result(adc_values.size(), 0);

	for (int i = 0; i < adc_values.size(); i++) {
		// do the calibration
		auto temp = (double)adc_values[i];
		// check to make sure we have a adc value and that we have a calibration
		if ((temp > 0) && (this->slope[i] > 0)) {
			result[i] = std::round(
				(this->slope[i] * temp + this->intercept[i]) +
				uniform_sample(generator));
		}
	}

	return result;
}

// Counters
int totalCounter = 0;

void EngeSort::Initialize()
{
	std::string hname;

	// Now we initialize all of the histograms defined above.

	//--------------------
	// 1D Histograms

	// Raw values first
	// NaI
	for (int i = 0; i < 16; i++) {
		hNaIADC[i] = new Histogram(name_with_index("NaI ADC ", i),
					   Channels1D, 1);
	}

	for (int i = 0; i < 16; i++) {
		hNaITDC[i] = new Histogram(name_with_index("NaI TDC ", i),
					   ChannelsTDC, 1);
	}

	// Sum and multiplicity
	hNaIsumE = new Histogram("NaI Sum Energy", Channels1D, 1);
	hMulti = new Histogram("NaI Multiplicity", 20, 1);

	// Plastic Scintillator ADC
	for (int i = 0; i < 9; i++) {
		hSciADC[i] = new Histogram(name_with_index("Sci ADC ", i),
					   Channels1D, 1);
	}
	// TDC
	for (int i = 0; i < 9; i++) {
		hSciTDC[i] = new Histogram(name_with_index("Sci TDC ", i),
					   Channels1D, 1);
	}

	hGe = new Histogram("HPGe Singles", Channels1D, 1);
	hGeE = new Histogram("HPGe Singles Energy", 10000, 1);

	hPulser = new Histogram("Pulser", Channels1D, 1);

	hBeam = new Histogram("Beam Pulse", ChannelsTDC, 1);
	
	// Scalers
	sPulser = new Scaler("BCI", 0); // Name, index
	sBCI = new Scaler("Pulser", 1); // Name, index
	sTriggers = new Scaler("Trigger", 2); // Name, index
	sLN2 = new Scaler("LN2", 3); // Name, index
	//--------------------
	// 2D Histograms
	//--------------------

	h2dGevsNaIsumE_no_gates = new Histogram("HPGe v NaI No Gates", Channels2D, 2);
	h2dGevsNaIsumE = new Histogram("HPGe v NaI", Channels2D, 2);

	//------
	// Gates
	//------

	// NaI
	for (int i = 0; i < 16; i++) {
		hNaIADC[i]->addGate(name_with_index("NaI ADC Gate ", i));
		hNaITDC[i]->addGate(name_with_index("NaI TDC Gate ", i));
	}
	// Plastic Scintillator
	for (int i = 0; i < 9; i++) {
		hSciADC[i]->addGate(name_with_index("Sci ADC Gate", i));
		hSciTDC[i]->addGate(name_with_index("Sci TDC Gate", i));
	}

	// 2D gates
	h2dGevsNaIsumE->addGate("2D_1");
	h2dGevsNaIsumE->addGate("2D_2");
	h2dGevsNaIsumE->addGate("2D_3");
	h2dGevsNaIsumE->addGate("2D_4");

	//--------------------
	// Gated Histograms
	//--------------------

	for (int i = 0; i < 4; i++) {
		hGeNaITE2d[1] = new Histogram(name_with_index("Ge T E 2D_", i),
					      Channels1D, 1);
	}

	for (int i = 0; i < 4; i++) {
		hGeNaITE2d_SV[i] = new Histogram(
			name_with_index("Ge T E SV 2D_", i), Channels1D, 1);
	}

	// Initialize the energy calibrator
	this->calibrator_annulus_ps.load_file("nai_cal.csv");
	this->calibrator_hpge.load_file("hpge_cal.csv");
}

//======================================================================
// This is the equivalent to the "sort" function in jam
// Arguments are the raw events
void EngeSort::sort(MDPPEvent &event_data)
{
	totalCounter++;

	// get the data to make it easier for later
	vec_u32 scp_adc = event_data.get_data("scp1").adc;
	vec_u32 scp_tdc = event_data.get_data("scp1").tdc;

	vec_u32 qdc_adc = event_data.get_data("qdc1").adc;
	vec_u32 qdc_tdc = event_data.get_data("qdc1").tdc;

	IntVector cal_values = this->calibrator_annulus_ps.calibrate(qdc_adc);
	IntVector hpge_cal_values = this->calibrator_hpge.calibrate(scp_adc);

	// pulser count
	int cpulser = scp_adc[4];
	bool pulser_event = false;
	if (cpulser > 0) {
		hPulser->inc(
			scp_adc[0]); // The pulser histogram are the counts from the HPGe.
		pulser_event = true;
	}

	// Increment 1D histograms
	// NaI
	for (int i = 0; i < 16; i++) {
		hNaIADC[i]->inc(qdc_adc[i]);
		hNaITDC[i]->inc(qdc_tdc[i]);
	}

	// Plastic Scintillator
	for (int i = 0; i < 9; i++) {
		hSciADC[i]->inc(qdc_adc[i + 17]);
		hSciTDC[i]->inc(qdc_adc[i + 17]);
	}

	// HPGe data
	if (!pulser_event) {
		hGe->inc(scp_adc[0]);
		hGeE->inc(hpge_cal_values[0]);
	}

	hBeam -> inc(scp_tdc[6]);
	
	// So now sum the NaI segments

	double SumNaIE = 0.0;
	double SumNaIE_no_gates = 0.0;
	int multi = 0;
	bool first_hit = true;
	int NaITDC;
	// sum up the calibrated annulus
	for (int i = 0; i < 16; i++) {
		Gate &GNaIADC = hNaIADC[i]->getGate(0);
		Gate &GNaITDC = hNaITDC[i]->getGate(0);
		SumNaIE_no_gates += (double)cal_values[i];
		if (GNaIADC.inGate(qdc_adc[i]) && GNaITDC.inGate(qdc_tdc[i])) {
			SumNaIE += (double)cal_values[i];
			multi += 1;
		}
	}

	hNaIsumE->inc(SumNaIE);
	hMulti->inc(multi);

	// // ------------------------------------------------------------
	// // Compressed versions for 2D spectra
	// // ------------------------------------------------------------
	double compressionE =
		Channels2D / 5000.0; //(double)Channels1D/ (double)Channels2D;
	double compressionT =
		Channels2D / 5000.0; //(double)Channels1D/ (double)Channels2D;
	int cNaISumE = (int)std::floor(SumNaIE * compressionE);
	int cNaISumE_no_gates = (int)std::floor(SumNaIE_no_gates * 0.1);
	int cGeE = (int)std::floor((double)hpge_cal_values[0] * compressionE);

	// // // Increment 2D histograms
	h2dGevsNaIsumE_no_gates->inc(cGeE, cNaISumE_no_gates);
	h2dGevsNaIsumE->inc(cGeE, cNaISumE);

	// Check if the veto fired.
	bool scint_veto = false;
	for (int i = 0; i < 9; i++) {
		Gate &gSciADC = hSciADC[i]->getGate(0);
		Gate &gSciTDC = hSciTDC[i]->getGate(0);
		scint_veto = scint_veto || (gSciADC.inGate(qdc_adc[i + 17]) &&
					    gSciTDC.inGate(qdc_tdc[i + 17]));
	}
	// Now increment the HPGe gated histograms.
	for (int i = 0; i < 4; i++) {
		Gate &g2d = h2dGevsNaIsumE->getGate(i);
		// Do we pass the ith 2D gate?
		if (g2d.inGate(cGeE, cNaISumE)) {
			hGeNaITE2d[i]->inc(hpge_cal_values[0]);
			// Does the scintillator veto this event?
			if (!scint_veto) {
				hGeNaITE2d_SV[i]->inc(hpge_cal_values[0]);
			}
		}
	}
}

// Increment the scalers
// TODO: make this automatic. If the scaler is
// defined we should assume that the user wants to increment it
void EngeSort::incScalers(uint32_t *dSCAL)
{
	sPulser->inc(dSCAL);
	sBCI->inc(dSCAL);
	sTriggers->inc(dSCAL);
	sLN2->inc(dSCAL);
}

// Connect the analyzer to midas
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
	Py_END_ALLOW_THREADS

		return 0;
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
	BoolVector is2d;
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
					//G1.Print();

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
					//G1.Print();
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

//----------------------------------------------------------------------

/* 
   manalyzer module
*/
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
	printf("Counted %d events\n", fTotalEventCounter);
	std::cout << "number of spectra: " << eA->getSpectrumNames().size()
		  << std::endl;
	eA->setIsRunning(false);
}
TARunObject *MidasAnalyzerModule::NewRunObject(TARunInfo *runinfo)
{
	printf("NewRunObject, run %d, file %s\n", runinfo->fRunNo,
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

/* 
   manalyzer run
*/
void MidasAnalyzerRun::BeginRun(TARunInfo *runinfo)
{
	printf("Begin run %d\n", runinfo->fRunNo);
	uint32_t run_start_time_binary = 0;
	//runinfo->fOdb->RU32("/Runinfo/Start time binary", &run_start_time_binary);
	time_t run_start_time = run_start_time_binary;
	printf("ODB Run start time: %d: %s", (int)run_start_time,
	       ctime(&run_start_time));

	fRunEventCounter = 0;
}

void MidasAnalyzerRun::EndRun(TARunInfo *runinfo)
{
	printf("End run %d\n", runinfo->fRunNo);
	printf("Counted %d events\n", fRunEventCounter);
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
