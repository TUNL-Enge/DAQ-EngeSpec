
#include <vector>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "EngeAnalyzerlib.h"

//----------------------------------------------------------------------
// HELLO AND GOODBYE MESSAGES
std::string Messages::sayhello(std::string mesg){

  std::string s = "Hello! This is " + mesg + " sort routine running in c++!!!";
  return s;
}

std::string Messages::saygoodbye( ) {
    
  return "Goodbye! I hope I served you well";
}

std::string Messages::saysomething(std::string mesg){
  std::string s = "EngeSort: " + mesg;
  return s;
}

//----------------------------------------------------------------------
// HISTOGRAMS

// Generic Histograms
Histogram::Histogram(std::string name, int nchannels, int dims){
  Name = name;
  nDims = dims;
  nChannels = nchannels;
  
  // Fill an empty spectrum
  switch(dims){
  case 1:
    Bins1D.resize(nChannels);
    nHist1D++;
    break;
  case 2:
    std::vector<int> row;
    row.resize(nChannels,0);
    for(int i=0; i<nChannels; i++)
      Bins2D.push_back(row);
    nHist2D++;
    break;
    //  default:
    //std::cout << "You have the wrong number of dimensions! " << dims << std::endl;
  }
  
  //  std::cout << "Made histogram: " << Name << " with " << nChannels << " channels" << std::endl;

  Histograms.push_back(this);
}
void Histogram::inc(int c){
  if(c>0 && c<nChannels)
    Bins1D[c]++;
}
void Histogram::inc(int cx, int cy){
  if(cx > 0 && cy > 0 && cx<nChannels && cy<nChannels)
    Bins2D[cx][cy]++;
}
void Histogram::Print(int minBin=0, int maxBin=10){

  std::cout << "Histogram: " << Name << std::endl;
  for(int i=minBin; i<maxBin; i++)
    std::cout << Bins1D[i] << " ";
  std::cout << std::endl;

}
void Histogram::Print(int minBinx=0, int maxBinx=10, int minBiny=0, int maxBiny=0){

  std::cout << "Histogram: " << Name << std::endl;
  for(int i=minBinx; i<maxBinx; i++){
    for(int j=minBiny; j<maxBiny; j++){
      std::cout << Bins2D[i][j] << " ";
    }
    std::cout << std::endl;
  }

}

void Histogram::addGate(std::string gname){

  //std::cout << "Adding gate: " << gname << " to histogram " << Name << std::endl;
  Gate G(gname);
  GateCollection.push_back(G);
  //std::cout << GateCollection.size() << "   " << nGates << std::endl;
}

// Clear the histogram
void Histogram::Clear(){

  //  std::cout << "Clearing histogram: " << Name << std::endl;
  if(nDims == 1){
    std::fill(Bins1D.begin(), Bins1D.end(),0);
  } else {
    for(auto& elem : Bins2D)std::fill(elem.begin(), elem.end(), 0);
  }
}

//----------------------------------------------------------------------
// GATES

Gate::Gate(std::string name){

  Name = name;
  //std::cout << "Made a gate called " << name << std::endl;

}

void Gate::Print(){

  std::cout << "This is the gate called " << Name << std::endl;
  std::cout << "Number of vertices: " << Points.size() << std::endl;
  
}

void Gate::Clear(){

  Points.clear();
  
}

// Add a vertex to the gate
void Gate::addVertex(std::vector<double> v){

  //  std::cout << "Adding vertex: " << v[0] << ", " << v[1] << std::endl;
  
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

// Main gate testing function for 1D gates
int Gate::inGate(double testx){

  int c = 0;
  // Is the gate defined?
  if(Points.size()>0){
    if(testx > minx && testx < maxx)
      c=1;
  }
  return c;
}

//----------------------------------------------------------------------
// SCALERS

// Make a scaler
Scaler::Scaler(std::string name, int index){

  Name = name;
  Index = index;
  count = 0;
  //std::cout << "Made a scaler called " << name << " with index: " << index << std::endl;

  Scalers.push_back(this);
  
}

// Increment a scaler
void Scaler::inc(uint32_t *scal){
  //std::cout << "incrementing " << Name << " at index " << Index << " by " << scal[Index] << std::endl;
  count = count+scal[Index];
}

// Print the Scaler
void Scaler::Print(){
  std::cout << "Scaler: " << Name << " is mapped to index " << Index <<
    " and has count: " << count << std::endl;
}

// Clear the scaler
void Scaler::Clear(){
  count = 0;
}
