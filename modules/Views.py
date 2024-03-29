import sys, os
import matplotlib
##from PyQt5.QtCore import Qt, QThread, QTimer
from PySide6 import QtCore, QtWidgets, QtGui
from PySide6.QtWebEngineWidgets import QWebEngineView
from PySide6.QtCore import QUrl
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolbar
from matplotlib.backend_tools import ToolBase
import time
from queue import Queue, Empty

class Ui_MainWindow(QtWidgets.QMainWindow):
    def __init__(self, SpecCanvas):

        super(Ui_MainWindow,self).__init__()

        self.SpecCanvas = SpecCanvas
        ## Grab the spectrum collection
        self.SpecColl = self.SpecCanvas.SpecColl

        self.scalersRunning = False
        
        ##  -----------------------------------------------------------------
        ##  Menu ..  ..                                                Help
        ##  -----------------------------------------------------------------
        ##  Control buttons (run, stop, etc.)
        ##  -----------------------------------------------------------------
        ##            |
        ##   Spectrum |  Spectrum tabs
        ##   Tree     |
        ##            |
        ##            |
        ##            |
        ##            |
        ##            |
        ##            |
        ##            |------------------------------------------------------
        ##   Logo     |   Editor
        ##            |
        ##  -----------------------------------------------------------------

        ## The menu bar
        self.createMenus()

        ## The main widget that holds everything else
        self.main_widget = QtWidgets.QWidget(self)


        self.rebinSlider = QtWidgets.QSlider(QtCore.Qt.Horizontal)
        self.rebinSlider.setMinimum(1)
        self.rebinSlider.setMaximum(20)
        self.rebinSlider.setValue(1)
        self.rebinSlider.setTickInterval(1)
        self.rebinSlider.setTickPosition(QtWidgets.QSlider.TicksAbove)
        self.rebinSlider.valueChanged.connect(self.rebin_action)
        self.n = 1
        
        self.rebinLabel = QtWidgets.QLabel()
        self.rebinLabel.setText("Rebin: "+str(self.rebinSlider.value()))

        self.runningIndicator = self.SpecColl.statusBar
        self.setStatusBar(self.runningIndicator)
        
        ## Start with two vertical groups
        ## - toolbar holds all of the run start, stop, gates, etc.
        ## - mainFrame holds the tree, spectrum, etc
        self.makeToolbar()
        ## The main frame that holds everything
        mainFrame = QtWidgets.QFrame()
        ##mainFrame.setFrameShape(QtWidgets.QFrame.StyledPanel)
        ##mainFrame.setFrameShadow(QtWidgets.QFrame.Raised)
        
        ## Layout of vertical groups
        ## Note: only one group now
        gridmain = QtWidgets.QGridLayout(self.main_widget)
        gridmain.setSpacing(10)
        gridmain.addWidget(mainFrame, 1, 0)
        
        ## Make a second grid for the mainFrame
        ## - treeFrame is Left-hand frame for the tree
        ## - dataFrame holds the spectrum tabs and command window
        treeFrame = QtWidgets.QFrame()
        treeFrame.setMinimumSize(260,720)
        ##treeFrame.setMaximumWidth(260)
        ##treeFrame.setFrameShape(QtWidgets.QFrame.StyledPanel)
        dataFrame = QtWidgets.QFrame()
        dataFrame.setMinimumSize(900,600)
        ##
        scalerFrame = QtWidgets.QFrame()
        scalerFrame.setMinimumSize(100,720) ## (200, 720) crashes X11 on Windows- Will

        ##----------------------------------------------------------------------
        ## The tree widget
        self.treeWidget = QtWidgets.QTreeWidget()
        self.treeWidget.setColumnCount(1)
        header = QtWidgets.QTreeWidgetItem(["Spectra"])
        self.treeWidget.setHeaderItem(header)
        item = QtWidgets.QTreeWidgetItem(self.treeWidget, [self.SpecCanvas.Spec.Name])
        item.spec = SpecCanvas.Spec
        self.treeWidget.addTopLevelItem(item)
        self.treeWidget.itemClicked.connect(self.itemclicked)
        self.treeWidget.setSelectionMode(QtWidgets.QAbstractItemView.ExtendedSelection)


        ## FENRIS logo
        pixmap = QtGui.QPixmap('images/FENRISLogo-notext.png')
        label = QtWidgets.QLabel()
        label.resize(240, 100)
        label.setPixmap(pixmap)
        ## Within the tree frame, make a vertical box layout
        treeFramevbox = QtWidgets.QVBoxLayout()
        treeFramevbox.addWidget(self.treeWidget)
        #        treeFramevbox.addStretch(1)
        treeFramevbox.addWidget(label)

        treeFrame.setLayout(treeFramevbox)

        self.PopulateTree()

        ##----------------------------------------------------------------------
        ## The scaler window
        scalertitle = QtWidgets.QLabel()
        scalertitle.setText("Scalers")
        scalertitle.setAlignment(QtCore.Qt.AlignCenter)
        self.scalerFramevbox = QtWidgets.QVBoxLayout()
        self.scalerFramevbox.addWidget(scalertitle)
        self.scalerFramevbox.setAlignment(QtCore.Qt.AlignTop)
        ## now two vboxes in the gbox
        ##self.scalerlabvbox = QtWidgets.QVBoxLayout()
        ##self.scalerlabvbox.setAlignment(QtCore.Qt.AlignTop)
        ##self.scalervalvbox = QtWidgets.QVBoxLayout()
        ##self.scalervalvbox.setAlignment(QtCore.Qt.AlignTop)
        
        ##scalerFramegbox.addWidget(self.scalerlabvbox,1,0,1,0)
        ##scalerFramegbox.addWidget(self.scalervalvbox,1,1,1,1)
        
        scalerFrame.setLayout(self.scalerFramevbox)
        
        ##----------------------------------------------------------------------
        ## Layout the mainFrame grid
        gridmainFrame = QtWidgets.QGridLayout(mainFrame)
        gridmainFrame.setSpacing(10)

        splitter = QtWidgets.QSplitter(QtCore.Qt.Horizontal)##
        splitter.addWidget(treeFrame)#,1,0)
        splitter.addWidget(dataFrame)#,1,1)
        splitter.addWidget(scalerFrame)
        ##splitter.setSizes([200,600])
        gridmainFrame.addWidget(splitter,1,0)
        ## Make a third grid for the spectrum and command window
        ## - tabWidget holds the spectra
        ## - commandWidget is the command editor

        ## Make the spectrum tabs
        tabWidget = QtWidgets.QTabWidget()
        self.tabWidget = tabWidget
        ## Tab 1
        tab1 = QtWidgets.QWidget()

        self.Matplotlib = QtWidgets.QWidget()
        l = QtWidgets.QVBoxLayout()
        toolbar = MyCustomToolbar(self.SpecCanvas, self)
        l.addWidget(toolbar)

        h = QtWidgets.QHBoxLayout()
        h.addWidget(self.SpecCanvas)

        vscroll = QtWidgets.QScrollBar(QtCore.Qt.Vertical)
        self.SpecCanvas.setupVSlider(vscroll)
        h.addWidget(vscroll)
        l.addLayout(h)
        hscroll = QtWidgets.QScrollBar(QtCore.Qt.Horizontal)
        hscroll.setPageStep(0)
        self.SpecCanvas.setupSlider(hscroll)
        l.addWidget(hscroll)

        tab1.setLayout(l)
        
        tabWidget.addTab(tab1,"")
        tabWidget.setTabText(tabWidget.indexOf(tab1), "Spectrum Inspector")
        ## Tab 2
        tab2 = QtWidgets.QWidget()
        self.tab2 = tab2
        tabWidget.addTab(tab2,"There's nothing in this tab!")
        tabWidget.setTabText(tabWidget.indexOf(tab2), "Empty Tab")
        
        ## the command editor
        self.commandWidget = QtWidgets.QTextEdit()
        self.commandWidget.setText("Welcome to EngeSpec!\n")
        self.commandWidget.setMaximumHeight(100)

        ## Add the output streams to the text editor
        ##sys.stdout = OutLog(commandWidget, sys.stdout)
        ##sys.stderr = OutLog(commandWidget, sys.stderr, QtGui.QColor(255,0,0) )

        
        gridDataFrame = QtWidgets.QGridLayout(dataFrame)
        gridDataFrame.setSpacing(10)
        gridDataFrame.addWidget(tabWidget,1,0)
        gridDataFrame.addWidget(self.commandWidget,2,0)
        
        self.setLayout(gridmain) 

        self.main_widget.setFocus()
        self.setCentralWidget(self.main_widget)

        self.setGeometry(300, 300, 350, 300)
        self.setWindowTitle('EngeSpec')    
        self.show()




    def fileQuit(self):
        self.close()

    def about(self):
        QtWidgets.QMessageBox.about(self, "About",
                          """This is EngeSpec!"""
        )

    ##--------------------------------------------------
    ## Settings
    def setting_autobin(self):
        if self.SpecCanvas.autobin:
            self.SpecCanvas.autobin=False
        else:
            self.SpecCanvas.autobin=True

    def setting_dots(self):
        if self.SpecCanvas.dots:
            self.SpecCanvas.dots=False
        else:
            self.SpecCanvas.dots=True


    def rebin_action(self):
         self.rebinLabel.setText("Rebin: "+str(self.rebinSlider.value()))
         self.n = self.rebinSlider.value()
         if self.SpecCanvas.twoD == False:
             self.SpecCanvas.ReBin(self.n)
         else:
             self.SpecCanvas.ReBin2D(self.n)
    ## --------------------------------------------------
    ## Function to write to the command editor
    def append_text(self, text, col=None):
        if col:
            tc = self.commandWidget.textColor()
            self.commandWidget.setTextColor(col)
        self.commandWidget.moveCursor(QtGui.QTextCursor.End)
        self.commandWidget.insertPlainText(text)
        if col:
            self.commandWidget.setTextColor(tc)

    ## Make the menus
    def createMenus(self):
        ## -----
        ## File menu
        self.file_menu = QtWidgets.QMenu('&File', self)
        ## Load ascii spectrum
        self.file_menu.addAction('&Load Spectrum File (ascii)',
                                 self.LoadASCIIData)
        ## Save ascii spectrum
        self.file_menu.addAction('&Save Spectrum File (ascii)',
                                 self.SaveASCIIData)
        ## Load ADDITIONAL ascii spectrum
        self.file_menu.addAction('&Load Additional Spectrum File (ascii)',
                                 self.LoadAdditionalASCIIData)

        self.file_menu.addSeparator()
        ## Load pickle data
        self.file_menu.addAction('&Load Pickle File',
                                 self.LoadPickleData)
        ## Save pickle data
        self.file_menu.addAction('&Save Pickle File',
                                 self.SavePickleData)

        self.file_menu.addSeparator()
        ## Quit
        self.file_menu.addAction('&Quit', self.fileQuit,
           QtCore.Qt.CTRL | QtCore.Qt.Key_Q)
        self.menuBar().addMenu(self.file_menu)

        ## -----
        ## Connection options
        self.connections_menu = QtWidgets.QMenu('&Connect MIDAS', self)
        self.menuBar().addSeparator()
        self.menuBar().addMenu(self.connections_menu)

        ## Connect to MIDAS
        self.connectOnlineAction = QtGui.QAction('&Online MIDAS')
        self.connectOnlineAction.triggered.connect(self.connectmidas)
        self.connections_menu.addAction(self.connectOnlineAction)

        self.connections_menu.addSeparator()
        ## Connect to MIDAS offline
        self.connectOfflineAction = QtGui.QAction('&Offline MIDAS')
        self.connectOfflineAction.triggered.connect(self.offlinemidas)
        self.connections_menu.addAction(self.connectOfflineAction)

        ## Sort a file in offline mode
        self.sortAction = QtGui.QAction('&Queue sort file(s)', self)
        self.sortAction.setEnabled(False)
        self.sortAction.triggered.connect(self.sort)
        self.connections_menu.addAction(self.sortAction)
        
        ## -----
        ## Settings Menu
        self.settings_menu = QtWidgets.QMenu('&Settings', self)
        self.menuBar().addSeparator()
        self.menuBar().addMenu(self.settings_menu)
        # autobinAction = QtGui.QAction('&Auto bin',self.settings_menu)
        # autobinAction.setCheckable(True)
        # autobinAction.setChecked(False)
        # autobinAction.triggered.connect(self.setting_autobin)
        # self.settings_menu.addAction(autobinAction)
        
        dotsAction = QtGui.QAction('&dots',self.settings_menu)
        dotsAction.setCheckable(True)
        dotsAction.setChecked(False)
        dotsAction.triggered.connect(self.setting_dots)
        self.settings_menu.addAction(dotsAction)
        
        ## -----
        ## Help Menu
        self.help_menu = QtWidgets.QMenu('&Help', self)
        self.menuBar().addSeparator()
        self.menuBar().addMenu(self.help_menu)
        self.help_menu.addAction('&About', self.about)

    ## Make the toolbar for starting, stopping runs etc.
    def makeToolbar(self):
        iconDir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 
                               "..", "images", "icons", "")
        exitAction = QtGui.QAction(QtGui.QIcon(iconDir + 'Exit.ico'), 'Exit', self)
        exitAction.triggered.connect(self.close)
        startAction = QtGui.QAction(QtGui.QIcon(iconDir + 'Start.ico'), 'Start Run/Sort', self)
        startAction.triggered.connect(self.startmidas)
        stopAction = QtGui.QAction(QtGui.QIcon(iconDir + 'Stop.ico'), 'Stop Run', self)
        stopAction.triggered.connect(self.stopmidas)
        gateAction = QtGui.QAction(QtGui.QIcon(iconDir + 'MakeGate.ico'), 'Set Gate', self)
        gateAction.triggered.connect(self.setgate)

        self.runControlsToolbar = self.addToolBar('Exit')
        self.runControlsToolbar.addAction(exitAction)
        self.runControlsToolbar.addAction(startAction)
        self.runControlsToolbar.addAction(stopAction)
        self.runControlsToolbar.addAction(gateAction)

        #self.rebinSlider.setFixedWidth(20)
        right_spacer = QtWidgets.QWidget()
        right_spacer.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding)
        
        self.runControlsToolbar.addWidget(right_spacer)
        self.runControlsToolbar.addWidget(self.rebinLabel)
        self.runControlsToolbar.addWidget(self.rebinSlider)

        
    def LoadASCIIData(self):
        ## Clear the old spectrum collection
        self.SpecColl.ClearCollection()
        self.SpecCanvas.LoadASCIIData()
        self.rebinSlider.setValue(self.n)
        Ui_MainWindow.rebin_action(self)
        self.PopulateTree()

    def SaveASCIIData(self):
        self.SpecCanvas.SaveASCIIData()

    def LoadAdditionalASCIIData(self):
        ##self.SpecColl.addSpectrum("New Spectrum")
        ##self.SpecCanvas.setSpecIndex(len(self.SpecColl.spec1d)-1, is2D=False)
        self.SpecCanvas.LoadASCIIData()
        self.rebinSlider.setValue(self.n)
        Ui_MainWindow.rebin_action(self)
        self.PopulateTree()

    def LoadPickleData(self):
        print("Loading Pickle Data")
        self.SpecCanvas.LoadPickleData()
        self.rebinSlider.setValue(self.n)
        Ui_MainWindow.rebin_action(self)
        self.PopulateTree()

    def SavePickleData(self):
        print("Saving Pickle Data")
        self.SpecCanvas.SavePickleData()

    def connectmidas(self):
        self.SpecColl.connectmidas()
        self.PopulateTree()
        self.PopulateScalers()
        self.SpecCanvas.setSpecIndex(0,False)

        view = QWebEngineView()
        view.load(QUrl("http://localhost:8080/"))
        self.tabWidget.removeTab(1)
        self.tabWidget.addTab(view,"Midas info")
       # view.show()
        ## make a scaler update thread
        self.scaler_thread = ScalerCollectionThread(self)

        ## Grey out unsafe menu items!
        self.connectOnlineAction.setEnabled(False)
        self.connectOfflineAction.setEnabled(False)

    def offlinemidas(self):
        self.SpecColl.offlinemidas()
        self.PopulateTree()
        self.PopulateScalers()
        self.SpecCanvas.setSpecIndex(0,False)
        ## make a scaler update thread
        self.scaler_thread = ScalerCollectionThread(self)

        ## Grey out unsafe menu items!
        self.sortAction.setEnabled(True)
        self.connectOnlineAction.setEnabled(False)
        self.connectOfflineAction.setEnabled(False)
        
    def sort(self):
        self.SpecColl.sort()
        
    def startmidas(self):
        ##print("Running midas")


        
        self.SpecColl.startmidas()
        self.runningIndicator.showMessage("MIDAS is running...")
        
        if not self.scalersRunning:
            self.scaler_thread.start()
            self.scalersRunning=True

        
    def stopmidas(self):
        ##print("Stopping midas")
        self.SpecColl.stopmidas()
        self.runningIndicator.showMessage("")
        #if self.SpecColl.isOnline:
            #os.system("odbedit -c stop")
        #self.MIDASisRunning = False
        
    def setgate(self):
        self.SpecCanvas.getGate()

    def PopulateTree(self):
        ## Now get the list of spectra and add them to the selection
        ## tree
        ## Remove the old tree first
        self.treeWidget.clear()
        ## Grab the spectrum collection
        SpecColl = self.SpecCanvas.SpecColl
        ## Fill the 1d and 2d items
        l1d = len(SpecColl.spec1d)
        for i in range(l1d):
            spec = SpecColl.spec1d[i]
            name = spec.Name
            NGates = spec.NGates
            gates = spec.gates
            item = QtWidgets.QTreeWidgetItem(self.treeWidget, [name])
            item.spec = spec
            ##
            count = 0
            for gObj in gates:
                subitem = QtWidgets.QTreeWidgetItem(item, [gObj.name])
                subitem.index = count
                count = count+1
            self.treeWidget.addTopLevelItem(item)
            
        l2d = len(SpecColl.spec2d)
        for i in range(l2d):
            spec = SpecColl.spec2d[i]
            name = spec.Name
            NGates = spec.NGates
            gates = spec.gates
            item = QtWidgets.QTreeWidgetItem(self.treeWidget, [name])
            item.spec = spec
            #for i in range(NGates):
            #    subitem = QtWidgets.QTreeWidgetItem(item, ["Gate {}".format(i)])
            #    subitem.spec = spec.gate
            count = 0
            for gObj in gates:
                subitem = QtWidgets.QTreeWidgetItem(item, [gObj.name])
                subitem.index = count
                count = count+1
                self.treeWidget.addTopLevelItem(item)

        self.treeWidget.expandAll()
           

    def itemclicked(self,it,col):
        ## Loop through all items to find the clicked ones
        allitems = self.treeWidget.selectedItems()
        
        ##print("Old item: ",allitems[0])
        ##print("Selected: ",allitems)
        ##print("Parent: ",allitems[0].parent())
        if it.parent() is None:
            self.SpecCanvas.setSpecIndex(allitems[0].spec.num,allitems[0].spec.is2D,-1)
            if(len(allitems)>1):
                self.SpecCanvas.setOverlayIndex(allitems[1].spec.num)
        else:
            self.SpecCanvas.setSpecIndex(allitems[0].parent().spec.num,
                                         allitems[0].parent().spec.is2D,allitems[0].index)
        self.rebinSlider.setValue(self.n)
        Ui_MainWindow.rebin_action(self)
    ## Build the list of scalers
    def PopulateScalers(self):
        ## Build a bunch of labels in the right-hand scaler frame
        SpecColl = self.SpecCanvas.SpecColl

        lsclr = len(SpecColl.sclr)
        isclr = 0
        self.sclrlab = []
        self.sclrval = []
        for sc in SpecColl.sclr:
            hbox = QtWidgets.QHBoxLayout()
            lab = QtWidgets.QLabel()
            val = QtWidgets.QLabel()
            val.setAlignment(QtCore.Qt.AlignRight)
            self.sclrlab.append(lab)
            self.sclrval.append(val)
            self.sclrlab[isclr].setText(sc.Name)
            self.sclrval[isclr].setText("{}".format(sc.N))
            hbox.addWidget(self.sclrlab[isclr])
            hbox.addWidget(self.sclrval[isclr])
            self.scalerFramevbox.addLayout(hbox)
            isclr = isclr+1
        
    ## Update scaler values
    def UpdateScalers(self):
        SpecColl = self.SpecCanvas.SpecColl
        isclr = 0
        for sc in SpecColl.sclr:
            ##txt = "{name:<17}:  {num:>8}"
            self.sclrval[isclr].setText("{}".format(sc.N))
            ##self.sclrval[isclr].setText(format(sc.N))
            isclr = isclr+1
        
