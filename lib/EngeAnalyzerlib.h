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
class Histogram1D{

 public:

  Histogram1D(std::string name, int nChannels);

  void Print(int, int);
  
 private:

  std::string Name;
  std::vector<int> Bins;
};

//----------------------------------------------------------------------
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
