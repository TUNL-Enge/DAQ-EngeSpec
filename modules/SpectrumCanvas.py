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

class SpectrumCanvas(FigureCanvas):
    """Ultimately, this is a QWidget (as well as a FigureCanvasAgg, etc.)."""
    def __init__(self, SpecColl=None, parent=None, width=5, height=4, dpi=100):

        ## Keep track of what spectrum is currently being displayed on this canvas
        self.sindex1d = 0
        self.sindex2d = 0
        
        self.SpecColl = SpecColl
        self.Spec = SpecColl.spec1d[self.sindex1d]
        self.Spec2D = SpecColl.spec2d[self.sindex2d]
        self.is2D = False
        
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
        
    def setSpecIndex(self,i,is2D):
        if is2D:
            self.sindex1d = 0
            self.sindex2d = i
            self.Spec2D = self.SpecColl.spec2d[self.sindex2d]
        else:            
            self.sindex1d = i
            self.sindex2d = 0
            self.Spec = self.SpecColl.spec1d[self.sindex1d]
        self.is2D = is2D
        self.PlotGeneral(is2D)

    def PlotGeneral(self,is2D):
        if not is2D:
            self.PlotData()
        else:
            self.PlotData2D()
            
    def LoadData(self):
        self.Spec.LoadData()
        self.PlotData()
        
    def LoadHDFData(self):
        self.SpecColl.LoadHDFData()
        self.sindex1d = 0
        self.Spec = self.SpecColl.spec1d[self.sindex1d]
        self.PlotData()
        ##self.PlotData2D()
        
    def PlotData(self):
        x = np.array([x for x in range(0,4096)],dtype=int)
        y = self.Spec.spec
        self.a.clear()
        self.a.step(x,y,'k',where='mid')
        self.Resize()

    def PlotData2D(self):
        H = self.Spec2D.spec2d.T
        xe = self.Spec2D.xedges
        ye = self.Spec2D.yedges
        X, Y = np.meshgrid(xe,ye)
        self.a.clear()
        self.a.pcolormesh(X,Y,H)
        self.Resize()

    ## TODO: Clean this up. It's not very efficient currently
    def UpdatePlot(self):
        xmin  = self.a.get_xlim()[0]
        xmax  = self.a.get_xlim()[1]
        ymin    = self.a.get_ylim()[0]
        ymax    = self.a.get_ylim()[1]

        ## Update the background data in all plots
        for sp in self.SpecColl.spec1d:
            sp.spec[:] = sp.spec_temp
        for sp in self.SpecColl.spec2d:
            sp.spec2d[:] = sp.spec2d_temp

        if not self.is2D:
            x = np.array([x for x in range(0,4096)],dtype=int)
            ## The displayed selfpectrum is only updated when we hit the UpdatePlot button
            y = self.Spec.spec

            self.a.clear()
            self.a.step(x,y,'k',where='mid')
            self.a.set_xlim([xmin,xmax])
            self.a.set_ylim([ymin,ymax])
            
        else:
            H = self.Spec2D.spec2d.T
            xe = self.Spec2D.xedges
            ye = self.Spec2D.yedges
            X, Y = np.meshgrid(xe,ye)
            self.a.clear()

            self.a.set_xlim([xmin,xmax])
            self.a.set_ylim([ymin,ymax])
            self.a.pcolormesh(X,Y,H)

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
        if not self.is2D:
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
        if not self.is2D:
            if(self.isLogPlot==True):
                self.a.set_ylim([1,1.20*self.GetMax()])
            else:                    
                self.a.set_ylim([0,1.20*self.GetMax()])
        else:
            self.a.set_ylim(0,self.maximumX)
        self.fig.canvas.draw()

    def ToggleLog(self):
        if not self.is2D:
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
        ## Get the current settings
        xlow,xhigh = self.a.get_xlim()
        ylow,yhigh = self.a.get_ylim()
        numbinsx=xhigh-xlow
        midbinx =xlow+numbinsx/2
        numbinsy=yhigh-ylow
        midbiny=ylow+numbinsy/2
        ## Do the scaling
        newlowx,newhighx = (midbinx-0.80*(midbinx-xlow),midbinx+0.80*(xhigh-midbinx))
        ## Check that it's not out of bounds
        newlowx = max(newlowx,0)
        newhighx = min(newhighx,4096)
        ## Same for y
        newlowy,newhighy = (midbiny-0.80*(midbiny-ylow),midbiny+0.80*(yhigh-midbiny))
        newlowy = max(newlowy,0)
        newhighy = min(newhighy,4096)
        
        ## Apply to the spectrum
        self.a.set_xlim(newlowx,newhighx)
        if self.is2D:
            self.a.set_ylim(newlowy,newhighy)
        self.fig.canvas.draw()
     
    def xZoomOut(self):
        xlow,xhigh = self.a.get_xlim()
        ylow,yhigh = self.a.get_ylim()
        numbinsx=xhigh-xlow
        midbinx =xlow+numbinsx/2
        newlowx,newhighx = (midbinx-1.20*(midbinx-xlow),midbinx+1.20*(xhigh-midbinx))
        newlowx = max(newlowx,0)
        newhighx = min(newhighx,4096)
        ## now for y
        numbinsy=yhigh-ylow
        midbiny =ylow+numbinsy/2
        newlowy,newhighy = (midbiny-1.20*(midbiny-ylow),midbiny+1.20*(yhigh-midbiny))
        newlowy = max(newlowy,0)
        newhighy = min(newhighy,4096)

        ##if( newlow >= 0 and newhigh <= 4096):
        self.a.set_xlim(newlowx,newhighx)
        if self.is2D:
            self.a.set_ylim(newlowy,newhighy)
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

    def ZeroAll(self):
        self.SpecColl.ZeroAll()
        self.UpdatePlot()
        
    def getGate(self):
        tup = self.fig.ginput(n=-1,mouse_stop=3,mouse_pop=2)
        x = [i[0] for i in tup]
        x.append(x[0])
        y = [i[1] for i in tup]
        y.append(y[0])
        self.a.plot(x,y, 'r-')
        self.fig.canvas.draw()
        self.Spec2D.gate = (x,y)
        ##        print(self.Spec2D.gate)
        
        ## Send the gate over to c++
        self.SpecColl.dm.putGate(self.Spec2D.Name,self.Spec2D.gate[0],self.Spec2D.gate[1])

    def getSingle(self,color="red"):
        clicks = self.fig.ginput(2)
        xcut = list(range(int(np.floor(clicks[0][0])),int(1+np.ceil(clicks[1][0]))))
        ##print(xcut)
        ##print(self.Spec.spec)
        ##        ycut = list(self.Spec.spec[xcut])
        ycut = [self.Spec.spec[i] for i in xcut]
        self.a.step(xcut,ycut,color,where='mid')
        
        ## Do some weird python shit to shade the region
        xplot = xcut.copy()
        yplot = ycut.copy()
        xplot.insert(0,xcut[0])
        xplot.extend([xplot[-1],xplot[0]])
        yplot.insert(0,0.1)
        yplot.extend([0,0])
        ##        self.a.fill(xplot,yplot,facecolor=color,alpha=0.5)
        self.a.fill_between(xcut,ycut, step="mid", alpha=0.4, color=color)
        
        self.fig.canvas.draw()

        return xcut,ycut
        
    def netArea(self):
        print("Click around peak")
        if not self.is2D:
            points = self.getSingle("forestgreen")
            #print(points[1])
            area = sum(points[1])
            darea = np.sqrt(area)
            print("From ",points[0][0]," to ",points[0][-1])
            dprecis = self.getprecis(darea)
            nprecis = self.getprecis(area)
            print("Area = ", self.round_to_n(area,nprecis), " +/- ",
                  self.round_to_n(darea,dprecis))
        else:
            clicks = self.fig.ginput(4)

    def grossArea(self):
        if self.is2D:
            print("Not available in 2D spectra")
        else:
            ## Collect the regions
            print("Click first background region")
            bg1points = self.getSingle("firebrick")
            print("Click second background region")
            bg2points = self.getSingle("firebrick")
            print("Click peak")
            peakpoints = self.getSingle("forestgreen")

            ## Calculate the background
            bgx = bg1points[0] + bg2points[0]
            bgy = bg1points[1] + bg2points[1]
            bgfit = np.polyfit(bgx,bgy,deg=1)
            ## Draw background line
            xplot = list(range(min(bgx),max(bgx)))
            yplot = np.poly1d(bgfit)
            self.a.plot(xplot,yplot(xplot),"firebrick")
            self.fig.canvas.draw()

            ## Calculate the number of counts
            bgsum = sum(np.poly1d(bgfit)(peakpoints[0]))
            totalsum = sum(peakpoints[1])
            gross = totalsum - bgsum

            print("From",peakpoints[0][0],"to",peakpoints[0][-1])
            print("Gross Area =",gross)

            
    def getprecis(self,x):
        l = np.log10(x)
        return(int(l))
            
    def round_to_n(self, x, n):
        if n < 1:
            raise ValueError("number of significant digits must be >= 1")
        # Use %e format to get the n most significant digits, as a string.
        format = "%." + str(n-1) + "e"
        as_string = format % x
        return float(as_string)
