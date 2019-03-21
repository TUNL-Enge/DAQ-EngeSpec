## All of the stuff for opening spectra
from PySide2.QtWidgets import QFileDialog
import numpy as np
import pandas as pd
import matplotlib as plt

class SpectrumObject:
    def __init__(self, num):
        self.num = num
        self.isSpectrum = True
        self.isVisible = True
        self.is2D = False

        self.spec = np.zeros(4096)
        self.spec_temp = np.zeros(4096)   ## The temporary spectrum in memory
        self.Name = "Test Spectrum"

    def LoadData(self):
        filename = QFileDialog.getOpenFileName(None,
                                               "Open Spectrum", "./",
                                               "Spectrum Files (*.dat)")
        print("Loading: ",filename[0])
        if filename != '':
            TempHistogramArray=[]
            input = open(filename[0],"r")
            for line in input:
                Bin,Content=line.split()
                TempHistogramArray.append( float(Content) ) 
            input.close()
                ##return TempHistogramArray
                ##
            self.spec = TempHistogramArray
            self.spec_temp[:] = self.spec
            ##StrVar.set(filename.split("/")[-1])
            ##self.mpcanvas.PlotData()

    def initialize(self):
        t = np.arange(0.0, 4096, 1)
        s = 200+100*np.sin(2*np.pi*t/1000.0)
        self.spec = s
        self.spec_temp[:] = self.spec
        
    ##def getSpec(self):
    ##    return self.spec

    def simulate(self,mu,sig):
        samp = int(np.random.normal(loc=mu,scale=sig))
        ##print(samp,self.spec[samp],self.spec_temp[samp])
        self.spec_temp[samp] += 1
        ##print(samp,self.spec[samp],self.spec_temp[samp])

    def simulate_a_peak(self):
        NCounts = 1000
        mu = 4000
        sig = 2
        for i in range(0,NCounts):
            self.simulate(mu,sig)
        ##print(max(self.spec[3900:4100]))
        ##print(max(self.spec_temp[3900:4100]))
    
    def __str__(self):
        return '1D Spectrum Name: {}'.format(self.Name)


class SpectrumObject2D:
    def __init__(self, num):
        self.num = num
        self.isSpectrum = True
        self.isVisible = True
        self.is2D = True

        self.nx = 256
        self.ny = 256
        self.xedges = None
        self.yedges = None
        self.spec2d = np.zeros(shape=(self.nx,self.ny))
        self.spec2d_temp = np.zeros(shape=(self.nx,self.ny))   ## The temporary spectrum in memory
        self.Name = "2D Test Spectrum"

        self.gate = None
        
    ## Load already-made spectrum data
    def LoadData(self):
        filename = QFileDialog.getOpenFileName(None,
                                               "Open Spectrum", "./",
                                               "Spectrum Files (*.dat)")
        print("Loading: ",filename[0])
        if filename != '':
            TempHistogramArray=[]
            input = open(filename[0],"r")
            for line in input:
                Bin,Content=line.split()
                TempHistogramArray.append( float(Content) ) 
            input.close()
                ##return TempHistogramArray
                ##
            self.spec = TempHistogramArray
            self.spec_temp[:] = self.spec
            ##StrVar.set(filename.split("/")[-1])
            ##self.mpcanvas.PlotData()

    ## Load raw events from an HDF file
    def LoadHDFData(self):
        filename = QFileDialog.getOpenFileName(None,
                                               "Open Spectrum", "./",
                                               "Spectrum Files (*.dat, *.hdf)")
        print("Loading: ",filename[0])
        if filename != '':
            df = pd.read_hdf(filename[0])
            self.spec2d, self.xedges, self.yedges = np.histogram2d(
                x=df.loc[:,"Pos1"],
                y=df.loc[:,"DE"],
                bins=self.nx)
            #print(self.hist2d)
          
    def __str__(self):
        return '2D Spectrum Name: {}'.format(self.Name)

            
if __name__ == '__main__':
    #Spec = SpectrumObject()
    #print(Spec)
    #Spec.initialize()
    #print(Spec.spec)
    Spec2D = SpectrumObject2D(0)
    print(Spec2D)
    Spec2D.LoadHDFData()
