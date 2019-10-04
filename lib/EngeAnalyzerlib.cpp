
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

//----------------------------------------------------------------------
// HISTOGRAMS

// Generic Histograms
Histogram::Histogram(std::string name, int nchannels, int dims){
  Name = name;
  nDims = dims;
  hasGate = false;
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
  
  std::cout << "Made histogram: " << Name << " with " << nChannels << " channels" << std::endl;

  Histograms.push_back(this);
}
void Histogram::inc(int c){
  Bins1D[c]++;
}
void Histogram::inc(int cx, int cy){
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

void Histogram::addGate(std::string name){

  std::cout << "Adding gate: " << name << " to histogram " << Name << std::endl;
  Gate G1(name);
  GateCollection.push_back(G1);
  hasGate = true;
}


//----------------------------------------------------------------------
// GATES

Gate::Gate(std::string name){

  Name = name;
  std::cout << "Made a gate called " << name << std::endl;

}

void Gate::Print(){

  std::cout << "This is the gate called " << Name << std::endl;
  std::cout << "Number of vertices: " << Points.size() << std::endl;
  
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
