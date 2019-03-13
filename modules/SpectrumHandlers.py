## All of the stuff for opening spectra
from PySide2.QtWidgets import QFileDialog
import numpy as np


class SpectrumObject:
    def __init__(self, num):
        self.num = num
        self.isSpectrum = True
        self.isVisible = True

        self.spec = np.zeros(4096)
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
            ##StrVar.set(filename.split("/")[-1])
            ##self.mpcanvas.PlotData()

    def initialize(self):
        t = np.arange(0.0, 4096, 1)
        s = 200+100*np.sin(2*np.pi*t/1000.0)
        self.spec = s
        
    def getSpec(self):
        return self.spec

    def simulate(self,center,width):
        samp = int(np.random.normal(loc=center,scale=width))
        self.spec[samp] += 1
    
    def __str__(self):
        return 'Spectrum Name: {}'.format(self.Name)

if __name__ == '__main__':
    Spec = SpectrumObject()
    print(Spec)
    Spec.initialize()
    print(Spec.spec)
