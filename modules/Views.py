import sys, os
import matplotlib
##matplotlib.use("Qt5Agg")
from PySide2 import QtCore
from PySide2 import QtGui
from PySide2.QtWidgets import QMainWindow, QFrame, QMenu, QVBoxLayout, QHBoxLayout, \
    QSizePolicy, QMessageBox, QWidget, QToolBar, QFileDialog, QPushButton, QLabel, QTabWidget,\
    QMenuBar, QStatusBar, QTextEdit, QSplitter, QTreeWidget, QTreeWidgetItem, QSpinBox
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolbar
from matplotlib.backend_tools import ToolBase

class Ui_MainWindow(QMainWindow):
    def __init__(self, SpecCanvas):

        ## super().__init__ allows us to have all properties of
        ## QMainWindw available
        super().__init__()

        ##self.Spec = Spec
        self.SpecCanvas = SpecCanvas 

        ## Grab the spectrum collection
        self.SpecColl = self.SpecCanvas.SpecColl
        
        ## The main window
        self.setObjectName("MainWindow")
        self.resize(1200, 780)
        self.setWindowTitle("EngeSpec")

        self.centralwidget = QWidget()
        self.centralwidget.setObjectName("centralwidget")


        self.horizontalLayoutWidget = QWidget(self.centralwidget)
        self.horizontalLayoutWidget.setGeometry(QtCore.QRect(10, 10, 1190, 730))
        self.horizontalLayoutWidget.setObjectName("horizontalLayoutWidget")
        self.horizontalLayout = QHBoxLayout(self.horizontalLayoutWidget)
        self.horizontalLayout.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout.setObjectName("horizontalLayout")

        self.frame = QFrame(self.horizontalLayoutWidget)
        self.frame.setFrameShape(QFrame.StyledPanel)
        self.frame.setFrameShadow(QFrame.Raised)
        self.frame.setObjectName("frame")
        
        ## ----------------------------------------------------------------------
        ## The Left-hand menu frame
        self.LHMenuFrame = QFrame(self.frame)
        self.LHMenuFrame.setGeometry(QtCore.QRect(10, 10, 260, 720))
        self.LHMenuFrame.setFrameShape(QFrame.StyledPanel)
        self.LHMenuFrame.setFrameShadow(QFrame.Raised)
        self.LHMenuFrame.setObjectName("LHMenuFrame")

        ## A couple of buttons
        ## Load a file
        self.loadButton = QPushButton(self.LHMenuFrame)
        self.loadButton.setGeometry(QtCore.QRect(10, 10, 240, 25))
        self.loadButton.setObjectName("loadButton")
        self.loadButton.setText("Load Spectrum File (ascii)")
        self.loadButton.clicked.connect(SpecCanvas.LoadData)
        ## Connect the simulation
        self.connectsimButton = QPushButton(self.LHMenuFrame)
        self.connectsimButton.setGeometry(QtCore.QRect(10, 40, 240, 25))
        self.connectsimButton.setObjectName("connectsimButton")
        self.connectsimButton.setText("Connect simulation")
        self.connectsimButton.clicked.connect(self.connectsim)
        ## Start/stop simulation
        self.simButton = QPushButton(self.LHMenuFrame)
        self.simButton.setGeometry(QtCore.QRect(10, 70, 240, 25))
        if not self.SpecColl.isRunning:
            self.simButton.setText("Start Simulation")
        else:
            self.simButton.setText("Stop Simulation")
        self.simButton.clicked.connect(self.sim)
        
        ## Collect some clicks
        self.clicksButton = QPushButton(self.LHMenuFrame)
        self.clicksButton.setGeometry(QtCore.QRect(10, 130, 240, 25))
        self.clicksButton.setObjectName("clicksButton")
        self.clicksButton.setText("Test some clicks")
        self.clicksButton.clicked.connect(SpecCanvas.getClicks)
        ## Make a 2D Gate
        self.gateButton = QPushButton(self.LHMenuFrame)
        self.gateButton.setGeometry(QtCore.QRect(10, 160, 240, 25))
        self.gateButton.setObjectName("gateButton")
        self.gateButton.setText("Make a gate")
        self.gateButton.clicked.connect(SpecCanvas.getGate)
        ## Read HDF 2D Data
        self.load2DButton = QPushButton(self.LHMenuFrame)
        self.load2DButton.setGeometry(QtCore.QRect(10, 220, 240, 25))
        self.load2DButton.setObjectName("load2DButton")
        self.load2DButton.setText("Load 2D HDF")
        self.load2DButton.clicked.connect(self.LoadHDFData)

        ## A selection tree
        self.treeWidget = QTreeWidget(self.LHMenuFrame)
        self.treeWidget.setGeometry(QtCore.QRect(10,330,240,240))
        self.treeWidget.setColumnCount(1)
        header = QTreeWidgetItem(["Spectra"])
        self.treeWidget.setHeaderItem(header)
        item = QTreeWidgetItem(self.treeWidget, [self.SpecCanvas.Spec.Name])
        item.spec = SpecCanvas.Spec
        self.treeWidget.addTopLevelItem(item)
        self.treeWidget.itemClicked.connect(self.itemclicked)

        ## The FENRIS Logo
        pixmap = QtGui.QPixmap('images/FENRISLogo-notext.png')
        label = QLabel(self.LHMenuFrame)
        label.setGeometry(QtCore.QRect(10, 600, 240, 100))
        label.setPixmap(pixmap)
        
        
        ##----------------------------------------------------------------------
        ## The Right-hand plotting area
        self.tabWidget = QTabWidget(self.frame)
        self.tabWidget.setGeometry(QtCore.QRect(280, 10, 900, 600))
        self.tab = QWidget()

        ## The actual plot sits in the first tab!
        self.Matplotlib = QWidget(self.tab)
        self.Matplotlib.setGeometry(QtCore.QRect(10, 10, 880, 580))
        self.Matplotlib.setObjectName("Matplotlib")
        l = QVBoxLayout(self.Matplotlib)
        toolbar = MyCustomToolbar(self.SpecCanvas, self)
        l.addWidget(toolbar)
        l.addWidget(self.SpecCanvas)

        ## Second tab (nothing here!)
        self.tabWidget.addTab(self.tab, "")
        self.tab_2 = QWidget()
        self.tab_2.setObjectName("tab_2")
        self.tabWidget.addTab(self.tab_2, "There's nothing in this tab!")

        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab), "Spectrum Inspector")
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_2), "Empty Tab")

        ## textedit below plotting window
        self.TextEdit = QTextEdit(self.frame)
        ## Add the output streams to the text editor
        ##sys.stdout = OutLog(self.TextEdit, sys.stdout)
        ##sys.stderr = OutLog(self.TextEdit, sys.stderr, QtGui.QColor(255,0,0) )

        self.TextEdit.setGeometry(QtCore.QRect(280, 620, 900, 100))
        self.TextEdit.setText("Welcome to EngeSpec!\n")
        
        ## Finally get everything laid out and set up
        self.horizontalLayout.addWidget(self.frame)
        self.setCentralWidget(self.centralwidget)
        self.menubar = QMenuBar(self)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 900, 30))
        
        self.setMenuBar(self.menubar)
        self.statusbar = QStatusBar(self)
        self.setStatusBar(self.statusbar)

        self.tabWidget.setCurrentIndex(0)
        QtCore.QMetaObject.connectSlotsByName(self)

    def LoadHDFData(self):
        print("Loading HDF Data")
        self.SpecCanvas.LoadHDFData()
        self.PopulateTree()
        
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
            item = QTreeWidgetItem(self.treeWidget, [name])
            item.spec = spec
            if hasGate:
                subitem = QTreeWidgetItem(item, ["Gate"])
                subitem.spec = spec
            self.treeWidget.addTopLevelItem(item)
            
        l2d = len(SpecColl.spec2d)
        for i in range(l2d):
            spec = SpecColl.spec2d[i]
            name = spec.Name
            hasGate = spec.hasGate
            item = QTreeWidgetItem(self.treeWidget, [name])
            item.spec = spec
            if hasGate:
                subitem = QTreeWidgetItem(item, ["Gate"])
                subitem.spec = spec
            self.treeWidget.addTopLevelItem(item)

        
    def connectsim(self):
        self.SpecColl.connectsim()
        self.PopulateTree()
        self.SpecCanvas.setSpecIndex(0,False)
        
    def sim(self):
        ## Start or stop a simulation, which runs in c++
        if not self.SpecColl.isRunning:
            self.simButton.setText("Stop Simulation")
            self.SpecColl.startsim()
        else:
            self.simButton.setText("Start Simulation")
            self.SpecColl.stopsim()
            

    def itemclicked(self,it,col):
        self.SpecCanvas.setSpecIndex(it.spec.num,it.spec.is2D)
       
        
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

        ## XRange
        self.a = self.addAction(QtGui.QIcon(iconDir + "XRangeIcon.ico"),
                                "X Range", plotCanvas.xInteractiveZoom)
        self.a.setToolTip("Adjust the x-range")
        self._actions['xrange'] = self.a

        ## YRange
        self.a = self.addAction(QtGui.QIcon(iconDir + "YRangeIcon.ico"),
                                "Y Range", plotCanvas.yInteractiveZoom)
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
        self.a = self.addWidget(QSplitter())

        ## net area
        self.a = self.addAction(QtGui.QIcon(iconDir + "NetAreaIcon.ico"),
                                "Net Area", plotCanvas.netArea)
        self.a.setToolTip("Calculate the net area under a peak")
        self._actions['netarea'] = self.a

        ## gross area
        self.a = self.addAction(QtGui.QIcon(iconDir + "GrossAreaIcon.ico"),
                                "Gross Area", plotCanvas.grossArea)
        self.a.setToolTip("Calculate the gross, background-subtracted area under a peak")
        self._actions['grossarea'] = self.a
        
        ## Add a Splitter
        self.a = self.addWidget(QSplitter())
        
    def select_tool(self):
        print("You clicked the selection tool")
