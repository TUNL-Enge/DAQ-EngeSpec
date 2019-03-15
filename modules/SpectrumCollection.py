## All of the stuff for opening spectra
from PySide2.QtWidgets import QApplication, QFileDialog
import numpy as np
import pandas as pd
import matplotlib as plt

from SpectrumHandlers import *

class SpectrumCollection:
    def __init__(self, num):
        self.num = num

        self.spec1d = SpectrumObject(0)
        self.spec2d = SpectrumObject2D(0)
        self.Name = "Test Collection of Spectra"

    def __str__(self):
        return 'Spectrum Collection Name: {}'.format(self.Name)

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
            print(list(df))
            #print(self.hist2d)

if __name__ == '__main__':
    SpecColl = SpectrumCollection(0)
    print(SpecColl)

    app = QApplication([])
    SpecColl.LoadHDFData()
