#include <iostream>
#include <vector>

#include "MakeData.h"

char const* DataMaker::sayhello( ) {
    return "Hello! This is the data maker running in c++!!!";
}

void DataMaker::GenerateData(int n){

  for(int i=0; i<n; i++){
    Dat.push_back(500);
  }
  
}

void DataMaker::PrintData(){
  for(std::vector<int>::iterator it = Dat.begin(); it != Dat.end(); ++it)
    std::cout << " " << *it;
  std::cout << "\n";
}

void DataMaker::ClearData(){
  Dat.clear();
}

// Use to simulate data slowly coming in. Need some kind of delay, but
// also this should run in a different thread so it doesn't use all of
// the memory
void DataMaker::SimulateData(){

}
