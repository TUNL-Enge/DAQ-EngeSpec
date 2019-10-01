
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
Histogram1D::Histogram1D(std::string name, int nChannels){

  Name = name;
  
  // Fill an empty spectrum
  Bins.resize(nChannels);

  std::cout << "Make histogram: " << Name << " with " << nChannels << " channels" << std::endl;
}
  
void Histogram1D::Print(int minBin=0, int maxBin=10){

  std::cout << "Histogram: " << Name << std::endl;
  for(int i=minBin; i<maxBin; i++)
    std::cout << Bins[i] << " ";
  std::cout << std::endl;

}

//----------------------------------------------------------------------
// DATA STREAMS

// Clear the data
void Data::ClearData(){
  DataNames.clear();
  DataMatrix.clear();

  std::cout << "Length of DataNames = " << DataNames.size() << std::endl;

}

// Define a data channel
int Data::Histogram1D(std::string name, int index){

  DataNames.push_back(name);
  
  // Fill an empty spectrum
  std::vector<int> tempSpec;
  tempSpec.resize(4096,0);
  DataMatrix.push_back(tempSpec);
  
}

// Print details
void Data::PrintData(){

  for(int i=0; i<DataNames.size(); i++){
    std::cout << "Name: " << DataNames[i] << std::endl;
    for(int j=0; j<10; j++){
      std::cout << DataMatrix[i][j] << "  ";
    }
    std::cout << std::endl;
  }
  
}

//----------------------------------------------------------------------
// GATES

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
