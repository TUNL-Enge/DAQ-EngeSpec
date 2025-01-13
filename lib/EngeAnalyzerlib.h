//#ifndef EngeAnalyzerlib_H
//#define EngeAnalyzerlib_H
typedef std::vector<std::string> StringVector;

//----------------------------------------------------------------------
class Messages{

 public:

  Messages(){};

  std::string sayhello(std::string mesg);
  std::string saygoodbye();
  std::string saysomething(std::string mesg);

};

//----------------------------------------------------------------------
class Gate{
  
 public:
  
  Gate(std::string name);
  
  void addVertex(std::vector<double> v);
  std::vector<std::vector<double>> getPoints(){return Points;}

  // For 2D gates
  bool inBound(double x, double y);
  int pnpoly(double testx, double testy);
  int inGate(double x, double y);
  // For 1D gates
  int inGate(double x);

  void Print();
  void Clear();
  std::string getName(){return Name;}
  
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
  //void setNGates(int ng){nGates = ng;}
  std::string getName(){return Name;}
  int getnChannels(){return nChannels;}
  std::vector<int>& getData1D(){return Bins1D;}
  std::vector<std::vector<int>>& getData2D(){return Bins2D;}
  int getNGates(){return GateCollection.size();}
  void addGate(std::string gname);
  Gate& getGate(int i){return GateCollection[i];}

  void Clear();
  
 private:

  std::string Name;
  int nDims;
  int nChannels;
  //int nGates;
  std::vector<int> Bins1D;
  std::vector<std::vector<int>> Bins2D;
  std::vector<Gate> GateCollection;

};
std::vector<Histogram*> Histograms;
int nHist1D = 0;
int nHist2D = 0;


//----------------------------------------------------------------------
// SCALERS

// A class to hold scalers. When a scaler is added it should have an
// index to where in the scaler bank it should be grabbed from, a
// number that's incremented, and a name
class Scaler{

 public:

  Scaler(std::string name, int index);

  void inc(uint32_t*);
  void Print();
  std::string getName(){return Name;}
  int getIndex(){return Index;}
  int getValue(){return count;}
  void Clear();
  
 private:

  std::string Name;
  int Index;
  int count;

};

std::vector<Scaler*> Scalers;


//#endif
