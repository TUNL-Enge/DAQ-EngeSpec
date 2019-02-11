from tkinter import *
from Events import SelectRange
from Events import chooseROI
import FitObject
from ViewScreen import ViewScreen,ResidualsScreen
import copy
import numpy as np
from PIL import Image,ImageTk
class bkFrame(Frame):
    def __init__(self,master,FitObject,**kwargs):
        self.bigFrame=Frame(master)
        Frame.__init__(self,self.bigFrame,width=280,height=200,**kwargs)

        self.separator = Frame(self.bigFrame, height=2, bd=1, width=280, relief=SUNKEN)
        self.separator.grid(row=0, column=0)

        self.grid(row=1,column=0)
       
        Label(self, text="Check boxes to indicate background sources").grid(column=0,row=0,columnspan=2,rowspan=1)
        Frame(self, height=2, bd=1, width=280, relief=SUNKEN).grid(column=0,columnspan=2,row=2)
        for par in range(0,len(FitObject.templateSpectra)):
            Template=FitObject.templateSpectra[par]
            Label(self, text="{0}".format(Template.filename),justify=LEFT).grid(column=0,columnspan=1,row=2*par+4)
            Checkbutton(self, text="bkgd?", variable=Template.isBackground).grid(column=1,columnspan=1,row=2*par+4)

        parent_name = self.bigFrame.winfo_parent()
        parent = self.bigFrame._nametowidget(parent_name)
        Button(self,text="OK",command= lambda : parent.destroy() ).grid(row=len(FitObject.templateSpectra)*2+3,column=0,columnspan=2,sticky=W+E)
            
    def grid_(self, **kwargs): ######## grid -> grid_
        self.bigFrame.grid(**kwargs)
        
class ToolFrame(Frame):
    def __init__(self,parent,**kwargs):
        self.ViewScreen=None
        self.parent=parent
        Frame.__init__(self,self.parent,height=40,width=1200,bg='gray',relief=GROOVE,borderwidth=2,**kwargs)
        self.pack(fill=X,side=TOP,expand=0) 
    
        Button(self,text="log-lin",command= lambda : self.ViewScreen.ToggleLog(),highlightbackground='gray' ).pack(side=LEFT,fill=X,expand=1)
        Button(self,text="Resize",command= lambda  :  self.ViewScreen.Resize(),highlightbackground='gray' ).pack(side=LEFT,fill=X,expand=1)
        Button(self,text="X range",command= lambda : SelectRange(self.ViewScreen),highlightbackground='gray'  ).pack(side=LEFT,fill=X,expand=1)
        Button(self,text="X zoom-in",command= lambda  :  self.ViewScreen.xZoomIn(),highlightbackground='gray'  ).pack(side=LEFT,fill=X,expand=1)
        Button(self,text="X zoom-out",command= lambda : self.ViewScreen.xZoomOut(),highlightbackground='gray'  ).pack(side=LEFT,fill=X,expand=1)
        Button(self,text="Y zoom-in",command= lambda  :  self.ViewScreen.yZoomIn(),highlightbackground='gray').pack(side=LEFT,fill=X,expand=1)
        Button(self,text="Y zoom-out",command= lambda : self.ViewScreen.yZoomOut(),highlightbackground='gray').pack(side=LEFT,fill=X,expand=1)
        Button(self,text="Auto range",command= lambda : self.ViewScreen.Autosize(),highlightbackground='gray').pack(side=LEFT,fill=X,expand=1)

