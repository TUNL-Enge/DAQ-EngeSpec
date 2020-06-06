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

        self.sclr = [ScalerObject(0)]

        ## Load the data library
        if os.path.exists('EngeSort.so'):
            self.dm = EngeSort.EngeSort()

        self.isRunning = False
        self.isOnline = False
        self.MIDASisRunning = False

        self.offlinefiles = [""]
        
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

    def ClearCollection(self):
        self.spec1d.clear()
        self.spec2d.clear()
        print("Cleared spectrum collection")



    def LoadPickleData(self):
        filename = QFileDialog.getOpenFileName(None,
                                               "Open Data Collection", "./",
                                               "Data Files (*.pkl)")
        print("Loading: ",filename[0])
        with open(filename[0], 'rb') as input:
            data = pickle.load(input)
        self.spec1d = []
        self.spec2d = []
        for i in range(len(data)):
            data2 = data[i]
            for j in range(len(data2)):
                if(data2[j].is2D):
                    self.spec2d.append(data2[j])
                else:
                    self.spec1d.append(data2[j])
                

    ## Save events to a "pickle" file
    def SavePickleData(self):
        filename = QFileDialog.getSaveFileName(None,
                                               "Save Data Collection", "./",
                                               "Data Files (*.pkl)")
        print("Saving: ",filename[0])

        ## First make a dataframe out of all of the spectra
        ## Compare "Sort" below with "Create2D.py" script.
        self.printSummary()

        #### Make simplified 1D and 2D spectra
        my_list = list([self.spec1d,self.spec2d])
        print(my_list)

        def save_object(obj, filename):
            with open(filename, 'wb') as output:  # Overwrites any existing file.
                pickle.dump(obj, output, pickle.HIGHEST_PROTOCOL)


        save_object(my_list, filename[0])
        
    def ZeroAll(self):
        for i in self.spec1d:
            i.spec.fill(0)
            i.spec_temp.fill(0)
        for i in self.spec2d:
            i.spec2d.fill(0)
            i.spec2d_temp.fill(0)
        for i in self.sclr:
            i.N = 0

        self.dm.ClearData();
            
    ## Connect midas for data collection
    def connectmidas(self):
        self.midas_thread = MidasThread(self)
        self.isOnline = True

    ## Replay midas
    def offlinemidas(self):
        filename = QFileDialog.getOpenFileNames(None,
                                               "Sort MIDAS File(s)", "./",
                                               "MIDAS Files (*.mid *.mid.*)")
        self.offlinefiles = filename[0]#"run00031.mid.lz4"
        self.midas_thread = MidasThread(self)
        self.isOnline = False
        

    ## Actually run the analyzer
    def startmidas(self):
        if not self.MIDASisRunning:
            self.midas_collection_thread = MidasCollectionThread(self)
        self.midas_thread.start()
        
        ##self.midas_collection_thread.start()
        if self.isOnline:
            os.system("odbedit -c start")
        
## Run MIDAS in a separate thread so it doesn't lock up the GUI
class MidasThread(QThread):
    def __init__(self,specColl):
        super().__init__()
        
        self.specColl = specColl

        print(self.specColl.offlinefiles)
        
        print(self.specColl.dm.sayhello())
        ## Initialize the DataMaker
        self.specColl.dm.Initialize()
        
        ## First get the list of defined spectra in the datastream
        self.names = self.specColl.dm.getSpectrumNames()
        self.sclrnames = self.specColl.dm.getScalerNames()
        ##self.sclr = [0] * len(self.sclrnames)   ## fill the scalers with 0
        ## print(len(self.sclrnames)," scalers have been made:")
        ## for name in self.sclrnames:
        ##     print(" - ",name)
        ## print(len(self.names)," Spectra have been made:")
        ## for name in self.names:
        ##     print(" - ",name)

        self.is2Ds = self.specColl.dm.getis2Ds()
        self.NGates = self.specColl.dm.getNGates()
        
        ## First delete the old spectra
        self.specColl.spec1d = []
        self.specColl.spec2d = []

        ## Delete the old scalers
        self.specColl.sclr = []

        ## Make the empty spectra
        counter1d=0
        counter2d=0
        for i in range(len(self.names)):
            if not self.is2Ds[i]:
                sObj = SpectrumObject(counter1d)
                sObj.Name = self.names[i]
                sObj.NGates = self.NGates[i]
                ## Fill the gate names
                if(self.NGates[i]>0):
                    gnames = self.specColl.dm.getGateNames(sObj.Name)
                    for j in range(len(gnames)):
                        gObj = GateObject()
                        gObj.name = gnames[j]
                        sObj.gate.append(gObj)
                                            
                sObj.spec = np.zeros(sObj.NBins)
                ## TODO: FIX THIS! Just so scaling works on an empty spectrum
                sObj.spec[0] = 1
                sObj.spec_temp[:] = sObj.spec
                self.specColl.spec1d.append(sObj)
                counter1d = counter1d+1
            else:
                sObj = SpectrumObject2D(counter2d)
                sObj.Name = self.names[i]
                sObj.NGates = self.NGates[i]
                ## Fill the gate names
                if(sObj.NGates>0):
                    gnames = self.specColl.dm.getGateNames(sObj.Name)
                    for j in range(len(gnames)):
                        gObj = GateObject()
                        gObj.name = gnames[j]
                        sObj.gates.append(gObj)
                sObj.xedges = np.array([x for x in range(0,sObj.NBins)])
                sObj.yedges = np.array([y for y in range(0,sObj.NBins)])
                ## TODO: FIX THIS! Just so scaling works on an empty spectrum
                sObj.spec2d[0,0] = 1
                sObj.spec2d[sObj.NBins-1,sObj.NBins-1] = 1
                sObj.spec2d_temp[:] = sObj.spec2d
                self.specColl.spec2d.append(sObj)
                counter2d = counter2d+1

        ## Make the empty scalers
        for i in range(len(self.sclrnames)):
            scObj = ScalerObject(i)
            scObj.Name = self.sclrnames[i]
            self.specColl.sclr.append(scObj)

        ## Connect the analyzer to MIDAS
        self.specColl.dm.connectMidasAnalyzer()
            
    def run(self):
        print("Running MIDAS")
        self.specColl.MIDASisRunning = True
        self.specColl.dm.runMidasAnalyzer(self.specColl.offlinefiles)

        while self.specColl.dm.getIsRunning():
            time.sleep(1)
            
        print("MIDAS finished running")
        ## Collect the last bunch of data 
        self.specColl.midas_collection_thread.start()
        self.specColl.MIDASisRunning = False
#        self.specColl.dm.connectMidasAnalyzer()

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
        self.NGates = self.specColl.dm.getNGates()

    def run(self):
        print("Collecting MIDAS data")
        while True: ##self.specColl.MIDASisRunning:
##            print("isRunning!")
            dat = np.transpose(self.specColl.dm.getData())
            dat2d = self.specColl.dm.getData2D()

            ## Update the scalers
            sclrvals = self.specColl.dm.getScalers()
            for i in range(len(self.specColl.sclr)):
                self.specColl.sclr[i].N = sclrvals[i]

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
                
            #time.sleep(1)
            break  ## run only once for data collection when the update button is pressed
        
            
## Run this if this file is run alone for debugging purposes            
if __name__ == '__main__':
    import matplotlib.pyplot as plt
    SpecColl = SpectrumCollection(0)

    app = QApplication([])
    SpecColl.printSummary()
