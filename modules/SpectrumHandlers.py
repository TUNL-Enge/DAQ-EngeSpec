## All of the stuff for opening spectra
from PySide2.QtWidgets import QFileDialog
import numpy as np


class SpectrumObject:
    def __init__(self, num):
        self.num = num
        self.isSpectrum = True
        self.isVisible = True

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
        return 'Spectrum Name: {}'.format(self.Name)

if __name__ == '__main__':
    Spec = SpectrumObject()
    print(Spec)
    Spec.initialize()
    print(Spec.spec)
