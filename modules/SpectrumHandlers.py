## All of the stuff for opening spectra
from PySide2.QtWidgets import QFileDialog
import numpy as np
import pandas as pd
import csv
import matplotlib as plt
import os.path

## Import my own c++ library!
if os.path.exists('EngeSort.so'):
    import EngeSort

class SpectrumObject:
    def __init__(self, num):
        self.num = num
        self.isSpectrum = True
        self.isVisible = True
        self.is2D = False

        self.NBins = 2**12
        
        self.spec = np.zeros(self.NBins)
        self.spec_temp = np.zeros(self.NBins)   ## The temporary spectrum in memory
        self.Name = "Test Spectrum"

        self.xzoom = [0,self.NBins]
        self.yzoom = [0,1]

        ## Load the data library
        ##if os.path.exists('EngeSort.so'):
        ##    self.dm = EngeSort.EngeSort()

        self.gate = None
        self.hasGate = False

        
    ## Load ascii file
    def LoadASCIIData(self):
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
            self.NBins = len(TempHistogramArray)
            self.spec = TempHistogramArray
            self.spec_temp = self.spec
            self.spec_temp[:] = self.spec
            self.xzoom = [0,self.NBins]

    ## Save ascii file
    def SaveASCIIData(self):
        print("Save ASCII Data!")
        filename = QFileDialog.getSaveFileName(None,
                                               "Save Spectrum", "./",
                                               "Spectrum Files (*.dat)")
        print("Saving: ",filename[0])
        if filename != '':
            with open(filename[0], "w") as ofile:
                writer = csv.writer(ofile, delimiter='\t')
                writer.writerows(zip(range(len(self.spec)),self.spec))
                #for bin in range(len(self.spec)):
                #ofile.write([bin,self.spec[bin]])
            #ofile.close()

    ## Initial sine-wave histogram
    def initialize(self):
        t = np.arange(0.0, self.NBins, 1)
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

        self.xzoom = [1,256]
        self.yzoom = [1,256]

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
