import cv2
import numpy
import imutils
import numpy as np
import time
from picamera.array import PiRGBArray
from picamera import PiCamera

#to check  the ouput of the last command given to the arduino
#copy paste in your file
def checkOuput(arduino):
    #global arduino
    cont=True
    while cont:
        reci=arduino.readline()
        if(reci.find('Ended')==0):
            cont=False
    return

#checkAvailable take an argument which is name of component as in gerber file which is given from thr final.py
#it should return true if the component can be solved by  this program
#for e. this resistor.py can work for R1,R2,R3,R4,etc
def checkAvailable(name):
    print("got the ", name," from master")
    parts=['R1','R2','R3','R4']#add according to your need
    for i in parts:
        if i == name.encode('utf8'):
            print ("checked")
            return True
    return False
#it is called from the final.py if checkAvailable returns True
#argument are final degree of the component as in gerberfile, arduino port and other command, source and camera info
def offsetAngle(deg,arduino,s,c):
    angle=int(checkCentre(s,c)[0])
    while angle!=0:
        if angle==180:
            break
        angle=int(checkCentre(s,c)[0])
        if angle>90:
            angle=180-angle
            cnd="a "+str(-1*angle)
            print cnd
            arduino.write(cnd)
            checkOuput(arduino)
        else:
            cnd="a "+str(angle)
            print cnd+"in 90"
            arduino.write(cnd)
            checkOuput(arduino)
    cnd="a "+str(deg)
    print cnd
    arduino.write(cnd)
    checkOuput(arduino)
    return (checkCentre(s,c)[1])
# add your own featuers inside the for loop
def checkCentre(s,c):
    centerx_offset=0
    centery_offset=0
    angle=-1
    times=0
    #customize the below loop according to your need in order to add new component
    for frame1 in c.capture_continuous(s, format="bgr", use_video_port=True):
        #finding frame centre
        frame = frame1.array
        #print frame
        cv2.imshow('original',frame)
        midx = frame.shape[1]/2+centerx_offset
        midy = frame.shape[0]/2+centery_offset
        img=frame
        #cv2.circle(frame, (midx, midy), 7, (255, 0, 0), -1)

        #masking process
        #mask = np.zeros(img.shape,'uint8')
        gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
        #removing texture use the first when required
        blured = cv2.GaussianBlur(gray,(5,5),100)
        #use it if required
        _,th = cv2.threshold(blured.copy(),0,255,cv2.THRESH_BINARY+cv2.THRESH_OTSU)
        kernel = np.ones((5,5),np.uint8)
        erosion = cv2.erode(blured,kernel,iterations = 10)
        dilated = cv2.dilate(erosion,kernel,iterations = 10)
        edges = cv2.Canny(gray,50,150,apertureSize = 3)
        edges = cv2.bilateralFilter(edges,7,75,75)

        cnts = cv2.findContours(edges.copy(), cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)
        cnts = cnts[0] if imutils.is_cv2() else cnts[1]

        #canvas
        canvas = np.zeros(img.shape,'uint8')
        cv2.drawContours(edges.copy(),cnts,-1,0,2)
        for c in cnts:
            # compute the center of the contour
            M = cv2.moments(c)
            if(int(M["m00"])!=0):
                cY = int(M["m01"] / M["m00"])
                cX = int(M["m10"] / M["m00"])

            # draw the contour and center of the shape on the image
                area=cv2.contourArea(c)
                if(area>1200):
                    rect = cv2.minAreaRect(c)
                    angle = rect[2]
                    if rect[1][0] < rect[1][1]:
                        angle = 180 + angle
                    else:
                        angle = 90+angle
                    angle = round(angle,0)
                    times+=1
                    cv2.drawContours(canvas, [c], -1, (0, 255, 0), 2)
                    cv2.drawContours(img, [c], -1, (0, 255, 0), 2)
                    cv2.circle(canvas, (cX, cY), 7, (255, 255, 255), -1)
                    cv2.circle(img, (cX, cY), 7, (0, 255, 0), -1)
                    cv2.circle(edges, (cX, cY), 7, (255, 255, 255), -1)
                    cv2.putText(canvas, str(angle), (cX - 20, cY - 20),cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)
                    cv2.putText(img, str(angle), (cX - 20, cY - 20),cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
                    cv2.putText(edges, "center", (cX - 20, cY - 20),cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)
        cv2.imshow('chip',canvas)
        s.truncate(0)
        if angle != -1 and times == 3:
            break
    return (angle,(midx-cX,midy-cY))
