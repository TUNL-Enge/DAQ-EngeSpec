import matplotlib
import numpy as np
from PySide2.QtWidgets import QSizePolicy
from matplotlib.backends.backend_qt5agg import (FigureCanvasQTAgg as FigureCanvas, \
                                                NavigationToolbar2QT as NavigationToolbar)
matplotlib.rcParams['toolbar'] = 'toolmanager'
#matplotlib.rcParams['toolbar'] = 'toolmanager'
from matplotlib.figure import Figure
from matplotlib.widgets import SpanSelector
from matplotlib.backend_tools import ToolBase, ToolToggleBase
import matplotlib.colors as colors
import matplotlib.cm as cm
import copy

##import Events

class SpectrumCanvas(FigureCanvas):
    """Ultimately, this is a QWidget (as well as a FigureCanvasAgg, etc.)."""
    def __init__(self, Spec=None, Spec2D=None, parent=None, width=5, height=4, dpi=100):

        self.Spec = Spec
        self.Spec2D = Spec2D

        self.fig = Figure(figsize=(width, height), dpi=dpi)
        self.fig.subplots_adjust(top=0.96,bottom=0.115,left=0.082,right=.979)
        self.a = self.fig.add_subplot(111)

        
        self.a.set_xlim([0,4095])
        #self.a.set_ylim([0,800])
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

        ## This shouldn't happen, but check that we don't have a zeros spectrum
        if not self.Spec.spec.any():
            print("WARNING: Spectrum hasn't been initialized!")
            self.FillTestData()
        else:
            print("Filling default data")
            self.PlotData()

        ## Add extra tools
        ##self.addTools()
        ##self.fig.canvas.manager.toolbar.add_tool('zoom', 'foo')
        ##self.compute_initial_figure()
        
        ##self.fig.canvas.draw()
        
    def compute_initial_figure(self):
        pass

    def LoadData(self):
        self.Spec.LoadData()
        self.PlotData()
    def LoadHDFData(self):
        self.Spec2D.LoadHDFData()
        self.PlotData2D()
        
    def PlotData(self):
        ##Template = self.fitObject.dataSpectrum
        ##for line in Template.LineGraphics: 
        ##    if (line in self.a.lines):
        ##        line.remove()
        ##    line=None
        x = np.array([x for x in range(0,4096)],dtype=int)
        y = self.Spec.spec
        ##x = np.ravel(list(zip(x,x+1)))
        ##y = np.ravel(list(zip(y,y)))
        ##Template.LineGraphics.append(self.a.plot(x,y,'k')[0])
        self.a.clear()
        self.a.step(x,y,'k')
        self.fig.canvas.draw()

    def PlotData2D(self):
        H = self.Spec2D.spec2d.T
        xe = self.Spec2D.xedges
        ye = self.Spec2D.yedges
        X, Y = np.meshgrid(xe,ye)
        self.a.clear()
        ##self.a.imshow(H, extent=[xe[0],xe[-1],ye[0],ye[-1]])
        self.a.pcolormesh(X,Y,H)
        self.fig.canvas.draw()

    ## TODO: Clean this up. It's not very efficient currently
    def UpdatePlot(self):
        x = np.array([x for x in range(0,4096)],dtype=int)
        ## The displayed spectrum is only updated when we hit the UpdatePlot button
        self.Spec.spec[:] = self.Spec.spec_temp
        y = self.Spec.spec
        
        xmin  = self.a.get_xlim()[0]
        xmax  = self.a.get_xlim()[1]
        ymin    = self.a.get_ylim()[0]
        ymax    = self.a.get_ylim()[1]
        self.a.clear()
        self.a.step(x,y,'k')
        self.a.set_xlim([xmin,xmax])
        self.a.set_ylim([ymin,ymax])
        self.fig.canvas.draw()
    

    def GetMax(self):
        binlow = max(0,int(self.a.get_xlim()[0]))
        binhigh= min(4095,int(self.a.get_xlim()[1]))
        maxarray=self.Spec.spec[binlow:binhigh]
        return max(maxarray)

    def GetMin(self):
        binlow = int(self.a.get_xlim()[0])
        binhigh= int(self.a.get_xlim()[1])
        maxarray=self.Spec.spec[binlow:binhigh]
        return min(maxarray)
    
    def Autosize(self):
        if self.isLogPlot == True:
            ymin = 0.1 #max(0.1,0.9*self.GetMin())
            self.a.set_ylim([ymin,1.10*self.GetMax()])
        else:
            ymin = 0 #max(0,0.9*self.GetMin())
            self.a.set_ylim([ymin,1.10*self.GetMax()])
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
                self.a.set_ylim([0.1,self.a.get_ylim()[1]])
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

    def xZoomRange(self, minx, maxx):
        self.a.set_xlim(int(minx),int(maxx))
        self.fig.canvas.draw()
        self.span.set_visible(False)
            
    def xInteractiveZoom(self):
        self.span = SpanSelector(self.a, self.xZoomRange, 'horizontal', useblit=False,
                    rectprops=dict(alpha=0.5, facecolor='red'))
        
    def yZoomRange(self, miny, maxy):
        self.a.set_ylim(miny,maxy)
        self.fig.canvas.draw()
        self.span.set_visible(False)
            
    def yInteractiveZoom(self):
        self.span = SpanSelector(self.a, self.yZoomRange, 'vertical', useblit=False,
                    rectprops=dict(alpha=0.5, facecolor='red'))
        
    def getClicks(self,n=1):
        print("Click ",n," times\n")
        x = self.fig.ginput(n)
        print(x)

    def simulate_a_peak(self):
        self.Spec.simulate_a_peak()

