import sys, os
from PySide2 import QtCore, QtWidgets, QtGui

class Ui_MainWindow(QtWidgets.QMainWindow):
    def __init__(self):

        super(Ui_MainWindow,self).__init__()

        ##self.SpecCanvas = SpecCanvas
        #### Grab the spectrum collection
        ##self.SpecColl = self.SpecCanvas.SpecColl
        
        ##  -----------------------------------------------------------------
        ##  Menu ..  ..                                                Help
        ##  -----------------------------------------------------------------
        ##  Control buttons (run, stop, etc.)
        ##  -----------------------------------------------------------------
        ##            |
        ##   Spectrum |  Spectrum tabs
        ##   Tree     |
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
        treeFrame.setMaximumWidth(260)
        ##        treeFrame.setFrameShape(QtWidgets.QFrame.StyledPanel)
        dataFrame = QtWidgets.QFrame()
        dataFrame.setMinimumSize(900,600)

        ##----------------------------------------------------------------------
        ## Thre tree widget
        self.treeWidget = QtWidgets.QTreeWidget()
        self.treeWidget.setColumnCount(1)
        header = QtWidgets.QTreeWidgetItem(["Spectra"])
        self.treeWidget.setHeaderItem(header)
        item = QtWidgets.QTreeWidgetItem(self.treeWidget, ["tmp"])#[self.SpecCanvas.Spec.Name])
        #        item.spec = SpecCanvas.Spec
        self.treeWidget.addTopLevelItem(item)
        self.treeWidget.itemClicked.connect(self.itemclicked)

        ## FENRIS logo
        pixmap = QtGui.QPixmap('../images/FENRISLogo-notext.png')
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
        ## Layout the mainFrame grid
        gridmainFrame = QtWidgets.QGridLayout(mainFrame)
        gridmainFrame.setSpacing(10)
        gridmainFrame.addWidget(treeFrame,1,0)
        gridmainFrame.addWidget(dataFrame,1,1)



        ## Make a third grid for the spectrum and command window
        ## - tabWidget holds the spectra
        ## - commandWidget is the command editor

        ## Make the spectrum tabs
        tabWidget = QtWidgets.QTabWidget()
        ## Tab 1
        tab1 = QtWidgets.QWidget()
        test2Edit = QtWidgets.QTextEdit(tab1)
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
                                 self.LoadData)
        ## Load HDF data
        self.file_menu.addAction('&Load HDF File',
                                 self.LoadHDFData)
        ## Connect to a simulation
        self.file_menu.addAction('&Connect simulation',
                                 self.connectsim)
        ## Connect to MIDAS
        self.file_menu.addAction('&Connect MIDAS',
                                 self.connectmidas)
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
        startAction = QtWidgets.QAction(QtGui.QIcon(iconDir + 'Start.ico'), 'Start Run', self)
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

        
    def LoadData(self):
        self.SpecCanvas.LoadData()
        
    def LoadHDFData(self):
        print("Loading HDF Data")
        self.SpecCanvas.LoadHDFData()
        self.PopulateTree()

    def connectsim(self):
        self.SpecColl.connectsim()
        self.PopulateTree()
        self.SpecCanvas.setSpecIndex(0,False)

    def connectmidas(self):
        if not self.SpecColl.isRunning:
            self.SpecColl.isRunning = True
            print("Running!")
        else:
            self.SpecColl.isRunning = False
        self.SpecColl.connectmidas()
        self.PopulateTree()
        self.SpecCanvas.setSpecIndex(0,False)
            ##self.SpecColl.midasrun()
    def startmidas(self):
        print("Running midas")
    def stopmidas(self):
        print("Stopping midas")
    def setgate(self):
        SpecCanvas.getGate()

    def itemclicked(self,it,col):
        self.SpecCanvas.setSpecIndex(it.spec.num,it.spec.is2D)

        
def main():
    
    app = QtWidgets.QApplication(sys.argv)
    ex = Ui_MainWindow()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
