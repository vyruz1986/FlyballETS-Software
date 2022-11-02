from ast import While
import serial
import time
import os
from serial.tools import list_ports

serialPorts = list(list_ports.comports())
serialPortIndex = 0
for p in serialPorts:
    serialPortIndex += 1
    print(serialPortIndex, ". ", end='')
    print(p)
numberOfSerialPorts = len(serialPorts)
selectedPortNumber = int(input("Select port: "))
print(serialPorts[selectedPortNumber - 1])
ser = serial.Serial(serialPorts[selectedPortNumber - 1].device, 115200)
time.sleep(1)
if ser.isOpen():
    print("SERIAL port is open")

ser.write(b"reset" + b"\n")
time.sleep(1)

addCommands = input("Enter additional commands (setup for certain races) or 'done' to continue: ")
while addCommands != 'done':
    ser.write(addCommands.encode('utf-8') + b"\n")
    time.sleep(1)
    addCommands = input("Enter additional commands (setup for certain races) or 'done' to continue: ")

selectedrace = input("Enter the race number: ")
testqueue = False
if '-' in selectedrace:
        racerange = selectedrace.split("-")
        firstrace = int(racerange[0])
        lastrace = int(racerange[1])
        selectedrace = racerange[0]
        testqueue = True
while selectedrace.isdigit():
    ser.write(b"reset" + b"\n")
    time.sleep(1)
    testcase = open(os.getcwd() + "/testcases" + "/RACE" + selectedrace + ".txt", "wb")
    ser.write(b"race " + selectedrace.encode('utf-8') + b"\n")
    time.sleep(1)
    ser.write(b"start\n")
    time.sleep(1)
    readline = ser.readline()
    while b"Last dog came back." not in readline:
        readline = ser.readline()
        print(readline)
    print('###')
    
    writeline = ser.readline().split(b"(): ")
    stopline = b""
    while b"  CT: " not in stopline:
        print(writeline[1])
        testcase.write(writeline[1])
        stopline = writeline[1]
        writeline = ser.readline().split(b"(): ")
    
    testcase.close()
    
    if testqueue:
            firstrace += 1
            selectedrace = str(firstrace)
            if firstrace == lastrace:
                testqueue = False
    else:
        selectedrace = input("Enter the race number (blank to end programm): ")
        if '-' in selectedrace:
            racerange = selectedrace.split("-")
            firstrace = int(racerange[0])
            lastrace = int(racerange[1])
            selectedrace = racerange[0]
            testqueue = True
