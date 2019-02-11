# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'LENAFracFitGUI.ui'
#
# Created by: PyQt5 UI code generator 5.6
#
# WARNING! All changes made in this file will be lost!

import sys
sys.path.append('./modules')

import random
import matplotlib
##matplotlib.use("Qt5Agg")
from PySide2 import QtCore, QtGui, QtWidgets
from PySide2.QtWidgets import QApplication, QMainWindow, QMenu, QVBoxLayout, QSizePolicy, QMessageBox, QWidget, QToolBar

from SpectrumCanvasQt import *

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):

        ## The main window
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(1200, 780)
        MainWindow.setWindowTitle("LENAFracFit")

        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")

        self.horizontalLayoutWidget = QtWidgets.QWidget(self.centralwidget)
        self.horizontalLayoutWidget.setGeometry(QtCore.QRect(10, 10, 1190, 730))
        self.horizontalLayoutWidget.setObjectName("horizontalLayoutWidget")
        self.horizontalLayout = QtWidgets.QHBoxLayout(self.horizontalLayoutWidget)
        self.horizontalLayout.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout.setObjectName("horizontalLayout")

        self.frame = QtWidgets.QFrame(self.horizontalLayoutWidget)
        self.frame.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.frame.setFrameShadow(QtWidgets.QFrame.Raised)
        self.frame.setObjectName("frame")
        
        ## ----------------------------------------------------------------------
        ## The Left-hand menu frame
        self.LHMenuFrame = QtWidgets.QFrame(self.frame)
        self.LHMenuFrame.setGeometry(QtCore.QRect(10, 10, 260, 720))
        self.LHMenuFrame.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.LHMenuFrame.setFrameShadow(QtWidgets.QFrame.Raised)
        self.LHMenuFrame.setObjectName("LHMenuFrame")

        ## A couple of buttons
        self.loadButton = QtWidgets.QPushButton(self.LHMenuFrame)
        self.loadButton.setGeometry(QtCore.QRect(10, 10, 240, 25))
        self.loadButton.setObjectName("loadButton")
        self.loadButton.setText("Load Data File (ascii)")
        self.testButton = QtWidgets.QPushButton(self.LHMenuFrame)
        self.testButton.setGeometry(QtCore.QRect(10, 60, 240, 25))
        self.testButton.setObjectName("testButton")
        self.testButton.setText("This button does nothing!")

        ## Some text
        self.label = QtWidgets.QLabel(self.LHMenuFrame)
        self.label.setGeometry(QtCore.QRect(10, 360, 58, 18))
        self.label.setText("Channel")
        self.label_2 = QtWidgets.QLabel(self.LHMenuFrame)
        self.label_2.setGeometry(QtCore.QRect(190, 360, 58, 18))
        self.label_2.setText("---")

        ##----------------------------------------------------------------------
        ## The Right-hand plotting area

        self.tabWidget = QtWidgets.QTabWidget(self.frame)
        self.tabWidget.setGeometry(QtCore.QRect(280, 10, 900, 600))
        self.tab = QtWidgets.QWidget()

        ## The actual plot sits in the first tab!
        self.Matplotlib = QtWidgets.QWidget(self.tab)
        self.Matplotlib.setGeometry(QtCore.QRect(10, 10, 880, 580))
        self.Matplotlib.setObjectName("Matplotlib")
        l = QVBoxLayout(self.Matplotlib)
        mpcanvas = NormalSpectrumCanvas(self.Matplotlib)
        toolbar=NavigationToolbar(mpcanvas, MainWindow)
        l.addWidget(toolbar)
        l.addWidget(mpcanvas)

        ## Second tab (nothing here!)
        self.tabWidget.addTab(self.tab, "")
        self.tab_2 = QtWidgets.QWidget()
        self.tab_2.setObjectName("tab_2")
        self.tabWidget.addTab(self.tab_2, "There's nothing in this tab!")

        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab), "Spectrum Inspector")
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_2), "Empty Tab")

        ## Finally get everything laid out and set up
        self.horizontalLayout.addWidget(self.frame)
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 900, 30))
        
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        MainWindow.setStatusBar(self.statusbar)

        self.tabWidget.setCurrentIndex(0)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    MainWindow = QtWidgets.QMainWindow()
    ui = Ui_MainWindow()
    ui.setupUi(MainWindow)
    MainWindow.show()
    sys.exit(app.exec_())

