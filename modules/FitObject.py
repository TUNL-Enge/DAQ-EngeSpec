import numpy as np
import pandas as pd
from scipy import optimize
import os
from tkinter import *
from FitEvents import WriteTex

import numpy as np
import matplotlib.pyplot as plt
import seaborn
import ctypes
from ctypes import *

from time import clock
from time import gmtime,strftime

from TkFrames import ComparisonFrame
from TkFrames import MCMCFrame

import emcee
import corner


class SpectrumObject:
    def __init__(self,TupleArray=None,filename=None,Data=None):
        if( TupleArray is None and filename is None and Data is not None):

            self.Data = Data
            self.LineGraphics = []
            self.LowestBin    = 0
            self.HighestBin   = 0
            self.NumberOfBins = 0
            self.isExcluded=BooleanVar()

            self.fitArea   = 1.0
            self.pValue    = 1.0 
            self.pError    = 0.0
            self.plotFactor= 1.0


        else:
            self.filename=filename
            self.fig=None
            self.isBackground=BooleanVar()
            self.isBackground.set(0) #False
            self.isExcluded=BooleanVar()
            self.isExcluded.set(0) #False
            self.InitializationData=TupleArray
            self.Data = None 
            self.LineGraphics = []
            self.ViewScreen=None

            self.LowestBin    = 0
            self.HighestBin   = 0
            self.NumberOfBins = 0

            self.fitRange=(self.LowestBin,self.HighestBin)
            self.fitArea = 1.0
            self.BranchingRatio = 0.0
            self.BranchingRatioError = 0.0
            self.PartialNumberofReactions = 1.0
            self.pValue  = 1.0 
            self.pError  = 0.0
            self.HPD = None
            self.plotFactor=1.0

            self.IntegrityCheck()

    def IntegrityCheck(self):
        
        self.LowestBin =int(self.InitializationData[0][0])
        self.HighestBin= int(self.InitializationData[-1][0])
        self.NumberOfBins = len(self.InitializationData)
        #self.NumberOfBins = self.HighestBin - self.LowestBin + 1
        self.Data = np.empty(self.NumberOfBins)
        for num in range(0,len(self.Data)):
            self.Data[num]=self.InitializationData[num][1]

###############################################################

class FitObject:
    def __init__(self):
        self.templateSpectra     = []
        self.templateSpectraCopy = []
        self.ViewScreen=None
        self.FitStatus=False
        self.isDataSet=False
        self.isTemplateSet=False

        self.dataSpectrum = None

        self.lowerFitLimit = IntVar()
        self.upperFitLimit = IntVar()
        self.MCMCSamples = IntVar()
        self.MCMCSamples.set(10000)
        self.MCMCBurn = IntVar()
        self.MCMCBurn.set(1000)
        self.MCMCThin = IntVar()
        self.MCMCThin.set(1)

        self.NumberofSimulations = DoubleVar()
        self.NumberofSimulations.set(2.0E6)
        self.defaultFitLimits = (0,4095)
        self.ChiSquare=0
        self.NDOF=0

        self.slope=1.0
        self.intercept=0.0
      
        self.stream=None

        self.FitEngine = ctypes.CDLL('./lib/libMIN.so')
        self.FitEngine.GetLogLikelihood.restype  = c_double
        self.FitEngine.GetLogLikelihood.argtypes = [POINTER(c_double),c_int,c_int]
        self.FitEngine.GetParamValue.restype = c_double
        self.FitEngine.GetParamError.restype = c_double
        self.FitEngine.GetfIntegralData.restype = c_double
        self.FitEngine.GetfIntegralMCs.restype = c_double
        self.FitEngine.GetChiSquare.restype = c_double

        self.setFitLimits(0,4095)


###############################################################S

    def FitInit(self):
        if self.isDataSet==False or self.isTemplateSet==False:
            self.myWriteTex("Unable to initialize fit. Missing data or templates")
        if self.FitEngine.GetInitStatus()==1:
            self.FitEngine.setDataTemplate((c_double*len(self.dataSpectrum.Data)) (*self.dataSpectrum.Data),c_int(len(self.dataSpectrum.Data)))
            for template in [x for x in self.templateSpectra if x.isExcluded.get()==0]:
                template.HPD=None
                self.FitEngine.setMCTemplate((c_double*len(template.Data)) (*template.Data),c_int(len(template.Data)))
            self.FitEngine.setXRange(self.lowerFitLimit.get(),self.upperFitLimit.get())
            self.FitEngine.initFit()
            self.myWriteTex("Fitter initialized")
            self.myWriteTex("Attempting Fit from channel {0} to {1}".format(self.FitEngine.GetXlow(),self.FitEngine.GetXhigh()))
        else:
            self.FitEngine.UnSetFit()
            self.FitInit()

