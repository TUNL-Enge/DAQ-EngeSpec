import sys
import matplotlib
##matplotlib.use("Qt5Agg")
from PySide2 import QtCore
from PySide2 import QtGui
from PySide2.QtWidgets import QMainWindow, QFrame, QMenu, QVBoxLayout, QHBoxLayout, \
    QSizePolicy, QMessageBox, QWidget, QToolBar, QFileDialog, QPushButton, QLabel, QTabWidget,\
    QMenuBar, QStatusBar, QTextEdit
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolbar
from matplotlib.backend_tools import ToolBase

class Ui_MainWindow(QMainWindow):
    def __init__(self, SpecCanvas):

        ## super().__init__ allows us to have all properties of
        ## QMainWindw available
        super().__init__()

        ##self.Spec = Spec
        self.SpecCanvas = SpecCanvas 
        
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
        self.loadButton.setText("Load Data File (ascii)")
        self.loadButton.clicked.connect(SpecCanvas.LoadData)
        ## Set log scale
        self.logButton = QPushButton(self.LHMenuFrame)
        self.logButton.setGeometry(QtCore.QRect(10, 40, 240, 25))
        self.logButton.setObjectName("logButton")
        self.logButton.setText("Log Scale")
        self.logButton.clicked.connect(SpecCanvas.ToggleLog)
        ## Auto scale
        self.autoscaleButton = QPushButton(self.LHMenuFrame)
        self.autoscaleButton.setGeometry(QtCore.QRect(10, 70, 240, 25))
        self.autoscaleButton.setObjectName("autoscaleButton")
        self.autoscaleButton.setText("Auto Scale")
        self.autoscaleButton.clicked.connect(SpecCanvas.Autosize)
        ## Zoom in X-direction
        self.testButton = QPushButton(self.LHMenuFrame)
        self.testButton.setGeometry(QtCore.QRect(10, 100, 240, 25))
        self.testButton.setObjectName("testButton")
        self.testButton.setText("X zoom in")
        self.testButton.clicked.connect(SpecCanvas.xZoomIn)
        ## Collect some clicks
        self.clicksButton = QPushButton(self.LHMenuFrame)
        self.clicksButton.setGeometry(QtCore.QRect(10, 130, 240, 25))
        self.clicksButton.setObjectName("clicksButton")
        self.clicksButton.setText("Test some clicks!")
        self.clicksButton.clicked.connect(SpecCanvas.getClicks)
        ## Zoom slider x
        self.zoomxButton = QPushButton(self.LHMenuFrame)
        self.zoomxButton.setGeometry(QtCore.QRect(10, 160, 240, 25))
        self.zoomxButton.setObjectName("zoomxButton")
        self.zoomxButton.setText("Zoom x-range!")
        self.zoomxButton.clicked.connect(SpecCanvas.xInteractiveZoom)
        ## Zoom slider y
        self.zoomyButton = QPushButton(self.LHMenuFrame)
        self.zoomyButton.setGeometry(QtCore.QRect(10, 190, 240, 25))
        self.zoomyButton.setObjectName("zoomyButton")
        self.zoomyButton.setText("Zoom y-range!")
        self.zoomyButton.clicked.connect(SpecCanvas.yInteractiveZoom)

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
        toolbar = MyCustomToolbar(self.SpecCanvas, self)
        ##toolbar = NavigationToolbar(self.SpecCanvas, self)
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
        sys.stdout = OutLog(self.TextEdit, sys.stdout)
        sys.stderr = OutLog(self.TextEdit, sys.stderr, QtGui.QColor(255,0,0) )

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
        ##self.remove_tool('forward')
        # remove the unwanted button
        #POSITION_OF_CONFIGURE_SUBPLOTS_BTN = 6
        #self.DeleteToolByPos(POSITION_OF_CONFIGURE_SUBPLOTS_BTN) 
        self.toolitems = (('Home', 'Lorem ipsum dolor sit amet', 'home', 'home'),
                          ('Back', 'consectetuer adipiscing elit', 'back', 'back'),
                          ('Forward', 'sed diam nonummy nibh euismod', 'forward', 'forward'),
                          (None, None, None, None),
                          ('Pan', 'tincidunt ut laoreet', 'move', 'pan'),
                          ('Zoom', 'dolore magna aliquam', 'zoom_to_rect', 'zoom'),
                          (None, None, None, None),
                          ('Subplots', 'putamus parum claram', 'subplots', 'configure_subplots'),
                          ('Save', 'sollemnes in futurum', 'filesave', 'save_figure'),
                          ('Port', 'Select', "select", 'select_tool'),
        )
        # create the default toolbar
        NavigationToolbar.__init__(self, plotCanvas, parent)
        #NavigationToolbar.add_tool("test",NewTool)

    def select_tool(self):
        print("You clicked the selection tool")

class NewTool(ToolBase):
        image = r"/home/longland/project/Logos-Photos/FENRISLogo.png"
