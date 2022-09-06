# test ESP32
import os
import re
import serial
import time
import string
import sys
from datetime import datetime
from threading import Timer
from serial.tools import list_ports

def flat2gen(alist):
            for item in alist:
                if isinstance(item, list):
                    for subitem in item: yield subitem
                else:
                    yield item

def command_send_midprogramm(command):
    ser.write(command.encode('utf-8') + b"\n")

    
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

OutputRootFolder = "results"
pathOutputRootFolder = os.path.join(os.getcwd(), OutputRootFolder)
if not os.path.exists(pathOutputRootFolder):
    os.mkdir(pathOutputRootFolder)

pathTestsOutputFolder = os.path.join(str(pathOutputRootFolder), str(datetime.now().strftime("%Y-%m-%d %H-%M-%S")))
os.mkdir(pathTestsOutputFolder)

fileTestsSummary = open(pathTestsOutputFolder + "/!summary.txt", "wb")

#ser.write(b"reboot" + b"\n")
time.sleep(2)
ser.write(b"stop" + b"\n")
time.sleep(2)
ser.write(b"reset" + b"\n")
time.sleep(2)

ser.write(b"preparefortesting\n")
readlineSkip = ser.readline()[:-2]
while b"Simulation mode" not in readlineSkip:
    readlineSkip = ser.readline()[:-2]

racenumber = 0
debugmode = False
endless = False
invalidinput = False

# selectedrace = input("Select race: ") # 0/1/2/20 or end
flatten_listOfRaces = []
argument_number = 1
while argument_number < len(sys.argv):
    #print(argument_number)
    selectedrace = str(sys.argv[argument_number])
    #print(selectedrace)
    if selectedrace[0].isdigit():
        listOfRaces = list(selectedrace.split(","))
        #print(listOfRaces)
        elem_iter = 0
        for elem in listOfRaces:
            #print(elem)
            if '-' in elem:
                racerange = elem.split("-")
                #print(racerange)
                firstrace = int(racerange[0])
                lastrace = int(racerange[1])
                #print(firstrace)
                elem = []
                while firstrace <= lastrace:
                    elem.append(str(firstrace))
                    #print(elem)
                    firstrace += 1
                listOfRaces[elem_iter] = elem
            elem_iter += 1
        flatten_listOfRaces = list(flat2gen(listOfRaces))
    elif selectedrace == "-d":
        debugmode = True
    elif selectedrace == "-all" or selectedrace == "-stab":
        initial_count = 0
        if selectedrace == "-stab":
            endless = True
        for Path in os.listdir(os.getcwd() + "/testcases"):
            if os.path.isfile(os.path.join(os.getcwd() + "/testcases", Path)):
                flatten_listOfRaces.append(str(initial_count))
                initial_count += 1
    else:
        print("Error: Invalid input")
        invalidinput = True
    argument_number += 1
#print(flatten_listOfRaces)

if endless:
    stabTime = time.time()
    #print(stabTime)

