from FitObject import *
from TkFrames import bkFrame
from TkFrames import OptionsFrame
from TkFrames import CalibrationFrame
from TkFrames import FitParametersFrame
from tkinter import *
from tkinter.ttk import Notebook
import tkinter.filedialog

def LaunchOptions(ViewScreen,FitObject):
    toplevel = Toplevel(takefocus=True)
    toplevel.title("Options")
    note=Notebook(toplevel)
    tab1 = OptionsFrame(note,ViewScreen,FitObject)
    tab1.grid_(row=0,column=0,sticky=W+E,pady=20)
    tab2 = CalibrationFrame(note,FitObject)
    tab2.grid_(row=0,column=0,sticky=W+E,pady=20)
    tab3 = FitParametersFrame(note,FitObject)
    tab3.grid_(row=0,column=0,sticky=W+E,pady=20)
    note.add(tab1,text="Template Adjustment")
    note.add(tab2,text="Energy Calibration")
    note.add(tab3,text="MCMC Parameters")
    note.pack()



def DefineBackground(FitObject):
    toplevel = Toplevel(takefocus=True)
    toplevel.title("Define Templates")
    bkgd_define = bkFrame(toplevel,FitObject)
    bkgd_define.grid_(row=0,column=0,sticky=W+E,pady=20)

def ReadLimitEntries(ViewScreen,limitA,limitB):
    ViewScreen.fitObject.setFitLimits(int(limitA.get()),int(limitB.get()))
    ViewScreen.DrawFitLimits()

def LoadTemplates(ViewScreen,FitObject,StrVar):
    dirname = tkinter.filedialog.askdirectory(initialdir="./")
    if dirname != '':
        FitObject.PopTemplates(dirname)
        StrVar.set(dirname.split("/")[-1])
        ViewScreen.PlotTemplates()
    LaunchOptions(ViewScreen,FitObject)


def LoadData(ViewScreen,FitObject,StrVar):
    filename = tkinter.filedialog.askopenfilename(initialdir="./")
    if filename != '':
        FitObject.PopData(filename)
        StrVar.set(filename.split("/")[-1])
        ViewScreen.PlotData()

def ClearFitLimits(ViewScreen,FitObject,limitchooseA,limitchooseB):
    FitObject.setDefaultFitLimits()

    limitchooseA.delete(0,END)
    limitchooseB.delete(0,END)

    limitchooseA.insert(INSERT,FitObject.lowerFitLimit.get())
    limitchooseB.insert(INSERT,FitObject.upperFitLimit.get())

    ViewScreen.DrawFitLimits()

def WriteTex(tex,string):
    if tex != None:
        tex.configure(state=NORMAL)
        tex.insert(END,"<update>: "+string+'\n')
        tex.see(END)
        tex.configure(state=DISABLED)
    else:
        print(string)
