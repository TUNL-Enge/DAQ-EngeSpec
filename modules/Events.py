import matplotlib
from ViewScreen import *
from matplotlib.backend_bases import key_press_handler

class ScrollBot:
    def __init__(self,ViewScreen):
        self.ViewScreen = ViewScreen
        self.connect()
    def connect(self):
        self.linepick=self.ViewScreen.canvas.mpl_connect('pick_event',self.picknotice)
        self.cidscroll=self.ViewScreen.canvas.mpl_connect('scroll_event',self.scrollnotice)
    def picknotice(self,event):
        if event.mouseevent.button == 1:
            thisline=event.artist
            self.ViewScreen.fitObject.myWriteTex("line: {0}".format(thisline.get_label()))
    def scrollnotice(self,event):
        xlow,xhigh = self.ViewScreen.a.get_xlim()
        gain = 500
        if( (xlow-gain*event.step > 0) and (xhigh-gain*event.step < 4096)):
            self.ViewScreen.a.set_xlim(xlow-gain*event.step,xhigh-gain*event.step)
            self.ViewScreen.fig.canvas.draw()
        else:
            None

class SelectRange:
    def __init__(self,ViewScreen):
        self.coord = []
        self.press = None

        self.ViewScreen = ViewScreen
        self.ViewScreen.eventRegister = self
        self.ViewScreen.eventRegister.connect()
        if(self.ViewScreen.FitLimitEventRegister!=None):
            self.ViewScreen.FitLimitEventRegister.disconnect()
            self.ViewScreen.FitLimitEventRegister = None
    def connect(self):
        self.cidpress   = self.ViewScreen.canvas.mpl_connect('button_press_event',self.on_press)
        self.cidrelease = self.ViewScreen.canvas.mpl_connect('button_release_event',self.on_release)
        self.cidmotion  = self.ViewScreen.canvas.mpl_connect('motion_notify_event',self.on_motion)
    def on_press(self,event):
        self.coord.append(event.xdata)
        self.line,     = self.ViewScreen.a.plot([event.xdata,event.xdata],[0.01,int(1.10*self.ViewScreen.a.get_ylim()[1])],'g-',lw=1)
        self.linetemp, = self.ViewScreen.a.plot([event.xdata,event.xdata],[0.01,int(1.10*self.ViewScreen.a.get_ylim()[1])],'g-',lw=1)
        self.press=event
        if event.inaxes != self.ViewScreen.a.axes:
            self.disconnect()
        self.ViewScreen.fig.canvas.draw()
    def on_motion(self,event):
        if self.press is None: return
        self.linetemp.set_xdata([event.xdata,event.xdata])
        self.linetemp.set_ydata([0.01,int(1.20*self.ViewScreen.a.get_ylim()[1])])
        self.ViewScreen.fig.canvas.draw()
    def on_release(self,event):
        if event.inaxes == self.ViewScreen.a.axes:
            self.coord.append(event.xdata)
            self.line.remove()
            self.linetemp.remove()
            self.ViewScreen.a.set_xlim(min(self.coord),max(self.coord))
            self.ViewScreen.fig.canvas.draw()
            self.disconnect()
        else:
            self.line.remove()
            self.templine.remove()
            self.ViewScreen.fig.canvas.draw()
    def disconnect(self):
        self.ViewScreen.canvas.mpl_disconnect(self.cidpress)
        self.ViewScreen.canvas.mpl_disconnect(self.cidrelease)
        self.ViewScreen.canvas.mpl_disconnect(self.cidmotion)
        self.ViewScreen.eventRegister=None

