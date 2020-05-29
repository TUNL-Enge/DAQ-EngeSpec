#!/usr/bin/python3

from PySide2 import QtCore
from PySide2.QtWidgets import QApplication
import sys
import random

## All code is in the modules folder
sys.path.append('./modules')

from Views import Ui_MainWindow, WriteStream, MyReceiver
from SpectrumCanvas import *
from SpectrumCollection import *
#from SpectrumHandlers import *
from queue import Queue

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

## make a queue and direct sys.stdout to it
commandqueue = Queue()
sys.stdout = WriteStream(commandqueue)
sys.stderr = WriteStream(commandqueue)

## Now create a receiver to listen to the queue
commandthread = QtCore.QThread()
commandreceiver = MyReceiver(commandqueue)
commandreceiver.mysignal.connect(ui.append_text)
commandreceiver.moveToThread(commandthread)
commandthread.started.connect(commandreceiver.run)
commandthread.start()

#app.exit(app.exec_())

app.exec_()

## A little trick to make the text printer exit nicely
print("Exiting")
commandthread.quit()
#commandthread.wait()

app.exit(0)


