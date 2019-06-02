## All of the stuff for opening spectra
from PySide2.QtWidgets import QFileDialog
import numpy as np
import pandas as pd
import matplotlib as plt

## Import my own c++ library!
import libMakeData

class SpectrumObject:
    def __init__(self, num):
        self.num = num
        self.isSpectrum = True
        self.isVisible = True
        self.is2D = False

        self.spec = np.zeros(4096)
        self.spec_temp = np.zeros(4096)   ## The temporary spectrum in memory
        self.Name = "Test Spectrum"

        self.xzoom = [0,4096]
        self.yzoom = [0,1]

        ## Load the data library
        self.dm = libMakeData.DataMaker()
        
    ## Load ascii file
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
            self.spec = TempHistogramArray
            self.spec_temp[:] = self.spec

    ## Initial sine-wave histogram
    def initialize(self):
        t = np.arange(0.0, 4096, 1)
        s = 200+100*np.sin(2*np.pi*t/1000.0)
        self.spec = s
        self.spec_temp[:] = self.spec
        
        
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

        self.xzoom = [0,4096]
        self.yzoom = [0,4096]

        self.gate = None
        self.hasGate = False
                      
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
