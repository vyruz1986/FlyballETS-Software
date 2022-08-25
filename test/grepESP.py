#grep from log
import serial
import os
ser = serial.Serial('COM7', 115200)
if ser.isOpen():
    print("COM port is available")
else:
    print("COM port is not available")

interestingdata = open(os.getcwd() + "\\interestingDATA.txt", 'wb')
readline = ser.readline()[:-2]

race = int(input("Select race: "))
while "Last dog came back." not in readline:
    readline = ser.readline()[:-2]
    #decodeline = readline.decode('utf-8')
    #splitdecodeline = decodeline.split("(): ")

while "Net" not in splitdecodeline:
    readline = ser.readline()[:-2]
    decodeline = readline.decode('utf-8')
    splitdecodeline = decodeline.split("(): ")
    interestingdata.write(splitdecodeline[1].encode('utf-8') + b'\n')

interestingdata.close()