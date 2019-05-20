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

  // Make Gated spectra
  DataNames.push_back("DEvsPos1-gated");
  is2D.push_back(true);
  tempSpec2D.clear();
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

  // do rough gate
  /*  double xmax,xmin;
  if(GateCollection.size()>0){
    Gate G1 = GateCollection[0];
    xmin=G21[0][0];
    xmax=G1[1][0];
    //std::cout << "xmin, xmax = " << xmin << " " << xmax << std::endl;
  }
  */

  Gate G1;
  if(GateCollection.size()>0){
    G1 = GateCollection[0];
  }
  
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

    // The gated spectrum
    if(G1.getPoints().size()>0)
      if(G1.inBound(d1[i],d2[i]))//d1[i] > xmin & d1[i] < xmax)
	DataMatrix2D[1][int(d1[i]/16.0)][int(d2[i]/16.0)]++;
    
  }

  /*
  // Test the gate
  G1.inBound(0,0);
  G1.inBound(1000,1000);
  G1.inBound(500,500);
  G1.inBound(1000,500);
  */
  
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


void DataMaker::putGate(char* name, p::list x, p::list y){

  //std::cout << "The name is " << name << std::endl;
  p::ssize_t len = p::len(x);
  // Make a vector for the gate
  Gate G1;//std::vector<std::vector<double>> Gate;
  for(int i=0; i<len; i++){
    std::vector<double> tmp;
    tmp.push_back(p::extract<double>(x[i]));
    tmp.push_back(p::extract<double>(y[i]));
    G1.addVertex(tmp);
  }

  GateCollection.push_back(G1);

  /*
  for(int i=0; i<Gate.size(); i++){
    std::cout << Gate[i][0] << " " << Gate[i][1] << std::endl;
  }
  */
  
}

void DataMaker::ClearData(){

  // Clear the 1D data
  std::vector<int> row;
  row.resize(4096,0);
  for(int k=0; k<DataMatrix.size(); k++)
    DataMatrix[k] = row;
  
  // Clear the 2D data
  row.resize(256,0);
  for(int k=0; k<DataMatrix2D.size(); k++){
    for(int i=0; i<256; i++)
      DataMatrix2D[k][i] = row;
  }

}

//----------------------------------------------------------------------
// Gates
// Add a vertex to the gate
void Gate::addVertex(std::vector<double> v){
  Points.push_back(v);
  //std::cout << v[0] << " " << v[1] << std::endl;
}

bool Gate::inBound(double x, double y){

  // Find max and min x and y of gate
  double maxx = Points[0][0];
  double minx = Points[0][0];
  double maxy = Points[0][1];
  double miny = Points[0][1];

  for(int i=1; i<Points.size(); i++){
    if(Points[i][0] > maxx)maxx=Points[i][0];
    if(Points[i][0] < minx)minx=Points[i][0];
    if(Points[i][1] > maxy)maxy=Points[i][1];
    if(Points[i][1] < miny)miny=Points[i][1];
  }

  /*
  std::cout << "minx = " << minx << std::endl;
  std::cout << "maxx = " << maxx << std::endl;
  std::cout << "miny = " << miny << std::endl;
  std::cout << "maxy = " << maxy << std::endl;
  */
  bool inbound = false;
  if((x < maxx) & (x > minx) & (y < maxy) & (y > miny))inbound = true;

  /*
  std::cout << "The point at (x,y) = (" << x << "," << y << ") is ";
  if(inbound)
    std::cout << "INSIDE!" << std::endl;
  else
    std::cout << "outside the boundary" << std::endl;
  */
  
  return inbound;
}