testNum = 0
#for selectedrace in flatten_listOfRaces:
while testNum < len(flatten_listOfRaces):
    selectedrace = str(flatten_listOfRaces[testNum])
    ser.write(b"race " + selectedrace.encode('utf-8') + b"\n")
    time.sleep(1)
    '''
    if selectedrace == "-stab":
        endless = True
        ser.write(b"race 0" + b"\n")
        for a in range(2):
            numofraces = ser.readline()[:-2]
        splitnumofraces = numofraces.split(b"races: ")
        # print(splitnumofraces)
        ammountofraces = int(splitnumofraces[1])
        #exitfile.write(b"//race 0" + b'\n')
        racefile = open(os.getcwd() + "/RACE0.txt", "r")
    '''
    racefile = open(os.getcwd() + "/testcases" + "/RACE" + selectedrace + ".txt", "r")
    exitfile = open(pathTestsOutputFolder + "/race" + selectedrace + ".txt", "wb")
    time.sleep(1)
    linestoskip = 0
    additionalargs = racefile.readline()
    while additionalargs.startswith("$"):
        splitadditionalargs = additionalargs.split(";")
        # print(splitadditionalargs)
        splitargs_len = len(splitadditionalargs)
        i = 1
        while i < splitargs_len and splitadditionalargs[i] != "\n":
            if splitadditionalargs[0] == "$innit":
                innitcommands = splitadditionalargs[i]
                readline = ser.readline()[:-2]
                decodeline = readline.decode('utf-8')
                splitdecodeline = decodeline.split("(): ")
                # print(splitdecodeline[1])
                command_send_midprogramm(innitcommands)
                i += 1
                time.sleep(1)
            elif splitadditionalargs[0] == "$commands":
                command_sendtime = float(splitadditionalargs[i])
                command_name = (splitadditionalargs[i+1],)
                i += 2
                timer = Timer(command_sendtime, command_send_midprogramm, args=command_name)
                timer.start()
                linestoskip += 1
        additionalargs = racefile.readline()
    racefile.seek(0)
    for i in range(linestoskip):
       racefile.readline()

    if invalidinput:
        selectedrace = "end"
    else:
        bytetime = b'0'
        raceEND = False
        readline = ser.readline()
        ser.write(b"start" + b"\n")  # \x53\x54\x41\x52\x54\x0a (utf-8)
        time.sleep(1)
        stopline = ""
        consoleProgressPrint = "Running race " + selectedrace + "."
        print(consoleProgressPrint, end='')
        while raceEND != True:
            print("\r", end='')
            readline = ser.readline()[:-2]
            # print(readline)
            decodeline = readline.decode('utf-8')
            splitdecodeline = decodeline.split("(): ")
            exitfile.write(splitdecodeline[1].encode('utf-8') + b'\n')
            if debugmode:
                print(splitdecodeline[1])
            else:
                consoleProgressPrint += "."
                print(consoleProgressPrint, end='')
            if splitdecodeline[1] == "RS:  STOP  ":
                raceEND = True
                stopline = readline
                racenumber += 1

        notOKcount = 0
        while b"Net" not in stopline:
            readline = ser.readline()[:-2]
            stopline = readline
            decodeline = readline.decode('utf-8')
            splitdecodeline = decodeline.split("(): ")
            #exitfile.write(splitdecodeline[1].encode('utf-8'))
            if ((splitdecodeline[1].startswith("Dog ") or splitdecodeline[1].startswith(" Team") or splitdecodeline[1].startswith("  Net")) and raceEND):
                #exitfile.write(splitdecodeline[1].encode('utf-8') + b'\n')
                lengthofline = len(splitdecodeline[1])
                normdecodeline = splitdecodeline[1]
                for x in range(30-lengthofline):
                    normdecodeline = normdecodeline + " "
                expectedline = racefile.readline()[:-1]
                if expectedline.startswith("//RACE") or expectedline.startswith("$"):
                    expectedline = racefile.readline()[:-1]
                if splitdecodeline[1] == expectedline:
                    #print(normdecodeline, colored("OK", 'green'))
                    exitfile.write(normdecodeline.encode('utf-8') + b'  OK' + b'\n')
                else:
                    notOKcount += 1
                    #print(normdecodeline, colored("NOK", 'red'), " Exp: ", Fore.YELLOW + expectedline, Fore.RESET)
                    exitfile.write(normdecodeline.encode('utf-8') + b'  NOK' + b'   Exp: ' + expectedline.encode('utf-8') + b'\n')

        if notOKcount != 0:
            fileTestsSummary.write(b"Race " + selectedrace.encode('utf-8') + b" FAIL\n")
            print("FAIL")
        else:
            fileTestsSummary.write(b"Race " + selectedrace.encode('utf-8') + b" PASSED\n")
            print("PASSED")

        ser.write(b"reset" + b"\n")
        for i in range(4):
            readline = ser.readline()
        ser.write(b"preparefortesting\n")
        time.sleep(2)
        testNum += 1
        racefile.close()

        if endless:
            stabTime2 = time.time()
            #print(stabTime2)
            #print(stabTime2 - stabTime)
            if stabTime2 - stabTime >= 3600.0:
                testNum = len(flatten_listOfRaces)
exitfile.close()
fileTestsSummary.close()
