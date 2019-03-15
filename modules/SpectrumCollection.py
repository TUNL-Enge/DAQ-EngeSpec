## All of the stuff for opening spectra
from PySide2.QtWidgets import QApplication, QFileDialog
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

from SpectrumHandlers import *

class SpectrumCollection:
    def __init__(self, num):
        self.num = num

        self.spec1d = [SpectrumObject(0)]
        self.spec1d[0].initialize()
        self.spec2d = [SpectrumObject2D(0)]
        self.Name = "Test Collection of Spectra"

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

    ## Load raw events from an HDF file
    def LoadHDFData(self):
        filename = QFileDialog.getOpenFileName(None,
                                               "Open Data Collection", "./",
                                               "Data Files (*.hdf)")
        print("Loading: ",filename[0])
        if filename != '':
            df = pd.read_hdf(filename[0])
            ##self.spec2d, self.xedges, self.yedges = np.histogram2d(
            ##    x=df.loc[:,"Pos1"],
            ##    y=df.loc[:,"DE"],
            ##    bins=self.nx)
            ## first delete current sets of spectra
            self.spec1d = []
            self.spec2d = []
            ## For each column in df, make a 1D spectrum
            for i in range(len(list(df))):
                sObj = SpectrumObject(i)
                sObj.Name = list(df)[i]
                h, edges = np.histogram(df.loc[:,sObj.Name],bins=range(0,4096))
                sObj.spec = h
                self.spec1d.append(sObj)

            ## Make a 2D spectrum
            sObj = SpectrumObject2D(0)
            sObj.Name = list(df)[0] + "vs" + list(df)[1]
            sObj.spec2d, sObj.xedges, sObj.yedges = np.histogram2d(
                x=df.loc[:,list(df)[0]],
                y=df.loc[:,list(df)[1]],
                bins=sObj.nx)
            self.spec2d.append(sObj)

## Run this if this file is run alone for debugging purposes            
if __name__ == '__main__':
    import matplotlib.pyplot as plt
    SpecColl = SpectrumCollection(0)

    app = QApplication([])
    SpecColl.LoadHDFData()
    SpecColl.printSummary()
