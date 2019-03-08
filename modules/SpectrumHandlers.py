## All of the stuff for opening spectra
from PySide2.QtWidgets import QFileDialog



def SpectrumObject():
    def __init__(self):
        self.isSpectrum = True
        self.isVisible = True

        spec = None

        def LoadData():
            filename = QFileDialog.getOpenFileName(None,
                                                   "Open Spectrum", "./", "Spectrum Files (*.dat)")
            if filename != '':
                TempHistogramArray=[]
                input = open(filename[0],"r")
                for line in input:
                    Bin,Content=line.split()
                    TempHistogramArray.append( (float(Bin),float(Content)) )
                    input.close()
                    ##return TempHistogramArray
                    ##
                self.spec = TempHistogramArray
                ##StrVar.set(filename.split("/")[-1])
                self.mpcanvas.PlotData()