###############################################################S

    def Fit(self,notebook):
        self.FitInit()
        start=clock()
        self.FitStatus=True if (0==self.FitEngine.Fit()) else False
        if(self.FitStatus==True):
            end=clock()
            self.myWriteTex("Fit Successful, time elapsed: {0:.2f} s".format(end-start))
            self.dataSpectrum.fitArea = self.FitEngine.GetfIntegralData()
            self.ChiSquare = self.FitEngine.GetChiSquare()
            self.NDOF      = self.FitEngine.GetNDOF()
            num=0
            for template in [x for x in self.templateSpectra if x.isExcluded.get()==0]:
                template         = self.templateSpectra[num]
                template.pValue  = self.FitEngine.GetParamValue(c_int(num))
                template.pError  = self.FitEngine.GetParamError(c_int(num))
                template.fitArea = self.FitEngine.GetfIntegralMCs(c_int(num))
                template.plotFactor = (template.pValue*self.dataSpectrum.fitArea/(template.fitArea))
                num+=1
            self.ViewScreen.PlotTemplates()
            ComparisonFrame(notebook,self)
            self.WriteReport()
        else:
            self.myWriteTex("Fit Unsuccessful! Try adjusting fit limits or templates.")

###############################################################S
    def FitMCMCNew(self, notebook):

        def lnlike(theta):
            num_numbers = len(theta)
            array_type = c_double*num_numbers
            LogLikelihood = -self.FitEngine.GetLogLikelihood(array_type(*theta), c_int(num_numbers), c_int(0))
            return LogLikelihood

        def jeffreys_prior(theta):
            upper = 10.0
            lower = 1.0E-5
            pprob = 1.0
            for p_value in theta:
                if (p_value < upper) and (p_value > lower):
                    pprob = pprob*(-np.log(np.absolute(abs(p_value))))
                else:
                    return -np.inf
            return pprob

        def lnprob(theta):
            theta = abs(theta)
            lp  = jeffreys_prior(theta)
            if not np.isfinite(lp):
                return -np.inf
            else:
                return lp+lnlike(theta)

        self.FitInit()
    
        """ Initial Guesses """
        InitialGuesses = list()
        for template in [x for x in self.templateSpectra if x.isExcluded.get()==0]:
            InitialGuesses.append(1.0/len(self.templateSpectra))

        nll = lambda *args: -lnlike(*args)
        InitialResults = optimize.minimize(nll, InitialGuesses)['x']
        """ Fit parameters. These will need to be tested! """
        ndim, nwalkers = len(InitialGuesses), 50
        nsteps = self.MCMCSamples.get()
        nburn  = self.MCMCBurn.get()
        thin   = self.MCMCThin.get()

        """ Initial positions """
        pos_init = np.absolute([np.array(InitialResults)])[0] # for i in range(nwalkers)])


        #for parameter_set in pos_init:
        #    for parameter in parameter_set:
        #        if parameter < 1.0E-5:
        #            parameter = 1.0E-2
                

        #sampler = emcee.EnsembleSampler(nwalkers,ndim,lnprob)

        cov = 5.e-8 * np.identity(n=len([x for x in self.templateSpectra if x.isExcluded.get()==0]))
        sampler2 = emcee.MHSampler(cov,ndim,lnprob)

        for i, result in enumerate(sampler2.sample(pos_init,iterations=nsteps)):
            if (i+1) % int(nsteps/20.0) == 0:
                print(("{0:5.1%} complete".format(float(i)/nsteps)))
                self.myWriteTex("{0:5.1%} complete".format(float(i)/nsteps))

        samples = sampler2.chain[nburn::thin,:].reshape((-1,ndim))
        #samples = sampler2.chain[:,nburn::thin,:].reshape((-1,ndim))

        """ need to update """
            
        stat_lambda = lambda v: (v[1],v[2]-v[1])

        """ template_index """

        t_index = 0
        self.dataSpectrum.fitArea=self.FitEngine.GetfIntegralData()
        pValue_MAP = list()

        for template in [x for x in self.templateSpectra if x.isExcluded.get()==0]:
            template.pValue, template.pError = stat_lambda(np.percentile(samples[:,t_index],[16,50,84],axis=0))
            pValue_MAP.append(template.pValue)
            """ going to need an upper-limit case """
            template.fitArea = self.FitEngine.GetfIntegralMCs(c_int(t_index))
            template.plotFactor = (template.pValue*self.dataSpectrum.fitArea/(template.fitArea))
            print(("{0}:\t{1}".format(template.filename,template.fitArea)))
            t_index = t_index + 1

        """ Using MAP values, calculate ChiSquare """
        num_numbers = len(pValue_MAP)
        array_type = c_double*num_numbers

        """ Force calculation of ChiSquare """
        self.FitEngine.GetLogLikelihood(array_type(*pValue_MAP), c_int(num_numbers), c_int(2))

        self.ChiSquare = self.FitEngine.GetChiSquare()
        self.NDOF      = self.FitEngine.GetNDOF()
        self.dataSpectrum.fitArea=self.FitEngine.GetfIntegralData()

        """ end list """

        t_index = 0

        for template in [x for x in self.templateSpectra if x.isExcluded.get()==0]:
            perc = np.percentile(samples[:,t_index],[16,50,84])

            fig  = plt.figure(figsize=(10,5))
            grid = plt.GridSpec(2,5,hspace=0.2,wspace=0.2)

            main_hist  = fig.add_subplot(grid[:,3:])
            trace_plot = fig.add_subplot(grid[0,:2])

            trace_plot.plot(samples[:,t_index])

            main_hist.hist(samples[:,t_index],40,histtype='stepfilled')
            ycoords = main_hist.get_ylim()
            main_hist.plot([perc[1],perc[1]],ycoords,color='k',lw=1.0,ls='dashed')
            main_hist.plot([perc[0],perc[0]],ycoords,color='k',lw=1.0,ls='dashdot')
            main_hist.plot([perc[2],perc[2]],ycoords,color='k',lw=1.0,ls='dashdot')

            plt.savefig("{0}.png".format(template.filename))

            t_index = t_index + 1


        self.ViewScreen.PlotTemplates()
        self.WriteReport()

        ComparisonFrame(notebook,self)
        MCMCFrame(notebook,self)

    def WriteReport(self):
        TotalNumberofReactions = 0
        TotalNumberofReactionsError = 0
        for template in [x for x in self.templateSpectra if (x.isExcluded.get()==0 and x.isBackground.get()==0)]:
            template.PartialNumberofReactions      =  template.pValue*self.dataSpectrum.fitArea/(template.fitArea/self.NumberofSimulations.get())
            template.PartialNumberofReactionsError =  template.pError*self.dataSpectrum.fitArea/(template.fitArea/self.NumberofSimulations.get())
            TotalNumberofReactions += template.PartialNumberofReactions
        TotalNumberofReactionsError=np.sqrt(sum([(x.PartialNumberofReactionsError)**2 for x in self.templateSpectra if (x.isExcluded.get()==0 and x.isBackground.get()==0)]))
        for template in [x for x in self.templateSpectra if (x.isExcluded.get()==0 and x.isBackground.get()==0)]:
            template.BranchingRatio = 100*template.PartialNumberofReactions/TotalNumberofReactions
        for template in [x for x in self.templateSpectra if (x.isExcluded.get()==0 and x.isBackground.get()==0)]:
            template.BranchingRatioError =  100*np.sqrt( (1/TotalNumberofReactions)**2*(template.PartialNumberofReactionsError)**2 + (-template.PartialNumberofReactions/(TotalNumberofReactions**2))**2 * (TotalNumberofReactionsError)**2)

        self.myWriteTex("% % % % % % % % % % % % % % % % %")
        self.myWriteTex("")
        self.myWriteTex("Fit Results (Branching Ratios)")
        self.myWriteTex("")
        self.myWriteTex("                                                 B       dB")
        for template in [x for x in self.templateSpectra if (x.isExcluded.get()==0 and x.isBackground.get()==0)]:
            if(template.HPD is not None): # MCMC! !=
                if(template.HPD[1]/template.HPD[0] > 10.00): #LOWER-LIMIT ==0.0
                    self.myWriteTex("{0:40}:\t< {1:8.2f}".format(template.filename,template.BranchingRatio))
                else:
                    self.myWriteTex("{0:40}:\t{1:8.2f}\t{2:8.2f}".format(template.filename,template.BranchingRatio,template.BranchingRatioError))
            else: #REGULAR MINUIT
                self.myWriteTex("{0:40}:\t{1:8.2f}\t{2:8.2f}".format(template.filename,template.BranchingRatio,template.BranchingRatioError))

        self.myWriteTex("")
        self.myWriteTex("% % % % % % % % % % % % % % % % %")
        self.myWriteTex("")
        self.myWriteTex("Data Filename        : {0}".format(self.dataSpectrum.filename.split("/")[-1]))
        self.myWriteTex("Fit Limits           : {0},{1}".format(self.lowerFitLimit.get(),self.upperFitLimit.get()))
        self.myWriteTex("Reduced ChiSquare    : {0:.2f}".format(self.ChiSquare/self.NDOF))
        self.myWriteTex("Total Reaction Number: {0:.0f} ({1:.0f})".format(TotalNumberofReactions,TotalNumberofReactionsError))
        self.myWriteTex("")
        self.myWriteTex("% % % % % % % % % % % % % % % % %")

