import sys
from PySide2 import QtCore, QtWidgets, QtGui

class Ui_MainWindow(QtWidgets.QMainWindow):
    def __init__(self):

        super(Ui_MainWindow,self).__init__()

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
        self.file_menu = QtWidgets.QMenu('&File', self)
        self.file_menu.addAction('&Quit', self.fileQuit,
           QtCore.Qt.CTRL + QtCore.Qt.Key_Q)
        self.menuBar().addMenu(self.file_menu)
        self.help_menu = QtWidgets.QMenu('&Help', self)
        self.menuBar().addSeparator()
        self.menuBar().addMenu(self.help_menu)
        self.help_menu.addAction('&About', self.about)

        ## The main widget that holds everything else
        self.main_widget = QtWidgets.QWidget(self)

        ## Start with two vertical groups
        ## - runControlsFrame holds all of the run start, stop, gates, etc.
        ## - mainFrame holds the tree, spectrum, etc
        runControlsFrame = QtWidgets.QFrame()
        runControlsFrame.setMinimumSize(50,50)
        runControlsFrame.setMaximumHeight(50)
        mainFrame = QtWidgets.QFrame()
        mainFrame.setFrameShape(QtWidgets.QFrame.StyledPanel)
        mainFrame.setFrameShadow(QtWidgets.QFrame.Raised)
        
        ## Layout of vertical groups
        gridmain = QtWidgets.QGridLayout(self.main_widget)
        gridmain.setSpacing(10)
        gridmain.addWidget(runControlsFrame, 1, 0)
        gridmain.addWidget(mainFrame, 2, 0)

        
        ## Make a second grid for the mainFrame
        ## - treeFrame is Left-hand frame for the tree
        ## - dataFrame holds the spectrum tabs and command window
        treeFrame = QtWidgets.QFrame()
        treeFrame.setMinimumSize(260,720)
        treeFrame.setMaximumWidth(260)
        treeFrame.setFrameShape(QtWidgets.QFrame.StyledPanel)
        dataFrame = QtWidgets.QFrame()
        dataFrame.setMinimumSize(900,600)

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

        
def main():
    
    app = QtWidgets.QApplication(sys.argv)
    ex = Ui_MainWindow()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
