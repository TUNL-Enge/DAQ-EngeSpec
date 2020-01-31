## All of the stuff for opening spectra
from PyQt5.QtCore import Qt, QThread, QTimer
from PySide2.QtWidgets import QApplication, QFileDialog
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import time
import os.path
import pickle

from SpectrumHandlers import *

## Import my own c++ library!
if os.path.exists('EngeSort.so'):
    import EngeSort

class SpectrumCollection:
    def __init__(self, num):
        self.num = num

        self.spec1d = [SpectrumObject(0)]
        self.spec1d[0].initialize()
        self.spec2d = [SpectrumObject2D(0)]
        self.Name = "Test Collection of Spectra"

        ## Load the data library
        if os.path.exists('EngeSort.so'):
            self.dm = EngeSort.EngeSort()

        self.isRunning = False
        self.MIDASisRunning = False
        
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

    ## Save events from an HDF file
    def SaveHDFData(self):
        filename = QFileDialog.getSaveFileName(None,
                                               "Save Data Collection", "./",
                                               "Data Files (*.hdf)")
        print("Saving: ",filename[0])

        ## First make a dataframe out of all of the spectra
        ## Compare "Sort" below with "Create2D.py" script.
        self.printSummary()

        #### Make simplified 1D and 2D spectra
        ##df = pd.DataFrame()
        ##for i in range(len(self.spec1d)):
        ##    df[self.spec1d[i].Name] = self.spec1d[i].spec
        ##    
        ##df.to_hdf(filename[0], key='df', mode='w')
        ##
        ##for i in range(len(self.spec2d)):
        ##    df2d = pd.DataFrame(self.spec2d[i].spec2d)
        ##    df2d.to_hdf(filename[0], key='df', mode='a')
        my_list = list(self.spec1d)
        print(my_list)

        def save_object(obj, filename):
            with open(filename, 'wb') as output:  # Overwrites any existing file.
                pickle.dump(obj, output, pickle.HIGHEST_PROTOCOL)


        save_object(my_list, filename[0])
        ##spec_pd = pd.DataFrame([p for p in my_list])
        ##print(spec_pd.dtypes)
        ##print(spec_pd)
        ##spec_pd.to_hdf(filename[0], key='df', mode='w')
        

        
    def Sort(self):
        ## First delete the old spectra
        self.spec1d = []
        self.spec2d = []
        ## For each column in df, make a 1D spectrum
        for i in range(len(list(self.df))):
            sObj = SpectrumObject(i)
            sObj.Name = list(self.df)[i]
            h, edges = np.histogram(self.df.loc[:,sObj.Name],bins=range(0,sObj.NBins+1))
            sObj.spec = h
            sObj.spec_temp[:] = h
            self.spec1d.append(sObj)

        ## Make a 2D spectrum
        sObj = SpectrumObject2D(0)
        sObj.Name = list(self.df)[0] + "vs" + list(self.df)[1]
        sObj.spec2d, sObj.xedges, sObj.yedges = np.histogram2d(
            x=self.df.loc[:,list(self.df)[0]]/16,
            y=self.df.loc[:,list(self.df)[1]]/16,
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
        
    def ZeroAll(self):
        for i in self.spec1d:
            i.spec.fill(0)
            i.spec_temp.fill(0)
        for i in self.spec2d:
            i.spec2d.fill(0)
            i.spec2d_temp.fill(0)

        self.dm.ClearData();
            
    ## Is a count at x,y in the Gate G?
    def inGate(x,y,G):
        rough = x>min(G[:,1]) & x<max(G[:,1]) 
        return rough
    
    ## Start a simulation of data
    def connectsim(self):
        self.simulator_thread = SimulatorThread(self)

    def startsim(self):
        self.isRunning = True
        self.simulator_thread.start()
        
    def stopsim(self):
        self.isRunning = False
        print(self.dm.saygoodbye())

    ## Start a simulation of data
    def connectmidas(self):
        self.midas_thread = MidasThread(self)
        self.midas_thread.start()
        self.MIDASisRunning = True
        self.midas_collection_thread = MidasCollectionThread(self)
        self.midas_collection_thread.start()

    ##def midasrun(self):
    ##    self.midas_thread.start()

## Run MIDAS in a separate thread so it doesn't lock up the GUI
class MidasThread(QThread):
    def __init__(self,specColl):
        super().__init__()

        self.specColl = specColl
        
        print(self.specColl.dm.sayhello())
        ## Initialize the DataMaker
        self.specColl.dm.Initialize()
        
        ## First get the list of defined spectra in the datastream
        self.names = self.specColl.dm.getSpectrumNames()
        ##        print(len(self.names)," Spectra have been made:")
        ##        for name in self.names:
        ##            print(" - ",name)

        self.is2Ds = self.specColl.dm.getis2Ds()
        self.hasGates = self.specColl.dm.gethasGates()
        
        ## First delete the old spectra
        self.specColl.spec1d = []
        self.specColl.spec2d = []

        ## Make the empty spectra
        counter1d=0
        counter2d=0
        for i in range(len(self.names)):
            if not self.is2Ds[i]:
                sObj = SpectrumObject(counter1d)
                sObj.Name = self.names[i]
                sObj.hasGate = self.hasGates[i]
                sObj.spec = np.zeros(sObj.NBins)
                ## TODO: FIX THIS! Just so scaling works on an empty spectrum
                sObj.spec[0] = 1
                sObj.spec_temp[:] = sObj.spec
                self.specColl.spec1d.append(sObj)
                counter1d = counter1d+1
            else:
                sObj = SpectrumObject2D(counter2d)
                sObj.Name = self.names[i]
                sObj.hasGate = self.hasGates[i]
                sObj.xedges = np.array([x for x in range(0,256)])
                sObj.yedges = np.array([y for y in range(0,256)])
                ## TODO: FIX THIS! Just so scaling works on an empty spectrum
                sObj.spec2d[0,0] = 1
                sObj.spec2d[254,254] = 1
                sObj.spec2d_temp[:] = sObj.spec2d
                self.specColl.spec2d.append(sObj)
                counter2d = counter2d+1
                
                
    def run(self):
        print("Connecting MIDAS")
        self.specColl.dm.connectMidasAnalyzer()

class MidasCollectionThread(QThread):
    def __init__(self,specColl):
        super().__init__()

        self.specColl = specColl
        ## First get the list of defined spectra in the datastream
        self.names = self.specColl.dm.getSpectrumNames()
        ##print(len(self.names)," Spectra have been made:")
        ##for name in self.names:
        ##    print(" - ",name)

        self.is2Ds = self.specColl.dm.getis2Ds()
        self.hasGates = self.specColl.dm.gethasGates()

    def run(self):
#        print("Collecting MIDAS data")
        while self.specColl.MIDASisRunning:
#            print("isRunning!")
            dat = np.transpose(self.specColl.dm.getData())
            dat2d = self.specColl.dm.getData2D()

            ## Go through the names and fill them for the appropriate data
            counter1d=0
            counter2d=0
            for i in range(0,len(self.names)):
                if not self.is2Ds[i]:
                    sObj = self.specColl.spec1d[counter1d]
                    sObj.spec_temp[:] = dat[:,counter1d]
                    counter1d = counter1d+1
                else:
                    sObj = self.specColl.spec2d[counter2d]
                    sObj.spec2d_temp[:] = dat2d[counter2d,:,:]
                    counter2d = counter2d+1
                
            time.sleep(1)
        
            
## Run this if this file is run alone for debugging purposes            
if __name__ == '__main__':
    import matplotlib.pyplot as plt
    SpecColl = SpectrumCollection(0)

    app = QApplication([])
    SpecColl.LoadHDFData()
    SpecColl.printSummary()
