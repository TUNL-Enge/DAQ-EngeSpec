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
from mpl_toolkits.axes_grid1 import make_axes_locatable
import copy
import time
# Victor's additional libraries:
#import IPython.display as display
from scipy.optimize import curve_fit as cf
# Will's additional libraries:
from lmfit import Model

class SpectrumCanvas(FigureCanvas):
    """Ultimately, this is a QWidget (as well as a FigureCanvasAgg, etc.)."""
    def __init__(self, SpecColl=None, parent=None, width=5, height=4, dpi=100):

        ## Keep track of what spectrum is currently being displayed on this canvas
        self.sindex1d = 0
        self.sindex1dOverlay = 0
        self.sindex2d = 0
        
        self.SpecColl = SpecColl
        self.Spec = SpecColl.spec1d[self.sindex1d]
        self.SpecOverlay = 0
        self.Spec2D = SpecColl.spec2d[self.sindex2d]
        self.is2D = False

        ## Settings
        self.autobin = False
        self.dots = False
        
        self.fig = Figure(figsize=(width, height), dpi=dpi)
        self.fig.subplots_adjust(top=0.96,bottom=0.115,left=0.082,right=.979)
        self.a = self.fig.add_subplot(111)

        #self.a.format_coord = lambda x, y: "x = % 8.1f \ny = % 8.1f" % (x,y)
        
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
        ##self.isLogPlot  = False
        self.isReslot   = False

        self.stream=None
        self.key=0

        self.lincb = False
        self.colorbar_axes = None
        self.original_loc = self.a.get_axes_locator()
        
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
        self.cxdata = []
        self.cydata = []
        global cid
        cid = self.fc.mpl_connect(self, 'button_press_event', self.onclick)
        #print("entering a blocking loop")
        self.fc.start_event_loop(self)#,timeout=-1)
        
    def setSpecIndex(self,i,is2D,drawGate=-1):
        if is2D:
            ##self.sindex1d = 0
            self.sindex2d = i
            self.Spec2D = self.SpecColl.spec2d[self.sindex2d]
            self.Spec2D.GateIndex = drawGate
        else:            
            self.sindex1d = i
            ##self.sindex2d = 0
            self.Spec = self.SpecColl.spec1d[self.sindex1d]
        self.is2D = is2D
        self.SpecOverlay = []
        self.PlotGeneral(is2D,drawGate)

    def setOverlayIndex(self, i):
        ##print("Overlaying spectrum ",i)
        self.sindex1dOverlay = i
        self.SpecOverlay = self.SpecColl.spec1d[self.sindex1dOverlay]
        self.PlotData()
        
    def PlotGeneral(self,is2D,drawGate=-1):
        if not is2D:
            self.PlotData(drawGate)
        else:
            self.PlotData2D(drawGate)
            
    def LoadASCIIData(self):
        self.SpecColl.addSpectrum("Test Spectrum")
        idx = len(self.SpecColl.spec1d)-1
        self.setSpecIndex(idx, is2D=False)
        self.Spec = self.SpecColl.spec1d[idx]
        self.Spec.LoadASCIIData()
        self.PlotData()

    def SaveASCIIData(self):
        self.Spec.SaveASCIIData()

    def LoadAdditionalASCIIData(self):
        self.Spec.LoadASCIIData()
        self.PlotData()

    def LoadPickleData(self):
        self.SpecColl.LoadPickleData()
        self.sindex1d = 0
        for i in range(len(self.SpecColl.spec1d)):
            self.SpecColl.spec1d[i].isLog=False
        for i in range(len(self.SpecColl.spec2d)):
            self.SpecColl.spec2d[i].isLog=False
        self.Spec = self.SpecColl.spec1d[self.sindex1d]
        self.PlotData()
        ##self.PlotData2D()
    
    def SavePickleData(self):
        self.SpecColl.SavePickleData()

    def PlotData(self,drawGate=-1):
        x = np.array([x for x in range(0,self.Spec.NBins)],dtype=int)
        y = self.Spec.spec

        self.x = x
        self.y = y

        
        ## delete the 2D colorbar
        if self.lincb:
            self.image.remove()
            #self.lincb.remove()
            self.lincb = False
            self.fig.delaxes(self.colorbar_axes)
            self.colorbar_axes = None
            self.a.set_axes_locator(self.original_loc)

        ##self.a.set_axes_locator(self.original_loc)
        
        xmin = self.Spec.xzoom[0]
        xmax = self.Spec.xzoom[1]
        ymin = self.Spec.yzoom[0]
        ymax = self.Spec.yzoom[1]

        if self.autobin:
            ## Auto rebin.
            ## The principle, here, is to plot a maximum of
            ## nBinMax bins, so rebin the data to fit
            nBinMax = 1000
            byBin = round((xmax-xmin)/1000)
            print("auto binning by: ",byBin)
            x_rebin = np.array([x for x in range(0,self.Spec.NBins,byBin)],dtype=int)
            y_rebin = np.zeros(len(x_rebin))
            for i in range(len(x_rebin)):
                #y_rebin[i] = sum(y[slice(i*byBin,(i+1)*byBin)])
                y_rebin[i] = np.mean(y[slice(i*byBin,(i+1)*byBin)])
        else:
            x_rebin = x
            y_rebin = y

        
        self.a.format_coord = lambda x, y: "x = {0:>8.1f} \ny = {1:>8.1f}".format(x,y)
        self.a.clear()
        if not self.dots:
            self.a.step(x_rebin,y_rebin,'k',where='mid')
            #self.a.step(x,y,'k',where='mid')
        else:
            self.a.plot(x_rebin, y_rebin, 'k+')
            
        self.a.set_xlim([xmin,xmax])
        self.a.set_ylim([ymin,ymax])
        if self.Spec.isLog:
            self.a.set_ylim([0.1,self.a.get_ylim()[1]])
            self.a.set_yscale('log')
            #        self.Resize()

        ## Now the overlay spectrum
        if self.SpecOverlay:
            y = self.SpecOverlay.spec
            self.a.step(x,y,'red',alpha=0.6,where='mid')


        if drawGate == -1:
            for i in range(self.Spec.NGates):
                #print(i)
                if len(self.Spec.gates[i].x)>0:
                    self.drawGates(i)
        else:
            if len(self.Spec.gates[drawGate].x)>0:
                    self.drawGates(drawGate)
                    
        self.fig.canvas.draw()

    def ReBin(self, binNum):
        try:
            n = binNum
            new_y = []
            new_x = list(range(0,len(self.x),binNum))
            
            if new_x.count(self.x[-1])==0:
                new_x.append(self.x[-1])

            for i in range(len(new_x)):
                if i==len(new_x)-1:
                    y_vals = self.y[i*n+1:len(self.y)]
                    if len(y_vals)==0:
                        new_y.append(self.y[-1])
                    else:
                        new_y.append(sum(y_vals)/len(y_vals))

                else:
                    y_vals = self.y[i*n+1:(i+1)*n]
                    new_y.append(sum(y_vals)/len(y_vals))
                        
            self.a.clear()
            self.a.step(new_x,new_y,'k',where='mid')

            xmin = self.Spec.xzoom[0]
            xmax = self.Spec.xzoom[1]
            ymin = self.Spec.yzoom[0]
            ymax = self.Spec.yzoom[1]

            
            self.a.set_xlim([xmin,xmax])
            self.a.set_ylim([ymin,ymax])
            print("Re-binned")
            self.fig.canvas.draw()
            
        except:
            print("No bins currently displayed")
        
    def PlotData2D(self,drawGate=-1):
        xmin = self.Spec2D.xzoom[0]
        xmax = self.Spec2D.xzoom[1]
        ymin = self.Spec2D.yzoom[0]
        ymax = self.Spec2D.yzoom[1]

        H = self.Spec2D.spec2d.T
        xe = self.Spec2D.xedges
        ye = self.Spec2D.yedges
        #print(ye)
        X, Y = np.meshgrid(xe,ye)
        #print(X)
        #print(Y)
        
        x = xe[xe>xmin]# & xe<xmax]
        x = x[x<xmax].astype(int)
        y = ye[ye>ymin]# & ye<ymax]
        y = y[y<ymax].astype(int)
        Xcut, Ycut = np.meshgrid(x,y)

        if self.Spec2D.zmax == 0:
            Hmax = H[Xcut,Ycut].max()
            self.Spec2D.zmax = Hmax
        else:
            Hmax = self.Spec2D.zmax
        #Hmax = self.Spec2D.zmax
        Nc = 255
        cbreak = np.zeros(Nc+1)
        if not self.Spec2D.isLog:
            cbreak[1] = 1.0
            for i in range(1,Nc):
                cbreak[i+1] = 1+i* Hmax/(Nc-1)
        else:
            cbreak[1] = 0
            for i in range(1,Nc):
                cbreak[i+1] = i* np.log10(Hmax)/(Nc-1)
            cbreak = 10**(cbreak)
            cbreak[0]=0.1

        norm = BoundaryNorm(cbreak,Nc)
        
        def format_coord(x, y):
            col = int(x)
            row = int(y)
            if col >= 0 and col < self.Spec2D.NBins and row >= 0 and row < self.Spec2D.NBins:
                z = H[row, col]
                return "(x,y) = ({0:<4.0f}, {1:>4.0f}) \nz = {2:>8.0f}".format(x, y, z)

        self.a.format_coord = format_coord
        self.a.clear()
        ##self.image = self.a.pcolormesh(X,Y,H,vmin=0,vmax= Hmax,norm = norm,cmap=self.cols)
        
        ##self.image = self.a.pcolormesh(X,Y,H,norm = norm,cmap=self.cols)
        self.image = self.a.pcolormesh(H, norm = norm, cmap=self.cols)
        if self.lincb:
            self.lincb.update_normal(self.image)
        else:
            ##self.sm = cm.ScalarMappable(norm=norm,cmap= self.cols)
            divider = make_axes_locatable(self.a)
            cax = divider.append_axes("right", size="3%", pad="5%")
            blah = divider.append_axes("right",size="5%",pad="5%",add_to_figure=False)
            ##cax = make_axes_locatable(self.a).new_horizontal(size="3%", pad="1%")
            self.lincb = self.fig.colorbar(self.image,cax=cax)
            ##self.lincb = matplotlib.pyplot.colorbar(image)#,cax=cax)
        ##self.lincb = self.fig.colorbar(self.image,cax=self.colorbar_axes)
        self.a, self.colorbar_axes = self.fig.get_axes()

        self.a.set_xlim([xmin,xmax])
        self.a.set_ylim([ymin,ymax])

        #self.lincb.remove()
        #self.lincb = False
        #self.a.set_axes_locator(self.original_loc)
        if drawGate == -1:
            ##for g in self.Spec2D.gates:
            ##print("NGates = ",self.Spec2D.NGates)
            for i in range(self.Spec2D.NGates):
                ##print(i)
                if len(self.Spec2D.gates[i].x)>0:
                    self.drawGates2D(i)
        else:
            if len(self.Spec2D.gates[drawGate].x)>0:
                    self.drawGates2D(drawGate)

        self.updateSlider()
        self.fig.canvas.draw()

        
        

    ## TODO: Clean this up. It's not very efficient currently
    def UpdatePlot(self):
        ##xmin  = self.a.get_xlim()[0]
        ##xmax  = self.a.get_xlim()[1]
        ##ymin    = self.a.get_ylim()[0]
        ##ymax    = self.a.get_ylim()[1]

        ## Is MIDAS running? If so, run the collection thread
        if self.SpecColl.MIDASisRunning:
            self.SpecColl.midas_collection_thread.start()
            time.sleep(0.5)
            
        if self.SpecColl.MIDASLastAgg:
            self.SpecColl.midas_collection_thread.start()
            self.SpecColl.MIDASLastAgg = False
            time.sleep(0.5)
            
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

            if self.autobin:
                ## Auto rebin.
                ## The principle, here, is to plot a maximum of
                ## nBinMax bins, so rebin the data to fit
                nBinMax = 1000
                byBin = round((xmax-xmin)/1000)
                print("auto binning by: ",byBin)
                x_rebin = np.array([x for x in range(0,self.Spec.NBins,byBin)],dtype=int)
                y_rebin = np.zeros(len(x_rebin))
                for i in range(len(x_rebin)):
                    #y_rebin[i] = sum(y[slice(i*byBin,(i+1)*byBin)])
                    y_rebin[i] = np.mean(y[slice(i*byBin,(i+1)*byBin)])
            else:
                x_rebin = x
                y_rebin = y
            
            self.a.clear()
            if not self.dots:
                self.a.step(x_rebin,y_rebin,'k',where='mid')
                #self.a.step(x,y,'k',where='mid')
            else:
                self.a.plot(x_rebin, y_rebin, 'k+')
                #            self.a.step(x_rebin,y_rebin,'k',where='mid')
            self.a.set_xlim([xmin,xmax])
            self.a.set_ylim([ymin,ymax])
            
        else:
            self.PlotData2D()
           #self.fig.colorbar(cm.ScalarMappable(norm=norm,cmap= self.cols))

        self.updateSlider()
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
            if self.Spec.isLog == True:
                ymin = 0.1 #max(0.1,0.9*self.GetMin())
                self.a.set_ylim([ymin,1.10*self.GetMax()])
            else:
                ymin = 0 #max(0,0.9*self.GetMin())
                self.a.set_ylim([ymin,1.10*self.GetMax()])
            self.Spec.yzoom = self.a.get_ylim()
            self.fig.canvas.draw()
        else:
            zmax = 0
            self.Spec2D.zmax = zmax 
            self.PlotData2D()

        self.updateSlider()
            
    def Resize(self):
        if not self.is2D:
            self.a.set_xlim(0,self.Spec.NBins-1)
            if(self.Spec.isLog==True):
                self.a.set_ylim([1,1.20*self.GetMax()])
            else:                    
                self.a.set_ylim([0,1.20*self.GetMax()])
            self.Spec.yzoom = self.a.get_ylim()
            self.Spec.xzoom = self.a.get_xlim()
        else:
            self.a.set_xlim([0,self.Spec2D.NBins])
            self.a.set_ylim([0,self.Spec2D.NBins])#self.maximumX)
            self.Spec2D.yzoom = [0,self.Spec2D.NBins]#self.a.get_ylim()
            self.Spec2D.xzoom = [0,self.Spec2D.NBins]#self.a.get_xlim()
        self.updateSlider()
        self.fig.canvas.draw()

    def ToggleLog(self):
        if not self.is2D:
            if self.Spec.isLog == False:
                if self.a.get_ylim()[0] < 1:
                    self.a.set_ylim([0.1,self.a.get_ylim()[1]])
                    self.a.set_yscale('log')
                    self.Spec.isLog=True
            else:
                self.a.set_yscale('linear')
                self.Spec.isLog=False
            self.fig.canvas.draw()
            
        else:
            if not self.Spec2D.isLog:
                self.Spec2D.isLog = True
                self.PlotData2D()
            else:
                self.Spec2D.isLog = False
                self.PlotData2D()

    def onclick(self,event):
        '''
        Event handler for button_press_event
        @param event MouseEvent
        '''
        ## Count down from NClicks to zero
        self.NClicks = self.NClicks-1
        global ix, iy
        ix = event.xdata
        iy = event.ydata
        
        if ix is not None:
            #print('x = %f' %(ix))
            self.cxdata.append(ix) 
        else:
            #print('in margin')
            self.cxdata.append(-1)  
        if iy is not None:
            #print('x = %f' %(ix))
            self.cydata.append(iy) 
        else:
            #print('in margin')
            self.cydata.append(-1)       

        if self.NClicks == 0 or event.button == 3:
            #print("disconnecting clicker")
            self.fc.mpl_disconnect(self,cid)
            self.fc.stop_event_loop(self)
            #print("now we're unblocked")

            
    def JamZoom(self):
        print("Click on the zoom limits")
        self.getNClicks(2)
        xlow,xhigh = self.a.get_xlim()
        if self.cxdata[0] == -1:
            self.cxdata[0] = xlow
        if self.cxdata[1] == -1:
            self.cxdata[1] = xhigh
        newlowx,newhighx = min(self.cxdata),max(self.cxdata)
        self.a.set_xlim(newlowx,newhighx)

        if self.is2D:
            ylow,yhigh = self.a.get_ylim()
            if self.cydata[0] == -1:
                self.cydata[0] = ylow
            if self.cydata[1] == -1:
                self.cydata[1] = yhigh
            newlowy,newhighy = min(self.cydata),max(self.cydata)
            self.a.set_ylim(newlowy,newhighy)
            ## Save to spectrum
            self.Spec2D.xzoom = self.a.get_xlim()
            self.Spec2D.yzoom = self.a.get_ylim()
        else:
            self.Spec.xzoom = self.a.get_xlim()

        self.updateSlider()
        self.fig.canvas.draw()

    def JamZoomy(self):
        print("Click on the y-limits\n")
        #x = self.fig.ginput(2)
        #print(x)
        self.getNClicks(2)
        print(self.cydata)
        ylow,yhigh = self.a.get_ylim()
        if self.cydata[0] == -1:
            self.cydata[0] = ylow
        if self.cydata[1] == -1:
            self.cydata[1] = self.cydata[0]
            self.cydata[0] = ylow
        newlowy,newhighy = min(self.cydata),max(self.cydata)
        self.a.set_ylim(newlowy,newhighy)
        ## Save to spectrum
        if self.is2D:
            self.Spec2D.yzoom = self.a.get_ylim()
        else:
            self.Spec.yzoom = self.a.get_ylim()
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
        self.updateSlider()
     
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

        ## Update plot and save to spectrum
        self.a.set_xlim(newlowx,newhighx)
        if self.is2D:
            self.a.set_ylim(newlowy,newhighy)
            self.Spec2D.xzoom = self.a.get_xlim()
            self.Spec2D.yzoom = self.a.get_ylim()
        else:
            self.Spec.xzoom = self.a.get_xlim()
            self.Spec.yzoom = self.a.get_ylim()
        ## And plot!
        self.fig.canvas.draw()
        self.updateSlider()

    def yZoomIn(self):
        ylow,yhigh = self.a.get_ylim()
        newhigh = 0.80*yhigh
        self.a.set_ylim(ylow,newhigh)
        if self.is2D:
            self.Spec2D.yzoom = self.a.get_ylim()
        else:
            self.Spec.yzoom = self.a.get_ylim()

        self.fig.canvas.draw()

    def yZoomOut(self):
        ylow,yhigh = self.a.get_ylim()
        newhigh = 1.20*yhigh
        self.a.set_ylim(ylow,newhigh)
        if self.is2D:
            self.Spec2D.yzoom = self.a.get_ylim()
        else:
            self.Spec.yzoom = self.a.get_ylim()

        self.fig.canvas.draw()