###############################################################S

    def setFitLimits(self,low,high):
        if( (high > low) and (low >= 0)):
            self.lowerFitLimit.set(int(low))
            self.upperFitLimit.set(int(high))
            self.myWriteTex("new fit limits: {0}\t-->\t{1}".format(self.lowerFitLimit.get(),self.upperFitLimit.get()))
        else:
            self.myWriteTex("error setting fit limits")
            
###############################################################

    def myWriteTex(self,string):
        WriteTex(self.stream,string)

###############################################################

    def setDefaultFitLimits(self):
        self.setFitLimits(self.defaultFitLimits[0],self.defaultFitLimits[1])

###############################################################

    def ReadFile(self,filename):
        TempHistogramArray=[]
        input = open(filename,"r")
        for line in input:
            Bin,Content=line.split()
            TempHistogramArray.append( (float(Bin),float(Content)) )
        input.close()
        return TempHistogramArray

###############################################################

    def PopTemplates(self,dirname):
        if self.isTemplateSet==True:
            self.myWriteTex("replacing templates...")
            for Template in self.templateSpectra:
                for line in Template.LineGraphics:
                    if line in self.ViewScreen.a.lines:
                        line.remove()
                    line=None
            self.templateSpectra=[]
            self.templateSpectraCopy=[]
            self.myWriteTex("\told templates removed!")
            self.FitStatus = False
            self.isTemplateSet=False
        for filename in os.listdir(dirname):
            if filename[0] != ".":
                self.templateSpectra.append(SpectrumObject(self.ReadFile(dirname+'/'+filename),filename))
                self.templateSpectraCopy.append(SpectrumObject(self.ReadFile(dirname+'/'+filename),filename))
                self.isTemplateSet=True
                self.myWriteTex("new template set: [{0}] {1}".format(len(self.templateSpectra),filename.split("/")[-1]))
        if self.isDataSet==True:
            self.defaultFitLimits=(self.dataSpectrum.LowestBin,self.dataSpectrum.HighestBin)
        else:
            self.defaultFitLimits=(self.templateSpectra[0].LowestBin,self.templateSpectra[0].HighestBin)

###############################################################

    def PopData(self,filename):
        print(filename)
        if self.isDataSet == True:
            print("Warning: Replacing Data")
            for line in self.dataSpectrum.LineGraphics:
                if line in self.ViewScreen.a.lines:
                    line.remove()
                self.dataSpectrum.LineGraphics.remove(line)
        else:
            self.isDataSet = True
        self.dataSpectrum = SpectrumObject(self.ReadFile(filename),filename)
        self.myWriteTex("new data file set: {0}".format(filename.split("/")[-1]))
        


def main():
    myFitObject = FitObject()
    myFitObject.PopData("./spectra/435_SING_data/onRes_SING.dat") 
    myFitObject.PopTemplate("./spectra/435_SING_data/onRes_SING.dat") 
if __name__ == '__main__':
    main()
