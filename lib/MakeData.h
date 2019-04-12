#ifndef MAKEDATA_H
#define MAKEDATA_H

#include <vector>

class DataMaker {
 public:

  DataMaker() {}

  char const* sayhello( );
  void GenerateData(int n = 100);
  void ClearData();
  void SimulateData();
  void PrintData();

  std::vector<int>::iterator begin(){
    return Dat.begin();
  }
  std::vector<int>::iterator end(){
    return Dat.end();
  }
  
 private:
  std::vector<int> Dat;
};

#endif // MAKEDATA_H
