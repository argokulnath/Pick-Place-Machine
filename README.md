# About this P & P Machine

* Automated Surface Mount Device (SMD) Pick and Place (P&P) Machine
* we the Authors share equal rights to this Project and Repository. This code is liscenced under GPL v3.0 and any work which is done using this code must mention the Credits to the Authors.
* Authors: Gokulnath A R <argokulnath@yahoo.co.in>  & Chandrakumar <chandru.ck58@gmail.com>

# Description
* This project is an open source SMD pick and place machine an alternative approach in order to minimize the cost of P&P Machine and customizable according to the user needs.
* We used Arduino and Raspberry PI for this project inorder to be true opensource.
* We used Python(RPI) and Arduino(C++) for coding.

# Mechanical details
This code should work for any stepper motor
all Hbot designs should work
* but this code is for linear drive design not for h-bot or other corexy mechanisms (u can customise it if you want to)

# Necessary library for RPI
* Use raspbian os
* Use opencv 3.3.0
* openpyxl for python
* PiRGBArray for python
* PiCamera for python
* GPIO for python
* cv2 for python
* imutils for python
* numpy for python

# Running the program
python final <gerberfile.xlx>  <pickup_file.xlx> 

# Customizing the program

* add your own components for image recognition in the same folder where final.py.
* then add the file name in opencv.txt in next line.
* directions for creating your own file is given in resistor.py file
