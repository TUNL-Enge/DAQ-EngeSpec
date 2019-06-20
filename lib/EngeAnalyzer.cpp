#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include "EngeAnalyzer.h"
#include "TV792Data.hxx"

char const* EngeAnalyzer::sayhello( ) {
    return "Hello! This is the data maker running in c++!!!";
}
char const* EngeAnalyzer::saygoodbye( ) {
  std::cout << "Peak 1 has " << ipeak1 << " counts" << std::endl;
  std::cout << "Peak 2 has " << ipeak2 << " counts" << std::endl;
  std::cout << "Gated peak has " << igated << " counts" << std::endl;
  
  return "Goodbye! I hope I served you well";
}

void EngeAnalyzer::Initialize(){

  // Set the names of the data
  DataNames.clear();
  DataNames.push_back("Pos1");
  is2D.push_back(false);
  hasGate.push_back(false);
  DataNames.push_back("DE");
  is2D.push_back(false);
  hasGate.push_back(true);
  DataNames.push_back("Pos1-cut");
  is2D.push_back(false);
  hasGate.push_back(false);
  
  // Fill some empty spectra
  std::vector<int> tempSpec;
  tempSpec.resize(4096,0);
  DataMatrix.clear();
  for(int i=0; i<DataNames.size(); i++)
    DataMatrix.push_back(tempSpec);

  // Then make 2D spectrum
  DataNames.push_back("DEvsPos1");
  is2D.push_back(true);
  hasGate.push_back(true);
  std::vector<int> row;
  row.resize(256,0);
  std::vector<std::vector<int>> tempSpec2D;
  for(int i=0; i<256; i++)
    tempSpec2D.push_back(row);
  //  tempSpec2D[0].resize(256,0);
  DataMatrix2D.push_back(tempSpec2D);

  // Make Gated spectra
  DataNames.push_back("DEvsPos1-gated");
  is2D.push_back(true);
  hasGate.push_back(false);
  tempSpec2D.clear();
  for(int i=0; i<256; i++)
    tempSpec2D.push_back(row);
  //  tempSpec2D[0].resize(256,0);
  DataMatrix2D.push_back(tempSpec2D);
  
  
  std::cout << "Made " << DataNames.size() << " empty spectra" << std::endl;
  
}

int EngeAnalyzer::connectMidasAnalyzer(){

  MidasAnalyzerModule mAMod;
  TARegisterModule tarm(&mAMod);

  mAMod.ConnectEngeAnalyzer(this);
  
  Py_BEGIN_ALLOW_THREADS
    manalyzer_main(0,0);
  Py_END_ALLOW_THREADS
    
  return 0;
}
void EngeAnalyzer::GenerateDataMatrix(int n)
//void EngeAnalyzer::GenerateDataMatrix(int n)
{
  int nbins = 4096;
  int nspec = 2;

  std::normal_distribution<double> distribution1(1500.0,200.0);
  std::normal_distribution<double> distribution2(2000.0,100.0);
  std::normal_distribution<double> distribution3(1700.0,200.0);
  std::normal_distribution<double> distribution4(3000.0,100.0);
  std::normal_distribution<double> crap(100,50); 


  Gate G1;
  if(GateCollection.size()>0){
    G1 = GateCollection[0];
  }
  
  // Fill the spectra
  std::vector<double> d1, d2;
  for(int i=0; i<n; i++){
    if(crap(generator)>120){
      ipeak1++;
      d1.push_back(distribution1(generator));
      d2.push_back(distribution2(generator));
    } else {
      ipeak2++;
      d1.push_back(distribution3(generator));
      d2.push_back(distribution4(generator));
    }
    DataMatrix[0][int(d1[i])]++;
    DataMatrix[1][int(d2[i])]++;
    DataMatrix2D[0][int(d1[i]/16.0)][int(d2[i]/16.0)]++;

    
    // The gated spectrum
    // Is the gate defined?
    if(G1.inGate(d1[i],d2[i])){
      igated++;
      DataMatrix2D[1][int(d1[i]/16.0)][int(d2[i]/16.0)]++;
      DataMatrix[2][int(d1[i])]++;
    }

  }
  /*
  // Test the gate
  G1.pnpoly(0,0);
  G1.pnpoly(1000,1000);
  G1.pnpoly(500,500);
  G1.pnpoly(1000,500);
  */
  /*  
  for(int i=0; i<50; i++){
    for(int j=0; j<50; j++){
      std::cout << DataMatrix2D[0][i][j] << " ";
    }
    std::cout << std::endl;
  }
  */

  /*
    Py_BEGIN_ALLOW_THREADS
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    Py_END_ALLOW_THREADS
  */
  
}

