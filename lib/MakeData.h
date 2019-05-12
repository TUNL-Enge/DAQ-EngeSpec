#ifndef MAKEDATA_H
#define MAKEDATA_H

#include <vector>
#include <random>

class DataMaker {
 public:

  DataMaker() {}

  char const* sayhello( );
  void GenerateData(int n = 1000);
  void ClearData();
  void SimulateData();
  void PrintData();

  std::vector<int>::iterator begin(){
    return Dat[0].begin();
  }
  std::vector<int>::iterator end(){
    return Dat[0].end();
  }
  
 private:
  std::vector<int> Dat[2];
  std::default_random_engine generator;
  //  std::normal_distribution<double> distribution1(500.0,50.0);
  //  std::normal_distribution<double> distribution2(200.0,10.0);
};

#endif // MAKEDATA_H
