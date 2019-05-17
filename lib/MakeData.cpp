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

void DataMaker::Initialize(){

  // Set the names of the data
  DataNames.clear();
  DataNames.push_back("Pos1");
  is2D.push_back(false);
  DataNames.push_back("DE");
  is2D.push_back(false);
  
  // Fill some empty spectra
  std::vector<int> tempSpec;
  tempSpec.resize(4096,0);
  DataMatrix.clear();
  for(int i=0; i<DataNames.size(); i++)
    DataMatrix.push_back(tempSpec);

  // Then make 2D spectrum
  DataNames.push_back("DEvsPos1");
  is2D.push_back(true);
  std::vector<int> row;
  row.resize(256,0);
  std::vector<std::vector<int>> tempSpec2D;
  for(int i=0; i<256; i++)
    tempSpec2D.push_back(row);
  //  tempSpec2D[0].resize(256,0);
  DataMatrix2D.push_back(tempSpec2D);

  
  std::cout << "Made " << DataNames.size() << " empty spectra" << std::endl;
  
}

void DataMaker::GenerateDataMatrix(int n)
//void DataMaker::GenerateDataMatrix(int n)
{
  int nbins = 4096;
  int nspec = 2;

  std::normal_distribution<double> distribution1(1500.0,200.0);
  std::normal_distribution<double> distribution2(2000.0,100.0);
  std::normal_distribution<double> distribution3(1700.0,200.0);
  std::normal_distribution<double> distribution4(3000.0,100.0);
  std::normal_distribution<double> crap(100,50); 

  /*
  std::cout << "Name1 = " << DataNames[0] << std::endl;
  std::cout << "Name2 = " << DataNames[1] << std::endl;
  */

  // Fill the spectra
  std::vector<double> d1, d2;
  for(int i=0; i<n; i++){
    if(crap(generator)>120){
      d1.push_back(distribution1(generator));
      d2.push_back(distribution2(generator));
    } else {
      d1.push_back(distribution3(generator));
      d2.push_back(distribution4(generator));
    }
    DataMatrix[0][int(d1[i])]++;
    DataMatrix[1][int(d2[i])]++;
    DataMatrix2D[0][int(d1[i]/16.0)][int(d2[i]/16.0)]++;
  }

  /*
  for(int i=0; i<50; i++){
    for(int j=0; j<50; j++){
      std::cout << DataMatrix2D[0][i][j] << " ";
    }
    std::cout << std::endl;
  }
  */
  
}

np::ndarray DataMaker::getData(){
  
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

np::ndarray DataMaker::getData2D(){
  
  // Create the 3D matrix to return to python
  u_int n_t = DataMatrix2D.size();
  u_int n_rows = DataMatrix2D[0].size();
  u_int n_cols = DataMatrix2D[0][0].size();
  //std::cout << n_t << " " << n_rows << " " << n_cols << std::endl;
  p::tuple shape = p::make_tuple(n_t, n_rows, n_cols);
  p::tuple stride = p::make_tuple(sizeof(int));
  np::dtype dtype = np::dtype::get_builtin<int>();
  p::object own;
  np::ndarray converted = np::zeros(shape, dtype);

  for(u_int t = 0; t<n_t; t++){
    for (u_int i = 0; i < n_rows; i++)
      {
	shape = p::make_tuple(n_cols);
	converted[t][i] = np::from_data(DataMatrix2D[t][i].data(), dtype, shape, stride, own);
	/*      int sum=0;
		for(int j=0; j<4096; j++){
		sum += DataMatrix[i][j];
		}
		std::cout << "Sum = " << sum << std::endl;
      */
      }
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
