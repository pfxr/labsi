#!/usr/bin/python
# -*- coding: iso-8859-3 -*-
	 	
import sys, time, serial
 
from PyQt4 import QtCore, QtGui
 
from DialogoLed_Ui import Ui_DialogoLed

from threading import Thread

from time import sleep
	 
# Variables Globales
prontos=0
def receiving(ser,sel):
	global prontos
	while (1):		
		txtArduino = ser.readline()
		if(txtArduino[0]=='1' and txtArduino[1]=='4'):
			prontos=prontos+1
			sel.ui.label_5.setText("pronto")
				
		if(txtArduino[0]=='2' and txtArduino[1]=='4'):
			prontos=prontos+1
			sel.ui.label_6.setText("pronto") 
		if(prontos==2):
			prontos=0
			sel.ui.frame_2.hide()	
			sel.ui.frame_3.show()
			
		if(txtArduino[0]=='2' and txtArduino[1]=='3'):
			sel.ui.label_16.setText("1")
			#sel.ui.progressBar.setValue(2)
		txtArduino=" "
			

class DialogoLed(QtGui.QMainWindow):
	def __init__(self, parent=None):
	        super(DialogoLed, self).__init__(parent)
	        #QtGui.QWidget.__init__(self, parent)
	        self.ui = Ui_DialogoLed()
	        self.ui.setupUi(self)
	 
	        #self.arduino = serial.Serial('/dev/ttyACM1', 9600)
	 	self.ui.frame_2.hide()	
		self.ui.frame_3.hide()
		

	 
	    	#buttons
	        QtCore.QObject.connect(self.ui.pushButton,QtCore.SIGNAL("clicked()"), self.comeca)

	

	def comeca(self):
		jogador1=self.ui.lineEdit_2.text()
		jogador2=self.ui.lineEdit.text()

		self.ui.frame.hide()	
		self.ui.frame_2.show()
		
		
		self.ui.label_3.setText(jogador1)
		self.ui.label_2.setText(jogador2)
		self.ui.label_10.setText(jogador1)
		self.ui.label_9.setText(jogador2)
		

	      
		self.arduino.write("1")	
  		Thread(target=receiving, args=(self.arduino,self)).start()             
		
		
	 
	      
	 
	def quit(self):	
		Thread.stop()
	    	sys.exit(app.exec_())

	def closeEvent(self, event):
       	    	self.arduino.close();
	
		sys.exit(app.exec_())
		
	 
if __name__ == "__main__":
	app = QtGui.QApplication(sys.argv)
	myapp = DialogoLed()
	myapp.show()
	sys.exit(app.exec_())