#    def getClicks(self,n=1):
#        print("Click ",n," times\n")
#        x = self.fig.ginput(n)
#        print(x)

    def setupSlider(self,scroll):
        self.scroll = scroll
        self.scroll.setRange(self.Spec.NBins/2,self.Spec.NBins/2)
        self.scroll.setValue(self.Spec.NBins/2)
        self.scroll.setPageStep(self.Spec.NBins)
        self.scroll.actionTriggered.connect(self.sliderUpdate)
        self.sliderUpdate()

    def updateSlider(self):
        xmin = self.Spec.xzoom[0]
        xmax = self.Spec.xzoom[1]
        page = (xmax-xmin)
        self.scroll.setPageStep(page)
        self.scroll.setRange(page/2, self.Spec.NBins-page/2)
        self.scroll.setValue(round((xmax+xmin)/2))

    def sliderUpdate(self, evt=None):
        v = self.scroll.value()
        xmin = self.Spec.xzoom[0]
        xmax = self.Spec.xzoom[1]
        dspan=(xmax-xmin)/2
        newlowx = v-dspan
        newhighx = v+dspan
        self.a.set_xlim(newlowx,newhighx)
        ## Save to spectrum
        if self.is2D:
            self.Spec2D.xzoom = self.a.get_xlim()
            self.Spec2D.yzoom = self.a.get_ylim()
        else:
            self.Spec.xzoom = self.a.get_xlim()
            self.Spec.yzoom = self.a.get_ylim()
        ## And plot!
        self.fig.canvas.draw()

    def setupVSlider(self,vscroll):
        self.vscroll = vscroll
        self.vscroll.setRange(20,80)
        self.vscroll.setValue(50)
        self.vscroll.setPageStep(20)
