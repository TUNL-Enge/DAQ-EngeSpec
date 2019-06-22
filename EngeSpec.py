#!/opt/anaconda3/bin/python

from PySide2.QtWidgets import QApplication
import sys
import random

## All code is in the modules folder
sys.path.append('./modules')

from NewViews import Ui_MainWindow
from SpectrumCanvas import *
from SpectrumCollection import *
#from SpectrumHandlers import *

## Define the spectrum
##spec =
app = QApplication([])

## Load and initialize a collection of spectra 
SpecColl = SpectrumCollection(0)

## Load the spectrum drawing routines
SpecCanvas = SpectrumCanvas(SpecColl=SpecColl)

## Draw everything!
ui = Ui_MainWindow(SpecCanvas)  ## Pass the spectrum to the GUI
ui.show()

app.exit(app.exec_())
