## All of the stuff for opening spectra
from PySide2.QtWidgets import QApplication, QFileDialog
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

from SpectrumHandlers import *

## Import my own c++ library!
import libMakeData

class SpectrumCollection:
    def __init__(self, num):
        self.num = num

        self.spec1d = [SpectrumObject(0)]
        self.spec1d[0].initialize()
        self.spec2d = [SpectrumObject2D(0)]
        self.Name = "Test Collection of Spectra"

        ## Load the data library
        self.dm = libMakeData.DataMaker()

        self.isRunning = False
        
    def __str__(self):
        return 'Spectrum Collection Name: {}'.format(self.Name)

    ## Print some summary details
    def printSummary(self):
        print(self)
        ## 1D spectra
        l = len(self.spec1d)
        print("There are",l,"1D spectra:")
        for i in range(l):
            sObj = self.spec1d[i]
            print(sObj)

        ## 2D spectra
        l = len(self.spec2d)
        print("There are",l,"2D spectra:")
        for i in range(l):
            sObj = self.spec2d[i]
            print(sObj)

        ##plt.plot(range(1,4096),self.spec1d[1].spec)
        ##plt.show()

    def addSpectrum(self,name):
        ## Make a new 1D Histogram
        sObj = SpectrumObject(len(self.spec1d))
        sObj.Name = name
        self.spec1d.append(sObj)

        
    ## Load raw events from an HDF file
    def LoadHDFData(self):
        filename = QFileDialog.getOpenFileName(None,
                                               "Open Data Collection", "./",
                                               "Data Files (*.hdf)")
        print("Loading: ",filename[0])
        if filename != '':
            self.df = pd.read_hdf(filename[0])

            self.Sort()
            ##self.spec2d, self.xedges, self.yedges = np.histogram2d(
            ##    x=df.loc[:,"Pos1"],
            ##    y=df.loc[:,"DE"],
            ##    bins=self.nx)
            ## first delete current sets of spectra

    def Sort(self):
        ## First delete the old spectra
        self.spec1d = []
        self.spec2d = []
        ## For each column in df, make a 1D spectrum
        for i in range(len(list(self.df))):
            sObj = SpectrumObject(i)
            sObj.Name = list(self.df)[i]
            h, edges = np.histogram(self.df.loc[:,sObj.Name],bins=range(0,4097))
            sObj.spec = h
            sObj.spec_temp[:] = h
            self.spec1d.append(sObj)

        ## Make a 2D spectrum
        sObj = SpectrumObject2D(0)
        sObj.Name = list(self.df)[0] + "vs" + list(self.df)[1]
        sObj.spec2d, sObj.xedges, sObj.yedges = np.histogram2d(
            x=self.df.loc[:,list(self.df)[0]],
            y=self.df.loc[:,list(self.df)[1]],
            bins=sObj.nx)
        sObj.spec2d_temp[:] = sObj.spec2d
        self.spec2d.append(sObj)

        sObj = self.spec2d[0]
        print("is there a gate? ",sObj.gate)
        if sObj.gate:
            ## Make the cut spectra
            self.addSpectrum("Cut Spectrum")
            ## test
            x = 0
            y = 0
            print("Position x:",x,"y:",y,"In gate?",inGate(x,y,sObj.gate))
        

        
    ## Is a count at x,y in the Gate G?
    def inGate(x,y,G):
        rough = x>min(G[:,1]) & x<max(G[:,1]) 
        return rough
        
    ## Start a simulation of data
    def startsim(self):
        print(self.dm.sayhello())
        self.isRunning = True
        while 1:
            self.dm.GenerateData(500)

    def stopsim(self):
        print(self.dm.saygoodbye())
        self.isRunning = False
        
## Run this if this file is run alone for debugging purposes            
if __name__ == '__main__':
    import matplotlib.pyplot as plt
    SpecColl = SpectrumCollection(0)

    app = QApplication([])
    SpecColl.LoadHDFData()
    SpecColl.printSummary()
