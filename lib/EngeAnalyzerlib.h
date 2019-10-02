//#ifndef EngeAnalyzerlib_H
//#define EngeAnalyzerlib_H
typedef std::vector<std::string> StringVector;

//----------------------------------------------------------------------
class Messages{

 public:

  Messages(){};

  std::string sayhello(std::string mesg);
  std::string saygoodbye();

};

//----------------------------------------------------------------------
class Histogram{

 public:

  Histogram(std::string name, int nChannels, int dims);

  void inc(int);
  void inc(int,int);
  void Print(int, int);
  void Print(int, int, int, int);
  int getnDims(){return nDims;}

  
 private:

  std::string Name;
  int nDims;
  bool hasGate;
  std::vector<int> Bins1D;
  std::vector<std::vector<int>> Bins2D;

};
std::vector<Histogram> Histograms;
//----------------------------------------------------------------------
class Histogram1D{

 public:

  Histogram1D(std::string name, int nChannels);

  void inc(int);
  void Print(int, int);
  
 private:

  std::string Name;
  std::vector<int> Bins;

};
//----------------------------------------------------------------------
class Histogram2D{

 public:

  Histogram2D(std::string name, int nChannels);

  void inc(int, int);
  void Print(int, int, int, int);
  
 private:

  std::string Name;
  std::vector<std::vector<int>> Bins;
  double compress;                            // Compression factor for 2D histogram (unused)

};
//----------------------------------------------------------------------
/*
class Data{

 public:

  // Clear all data
  void ClearData();
  // Make a data stream
  int Histogram1D(std::string, int);
  // Print all data (poorly formatted)
  void PrintData();

 private:
  
  StringVector DataNames;
  std::vector<std::vector<int>> DataMatrix;

};
*/

//----------------------------------------------------------------------
class Gate{
  
 public:
  
  Gate(){};
  
  void addVertex(std::vector<double> v);
  std::vector<std::vector<double>> getPoints(){return Points;}
  bool inBound(double x, double y);
  int pnpoly(double testx, double testy);
  int inGate(double x, double y);
  
 private:
  
  std::vector<std::vector<double>> Points;
  double minx, maxx, miny, maxy;
  
};




//#endif