#        self.vscroll.actionTriggered.connect(self.vsliderUpdate)
        self.vscroll.valueChanged.connect(self.vsliderUpdate)


    def vsliderUpdate(self, evt=None):
        v = self.vscroll.value()
        if not self.is2D:
            ymin = self.Spec.yzoom[0]
            ymax = self.Spec.yzoom[1]
            dspan=(ymax-ymin)
            newymin = ymin
            newymax = ymin+dspan*(50+v)/100

            self.a.set_ylim(newymin,newymax)
            ## Save to spectrum
            self.Spec.xzoom = self.a.get_xlim()
            self.Spec.yzoom = self.a.get_ylim()

            ## And plot!
            self.fig.canvas.draw()
        else:
            scale = (100+(v-50)/2)/100
            self.Spec2D.zmax = self.Spec2D.zmax*scale
            ##print(self.Spec2D.zmax)
            ## Save to spectrum
            self.Spec2D.xzoom = self.a.get_xlim()
            self.Spec2D.yzoom = self.a.get_ylim()
            self.PlotData2D()
            
        self.vscroll.setValue(50)
        

    def ZeroAll(self):
        self.SpecColl.ZeroAll()
        self.UpdatePlot()
        
    def getGate(self):
        if self.is2D:
            ##tup = self.fig.ginput(n=-1,mouse_stop=3,mouse_pop=2)
            self.getNClicks(-1)
            #x = [i[0] for i in tup]
            x = self.cxdata
            x.append(x[0])
            #y = [i[1] for i in tup]
            y = self.cydata
            y.append(y[0])
            self.a.plot(x,y, 'r-')
            self.fig.canvas.draw()
            ##self.Spec2D.NGates = self.Spec2D.NGates+1
            ig = self.Spec2D.GateIndex
            self.Spec2D.gates[ig].setGate(x,y)
                    
            ## Send the gate over to c++
            self.SpecColl.dm.putGate(self.Spec2D.Name,self.Spec2D.gates[ig].name,
                                     self.Spec2D.gates[ig].x,self.Spec2D.gates[ig].y)
        else:
            ##tup = self.fig.ginput(n=2)
            self.getNClicks(2)
            x = self.cxdata
            ##x = [i[0] for i in tup]
            y = self.cydata
            ##y = [i[1] for i in tup]   ## we don't need this but it helps reuse the algorithms
            self.a.vlines(x=x, ymin=0.1, ymax=self.a.get_ylim()[1], color="red")
            self.fig.canvas.draw()

            self.Spec.NGates = self.Spec.NGates+1
            ig = self.Spec.GateIndex
            self.Spec.gates[ig].setGate(x, y)
            ## Send the gate over to c++
            self.SpecColl.dm.putGate(self.Spec.Name, self.Spec.gates[ig].name,
                                     self.Spec.gates[ig].x, self.Spec.gates[ig].y)

    def drawGates2D(self, i):
        x = self.Spec2D.gates[i].x
        y = self.Spec2D.gates[i].y
        self.a.plot(x,y,color="C{}".format(i))

    def drawGates(self, i):
        x = self.Spec.gates[i].x
        self.a.vlines(x=x,ymin=self.a.get_ylim()[0],ymax=self.a.get_ylim()[1],color="C{}".format(i))
        
    def getSingle(self,color="red"):
        ##clicks = self.fig.ginput(2)
        self.getNClicks(2)
        clicks = self.cxdata
        xcut = list(range(int(np.floor(clicks[0])),int(1+np.ceil(clicks[1]))))
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
            ##clicks = self.fig.ginput(4)
            self.getNClicks(4)
            clicks = self.cxdata
            

    # Will's Modifications (See SpectrumCanvas_old for original):
    #---------------------------------------------------------------------------------------------

    def line(self, x, m, b):
        return (m*x) + b

    def gaussian(self, x, A, c, sig):
        #return a * np.exp(-(x-c)**2/(2.0*(sig)**2))
        return (A / (np.sqrt(2 * np.pi) * sig)) * np.exp(-(x-c)**2/(2.0*(sig)**2))

    #def gauss_plus_line(self,x,a1,x1,sig1,m,b):
    #    return a1*np.exp(-(x-x1)**2/(2.0*(sig1)**2))+m*x+b

    #def double_gauss_plus_line(self,x,a1,x1,sig1,a2,x2,sig2,m,b):
    #    return a1*np.exp(-(x-x1)**2/(2.0*(sig1)**2))+m*x+b+a2*np.exp(-(x-x2)**2/(2.0*(sig2)**2))
    
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
            m,b = bgfit
            bgCounts = np.polyval(bgfit,bgx)
            res = bgy-bgCounts

            ubgCounts = np.mean(res)
            
            ## Draw background line
            xplot = list(range(min(bgx),max(bgx)))
            yplot = np.poly1d(bgfit)
            #self.a.plot(xplot,yplot(xplot), c = "firebrick", ls = "dotted")
            #self.fig.canvas.draw()

            ## Find the peak position
            Chn = peakpoints[0]
            #print(len(Chn))
            
            Counts = peakpoints[1] - np.poly1d(bgfit)(Chn)# - peakpoints[1]
            #print(len(Counts))
            centroid = sum(Chn*Counts/sum(Counts))
            #print(centroid)
            a1 = Counts[Chn.index(int(centroid))]
            #print(a1)
            sig1 = (peakpoints[0][-1] - peakpoints[0][0])/2.0
            ucentroid = sum(Counts * (Chn - centroid)**2)/(sum(Counts) - 1)
            ucentroid = np.sqrt(ucentroid)/np.sqrt(sum(Counts))
            
            ## Calculate the number of counts
            bgsum = sum(np.poly1d(bgfit)(peakpoints[0]))
            ubgsum = np.sqrt(bgsum + ubgCounts**2)
            totalsum = sum(peakpoints[1])
            net = totalsum - bgsum
            unet = np.sqrt(net + ubgsum**2)
            
            #self.guess = [a1,centroid,sig1,m,b] ## Peak height as param
            self.guess = [net,centroid,sig1,m,b] ## Net area as param
            
            #Gauss Fitting
            #Change Chn and PckPnts[1] to arrays
            gChn = np.array(Chn)
            gCnt = np.array(peakpoints[1])

            ## lmfit
            glmod = Model(self.gaussian) + Model(self.line)
            #pars = glmod.make_params(a=self.guess[0], c=self.guess[1], sig=self.guess[2], m=self.guess[3], b=self.guess[4])
            pars = glmod.make_params(A=self.guess[0], c=self.guess[1], sig=self.guess[2], m=self.guess[3], b=self.guess[4])
            result = glmod.fit(gCnt, pars, x=gChn)
            
            ## Best-fit values
            #print(result.fit_report())
            cent = result.best_values.get('c')
            #cent = result.params.get('c').value
            ucent = result.params.get('c').stderr
            sd = np.abs(result.best_values.get('sig'))
            usd = result.params.get('sig').stderr
            fwhm = 2 * np.sqrt(2 * np.log(2)) * sd
            #ufwhm = fwhm / np.sqrt(2 * net)
            ufwhm = 2 * np.sqrt(2 * np.log(2)) * usd ## Might be wrong
            netarea = result.best_values.get('A')
            unetarea = result.params.get('A').stderr
            print("Cent: ", cent) # Centroid
            print("uCent: ", ucent) # Uncertainty in centroid
            print("SD: ", sd) # standard deviation
            print("uSD: ", usd) # Uncertainty in standard deviation
            #print("FWHM: ", fwhm) # FWHM
            #print("UFWHM: ", ufwhm) # Uncertainty in FWHM
            print("NetArea (fit): ", netarea) # Net area of peak
            print("uNetArea (fit)", unetarea) # Uncertainty in net area
            
            ## Plotting fits
            #self.a.plot(gChn, result.init_fit, 'c--')
            self.a.plot(gChn, result.best_fit, c = 'C0', linewidth = 5.0)
            self.fig.canvas.draw()
            
            ## Plotting fit components
            comps = result.eval_components()
            #self.a.plot(gChn, comps['gaussian'], 'b--')
            self.a.plot(gChn, comps['line'], 'C3--')
            self.fig.canvas.draw()

            print("From",peakpoints[0][0],"to",peakpoints[0][-1]) 
            ## rounding
            dprecis = self.getprecis(ucent)
            nprecis = self.getprecis(cent)
            #print(centroid, ucentroid)
            #print(dprecis,nprecis)
            print("Peak at ",self.round_to_n(cent,1+nprecis), "+/-",
                  self.round_to_n(ucent,dprecis))
            ## rounding
            dprecis = self.getprecis(unet)
            nprecis = self.getprecis(net)
            #            print(net,unet)
            print("Net Area (tot - bkg) =",self.round_to_n(net,1+nprecis),"+/-",
                  self.round_to_n(unet,dprecis))
            
            ## Writing (appending) centroids and FWHM to .txt file
            '''
            cent_fwhm_data = open(r"/home/daq/midas/online/src/v1730/CentFWHMRuns/Co60Emulator/cent_fwhm_data_Co60Emulator.txt", "a+")
            cent_fwhm_data.write("\n")
            fit_data = [self.round_to_n(cent,1+self.getprecis(cent)), self.round_to_n(ucent,self.getprecis(ucent)), 
                        self.round_to_n(sd,1+self.getprecis(sd)), self.round_to_n(usd,self.getprecis(usd)), 
                        self.round_to_n(fwhm,1+self.getprecis(fwhm)), self.round_to_n(ufwhm,self.getprecis(ufwhm)),
                        self.round_to_n(net,1+self.getprecis(net)), self.round_to_n(unet,self.getprecis(unet))]
            for i in range(len(fit_data)):
                cent_fwhm_data.write("%5.2f\n" % fit_data[i])
            cent_fwhm_data.close()
            '''

    #---------------------------------------------------------------------------------------------
    
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
