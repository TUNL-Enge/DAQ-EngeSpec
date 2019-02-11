import matplotlib
import numpy as np
from PySide2.QtWidgets import QSizePolicy
from matplotlib.backends.backend_qt5agg import (FigureCanvasQTAgg as FigureCanvas, NavigationToolbar2QT as NavigationToolbar)
from matplotlib.figure import Figure
import matplotlib.colors as colors
import matplotlib.cm as cm
import copy

import Events

class SpectrumCanvas(FigureCanvas):
    """Ultimately, this is a QWidget (as well as a FigureCanvasAgg, etc.)."""
    def __init__(self, parent=None, width=5, height=4, dpi=100):
        self.fig = Figure(figsize=(width, height), dpi=dpi)
        self.fig.subplots_adjust(top=0.96,bottom=0.115,left=0.082,right=.979)
        self.a = self.fig.add_subplot(111)
        
        ##self.compute_initial_figure()
        
        self.a.set_xlim([0,4095])
        self.a.set_ylim([0,800])
        self.a.set_xlabel("channel")
        self.a.set_ylabel("counts")

        self.maximumX    = 4095
        self.isLogPlot  = False
        self.isReslot   = False

        self.stream=None
        self.key=0
        
        FigureCanvas.__init__(self, self.fig)
        self.setParent(parent)
        FigureCanvas.setSizePolicy(self,
                                   QSizePolicy.Expanding,
                                   QSizePolicy.Expanding)
        FigureCanvas.updateGeometry(self)
        
        self.FillTestData()
        ##self.fig.canvas.draw()
        
    def compute_initial_figure(self):
        pass

    def FillTestData(self):
        x = np.array([x for x in range(0,4096)],dtype=int)
        y = x
        x = np.ravel(list(zip(x,x+1)))
        y = np.ravel(list(zip(y,y)))
        self.a.plot(x,y,'k')

    def PlotData(self):
        ##Template = self.fitObject.dataSpectrum
        ##for line in Template.LineGraphics: 
        ##    if (line in self.a.lines):
        ##        line.remove()
        ##    line=None
        x = np.array([x for x in range(0,4096)],dtype=int)
        y = x
        x = np.ravel(list(zip(x,x+1)))
        y = np.ravel(list(zip(y,y)))
        ##Template.LineGraphics.append(self.a.plot(x,y,'k')[0])
        self.fig.canvas.draw()
    

    def GetMax(self):
        binlow = int(self.a.get_xlim()[0])
        binhigh= int(self.a.get_xlim()[1])
        maxarray=[100] #default value
        return max(maxarray)

    def Autosize(self):
        if self.isLogPlot == True:
            self.a.set_ylim([1,1.20*self.GetMax()])
        else:
            self.a.set_ylim([0,1.20*self.GetMax()])
        # JACK TEST
        self.fig.canvas.draw()

    def Resize(self):
        self.a.set_xlim(0,self.maximumX)
        if(self.isLogPlot==True):
            self.a.set_ylim([1,1.20*self.GetMax()])
        else:                    
            self.a.set_ylim([0,1.20*self.GetMax()])
        self.fig.canvas.draw()

    def ToggleLog(self):
        if self.isLogPlot == False:
            if self.a.get_ylim()[0] < 1:
                self.a.set_ylim([1,self.a.get_ylim()[1]])
            self.a.set_yscale('log')
            self.isLogPlot=True
        else:
            self.a.set_yscale('linear')
            self.isLogPlot=False
        self.fig.canvas.draw()

    def xZoomIn(self):
        xlow,xhigh = self.a.get_xlim()
        numbins=xhigh-xlow
        midbin =xlow+numbins/2
        newlow,newhigh = (midbin-0.80*(midbin-xlow),midbin+0.80*(xhigh-midbin))
        if( newlow >= 0 and newhigh <= 4096):
            self.a.set_xlim(newlow,newhigh)
            self.fig.canvas.draw()
     
    def xZoomOut(self):
        xlow,xhigh = self.a.get_xlim()
        numbins=xhigh-xlow
        midbin =xlow+numbins/2
        newlow,newhigh = (midbin-1.20*(midbin-xlow),midbin+1.20*(xhigh-midbin))
        if( newlow >= 0 and newhigh <= 4096):
            self.a.set_xlim(newlow,newhigh)
            self.fig.canvas.draw()

    def yZoomIn(self):
        ylow,yhigh = self.a.get_ylim()
        newhigh = 0.80*yhigh
        self.a.set_ylim(ylow,newhigh)
        self.fig.canvas.draw()

    def yZoomOut(self):
        ylow,yhigh = self.a.get_ylim()
        newhigh = 1.20*yhigh
        self.a.set_ylim(ylow,newhigh)
        self.fig.canvas.draw()

    
class NormalSpectrumCanvas(SpectrumCanvas):
    """Simple canvas with a sine plot."""
    def compute_initial_figure(self):
        t = np.arange(0.0, 4095, 1)
        s = 200+100*np.sin(2*np.pi*t/1000.0)
        self.a.plot(t, s)
