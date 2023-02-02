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
        self.isLog = False

        self.NBins = 2**13
        
        self.spec = np.zeros(self.NBins)
        self.spec_temp = np.zeros(self.NBins)   ## The temporary spectrum in memory
        self.Name = "Test Spectrum"

        self.xzoom = [0,self.NBins]
        self.yzoom = [0,1]

        ## Load the data library
        ##if os.path.exists('EngeSort.so'):
        ##    self.dm = EngeSort.EngeSort()

        self.gates = []
        self.NGates = 0
        self.GateIndex = -1  ## -1 for no gates selected
        
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
            self.Name = os.path.basename(filename[0])

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
        self.isLog = False

        self.NBins = 2**9
        
        #self.xedges = None
        #self.yedges = None
        self.xedges = np.array([x for x in range(0,self.NBins)])
        self.yedges = np.array([y for y in range(0,self.NBins)])
        self.spec2d = np.zeros(shape=(self.NBins,self.NBins))
        self.spec2d_temp = np.zeros(shape=(self.NBins,self.NBins))   ## The temporary spectrum in memory
        self.Name = "2D Test Spectrum"

        self.makeFakeData()
        
        self.xzoom = [1,self.NBins]
        self.yzoom = [1,self.NBins]
        self.zmax = 0

        self.gates = []
        self.NGates = 0
        self.GateIndex = -1  ## -1 for no gates selected

    def makeFakeData(self):
        n = 1000000
        cov = [[500**2,300**2],[300**2,500**2]]
        mean = [2400, 1700]
        x, y = np.random.multivariate_normal(mean,cov,n).T
        df = pd.DataFrame(np.array([x,y]).T, columns=['Pos1','DE'])

        self.spec2d, self.xedges, self.yedges = np.histogram2d(
            x=df.loc[:,list(df)[0]]/8,
            y=df.loc[:,list(df)[1]]/8,
            bins=self.NBins, range=[[0,self.NBins],[0,self.NBins]])
                      
    def __str__(self):
        return '2D Spectrum Name: {}'.format(self.Name)

class ScalerObject:
    def __init__(self, num):
        self.num = num
        self.Name = "Test Scaler"
        self.N = 0

    def __str__(self):
        return 'Scaler Name: {}'.format(self.Name)

class GateObject:
    def __init__(self):
        self.name = "Test Gate"
        self.x = []
        self.y = []

    def setGate(self,x,y):
        self.x = x
        self.y = y
        
if __name__ == '__main__':
    #Spec = SpectrumObject()
    #print(Spec)
    #Spec.initialize()
    #print(Spec.spec)
    Spec2D = SpectrumObject2D(0)
    print(Spec2D)
