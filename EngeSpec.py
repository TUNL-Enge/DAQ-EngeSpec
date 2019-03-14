from PySide2.QtWidgets import QApplication
import sys
import random

## All code is in the modules folder
sys.path.append('./modules')

from Views import Ui_MainWindow
from SpectrumCanvasQt import *
from SpectrumHandlers import *

## Define the spectrum
##spec =
app = QApplication([])

## Load and initialize a spectrum 
Spec = SpectrumObject(0)
Spec.initialize()
Spec2D = SpectrumObject2D(0)

## Load the spectrum drawing routines
SpecCanvas = SpectrumCanvas(Spec=Spec, Spec2D=Spec2D)

## Draw everything!
ui = Ui_MainWindow(SpecCanvas)  ## Pass the spectrum to the GUI
ui.show()

app.exit(app.exec_())