class OptionsFrame(Frame):
    def __init__(self,parent,ViewScreen,FitObject,**kwargs):
        self.FitObject=FitObject
        self.parent = parent
        self.MainViewScreen=ViewScreen
        Frame.__init__(self,self.parent,height=300,width=280,bg='gray',relief=GROOVE,borderwidth=2,**kwargs)

        text=Text(self,height=4,width=40)
        text.pack(fill=BOTH,side=TOP,expand=1)
        text.insert(END,"Specify below which templates correspond to background sources, i.e., signals not originating from the reaction of interest. You may also exclude templates from the fitting routine. Choose \"customize\" to make minor adjustments in resolution or peak position.")
        text.config(state=DISABLED)
        self.subFrame=Frame(self,width=280,bg='gray',relief=GROOVE,borderwidth=2)
            

        for num in range(0,len(self.FitObject.templateSpectra)):
            template=self.FitObject.templateSpectra[num]
            Label(self.subFrame,text=template.filename,anchor=W,bg='#ff9999' if (template.isExcluded.get()==1) else 'gray').grid(row=num,column=0)
            Checkbutton(self.subFrame,text="Remove?",variable=template.isExcluded,bg='gray').grid(row=num,column=1)
            Checkbutton(self.subFrame,text="BKGD?",variable=template.isBackground,bg='gray').grid(row=num,column=2)
            Button(self.subFrame,text="Customize",bg='gray',command=lambda x=copy.deepcopy(num) : self.LaunchCustomize(self.MainViewScreen,FitObject,x),highlightbackground='gray').grid(row=num,column=3)
        self.subFrame.pack(fill=X,side=TOP,expand=0)

    def grid_(self, **kwargs): ######## grid -> grid_
        self.grid(**kwargs)

    def LaunchCustomize(self,ViewScreen,FitObject,num):
        toplevel = Toplevel(takefocus=True)
        toplevel.title("{0}".format(FitObject.templateSpectra[num].filename))
        toplevel.grab_set()
        customwin = CustomizeFrame(toplevel,ViewScreen,FitObject,num)
        customwin.grid_(row=0,column=0,sticky=W+E,pady=20)


class CalibrationFrame(Frame):
    def __init__(self,parent,FitObject,**kwargs):
        self.FitObject=FitObject
        self.parent = parent

        Frame.__init__(self,self.parent,height=300,width=280,bg='gray',relief=GROOVE,borderwidth=2,**kwargs)

        self.slopeStr=StringVar()
        self.interceptStr=StringVar()
        self.UpdateStrVar()

        text=Text(self,height=4,width=40)
        text.pack(fill=BOTH,side=TOP,expand=1)
        text.insert(END,"By default, the spectrum is shown with channel number on the x-axis. To switch to energy units, enter the channel-energy pairs below. Alternatively, enter the known slope and intercept. This assumes linear energy dependence")
        text.config(state=DISABLED)
        self.subFrame=Frame(self,bg='gray',relief=GROOVE,borderwidth=2,padx=10,pady=20)
        

        Label(self.subFrame,text="Energy",bg='gray',padx=10).grid(row=0,column=0,sticky=W)
        Label(self.subFrame,text="Energy",bg='gray',padx=10).grid(row=1,column=0,sticky=W)
        
        self.e1=Entry(self.subFrame)
        self.e1.grid(row=0,column=1)
        self.e2=Entry(self.subFrame)
        self.e2.grid(row=1,column=1)

        Label(self.subFrame,text="Channel",bg='gray',padx=10).grid(row=0,column=2,sticky=W)
        Label(self.subFrame,text="Channel",bg='gray',padx=10).grid(row=1,column=2,sticky=W)

        self.b1=Entry(self.subFrame)
        self.b1.grid(row=0,column=3)
        self.b2=Entry(self.subFrame)
        self.b2.grid(row=1,column=3)

        self.e1.insert(0,"1460.822")
        self.e2.insert(0,"2614.511")
        self.b1.insert(0,"698")
        self.b2.insert(0,"1164")
      

        Button(self.subFrame,text="set calibration",padx=10,pady=10,command=lambda: self.setCalibration(),highlightbackground='gray').grid(row=4,column=0,columnspan=2,sticky=W+E)
        Label(self.subFrame,bg='gray').grid(row=5) 
        Label(self.subFrame,text="Slope",bg='gray',padx=10).grid(row=6,column=0,sticky=W)
        Label(self.subFrame,text="Intercept",bg='gray',padx=10).grid(row=7,column=0,sticky=W)
        
        self.slope=Entry(self.subFrame)
        self.slope.grid(row=6,column=1,sticky=E)
        self.intercept=Entry(self.subFrame)
        self.intercept.grid(row=7,column=1,sticky=E)
        self.slope.insert(0,"1.0")
        self.intercept.insert(0,"0.0")
        Button(self.subFrame,text="set slope/intercept",padx=10,pady=10,command=lambda: self.setLine(),highlightbackground='gray').grid(row=8,column=0,columnspan=2,sticky=W+E)

        Label(self.subFrame,text="Slope",bg='gray',padx=10).grid(row=10,column=0,sticky=W)
        Label(self.subFrame,text="Intercept",bg='gray',padx=10).grid(row=11,column=0,sticky=W)
        Label(self.subFrame,textvariable=self.slopeStr,bg='gray').grid(row=10,column=1,sticky=E)
        Label(self.subFrame,textvariable=self.interceptStr,bg='gray').grid(row=11,column=1,sticky=E)

        self.subFrame.grid_columnconfigure(0,weight=1)
        self.subFrame.pack(fill=BOTH,side=TOP,expand=1)

    def grid_(self, **kwargs): ######## grid -> grid_
        self.grid(**kwargs)

    def setCalibration(self):
        self.FitObject.slope= (float(self.e2.get()) - float(self.e1.get()))/(float(self.b2.get()) -float(self.b1.get()))
        self.FitObject.intercept=(float(self.e1.get())-self.FitObject.slope*float(self.b1.get()))
        self.UpdateStrVar()
    def setLine(self):
        self.FitObject.slope=(float(self.slope.get()))
        self.FitObject.intercept=(float(self.intercept.get()))
        self.UpdateStrVar()
    def UpdateStrVar(self):
        self.slopeStr.set("{0:.2f}".format(self.FitObject.slope))
        self.interceptStr.set("{0:.2f}".format(self.FitObject.intercept))

