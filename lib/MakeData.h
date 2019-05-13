#ifndef MAKEDATA_H
#define MAKEDATA_H

#include <vector>
#include <random>

class DataMaker {
 public:

  DataMaker() {}

  char const* sayhello( );
  char const* saygoodbye( );
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
  bool isRunning = false;
  std::vector<int> Dat[2];
  std::default_random_engine generator;
};

#endif // MAKEDATA_H