void EngeAnalyzer::putADC(uint32_t *dADC){

  //std::cout << "inserting data: " << dADC[0] << " " << dADC[1] << std::endl;
  
  DataMatrix[0][int(dADC[0])]++;
  DataMatrix[1][int(dADC[1])]++;
  DataMatrix2D[0][int(dADC[0]/16.0)][int(dADC[1]/16.0)]++;

  // The gated spectrum
  Gate G1;
  if(GateCollection.size()>0){
    G1 = GateCollection[0];
  }
  
  // Is the gate defined?
  if(G1.inGate(dADC[0],dADC[1])){
    igated++;
    DataMatrix2D[1][int(dADC[0]/16.0)][int(dADC[1]/16.0)]++;
    DataMatrix[2][int(dADC[0])]++;
  }
  
}

np::ndarray EngeAnalyzer::getData(){
  
  // Create the matrix to return to python
  u_int n_rows = DataMatrix.size();
  u_int n_cols = DataMatrix[0].size();
  p::tuple shape = p::make_tuple(n_rows, n_cols);
  p::tuple stride = p::make_tuple(sizeof(int));
  np::dtype dtype = np::dtype::get_builtin<int>();
  p::object own;
  np::ndarray converted = np::zeros(shape, dtype);
  
  for (u_int i = 0; i < n_rows; i++)
    {
      shape = p::make_tuple(n_cols);
      converted[i] = np::from_data(DataMatrix[i].data(), dtype, shape, stride, own);
      /*      int sum=0;
      for(int j=0; j<4096; j++){
	sum += DataMatrix[i][j];
      }
      std::cout << "Sum = " << sum << std::endl;
      */
    }

  return converted;
  
}

np::ndarray EngeAnalyzer::getData2D(){
  
  // Create the 3D matrix to return to python
  u_int n_t = DataMatrix2D.size();
  u_int n_rows = DataMatrix2D[0].size();
  u_int n_cols = DataMatrix2D[0][0].size();
  //std::cout << n_t << " " << n_rows << " " << n_cols << std::endl;
  p::tuple shape = p::make_tuple(n_t, n_rows, n_cols);
  p::tuple stride = p::make_tuple(sizeof(int));
  np::dtype dtype = np::dtype::get_builtin<int>();
  p::object own;
  np::ndarray converted = np::zeros(shape, dtype);

  for(u_int t = 0; t<n_t; t++){
    for (u_int i = 0; i < n_rows; i++)
      {
	shape = p::make_tuple(n_cols);
	converted[t][i] = np::from_data(DataMatrix2D[t][i].data(), dtype, shape, stride, own);
	/*      int sum=0;
		for(int j=0; j<4096; j++){
		sum += DataMatrix[i][j];
		}
		std::cout << "Sum = " << sum << std::endl;
      */
      }
  }

  return converted;
  
}


void EngeAnalyzer::putGate(char* name, p::list x, p::list y){

  //std::cout << "The name is " << name << std::endl;
  p::ssize_t len = p::len(x);
  // Make a vector for the gate
  Gate G1;//std::vector<std::vector<double>> Gate;
  for(int i=0; i<len; i++){
    std::vector<double> tmp;
    tmp.push_back(p::extract<double>(x[i]));
    tmp.push_back(p::extract<double>(y[i]));
    G1.addVertex(tmp);
  }

  GateCollection.push_back(G1);

  /*
  for(int i=0; i<Gate.size(); i++){
    std::cout << Gate[i][0] << " " << Gate[i][1] << std::endl;
  }
  */
  
}