class FitParametersFrame(Frame):
    def __init__(self,parent,FitObject,**kwargs):
        self.FitObject=FitObject
        self.parent = parent

        Frame.__init__(self,self.parent,height=300,width=280,bg='gray',relief=GROOVE,borderwidth=2,**kwargs)


        text=Text(self,height=4,width=40)
        text.pack(fill=BOTH,side=TOP,expand=1)
        text.insert(END,""" The MCMC (Markov Chain Monte Carlo) routine is handled by the PYMC framework. Parameters for the sampling routine are set here. For a chain of length i (iterations), the first j samples (burn-in) are discarded. Of the remaining samples, only every kth (the thinning interval) sample is kept.""")
        text.config(state=DISABLED)
        self.subFrame=Frame(self,bg='gray',relief=GROOVE,borderwidth=2,padx=10,pady=20)
        
        self.MCMCSamples_current=StringVar()
        self.MCMCBurn_current=StringVar()
        self.MCMCThin_current=StringVar()
        

        Label(self.subFrame,text="Iterations",bg='gray',padx=10).grid(row=0,column=0,sticky=W)
        Label(self.subFrame,text="Burn-In",bg='gray',padx=10).grid(row=1,column=0,sticky=W)
        Label(self.subFrame,text="Thinning Interval",bg='gray',padx=10).grid(row=2,column=0,sticky=W)
        
        self.e1=Entry(self.subFrame)
        self.e1.grid(row=0,column=1)
        self.e2=Entry(self.subFrame)
        self.e2.grid(row=1,column=1)
        self.e3=Entry(self.subFrame)
        self.e3.grid(row=2,column=1)


        self.UpdateStrVar()

        self.e1.insert(0,"{0}".format(self.FitObject.MCMCSamples.get()))
        self.e2.insert(0,"{0}".format(self.FitObject.MCMCBurn.get()))
        self.e3.insert(0,"{0}".format(self.FitObject.MCMCThin.get()))
      

        Button(self.subFrame,text="set parameters",padx=10,pady=10,command=lambda: self.setCalibration(),highlightbackground='gray').grid(row=4,column=0,columnspan=1,sticky=W+E)
        Label(self.subFrame,bg='gray').grid(row=5) 

        Label(self.subFrame,text="Interations",bg='gray',padx=10).grid(row=10,column=0,sticky=W)
        Label(self.subFrame,text="Burn-in",bg='gray',padx=10).grid(row=11,column=0,sticky=W)
        Label(self.subFrame,text="Thinning",bg='gray',padx=10).grid(row=12,column=0,sticky=W)
        Label(self.subFrame,textvariable=self.MCMCSamples_current,bg='gray').grid(row=10,column=1,sticky=E)
        Label(self.subFrame,textvariable=self.MCMCBurn_current,bg='gray').grid(row=11,column=1,sticky=E)
        Label(self.subFrame,textvariable=self.MCMCThin_current,bg='gray').grid(row=12,column=1,sticky=E)

        self.subFrame.grid_columnconfigure(0,weight=1)
        self.subFrame.pack(fill=BOTH,side=TOP,expand=1)

    def grid_(self, **kwargs): ######## grid -> grid_
        self.grid(**kwargs)

    def setCalibration(self):
        self.FitObject.MCMCSamples.set( (int(self.e1.get())))
        self.FitObject.MCMCBurn.set( (int(self.e2.get())))
        self.FitObject.MCMCThin.set( (int(self.e3.get())))
        self.UpdateStrVar()
    def UpdateStrVar(self):
        self.MCMCSamples_current.set("{0}".format(int(self.FitObject.MCMCSamples.get())))
        self.MCMCBurn_current.set("{0} ({1:.1f}%)".format(int(self.FitObject.MCMCBurn.get()),(100*float(self.FitObject.MCMCBurn.get())/self.FitObject.MCMCSamples.get()) ))
        self.MCMCThin_current.set("{0}".format(int(self.FitObject.MCMCThin.get())))

