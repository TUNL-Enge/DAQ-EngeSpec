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
from matplotlib.colors import BoundaryNorm
import matplotlib.cm as cm
from matplotlib.colors import ListedColormap
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

        ## Colormaps
        basecolormap = cm.get_cmap('inferno',256)
        newcolors = basecolormap(np.linspace(0,1,256))
        newcolors[:1,:] = np.array([0.99,0.99,0.99,1])
        self.cols = ListedColormap(newcolors)

        self.a.set_xlim(self.Spec.xzoom)
        #self.a.set_ylim([0,800])
        self.a.set_xlabel("channel")
        self.a.set_ylabel("counts")

        self.maximumX    = self.Spec.NBins-1
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
        self.fc = FigureCanvas

##        self.cid = FigureCanvas.mpl_connect(self, 'button_press_event', self.onclick)

        
        ## This shouldn't happen, but check that we don't have a zeros spectrum
        if not self.Spec.spec.any():
            print("WARNING: Spectrum hasn't been initialized!")
            self.FillTestData()
        else:
            print("Filling default data")
            self.PlotData()

    def getNClicks(self,n):
        self.NClicks = n
        self.cdata = []
        global cid
        cid = self.fc.mpl_connect(self, 'button_press_event', self.onclick)
        #print("entering a blocking loop")
        self.fc.start_event_loop(self)#,timeout=-1)
        
    def setSpecIndex(self,i,is2D,drawGate=False):
        if is2D:
            ##self.sindex1d = 0
            self.sindex2d = i
            self.Spec2D = self.SpecColl.spec2d[self.sindex2d]
        else:            
            self.sindex1d = i
            ##self.sindex2d = 0
            self.Spec = self.SpecColl.spec1d[self.sindex1d]
        self.is2D = is2D
        self.PlotGeneral(is2D,drawGate)

    def PlotGeneral(self,is2D,drawGate=False):
        if not is2D:
            self.PlotData(drawGate)
        else:
            self.PlotData2D(drawGate)
            
    def LoadData(self):
        self.Spec.LoadData()
        self.PlotData()
        
    def LoadHDFData(self):
        self.SpecColl.LoadHDFData()
        self.sindex1d = 0
        self.Spec = self.SpecColl.spec1d[self.sindex1d]
        self.PlotData()
        ##self.PlotData2D()
        
    def PlotData(self,drawGate=False):
        x = np.array([x for x in range(0,self.Spec.NBins)],dtype=int)
        y = self.Spec.spec

        xmin = self.Spec.xzoom[0]
        xmax = self.Spec.xzoom[1]
        ymin = self.Spec.yzoom[0]
        ymax = self.Spec.yzoom[1]

        self.a.clear()
        self.a.step(x,y,'k',where='mid')
        self.a.set_xlim([xmin,xmax])
        self.a.set_ylim([ymin,ymax])
        #        self.Resize()
        self.fig.canvas.draw()


    def PlotData2D(self,drawGate=False):
        xmin = self.Spec2D.xzoom[0]
        xmax = self.Spec2D.xzoom[1]
        ymin = self.Spec2D.yzoom[0]
        ymax = self.Spec2D.yzoom[1]

        H = self.Spec2D.spec2d.T
        ##        xe = self.Spec2D.xedges
        ##        ye = self.Spec2D.yedges
        ###        print(xe,ye)
        ##        X, Y = np.meshgrid(xe,ye)
        ##        self.a.clear()
        ##        self.a.pcolormesh(X,Y,H,cmap=self.cols)
        ##        self.a.set_xlim([xmin,xmax])
        ##        self.a.set_ylim([ymin,ymax])
        ##
        ##        if drawGate and self.Spec2D.hasGate and self.Spec2D.gate is not None:
        ##            self.drawGate()
        ##        
        ##        self.fig.canvas.draw()
        ##        ##        self.Resize()
        Hmax = H.max()
        Nc = 256
        cbreak = np.zeros(Nc)
        cbreak[1] = 1.0
        for i in range(1,Nc-1):
            cbreak[i+1] = i* Hmax/(Nc-2)
        norm = BoundaryNorm(cbreak,Nc)
        xe = self.Spec2D.xedges
        ye = self.Spec2D.yedges