class ScalerCollectionThread(QtCore.QThread):
    def __init__(self,view):
        super().__init__()

        self.view = view
        self.specColl = view.SpecCanvas.SpecColl
        self.names = self.specColl.dm.getScalerNames()
        ##print(len(self.names)," scalers have been made:")
        ##for name in self.names:
        ##    print(" - ",name)

    def run(self):
        
        while True: ##self.specColl.MIDASisRunning:
            ##print("Collecting Scalers")
            sclrvals = self.specColl.dm.getScalers()
            for i in range(len(self.specColl.sclr)):
                self.specColl.sclr[i].N = sclrvals[i]
            self.view.UpdateScalers()
            time.sleep(5)
            

class MyCustomToolbar(NavigationToolbar): 
    def __init__(self, plotCanvas, parent=None):
        self.toolitems = ()
        # create the default toolbar
        NavigationToolbar.__init__(self, plotCanvas, parent)

        ## --------------------------------------------------
        ## Custom items
        iconDir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 
                                    "..", "images", "icons", "")

        ## Update
        self.a = self.addAction(QtGui.QIcon(iconDir + "ReloadIcon.ico"),
                                "Reload", plotCanvas.UpdatePlot)
        self.a.setToolTip("Update the plot")
        self._actions['update'] = self.a

        ## LogLin
        self.a = self.addAction(QtGui.QIcon(iconDir + "LogLinIcon.ico"),
                                "LogLin", plotCanvas.ToggleLog)
        self.a.setToolTip("Change to Log or Linear Scale")
        self._actions['loglin'] = self.a

        ## Scale-all
        self.a = self.addAction(QtGui.QIcon(iconDir + "ScaleAllIcon.ico"),
                                "Auto Scale", plotCanvas.Resize)
        self.a.setToolTip("Resize to all")
        self._actions['scaleall'] = self.a
        
        ## Auto y-scale
        self.a = self.addAction(QtGui.QIcon(iconDir + "AutoScaleIcon.ico"),
                                "Auto Scale", plotCanvas.Autosize)
        self.a.setToolTip("Auto-scale the y-direction")
        self._actions['autoscale'] = self.a

        ## Jam-like zoom button
        self.a = self.addAction(QtGui.QIcon(iconDir + "JamZoomIcon.ico"),
                                "Zoom", plotCanvas.JamZoom)
        self.a.setToolTip("Zoom in with clicks")
        self._actions['zoom'] = self.a
        
        ## XRange
        ##self.a = self.addAction(QtGui.QIcon(iconDir + "XRangeIcon.ico"),
        ##                        "X Range", plotCanvas.xInteractiveZoom)
        ##self.a.setToolTip("Adjust the x-range")
        ##self._actions['xrange'] = self.a

        ## YRange
        self.a = self.addAction(QtGui.QIcon(iconDir + "YRangeIcon.ico"),
                                "Y Range", plotCanvas.JamZoomy)