class CustomizeFrame(Frame):
    def __init__(self,parent,MainViewScreen,MainFitObject,num, **kwargs):
        self.FitObject=FitObject.FitObject()
        self.parent = parent
        self.MainViewScreen=MainViewScreen
        self.MainFitObject=MainFitObject
        self.templatenum=num

        Frame.__init__(self,self.parent,height=300,width=280,bg='gray',relief=GROOVE,borderwidth=2,**kwargs)

        self.controlFrame =Frame(self,height=440,width=250,relief=RAISED,borderwidth=2,cursor='cross')
        self.plotFrame   = Frame(self,height=400,width=500,relief=RAISED,borderwidth=2,cursor='cross')

        self.controlFrame.pack(fill=Y,side=LEFT,expand=1)
        self.plotFrame.pack(fill=BOTH,side=TOP)
        self.toolFrame   = ToolFrame(self)

        # Everything in python is a reference, similar to pointers in C. While trying to implement the customize frame, 
        # I found that it was absolutely maddening trying to copy and plot, since the new (copied) object would contain 
        # all the same pointers as the original, thereby making any temporary customization VERY difficult to implement, since these
        # changes would occur in the original object. So now there is a new constructor for Spectrum Objects that allow 
        # the user to set the data directely, instead of through a tuple and filename pair.

        if(self.MainFitObject.isDataSet==True):
            self.FitObject.isDataSet=True
            self.FitObject.dataSpectrum=FitObject.SpectrumObject(None,None,copy.copy(self.MainFitObject.dataSpectrum.Data))


        for Template in self.MainFitObject.templateSpectra:
            self.FitObject.templateSpectra.append(FitObject.SpectrumObject(None,None,copy.copy(Template.Data)))
            self.FitObject.templateSpectraCopy.append(FitObject.SpectrumObject(None,None,copy.copy(Template.Data)))

        self.myViewScreen=ViewScreen(self.plotFrame,self.FitObject)
        self.toolFrame.ViewScreen = self.myViewScreen 

        self.ResValue=1.0
        self.ResValueStr=StringVar()
        self.ResValueStr.set("{0:.2f}".format(self.ResValue))

        if(self.FitObject.isDataSet == True):
            self.myViewScreen.PlotData()
        self.myViewScreen.PlotSingleTemplate(self.FitObject.templateSpectraCopy[self.templatenum])

        Button(self.controlFrame,text="Select ROI",command= lambda : chooseROI(self.myViewScreen,self.templatenum)).grid(row=0,column=0,columnspan=2,sticky=W+E)

        Label(self.controlFrame,text="Sigma").grid(row=1,column=0,sticky=W+E)
        self.e1 = Entry(self.controlFrame,textvariable=self.ResValueStr,width=6)
        self.e1.grid(row=1,column=1,columnspan=1,sticky=W)
        self.controlFrame.grid_columnconfigure(0,weight=1)
        self.controlFrame.grid_columnconfigure(1,weight=1)


        Button(self.controlFrame,text="Apply Resolution",command= lambda : self.ApplyResolution()).grid(row=2,column=0,columnspan=2,sticky=W+E)
        Button(self.controlFrame,text=" <-- ",command= lambda : self.TranslateROI(-1)).grid(row=3,column=0,columnspan=1,sticky=W+E)
        Button(self.controlFrame,text=" -->",command= lambda : self.TranslateROI(+1)).grid(row=3,column=1,columnspan=1,sticky=W+E)
        Button(self.controlFrame,text="Clear ROI",command= lambda : self.ClearROI()).grid(row=4,column=0,columnspan=2,sticky=W+E)

        Label(self.controlFrame).grid(row=6,column=0)
        Label(self.controlFrame).grid(row=7,column=0)
        # So that we can close the Toplevel window, this was giving me some difficulty
        # http://stackoverflow.com/questions/12892180/how-to-get-the-name-of-the-master-frame-in-tkinter
        #parent_name = self.parent.winfo_parent()
        #parent = self.parent._nametowidget(parent_name)
        Button(self.controlFrame,text="Save Changes",command= lambda : self.SaveChanges()).grid(row=8,column=0,columnspan=2,sticky=W+E)
        Button(self.controlFrame,text="Cancel",command= lambda : self.parent.destroy() ).grid(row=9,column=0,columnspan=2,sticky=W+E)

    def grid_(self,**kwargs):
        self.grid(**kwargs)

    def ClearROI(self):
        if(self.myViewScreen.ROIRegister!=None):
            if(self.myViewScreen.ROIRegister.fill in self.myViewScreen.a.collections):
                self.myViewScreen.ROIRegister.fill.remove()
            self.myViewScreen.ROIRegister=None
            self.myViewScreen.fig.canvas.draw()

    def SaveChanges(self):
        self.MainFitObject.templateSpectra[self.templatenum].Data=copy.copy(self.FitObject.templateSpectraCopy[self.templatenum].Data)
        self.MainViewScreen.PlotTemplates()
        self.parent.destroy()

    def TranslateROI(self,direction):
        ROI=self.myViewScreen.ROIRegister
        if (ROI == None):
            return
        TargetTemplate = self.FitObject.templateSpectraCopy[self.templatenum]
        newtemplate = np.zeros_like(TargetTemplate.Data,dtype=None)
        translateUnit = direction
        xlow  = self.myViewScreen.ROIRegister.xLOW
        xhigh = self.myViewScreen.ROIRegister.xHIGH
        delX = xhigh - xlow
        delY = TargetTemplate.Data[xhigh] - TargetTemplate.Data[xlow]
        slope = delY/delX
        intercept = TargetTemplate.Data[xhigh] - slope*xhigh 
        for bins in range(xlow,xhigh+1):
            DIFF = TargetTemplate.Data[bins]-((slope*bins+intercept))
            GAUSS = DIFF if (DIFF>0) else 0
            newtemplate[bins] += TargetTemplate.Data[bins] - GAUSS
            newtemplate[bins+translateUnit] += GAUSS
        TargetTemplate.Data[xlow:xhigh+1] = newtemplate[xlow:xhigh+1]
        self.myViewScreen.PlotSingleTemplate(TargetTemplate)
        
        ROI.fill.remove()
        ROI.xZ = np.array([ xx for xx in range(ROI.xLOW,ROI.xLOW+len(TargetTemplate.Data[ROI.xLOW:ROI.xHIGH]))],dtype=int)
        ROI.yZ = np.array(TargetTemplate.Data[ROI.xLOW:ROI.xHIGH],dtype=float)
        ROI.fill = ROI.ViewScreen.a.fill_between(x = np.ravel(list(zip(ROI.xZ,ROI.xZ+1))),y1=np.ravel(list(zip(ROI.yZ,ROI.yZ))),y2=0.01,interpolate=True,color='#33ff33')

        self.myViewScreen.fig.canvas.draw()

    def ApplyResolution(self):
        self.ResValueStr.set(self.e1.get())
        sigma = float(self.ResValueStr.get()) if (float(self.ResValueStr.get())) else 0.1
        ROI=self.myViewScreen.ROIRegister
        if (ROI == None):
            return

        TargetTemplate = copy.copy(self.FitObject.templateSpectra[self.templatenum].Data)
        NewTemplate = np.zeros_like(TargetTemplate,dtype=None)

        xlow  = self.myViewScreen.ROIRegister.xLOW
        xhigh = self.myViewScreen.ROIRegister.xHIGH
        
        delX = xhigh - xlow
        delY = TargetTemplate[xhigh] - TargetTemplate[xlow]
        slope = delY/delX
        intercept = TargetTemplate[xhigh] - slope*xhigh 
        for bins in range(xlow,xhigh+1):
            temparray=[]
            temphisto=[]
            DIFF = TargetTemplate[bins]-((slope*bins+intercept))
            GAUSS = DIFF if (DIFF>0) else 0
           

            temparray = np.array(np.random.normal(0,sigma,int(GAUSS)),dtype=float)
            
            
            
            
            temphisto = np.histogram(temparray,bins=41,range=(-20,21))[0]
            for relbin in range(-20,21):
                if ((bins+relbin) < 0) or ((bins+relbin) > 4096):
                    break
                NewTemplate[bins+relbin] += temphisto[20+relbin]

            NewTemplate[bins] += TargetTemplate[bins]-GAUSS
        TargetTemplate[xlow:xhigh+1] = NewTemplate[xlow:xhigh+1]

        self.FitObject.templateSpectraCopy[self.templatenum].Data = copy.copy(TargetTemplate)
        self.myViewScreen.PlotSingleTemplate(self.FitObject.templateSpectraCopy[self.templatenum])
        
        if(ROI.fill in ROI.ViewScreen.a.collections):
            ROI.fill.remove()
        ROI.xZ = np.array([ xx for xx in range(ROI.xLOW,ROI.xLOW+len(TargetTemplate[ROI.xLOW:ROI.xHIGH]))],dtype=int)
        ROI.yZ = np.array(TargetTemplate[ROI.xLOW:ROI.xHIGH],dtype=float)
        ROI.fill = ROI.ViewScreen.a.fill_between(x = np.ravel(list(zip(ROI.xZ,ROI.xZ+1))),y1=np.ravel(list(zip(ROI.yZ,ROI.yZ))),y2=0.01,interpolate=True,color='#33ff33')

        self.myViewScreen.fig.canvas.draw()

