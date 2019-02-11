import matplotlib
import numpy as np
from tkinter import *
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.colors as colors
import matplotlib.cm as cm
import copy

import Events

class ViewScreen:
    def __init__(self,parent,fitObject):
        self.fitObject=fitObject
        self.fitObject.ViewScreen=self
        self.fig = Figure(figsize=(8,4),dpi=100)
        self.fig.subplots_adjust(top=0.85,bottom=0.15,left=0.10,right=.90)
        self.a = self.fig.add_subplot(111)
        

        self.a.set_xlim([0,4095])
        self.a.set_ylim([0,800])
        self.a.set_xlabel("channel")
        self.a.set_ylabel("counts")

        self.maximumX    = 4095
        self.isLogPlot  = False
        self.isReslot   = False
        self.templateSum = None
        self.templateSumGraphic = None
        self.singleLineGraphic = None

        self.lowerFitLimitLine=None
        self.upperFitLimitLine=None

        self.stream=None
        self.key=0

        self.canvas = FigureCanvasTkAgg(self.fig,master=parent)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(side=TOP,fill=BOTH,expand=1)
        self.canvas._tkcanvas.pack(side=TOP,fill=BOTH,expand=1)
        
        self.ScrollBot = Events.ScrollBot(self)
        self.ROIRegister = None
        self.eventRegister = None
        self.FitLimitEventRegister = None
    

        self.fig.canvas.draw()


    def DrawFitLimits(self):
        
        if( (self.lowerFitLimitLine != None) and (self.upperFitLimitLine != None) ):
            self.lowerFitLimitLine.remove()
            self.upperFitLimitLine.remove()
            self.lowerFitLimitLine=None
            self.upperFitLimitLine=None
        self.lowerFitLimitLine,     = self.a.plot([self.fitObject.lowerFitLimit.get(),self.fitObject.lowerFitLimit.get()],[0.01,int(1.10*self.a.get_ylim()[1])],color="#56a0d3",lw=1)
        self.upperFitLimitLine,     = self.a.plot([self.fitObject.upperFitLimit.get(),self.fitObject.upperFitLimit.get()],[0.01,int(1.10*self.a.get_ylim()[1])],color="#56a0d3",lw=1)
        self.fig.canvas.draw()

    def PlotData(self):
        Template = self.fitObject.dataSpectrum
        for line in Template.LineGraphics: 
            if (line in self.a.lines):
                line.remove()
            line=None
        x = np.array([x for x in range(0,len(Template.Data))],dtype=int)
        y = np.array(Template.Data,dtype=float)
        x = np.ravel(list(zip(x,x+1)))
        y = np.ravel(list(zip(y,y)))
        Template.LineGraphics.append(self.a.plot(x,y,'k')[0])
        self.fig.canvas.draw()
    


    def PlotSingleTemplate(self,Template):
        x = None
        y = None
        for line in [x for x in Template.LineGraphics if ( x in self.a.lines)]:
            line.remove()
            Template.LineGraphics.remove(line)

        x = np.array([x for x in range(0,len(Template.Data))],dtype=int)
        y = (Template.plotFactor)*np.array(Template.Data,dtype=float)
        x1 = np.ravel(list(zip(x,x+1)))
        y1 = np.ravel(list(zip(y,y)))
        Template.LineGraphics.append(self.a.plot(x1,y1,color="red")[0])


    def PlotTemplates(self):
        
        if len( [x for x in self.fitObject.templateSpectra if x.isExcluded.get()==0]) == 0:
            return

        x = None
        y = None
        count = 0
        self.jet   = cm.get_cmap('gist_rainbow') 
        self.cNorm = colors.Normalize(vmin=0,vmax=len(self.fitObject.templateSpectra))
        self.scalarMap = cm.ScalarMappable(norm=self.cNorm,cmap=self.jet)

        # This block ensures that there are no line-graphics leftover from a previous plot 
        for Template in self.fitObject.templateSpectra:
            for line in Template.LineGraphics: 
                if (line in self.a.lines):
                    line.remove()
                Template.LineGraphics.remove(line)
        if self.templateSum is not None:
            if(self.templateSumGraphic in self.a.lines):
                self.templateSumGraphic.remove()
            self.templateSumGraphic=None
            self.templateSum = None

        # first plot each ACTIVE template


        for Template in [x for x in self.fitObject.templateSpectra if x.isExcluded.get()==0]:
            count+=1
            x  = np.array([x for x in range(0,len(Template.Data))],dtype=int)
            y  = (Template.plotFactor)*np.array(Template.Data,dtype=float)
            x1 = np.ravel(list(zip(x,x+1)))
            y1 = np.ravel(list(zip(y,y)))
            if self.templateSum is None:
                self.templateSum = np.copy(y)
            else:
                self.templateSum += y
            Template.LineGraphics.append(self.a.plot(x1,y1,color=self.scalarMap.to_rgba(count),picker=2,label="{0}".format(Template.filename))[0])

        # now we plot the template sum line
        x1 = np.ravel(list(zip(x,x+1)))
        y1 = np.ravel(list(zip(self.templateSum,self.templateSum)))
        self.templateSumGraphic, = self.a.plot(x1,y1,'r',picker=2,label="sum")
        self.a.legend(bbox_to_anchor=(1,1),loc=1,borderaxespad=0.,fontsize='xx-small')
        self.fig.canvas.draw()

    def GetMax(self):
        binlow = int(self.a.get_xlim()[0])
        binhigh= int(self.a.get_xlim()[1])
        maxarray=[100] #default value
        for Template in self.fitObject.templateSpectra:
            for line in Template.LineGraphics:
                if (line in self.a.lines):
                    maxarray.append(max(Template.plotFactor*Template.Data[binlow:binhigh]))

        for Template in self.fitObject.templateSpectraCopy:
            for line in Template.LineGraphics:
                if (line in self.a.lines):
                    maxarray.append(max(Template.plotFactor*Template.Data[binlow:binhigh]))

        if(self.templateSumGraphic in self.a.lines):
            maxarray.append(max(self.templateSum[binlow:binhigh]))
        if(self.fitObject.isDataSet==True):
            for line in self.fitObject.dataSpectrum.LineGraphics:
                if (line in self.a.lines):
                    maxarray.append(max(self.fitObject.dataSpectrum.Data[binlow:binhigh]))
        return max(maxarray)

    def Autosize(self):
        if self.isLogPlot == True:
            self.a.set_ylim([1,1.20*self.GetMax()])
        else:
            self.a.set_ylim([0,1.20*self.GetMax()])
        # JACK TEST
        self.ScrollBot = Events.ScrollBot(self)
        self.fig.canvas.draw()

    def Resize(self):
        self.a.set_xlim(0,self.maximumX)
        if(self.isLogPlot==True):
            self.a.set_ylim([1,1.20*self.GetMax()])
        else:                    
            self.a.set_ylim([0,1.20*self.GetMax()])
        self.fig.canvas.draw()

    def ToggleData(self):
        Template=self.fitObject.dataSpectrum
        if( self.fitObject.isDataSet == False ):
            return
        elif( Template.LineGraphics[0] in self.a.lines):
            Template.LineGraphics[0].remove()
        else:
            self.a.lines.append(Template.LineGraphics[0])
        self.fig.canvas.draw()

    def TestForChanges(self):
        tempkey=0
        num=0
        for Template in self.fitObject.templateSpectra:
            if( Template.isExcluded.get()==0  ):
                tempkey+=pow(2,num)
            num+=1
        if(tempkey != self.key):
            self.key=copy.copy(tempkey)
            return True
        else:
            self.key=copy.copy(tempkey)
            return False

    def TestForLines(self):
        for Template in self.fitObject.templateSpectra:
            for line in Template.LineGraphics: 
                if line in self.a.lines:
                    return False
        return True

    
    def ToggleTemplates(self):
        if(self.fitObject.isTemplateSet == False):
            return
        elif(self.TestForLines()):
            for Template in self.fitObject.templateSpectra:
                for line in Template.LineGraphics: 
                    self.a.lines.append(line)
            self.a.lines.append(self.templateSumGraphic)
        elif(self.TestForChanges()):
            self.PlotTemplates()
        else:
            for Template in self.fitObject.templateSpectra:
                for line in Template.LineGraphics: 
                    if (line in self.a.lines):
                        line.remove()
            if(self.templateSumGraphic in self.a.lines):
                self.templateSumGraphic.remove()
        self.fig.canvas.draw()

    def DebugGraphics(self):
        numLines = 0
        numDrawnLines = 0
        numTemplates = 0
        numLinesCopy = 0
        numDrawnLinesCopy = 0
        numTemplatesCopy = 0
        for Template in self.fitObject.templateSpectra:
            numTemplates += 1
            for line in Template.LineGraphics:
                numLines += 1
                if line in self.a.lines:
                    numDrawnLines += 1
        for Template in self.fitObject.templateSpectraCopy:
            numTemplatesCopy += 1
            for line in Template.LineGraphics:
                numLinesCopy += 1
                if line in self.a.lines:
                    numDrawnLinesCopy += 1

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