#                                "Y Range", plotCanvas.yInteractiveZoom)
        self.a.setToolTip("Adjust the y-range")
        self._actions['yrange'] = self.a

        ## Zoom in
        self.a = self.addAction(QtGui.QIcon(iconDir + "ZoomInIcon.ico"),
                                "Zoom in", plotCanvas.xZoomIn)
        self.a.setToolTip("Zoom in")
        self._actions['zoomin'] = self.a

        ## Zoom in
        self.a = self.addAction(QtGui.QIcon(iconDir + "ZoomOutIcon.ico"),
                                "Zoom out", plotCanvas.xZoomOut)
        self.a.setToolTip("Zoom out")
        self._actions['zoomout'] = self.a

        ## Zero all spectra
        self.a = self.addAction(QtGui.QIcon(iconDir + "ZeroIcon.ico"),
                                "Zero all", plotCanvas.ZeroAll)
        self.a.setToolTip("Zero all spectra")
        self._actions['zeroall'] = self.a


        
        ## Add a Splitter
        self.a = self.addWidget(QtWidgets.QSplitter())

        ## gross area
        self.a = self.addAction(QtGui.QIcon(iconDir + "GrossAreaIcon.ico"),
                                "Gross Area", plotCanvas.grossArea)
        self.a.setToolTip("Calculate the gross area under a peak")
        self._actions['grossarea'] = self.a

        ## net area
        self.a = self.addAction(QtGui.QIcon(iconDir + "NetAreaIcon.ico"),
                                "Net Area", plotCanvas.netArea)
        self.a.setToolTip("Calculate the net, background-subtracted area under a peak")
        self._actions['netarea'] = self.a
        
        ## Add a Splitter
        self.a = self.addWidget(QtWidgets.QSplitter())
        
    def select_tool(self):
        print("You clicked the selection tool")