class ComparisonFrame(Frame):
    def __init__(self,parent,MainFitObject,**kwargs):
        self.parent = parent
        self.MainFitObject=MainFitObject
        self.FitObject=FitObject.FitObject()

        if(self.MainFitObject.isDataSet==True):
            self.FitObject.isDataSet=True
            self.FitObject.dataSpectrum=FitObject.SpectrumObject(None,None,copy.copy(self.MainFitObject.dataSpectrum.Data))

        for Template in self.MainFitObject.templateSpectra:

            self.FitObject.templateSpectra.append(FitObject.SpectrumObject(None,None,copy.copy(Template.Data)))
            self.FitObject.templateSpectraCopy.append(FitObject.SpectrumObject(None,None,copy.copy(Template.Data)))
            self.FitObject.templateSpectra[-1].isExcluded.set( Template.isExcluded.get()   )
            self.FitObject.templateSpectra[-1].plotFactor = Template.plotFactor

        for tab in self.parent.tabs():
            if(self.parent.tab(tab,"text")=="Residuals Plot"):
                self.parent.forget(tab)

        Frame.__init__(self,self.parent,height=300,width=280,bg='gray',relief=GROOVE,borderwidth=2,**kwargs)
        self.parent.add(self,text="Residuals Plot")

        self.plotFrame   = Frame(self,height=400,width=500,relief=RAISED,borderwidth=2,cursor='cross')
        self.toolFrame   = ToolFrame(self)
        self.plotFrame.pack(fill=BOTH,side=TOP)

        self.myViewScreen=ResidualsScreen(self.plotFrame,self.FitObject)
        self.toolFrame.ViewScreen = self.myViewScreen 


        self.myViewScreen.PlotResiduals()
        self.parent.select(self)



    def grid_(self,**kwargs):
        self.grid(**kwargs)

