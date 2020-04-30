import sys, os
import matplotlib
from PySide2 import QtCore, QtWidgets, QtGui
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolbar
from matplotlib.backend_tools import ToolBase


class Ui_MainWindow(QtWidgets.QMainWindow):
    def __init__(self, SpecCanvas):

        super(Ui_MainWindow,self).__init__()

        self.SpecCanvas = SpecCanvas
        ## Grab the spectrum collection
        self.SpecColl = self.SpecCanvas.SpecColl
        
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
        scalerFrame.setMinimumSize(150,720)

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

        ##----------------------------------------------------------------------
        ## The scaler window
        scalertitle = QtWidgets.QLabel()
        scalertitle.setText("Scalers")
        scalertitle.setAlignment(QtCore.Qt.AlignCenter)
        self.scalerFramevbox = QtWidgets.QVBoxLayout()
        self.scalerFramevbox.addWidget(scalertitle)
        self.scalerFramevbox.setAlignment(QtCore.Qt.AlignTop)
        
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
        self.SpecCanvas.setupSlider(hscroll)
        l.addWidget(hscroll)
        tab1.setLayout(l)
        
        tabWidget.addTab(tab1,"")
        tabWidget.setTabText(tabWidget.indexOf(tab1), "Spectrum Inspector")
        ## Tab 2
        tab2 = QtWidgets.QWidget()
        tabWidget.addTab(tab2,"There's nothing in this tab!")
        tabWidget.setTabText(tabWidget.indexOf(tab2), "Empty Tab")
        
        ## the command editor
        commandWidget = QtWidgets.QTextEdit()
        commandWidget.setText("Welcome to EngeSpec!\n")
        commandWidget.setMaximumHeight(100)
        ## Add the output streams to the text editor
        ##sys.stdout = OutLog(commandWidget, sys.stdout)
        ##sys.stderr = OutLog(commandWidget, sys.stderr, QtGui.QColor(255,0,0) )

        
        gridDataFrame = QtWidgets.QGridLayout(dataFrame)
        gridDataFrame.setSpacing(10)
        gridDataFrame.addWidget(tabWidget,1,0)
        gridDataFrame.addWidget(commandWidget,2,0)
        
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
        ## Load HDF data
        self.file_menu.addAction('&Load HDF File',
                                 self.LoadHDFData)
        ## Load pickle data
        self.file_menu.addAction('&Load Pickle File',
                                 self.LoadPickleData)
        ## Save HDF data
        self.file_menu.addAction('&Save Pickle File',
                                 self.SavePickleData)
        ## Connect to MIDAS
        self.file_menu.addAction('&Connect MIDAS',
                                 self.connectmidas)
        ## Connect to MIDAS offline
        self.file_menu.addAction('&Offline MIDAS',
                                 self.offlinemidas)
        ## Quit
        self.file_menu.addAction('&Quit', self.fileQuit,
           QtCore.Qt.CTRL + QtCore.Qt.Key_Q)
        self.menuBar().addMenu(self.file_menu)

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
        exitAction = QtWidgets.QAction(QtGui.QIcon(iconDir + 'Exit.ico'), 'Exit', self)
        exitAction.triggered.connect(self.close)
        startAction = QtWidgets.QAction(QtGui.QIcon(iconDir + 'Start.ico'), 'Start Run/Sort', self)
        startAction.triggered.connect(self.startmidas)
        stopAction = QtWidgets.QAction(QtGui.QIcon(iconDir + 'Stop.ico'), 'Stop Run', self)
        stopAction.triggered.connect(self.stopmidas)
        gateAction = QtWidgets.QAction(QtGui.QIcon(iconDir + 'MakeGate.ico'), 'Set Gate', self)
        gateAction.triggered.connect(self.setgate)

        self.runControlsToolbar = self.addToolBar('Exit')
        self.runControlsToolbar.addAction(exitAction)
        self.runControlsToolbar.addAction(startAction)
        self.runControlsToolbar.addAction(stopAction)
        self.runControlsToolbar.addAction(gateAction)

        
    def LoadASCIIData(self):
        self.SpecCanvas.LoadASCIIData()

    def SaveASCIIData(self):
        self.SpecCanvas.SaveASCIIData()

    def LoadHDFData(self):
        print("Loading HDF Data")
        self.SpecCanvas.LoadHDFData()
        self.PopulateTree()

    def LoadPickleData(self):
        print("Loading Pickle Data")
        self.SpecCanvas.LoadPickleData()
        self.PopulateTree()

    def SavePickleData(self):
        print("Saving Pickle Data")
        self.SpecCanvas.SavePickleData()

    def connectmidas(self):
        self.SpecColl.connectmidas()
        self.PopulateTree()
        self.SpecCanvas.setSpecIndex(0,False)

    def offlinemidas(self):
        self.SpecColl.offlinemidas()
        self.PopulateTree()
        self.PopulateScalers()
        self.SpecCanvas.setSpecIndex(0,False)

    def startmidas(self):
        print("Running midas")
        self.SpecColl.startmidas()
        
        
    def stopmidas(self):
        print("Stopping midas")
        os.system("odbedit -c stop")
        
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
            hasGate = spec.hasGate
            item = QtWidgets.QTreeWidgetItem(self.treeWidget, [name])
            item.spec = spec
            if hasGate:
                subitem = QtWidgets.QTreeWidgetItem(item, ["Gate"])
                subitem.spec = spec
            self.treeWidget.addTopLevelItem(item)
            
        l2d = len(SpecColl.spec2d)
        for i in range(l2d):
            spec = SpecColl.spec2d[i]
            name = spec.Name
            hasGate = spec.hasGate
            item = QtWidgets.QTreeWidgetItem(self.treeWidget, [name])
            item.spec = spec
            if hasGate:
                subitem = QtWidgets.QTreeWidgetItem(item, ["Gate"])
                subitem.spec = spec.gate
            self.treeWidget.addTopLevelItem(item)

        self.treeWidget.expandAll()
           

    def itemclicked(self,it,col):
        if it.parent() is None:
            self.SpecCanvas.setSpecIndex(it.spec.num,it.spec.is2D,False)
        else:
            self.SpecCanvas.setSpecIndex(it.parent().spec.num,it.parent().spec.is2D,True)

    ## Build the list of scalers
    def PopulateScalers(self):
        ## Build a bunch of labels in the right-hand scaler frame
        SpecColl = self.SpecCanvas.SpecColl

        lsclr = len(SpecColl.sclr)
        isclr = 0
        self.sclrlab = []
        for sc in SpecColl.sclr:
            txt = "{name:<}:  {num:>12}"
            lab = QtWidgets.QLabel()
            self.sclrlab.append(lab)
            self.sclrlab[isclr].setText(txt.format(name=sc.Name,num=sc.N))
            self.scalerFramevbox.addWidget(self.sclrlab[isclr])
            isclr = isclr+1

    ## Update scaler values
    def UpdateScalers(self):
        print("updating scalers")

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
##
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