void EngeAnalyzer::ClearData(){

  // Clear the 1D data
  std::vector<int> row;
  row.resize(4096,0);
  for(int k=0; k<DataMatrix.size(); k++)
    DataMatrix[k] = row;
  
  // Clear the 2D data
  row.resize(256,0);
  for(int k=0; k<DataMatrix2D.size(); k++){
    for(int i=0; i<256; i++)
      DataMatrix2D[k][i] = row;
  }

  ipeak1 = ipeak2 = igated = 0;
  
}

//----------------------------------------------------------------------
// Gates
// Add a vertex to the gate
void Gate::addVertex(std::vector<double> v){
  Points.push_back(v);

  // Set the rough bound of the gate
  if(Points.size() == 1){
    minx = v[0];
    maxx = v[0];
    miny = v[1];
    maxy = v[1];
  } else {
    if(v[0] > maxx)maxx=v[0];
    if(v[0] < minx)minx=v[0];
    if(v[1] > maxy)maxy=v[1];
    if(v[1] < miny)miny=v[1];
  }
  //std::cout << v[0] << " " << v[1] << std::endl;
}

bool Gate::inBound(double testx, double testy){

  bool inbound = false;
  if((testx < maxx) & (testx > minx) &
     (testy < maxy) & (testy > miny))inbound = true;

  return inbound;
}

int Gate::pnpoly(double testx, double testy)
{
  int nvert = Points.size();
  //std::cout << "(x,y) = (" << testx << "," << testy << ")" << std::endl;
  
  int i, j, c = 0;
  for (i = 0, j = nvert-1; i < nvert; j = i++) {
    if ( ((Points[i][1]>testy) != (Points[j][1]>testy)) &&
	 (testx < (Points[j][0]-Points[i][0]) * (testy-Points[i][1]) /
	  (Points[j][1]-Points[i][1]) + Points[i][0]) )
       c = !c;
  }
  //std::cout << "c=" << c << std::endl;
  return c;
}

// Main gate testing function - is (testx,testy) in the gate?
int Gate::inGate(double testx, double testy){

  int c = 0;
  // Is the gate defined?
  if(Points.size()>0){
    // Is the point in the rough gate?
    if(inBound(testx, testy))
      // Is it in the true gate?
      c = pnpoly(testx,testy);
  }
  return c;
}

/* 
   manalyzer module
*/
void MidasAnalyzerModule::Init(const std::vector<std::string> &args){

  printf("Initializing Midas Analyzer Module\n");
  fTotalEventCounter = 0;
}
void MidasAnalyzerModule::Finish(){
  printf("Finish!");
  printf("Counted %d events\n",fTotalEventCounter);
  std::cout << "number of spectra: " << eA.DataNames.size() << std::endl;
}
TARunInterface* MidasAnalyzerModule::NewRun(TARunInfo* runinfo){
  printf("NewRun, run %d, file %s\n",runinfo->fRunNo, runinfo->fFileName.c_str());
  return new MidasAnalyzerRun(runinfo, this);
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
}
TAFlowEvent* MidasAnalyzerRun::Analyze(TARunInfo* runinfo, TMEvent* event,
				    TAFlags* flags, TAFlowEvent* flow){
  //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo,
  //	 event->serial_number, (int)event->event_id, event->data_size);

  if(event->event_id != 1)
    return flow;

  // Get the ADC Bank
  TMBank* bADC = event->FindBank("ADC1");

  uint32_t* dADC = (uint32_t*)event->GetBankData(bADC);
  //std::cout << dADC[0] << "  " << dADC[1] << "  " << dADC[2] << "  " << dADC[3] << std::endl;
  
  fRunEventCounter++;
  fModule->fTotalEventCounter++;
  fModule->eA.putADC(dADC);

  return flow;

}

