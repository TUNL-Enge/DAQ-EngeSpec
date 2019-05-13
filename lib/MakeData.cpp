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

np::ndarray DataMaker::GenerateDataMatrix(int n)
//void DataMaker::GenerateDataMatrix(int n)
{
  int nbins = 4096;
  int nspec = 2;

  std::normal_distribution<double> distribution1(500.0,50.0);
  std::normal_distribution<double> distribution2(200.0,10.0);

  // Set the names of the data
  DataNames.push_back("Pos1");
  DataNames.push_back("DE");

  /*
  std::cout << "Name1 = " << DataNames[0] << std::endl;
  std::cout << "Name2 = " << DataNames[1] << std::endl;
  */
  
  std::vector<int> tempSpec;
  tempSpec.resize(4096,0);
  for(int i=0; i<n; i++){
    double d1 = distribution1(generator);
    tempSpec[int(d1)]++;
  }
  DataMatrix.push_back(tempSpec);

  tempSpec.clear();
  tempSpec.resize(4096,0);
  for(int i=0; i<n; i++){
    double d2 = distribution2(generator);
    tempSpec[int(d2)]++;
  }
  DataMatrix.push_back(tempSpec);

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

  std::cout << "The data matrix is:\n";
  std::vector< std::vector<int> >::iterator itrow;
  std::vector<int>::iterator itcol;
  for(itrow = DataMatrix.begin(); itrow != DataMatrix.end(); ++itrow){
    for(itcol = itrow->begin(); itcol != itrow->end(); ++itcol)
      std::cout << " " << *itcol;
    std::cout << std::endl;
  }
}

void DataMaker::ClearData(){
  Dat[0].clear();
  Dat[1].clear();
  DataMatrix.clear();
}

// Use to simulate data slowly coming in. Need some kind of delay, but
// also this should run in a different thread so it doesn't use all of
// the memory
void DataMaker::SimulateData(){

  while(isRunning){
    GenerateData(500);
    //sleep(100);
  }
  
}
