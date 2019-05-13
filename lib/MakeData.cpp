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

// Test making a matrix
mat DataMaker::test(){
  int n = 1e4;
  mat result(n, vec(n, 0.));
  return result;
}
//np::ndarray DataMaker::GenerateDataMatrix(int n)
void DataMaker::GenerateDataMatrix(int n)
{
  int nbins = 4096;
  int nspec = 2;

  std::normal_distribution<double> distribution1(500.0,50.0);
  std::normal_distribution<double> distribution2(200.0,10.0);

  std::vector<int> tempSpec;
  tempSpec.resize(4096,0);
  for(int i=0; i<n; i++){
    double d1 = distribution1(generator);
    tempSpec[int(d1)]++;
  }
  DataMatrix.push_back(tempSpec);

  tempSpec.resize(4096,0);
  for(int i=0; i<n; i++){
    double d2 = distribution2(generator);
    tempSpec[int(d2)]++;
  }
  DataMatrix.push_back(tempSpec);

  //  PrintData();
  
  /*
  u_int n_rows = input.shape()[0];
  u_int n_cols = input.shape()[1];
  p::tuple shape = p::make_tuple(n_rows, n_cols);
  p::tuple strides = p::make_tuple(input.strides()[0]*sizeof(double),
				   input.strides()[1]*sizeof(double));
  np::dtype dtype = np::dtype::get_builtin<double>();
  p::object own;
  np::ndarray converted = np::from_data(input.data(), dtype, shape, strides, own);
  return converted;
  */
}
/*
np::ndarray DataMaker::convert_to_numpy(mat const & input)
{
    u_int n_rows = input.size();
    u_int n_cols = input[0].size();
    p::tuple shape = p::make_tuple(n_rows, n_cols);
    np::dtype dtype = np::dtype::get_builtin<double>();
    np::ndarray converted = np::zeros(shape, dtype);

    for (u_int i = 0; i < n_rows; i++)
    {
        for (u_int j = 0; j < n_cols; j++)
        {
            converted[i][j] = input[i][j];
        }
    }
    return converted;
}
*/
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