class ResidualsScreen(ViewScreen):
    def __init__(self,parent,fitObject):
        ViewScreen.__init__(self,parent,fitObject)

        self.a.set_xlim([0,4095])
        self.a.set_ylim([0,800])
        self.a.set_xlabel("channel")
        self.a.set_ylabel("counts")

        self.isResPlot=True
        self.Residuals=None

    def PlotResiduals(self):
        Experiment     = np.array(self.fitObject.dataSpectrum.Data, dtype=float)
        TemplateSum    = np.array(np.zeros_like(Experiment), dtype=float)
        self.Residuals = np.array(np.zeros_like(Experiment), dtype=float)

        for template in [(np.array(x.Data,dtype=float),x.plotFactor) for x in self.fitObject.templateSpectra if (x.isExcluded.get()==0)]:
            TemplateSum += template[0]*template[1]

        # So that np.divide cannot face a x/0.0 case, 
        # make sure that no TemplateSum index is 0.0!
        for i in range(0,len(TemplateSum)):
            if TemplateSum[i] == 0.0:
                TemplateSum[i] = 1.0
        
        np.divide(Experiment,TemplateSum,self.Residuals)


        x = np.array([x for x in range(0,len(Experiment))],dtype=int)
        y = self.Residuals
        x = np.ravel(list(zip(x,x+1)))
        y = np.ravel(list(zip(y,y)))

        self.a.plot(x,y,'k')[0]
        self.fig.canvas.draw()

    def GetMax(self):
        binlow = int(self.a.get_xlim()[0])
        binhigh= int(self.a.get_xlim()[1])
        maxarray=[100] #default value
        maxarray.append(max(np.sqrt([x**2 for x in self.Residuals if np.isfinite(x)])))
        return max(maxarray)

    def Autosize(self):
        if self.isLogPlot == True:
            self.a.set_ylim([-1.20*self.GetMax(),1.20*self.GetMax()])
        else:
            self.a.set_ylim([-1.20*self.GetMax(),1.20*self.GetMax()])
        self.fig.canvas.draw()

    def Resize(self):
        self.a.set_xlim(0,self.maximumX)
        if(self.isLogPlot==True):
            self.a.set_ylim([-1.20*self.GetMax(),1.20*self.GetMax()])
        else:                    
            self.a.set_ylim([-1.20*self.GetMax(),1.20*self.GetMax()])
        self.fig.canvas.draw()

    def yZoomIn(self):
        ylow,yhigh = self.a.get_ylim()
        if ylow < 0.0:
            newlow,newhigh = (0.8*ylow,0.8*yhigh)
        else:
            newlow,newhigh = (1.2*ylow,0.8*yhigh)

        self.a.set_ylim(newlow,newhigh)
        self.fig.canvas.draw()

    def yZoomOut(self):
        ylow,yhigh = self.a.get_ylim()
        if ylow < 0.0:
            newlow,newhigh = (1.2*ylow,1.2*yhigh)
        else:
            newlow,newhigh = (0.8*ylow,1.2*yhigh)

        self.a.set_ylim(newlow,newhigh)
        self.fig.canvas.draw()
