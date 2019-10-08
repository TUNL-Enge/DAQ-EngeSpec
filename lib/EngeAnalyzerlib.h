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
class Gate{
  
 public:
  
  Gate(std::string name);
  
  void addVertex(std::vector<double> v);
  std::vector<std::vector<double>> getPoints(){return Points;}
  bool inBound(double x, double y);
  int pnpoly(double testx, double testy);
  int inGate(double x, double y);
  void Print();
  
 private:

  std::string Name;
  std::vector<std::vector<double>> Points;
  double minx, maxx, miny, maxy;
  
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
  bool gethasGate(){return hasGate;}
  void sethasGate(bool tf){hasGate = tf;}
  std::string getName(){return Name;}
  int getnChannels(){return nChannels;}
  std::vector<int> getData1D(){return Bins1D;}
  std::vector<std::vector<int>> getData2D(){return Bins2D;}
  void addGate(std::string name);
  Gate* getGates(int i){return &GateCollection[i];}
  void Clear();
  
 private:

  std::string Name;
  int nDims;
  int nChannels;
  bool hasGate;
  std::vector<int> Bins1D;
  std::vector<std::vector<int>> Bins2D;
  std::vector<Gate> GateCollection;

};
std::vector<Histogram*> Histograms;
int nHist1D = 0;
int nHist2D = 0;





//#endif