#        print(xe,ye)
        X, Y = np.meshgrid(xe,ye)
        self.a.clear()
        self.a.pcolormesh(X,Y,H,norm = norm,cmap=self.cols)
        #self.lincb = self.fig.colorbar(cm.ScalarMappable(norm=norm,cmap= self.cols))
        #self.logcb = 0
        self.a.set_xlim([xmin,xmax])
        self.a.set_ylim([ymin,ymax])

        if drawGate and self.Spec2D.hasGate and self.Spec2D.gate is not None:
            self.drawGate()
        
        self.fig.canvas.draw()
        ##        self.Resize()

    ## TODO: Clean this up. It's not very efficient currently
    def UpdatePlot(self):
        ##xmin  = self.a.get_xlim()[0]
        ##xmax  = self.a.get_xlim()[1]
        ##ymin    = self.a.get_ylim()[0]
        ##ymax    = self.a.get_ylim()[1]

        ## Update the background data in all plots
        for sp in self.SpecColl.spec1d:
            sp.spec[:] = sp.spec_temp
        for sp in self.SpecColl.spec2d:
            sp.spec2d[:] = sp.spec2d_temp

        if not self.is2D:
            xmin = self.Spec.xzoom[0]
            xmax = self.Spec.xzoom[1]
            ymin = self.Spec.yzoom[0]
            ymax = self.Spec.yzoom[1]

            x = np.array([x for x in range(0,self.Spec.NBins)],dtype=int)
            ## The displayed selfpectrum is only updated when we hit the UpdatePlot button
            y = self.Spec.spec

            self.a.clear()
            self.a.step(x,y,'k',where='mid')
            self.a.set_xlim([xmin,xmax])
            self.a.set_ylim([ymin,ymax])
            
        else:
            xmin = self.Spec2D.xzoom[0]
            xmax = self.Spec2D.xzoom[1]
            ymin = self.Spec2D.yzoom[0]
            ymax = self.Spec2D.yzoom[1]
            
            H = self.Spec2D.spec2d.T
            xe = self.Spec2D.xedges
            ye = self.Spec2D.yedges
            X, Y = np.meshgrid(xe,ye)
            self.a.clear()

            x = xe[xe>xmin]# & xe<xmax]
            x = x[x<xmax].astype(int)
            y = ye[ye>ymin]# & ye<ymax]
            y = y[y<ymax].astype(int)
            Xcut, Ycut = np.meshgrid(x,y)

            Hmax = H[Xcut,Ycut].max()
            #Hmax = H.max()
            Nc = 256
            cbreak = np.zeros(Nc)
            cbreak[1] = 1.0
            for i in range(1,Nc-1):
                cbreak[i+1] = i* Hmax/(Nc-2)
            #print(cbreak)
            norm = BoundaryNorm(cbreak,Nc)                
            self.a.set_xlim([xmin,xmax])
            self.a.set_ylim([ymin,ymax])
            #self.a.pcolormesh(X,Y,H,vmin=0,vmax=Hmax,cmap=self.cols)
            self.a.pcolormesh(X,Y,H,norm=norm,cmap=self.cols)