class SelectFitLimits:
    def __init__(self,ViewScreen,limitchooseA,limitchooseB):
        self.islines = False
        self.coord = []
        self.line = None
        self.linetemp = None
        self.lines = []
        self.press = None

        # These are for relaying the new fit limits back to the GUI
        self.limitchooseA = limitchooseA
        self.limitchooseB = limitchooseB

        self.ViewScreen = ViewScreen
        self.ViewScreen.FitLimitEventRegister = self
        if(self.ViewScreen.eventRegister!=None):
            self.ViewScreen.eventRegister.disconnect()
            self.ViewScreen.eventRegister = None
        if(self.ViewScreen.fitObject.isDataSet==True):
            self.ViewScreen.FitLimitEventRegister = self
            self.ViewScreen.FitLimitEventRegister.connect()
        else:
            return
    def connect(self):
        self.cidpress   = self.ViewScreen.canvas.mpl_connect('button_press_event',self.on_press)
        self.cidrelease = self.ViewScreen.canvas.mpl_connect('button_release_event',self.on_release)
        self.cidmotion  = self.ViewScreen.canvas.mpl_connect('motion_notify_event',self.on_motion)
        if self.ViewScreen.fitObject.isDataSet == False:
            self.disconnect()
    def on_press(self,event):
        self.coord.append(event.xdata)
        self.line,     = self.ViewScreen.a.plot([event.xdata,event.xdata],[0.01,1E6],color="#56a0d3",lw=1)
        self.linetemp, = self.ViewScreen.a.plot([event.xdata,event.xdata],[0.01,1E6],color="#56a0d3",lw=1)
        self.press=event
        self.ViewScreen.fig.canvas.draw()
    def on_motion(self,event):
        if self.press is not None: 
            self.linetemp.set_xdata([event.xdata,event.xdata])
            self.linetemp.set_ydata([0.01,int(1.20*self.ViewScreen.a.get_ylim()[1])])
        self.ViewScreen.fig.canvas.draw()
    def on_release(self,event):
        if event.inaxes == self.ViewScreen.a.axes:
            self.coord.append(event.xdata)
            self.line2,     = self.ViewScreen.a.plot([event.xdata,event.xdata],[0.01,1E6],color="#56A0D3",lw=1)
            self.linetemp.remove()

            self.ViewScreen.fitObject.setFitLimits(min(self.coord),max(self.coord))
            self.limitchooseA.delete(0,END)
            self.limitchooseB.delete(0,END)
            self.limitchooseA.insert(INSERT,self.ViewScreen.fitObject.lowerFitLimit.get())
            self.limitchooseB.insert(INSERT,self.ViewScreen.fitObject.upperFitLimit.get())

            self.ViewScreen.fig.canvas.draw()
            self.islines=True
            self.disconnect()
        else:
            for line in [self.line,self.templine]:
                if line in self.ViewScreen.a.lines:
                    line.remove()
            self.ViewScreen.fig.canvas.draw()
    def disconnect(self):
        self.ViewScreen.canvas.mpl_disconnect(self.cidpress)
        self.ViewScreen.canvas.mpl_disconnect(self.cidrelease)
        self.ViewScreen.canvas.mpl_disconnect(self.cidmotion)
        self.RemoveLines()
        self.ViewScreen.DrawFitLimits()
        self.ViewScreen.eventRegister=None
    def RemoveLines(self):
        self.islines=False
        for line in [self.line,self.line2]:
            if line in self.ViewScreen.a.lines:
                line.remove()



class chooseROI:
    def __init__(self,ViewScreen,num):
        self.ViewScreen = ViewScreen
        if(self.ViewScreen.ROIRegister!=None):
            if(self.ViewScreen.ROIRegister.fill in self.ViewScreen.a.collections):
                self.ViewScreen.ROIRegister.fill.remove()
        self.ViewScreen.ROIRegister=self
        self.num = num
        self.islines = False
        self.coord = []
        self.lines = []
        self.press = None
        self.xLOW = 0
        self.xHIGH = 0
        self.fill=None
        self.connect()
    def connect(self):
        self.cidpress   = self.ViewScreen.canvas.mpl_connect('button_press_event', self.on_press)
        self.cidrelease = self.ViewScreen.canvas.mpl_connect('button_release_event', self.on_release)
        self.cidmotion  = self.ViewScreen.canvas.mpl_connect('motion_notify_event', self.on_motion)
    def on_press(self,event):
        self.coord.append(event.xdata)
        self.line,     = self.ViewScreen.a.plot([event.xdata,event.xdata],[0.01,int(1.20*self.ViewScreen.a.get_ylim()[1])],color="green",lw=1)
        self.x0 = int(event.xdata)
        self.linetemp, = self.ViewScreen.a.plot([event.xdata,event.xdata],[0.01,int(1.20*self.ViewScreen.a.get_ylim()[1])],color="green",lw=1)
        self.press = (event)
        self.ViewScreen.fig.canvas.draw()
    def on_motion(self,event):
        if self.press is not None: 
            self.linetemp.set_xdata([event.xdata,event.xdata])
            self.linetemp.set_ydata([0.01,int(1.20*self.ViewScreen.a.get_ylim()[1])])
        self.ViewScreen.fig.canvas.draw()
    def on_release(self,event):
        if event.inaxes == self.ViewScreen.a.axes:
            self.coord.append(event.xdata)
            self.xLOW=self.x0
            self.xHIGH=int(event.xdata)
            self.linetemp.remove()
            self.xZ = np.array([ xx for xx in range(self.xLOW,self.xLOW+len(self.ViewScreen.fitObject.templateSpectraCopy[self.num].Data[self.xLOW:self.xHIGH+1]))],dtype=int)
            self.yZ = np.array(self.ViewScreen.fitObject.templateSpectraCopy[self.num].Data[self.xLOW:self.xHIGH+1],dtype=float)
            self.fill = self.ViewScreen.a.fill_between(x = np.ravel(list(zip(self.xZ,self.xZ+1))),y1=np.ravel(list(zip(self.yZ,self.yZ))),y2=0.01,interpolate=True,color='#33ff33')
            self.line.remove()
            self.ViewScreen.fig.canvas.draw()
            self.islines = True
            self.disconnect()
        else:
            self.line.remove()
            self.linetemp.remove()
            self.ViewScreen.fig.canvas.draw()
            self.disconnect()
    def disconnect(self):
        self.ViewScreen.canvas.mpl_disconnect(self.cidpress)
        self.ViewScreen.canvas.mpl_disconnect(self.cidrelease)
        self.ViewScreen.canvas.mpl_disconnect(self.cidmotion)
        self.ViewScreen.fig.canvas.draw()
    def RemoveLines(self):
        self.islines=False
        self.line.remove()