##
##def main():
##    
##    app = QtWidgets.QApplication(sys.argv)
##    ex = Ui_MainWindow()
##    sys.exit(app.exec_())

##if __name__ == '__main__':
##    main()

## Class to write stdout to the GUI rather than the terminal
class OutLog:
    def __init__(self, edit, out=None, color=None):
        """(edit, out=None, color=None) -> can write stdout, stderr to a
        QTextEdit.
        edit = QTextEdit
        out = alternate stream ( can be the original sys.stdout )
        color = alternate color (i.e. color stderr a different color)
        """
        self.edit = edit
        self.out = None
        self.color = color

    def write(self, m):
        if self.color:
            tc = self.edit.textColor()
            self.edit.setTextColor(self.color)

        self.edit.moveCursor(QtGui.QTextCursor.End)
        self.edit.insertPlainText( m )

        if self.color:
            self.edit.setTextColor(tc)

        if self.out:
            self.out.write(m)


##----------------------------------------------------------------------
## New method for output logging
## See https://stackoverflow.com/questions/21071448/redirecting-stdout-and-stderr-to-a-pyqt4-qtextedit-from-a-secondary-thread

# The new Stream Object which replaces the default stream associated with sys.stdout
# This object just puts data in a queue!
class WriteStream(object):
    def __init__(self,queue):
        self.queue = queue

    def write(self, text):
        self.queue.put(text)

# A QObject (to be run in a QThread) which sits waiting for data to come through a Queue.Queue().
# It blocks until data is available, and one it has got something from the queue, it sends
# it to the "MainThread" by emitting a Qt Signal 
class MyReceiver(QtCore.QObject):
    mysignal = QtCore.Signal(str)

    def __init__(self,queue,*args,**kwargs):
        QtCore.QObject.__init__(self,*args,**kwargs)
        self.queue = queue

        ##    @pyqtSlot()
    def run(self):
        while True:
            text = self.queue.get()
            self.mysignal.emit(text)
            if text == "Exiting":
                break



