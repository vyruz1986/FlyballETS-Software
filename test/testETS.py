# test ESP32
import os
import serial
import time
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
time.sleep(2)
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
invalidInput = False

# selectedrace = input("Select race: ") # 0/1/2/20 or end
flatten_listOfRaces = []
argument_number = 1
while argument_number < len(sys.argv):
    #print(argument_number)
    selectedRace = str(sys.argv[argument_number])
    #print(selectedrace)
    if selectedRace[0].isdigit():
        listOfRaces = list(selectedRace.split(","))
        #print(listOfRaces)
        elem_iter = 0
        for elem in listOfRaces:
            #print(elem)
            if '-' in elem:
                raceRange = elem.split("-")
                #print(racerange)
                firstRace = int(raceRange[0])
                lastRace = int(raceRange[1])
                #print(firstrace)
                elem = []
                while firstRace <= lastRace:
                    elem.append(str(firstRace))
                    #print(elem)
                    firstRace += 1
                listOfRaces[elem_iter] = elem
            elem_iter += 1
        flatten_listOfRaces = list(flat2gen(listOfRaces))
    elif selectedRace == "-d":
        debugmode = True
    elif selectedRace == "-all" or selectedRace == "-stab":
        initial_count = 0
        stabNumOfLoops = 1
        if selectedRace == "-stab":
            argument_number += 1
            stabNumOfLoops = int(sys.argv[argument_number])
        for Path in os.listdir(os.getcwd() + "/testcases"):
            if os.path.isfile(os.path.join(os.getcwd() + "/testcases", Path)):
                flatten_listOfRaces.append(str(initial_count))
                initial_count += 1
    else:
        print("Error: Invalid input")
        invalidInput = True
    argument_number += 1
#print(flatten_listOfRaces)

for loop in range(stabNumOfLoops):
    testNum = 0
    #for selectedrace in flatten_listOfRaces:
    while testNum < len(flatten_listOfRaces):
        selectedRace = str(flatten_listOfRaces[testNum])
        ser.write(b"race " + selectedRace.encode('utf-8') + b"\n")
        time.sleep(2)

        racefile = open(os.getcwd() + "/testcases" + "/RACE" + selectedRace + ".txt", "r")
        if loop > 1:
            exitfile = open(pathTestsOutputFolder + "/race" + selectedRace + "_" + loop + ".txt", "wb")
            exitfile = open(pathTestsOutputFolder + "/race" + selectedRace + ".txt", "wb")
        time.sleep(2)

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
                    time.sleep(2)
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

        if invalidInput:
            selectedRace = "end"
        else:
            bytetime = b'0'
            raceEND = False
            readline = ser.readline()
            ser.write(b"start" + b"\n")  # \x53\x54\x41\x52\x54\x0a (utf-8)
            time.sleep(2)
            stopline = ""
            consoleProgressPrint = "Running race " + selectedRace + "."
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
                fileTestsSummary.write(b"Race " + selectedRace.encode('utf-8') + b" FAIL\n")
                print("FAIL")
            else:
                fileTestsSummary.write(b"Race " + selectedRace.encode('utf-8') + b" PASSED\n")
                print("PASSED")

            ser.write(b"reset" + b"\n")
            for i in range(4):
                readline = ser.readline()
            ser.write(b"preparefortesting\n")
            time.sleep(2)
            testNum += 1
            racefile.close()
exitfile.close()
fileTestsSummary.close()