#            if self.Spec2D.hasGate and self.Spec2D.gate is not None:
#                self.drawGate()

        self.fig.canvas.draw()
    

    def GetMax(self):
        binlow = max(1,int(self.a.get_xlim()[0]))
        binhigh= min(self.Spec.NBins-1,int(self.a.get_xlim()[1]))
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
        
        self.Spec.yzoom = self.a.get_ylim()
        self.fig.canvas.draw()

    def Resize(self):
        if not self.is2D:
            self.a.set_xlim(0,self.maximumX)
            if(self.isLogPlot==True):
                self.a.set_ylim([1,1.20*self.GetMax()])
            else:                    
                self.a.set_ylim([0,1.20*self.GetMax()])
            self.Spec.yzoom = self.a.get_ylim()
            self.Spec.xzoom = self.a.get_xlim()
        else:
            self.a.set_xlim([0,256])
            self.a.set_ylim([0,256])#self.maximumX)
            self.Spec2D.yzoom = [0,256]#self.a.get_ylim()
            self.Spec2D.xzoom = [0,256]#self.a.get_xlim()
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

    def onclick(self,event):
        '''
        Event handler for button_press_event
        @param event MouseEvent
        '''
        ## Count down from NClicks to zero
        self.NClicks = self.NClicks-1
        global ix
        ix = event.xdata
        if ix is not None:
            #print('x = %f' %(ix))
            self.cdata.append(ix) 
        else:
            #print('in margin')
            self.cdata.append(-1)        
            
        if self.NClicks == 0:
            #print("disconnecting clicker")
            self.fc.mpl_disconnect(self,cid)
            self.fc.stop_event_loop(self)
            #print("now we're unblocked")

            
    def JamZoom(self):
        print("Click on the x-limits\n")
        #x = self.fig.ginput(2)
        #print(x)
        self.getNClicks(2)
        xlow,xhigh = self.a.get_xlim()
        if self.cdata[0] == -1:
            self.cdata[0] = xlow
        if self.cdata[1] == -1:
            self.cdata[1] = xhigh
        newlowx,newhighx = min(self.cdata),max(self.cdata)
        self.a.set_xlim(newlowx,newhighx)
        ## Save to spectrum
        self.Spec.xzoom = self.a.get_xlim()
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
        newhighx = min(newhighx,self.Spec.NBins)
        ## Same for y
        newlowy,newhighy = (midbiny-0.80*(midbiny-ylow),midbiny+0.80*(yhigh-midbiny))
        newlowy = max(newlowy,0)
        newhighy = min(newhighy,self.Spec.NBins)
        
        ## Apply to the spectrum
        self.a.set_xlim(newlowx,newhighx)
        if self.is2D:
            self.a.set_ylim(newlowy,newhighy)
            ## Save to spectrum
            self.Spec2D.xzoom = self.a.get_xlim()
            self.Spec2D.yzoom = self.a.get_ylim()
        else:
            self.Spec.xzoom = self.a.get_xlim()
            self.Spec.yzoom = self.a.get_ylim()
        ## And plot!
        self.fig.canvas.draw()
     
    def xZoomOut(self):
        xlow,xhigh = self.a.get_xlim()
        ylow,yhigh = self.a.get_ylim()
        numbinsx=xhigh-xlow
        midbinx =xlow+numbinsx/2
        newlowx,newhighx = (midbinx-1.20*(midbinx-xlow),midbinx+1.20*(xhigh-midbinx))
        newlowx = max(newlowx,0)
        newhighx = min(newhighx,self.Spec.NBins)
        ## now for y
        numbinsy=yhigh-ylow
        midbiny =ylow+numbinsy/2
        newlowy,newhighy = (midbiny-1.20*(midbiny-ylow),midbiny+1.20*(yhigh-midbiny))
        newlowy = max(newlowy,0)
        newhighy = min(newhighy,self.Spec.NBins)

        ##if( newlow >= 0 and newhigh <= 4096):
        self.a.set_xlim(newlowx,newhighx)
        if self.is2D:
            self.a.set_ylim(newlowy,newhighy)
        ## Save to spectrum
        self.Spec.xzoom = self.a.get_xlim()
        self.Spec.yzoom = self.a.get_ylim()
        ## And plot!
        self.fig.canvas.draw()

    def yZoomIn(self):
        ylow,yhigh = self.a.get_ylim()
        newhigh = 0.80*yhigh
        self.a.set_ylim(ylow,newhigh)
        self.Spec.yzoom = self.a.get_ylim()
        self.fig.canvas.draw()

    def yZoomOut(self):
        ylow,yhigh = self.a.get_ylim()
        newhigh = 1.20*yhigh
        self.a.set_ylim(ylow,newhigh)
        self.Spec.yzoom = self.a.get_ylim()
        self.fig.canvas.draw()

    def xZoomRange(self, minx, maxx):
        self.a.set_xlim(int(minx),int(maxx))
        if self.is2D:
            self.Spec2D.xzoom = self.a.get_xlim()
        else:
            self.Spec.xzoom = self.a.get_xlim()
        self.fig.canvas.draw()
        self.span.set_visible(False)
            
    def xInteractiveZoom(self):
        self.span = SpanSelector(self.a, self.xZoomRange, 'horizontal', useblit=False,
                    rectprops=dict(alpha=0.5, facecolor='red'))
        
    def yZoomRange(self, miny, maxy):
        self.a.set_ylim(miny,maxy)
        if self.is2D:
            self.Spec2D.yzoom = self.a.get_ylim()
        else:
            self.Spec.yzoom = self.a.get_ylim()
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
        if self.is2D:
            tup = self.fig.ginput(n=-1,mouse_stop=3,mouse_pop=2)
            x = [i[0] for i in tup]
            x.append(x[0])
            y = [i[1] for i in tup]
            y.append(y[0])
            self.a.plot(x,y, 'r-')
            self.fig.canvas.draw()
            self.Spec2D.hasGate = True
            self.Spec2D.gate = (x,y)
            ##        print(self.Spec2D.gate)
        
            ## Send the gate over to c++
            self.SpecColl.dm.putGate(self.Spec2D.Name,self.Spec2D.gate[0],self.Spec2D.gate[1])

    def drawGate(self):
        x = self.Spec2D.gate[0]
        y = self.Spec2D.gate[1]
        self.a.plot(x,y,'r-')
        
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
        
    def grossArea(self):
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

    def netArea(self):
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
            bgCounts = np.polyval(bgfit,bgx)
            res = bgy-bgCounts

            ubgCounts = np.mean(res)
            
            ## Draw background line
            xplot = list(range(min(bgx),max(bgx)))
            yplot = np.poly1d(bgfit)
            self.a.plot(xplot,yplot(xplot),"firebrick")
            self.fig.canvas.draw()

            ## Find the peak position
            Chn = peakpoints[0]
            Counts = peakpoints[1] - np.poly1d(bgfit)(Chn)# - peakpoints[1]
            centroid = sum(Chn*Counts/sum(Counts))
            ucentroid = sum(Counts/((Chn-centroid)**2 *sum(Counts)))
            ucentroid = np.sqrt(ucentroid/(max(Chn)-min(Chn)))

            
            ## Calculate the number of counts
            bgsum = sum(np.poly1d(bgfit)(peakpoints[0]))
            ubgsum = np.sqrt(bgsum + ubgCounts**2)
            totalsum = sum(peakpoints[1])
            net = totalsum - bgsum
            unet = np.sqrt(net + ubgsum**2)



            print("From",peakpoints[0][0],"to",peakpoints[0][-1]) 
            ## rounding
            dprecis = self.getprecis(ucentroid)
            nprecis = self.getprecis(centroid)
            #print(centroid, ucentroid)
            #print(dprecis,nprecis)
            print("Peak at ",self.round_to_n(centroid,1+nprecis), "+/-",
                  self.round_to_n(ucentroid,dprecis))
            ## rounding
            dprecis = self.getprecis(unet)
            nprecis = self.getprecis(net)
            #            print(net,unet)
            print("Net Area =",self.round_to_n(net,1+nprecis),"+/-",
                  self.round_to_n(unet,dprecis))

            
    def getprecis(self,x):
        l = np.log10(x)
        return(int(np.ceil(l)))
            
    def round_to_n(self, x, n):
        if n < 2:
            n=2 #raise ValueError("number of significant digits must be >= 1")
        # Use %e format to get the n most significant digits, as a string.
        format = "%." + str(n-1) + "e"
        as_string = format % x
        return float(as_string)