class MCMCFrame(Frame):
    def __init__(self,parent,MainFitObject,**kwargs):
        self.parent = parent
        self.MainFitObject=MainFitObject
        self.FitObject=FitObject.FitObject()

        self.images = []
        self.display = []

        for tab in self.parent.tabs():
            if(self.parent.tab(tab,"text")=="MCMC Diagnostics"):
                self.parent.forget(tab)

        Frame.__init__(self,self.parent,height=300,width=280,bg='gray',relief=GROOVE,borderwidth=2,**kwargs)
        self.parent.add(self,text="MCMC Diagnostics")

        for template in [x for x in self.MainFitObject.templateSpectra if (x.isExcluded.get()==0)]:
            self.images.append(Image.open("{0}.png".format(template.filename)))
        for image in self.images:
            image.thumbnail((700,700),Image.ANTIALIAS)
            self.display.append(ImageTk.PhotoImage(image))


        self.n=0

        self.label=Label(self,image=self.display[self.n])
        self.label.pack(fill=BOTH,expand=0)

        Button(self,text="back",command=self.back).pack(side=LEFT,fill=X,expand=1)
        Button(self,text="next",command=self.__next__).pack(side=LEFT,fill=X,expand=1)

        self.parent.select(self)
    def __next__(self):
        self.n += 1
        self.n = self.n % len(self.images)

        self.label.configure(image=self.display[self.n])
        self.label.image=self.display[self.n]
    def back(self):
        self.n -= 1
        self.n = self.n % len(self.images)

        self.label.configure(image=self.display[self.n])
        self.label.image=self.display[self.n]

    def grid_(self,**kwargs):
        self.grid(**kwargs)

