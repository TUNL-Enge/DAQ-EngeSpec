import matplotlib
##matplotlib.use("Qt5Agg")
from PySide2 import QtCore
from PySide2.QtWidgets import QMainWindow, QFrame, QMenu, QVBoxLayout, QHBoxLayout, \
    QSizePolicy, QMessageBox, QWidget, QToolBar, QFileDialog, QPushButton, QLabel, QTabWidget,\
    QMenuBar, QStatusBar
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolbar

class Ui_MainWindow(QMainWindow):
    def __init__(self, SpectrumCanvas):

        ## super().__init__ allows us to have all properties of
        ## QMainWindw available
        super().__init__()
        
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
        self.loadButton = QPushButton(self.LHMenuFrame)
        self.loadButton.setGeometry(QtCore.QRect(10, 10, 240, 25))
        self.loadButton.setObjectName("loadButton")
        self.loadButton.setText("Load Data File (ascii)")
        ##self.loadButton.clicked.connect(spectrumObject.LoadData)
        self.testButton = QPushButton(self.LHMenuFrame)
        self.testButton.setGeometry(QtCore.QRect(10, 60, 240, 25))
        self.testButton.setObjectName("testButton")
        self.testButton.setText("This button does nothing!")

        ## Some text
        self.label = QLabel(self.LHMenuFrame)
        self.label.setGeometry(QtCore.QRect(10, 360, 58, 18))
        self.label.setText("Channel")
        self.label_2 = QLabel(self.LHMenuFrame)
        self.label_2.setGeometry(QtCore.QRect(190, 360, 58, 18))
        self.label_2.setText("---")

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
        self.mpcanvas = SpectrumCanvas(self.Matplotlib)
        toolbar=NavigationToolbar(self.mpcanvas, self)
        l.addWidget(toolbar)
        l.addWidget(self.mpcanvas)

        ## Second tab (nothing here!)
        self.tabWidget.addTab(self.tab, "")
        self.tab_2 = QWidget()
        self.tab_2.setObjectName("tab_2")
        self.tabWidget.addTab(self.tab_2, "There's nothing in this tab!")

        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab), "Spectrum Inspector")
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_2), "Empty Tab")

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
