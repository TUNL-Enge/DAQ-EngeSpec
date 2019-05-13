#include <iostream>
#include <vector>
#include <random>

#include "MakeData.h"

char const* DataMaker::sayhello( ) {
    return "Hello! This is the data maker running in c++!!!";
}
char const* DataMaker::saygoodbye( ) {
    return "Goodbye! I hope I served you well";
}

void DataMaker::GenerateData(int n){

  std::normal_distribution<double> distribution1(500.0,50.0);
  std::normal_distribution<double> distribution2(200.0,10.0);
  
  for(int i=0; i<n; i++){
    double d1 = distribution1(generator);
    double d2 = distribution2(generator);
    Dat[0].push_back(int(d1));
    Dat[1].push_back(int(d2));
  }
  
}

void DataMaker::PrintData(){
  for(std::vector<int>::iterator it = Dat[0].begin(); it != Dat[0].end(); ++it)
    std::cout << " " << *it ;
  std::cout << "\n";
}

void DataMaker::ClearData(){
  Dat[0].clear();
  Dat[1].clear();
}

// Use to simulate data slowly coming in. Need some kind of delay, but
// also this should run in a different thread so it doesn't use all of
// the memory
void DataMaker::SimulateData(){

}
