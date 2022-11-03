# test ESP32
from genericpath import isfile
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

def serialLineDecode():
    readline = ser.readline()[:-2]
    decodeline = readline.decode('utf-8')
    splitdecodeline = decodeline.split("(): ")
    if len(splitdecodeline) > 1:
        decodedLine = splitdecodeline[1]
    else:
        decodedLine = splitdecodeline[0]
    return decodedLine
    
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

#ser.write(b"reboot" + b"\n")
print("Preparing environment... ")
#time.sleep(2)
ser.write(b"stop" + b"\n")
time.sleep(2)
ser.write(b"reset" + b"\n")
time.sleep(1)

OutputRootFolder = "results"
pathOutputRootFolder = os.path.join(os.getcwd(), OutputRootFolder)
if not os.path.exists(pathOutputRootFolder):
    os.mkdir(pathOutputRootFolder)

ser.write(b"fwver\n")
firmware = ser.readline()[:-2]
while b"Firmware" not in firmware:
    firmware = ser.readline()[:-2]
firmware = firmware[68:].decode("utf-8")
print("Firmware: ", firmware)

pathTestsOutputFolder = os.path.join(str(pathOutputRootFolder), str(datetime.now().strftime("%Y-%m-%d %H-%M-%S v")+ firmware))
os.mkdir(pathTestsOutputFolder)

fileTestsSummary = open(pathTestsOutputFolder + "/!summary.txt", "wb")

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
numberOfStabilityLoops = 1
while argument_number < len(sys.argv):
    #print(argument_number)
    raceToRun = str(sys.argv[argument_number])
    #print(selectedrace)
    if raceToRun[0].isdigit():
        listOfRaces = list(raceToRun.split(","))
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
    elif raceToRun == "-d":
        debugmode = True
    elif raceToRun == "all" or raceToRun == "stab":
        initial_count = 0
        if raceToRun == "stab":
            argument_number += 1
            numberOfStabilityLoops = int(sys.argv[argument_number])
            if numberOfStabilityLoops < 1:
                invalidInput = True
        for Path in os.listdir(os.getcwd() + "/testcases"):
            if os.path.isfile(os.path.join(os.getcwd() + "/testcases", Path)):
                flatten_listOfRaces.append(str(initial_count))
                initial_count += 1
    else:
        invalidInput = True
    argument_number += 1
#print(flatten_listOfRaces)

print("Starting execution... ")
for stabilityLoopIndex in range(numberOfStabilityLoops):
    loopIndex = 0
    #for selectedrace in flatten_listOfRaces:
    while loopIndex < len(flatten_listOfRaces):
        raceToRun = str(flatten_listOfRaces[loopIndex])
        ser.write(b"race " + raceToRun.encode('utf-8') + b"\n")
        time.sleep(1)

        racefilePATH = os.getcwd() + "/testcases" + "/RACE" + raceToRun + ".txt"
        if os.path.isfile(racefilePATH):
            racefile = open(racefilePATH, "r")
            if stabilityLoopIndex > 0:
                exitfile = open(str(stabilityLoopIndex) + "_" + pathTestsOutputFolder + "/race" + raceToRun  + ".txt", "wb")
            else:
                exitfile = open(pathTestsOutputFolder + "/race" + raceToRun + ".txt", "wb")
            time.sleep(1)

            linestoskip = 0
            additionalArgs = racefile.readline()
            while additionalArgs.startswith("$"):
                splitAdditionalArgs = additionalArgs.split(";")
                # print(splitadditionalargs)
                splitargs_len = len(splitAdditionalArgs)
                additionalCommandsIndex = 1
                while additionalCommandsIndex < splitargs_len and splitAdditionalArgs[additionalCommandsIndex] != "\n":
                    if splitAdditionalArgs[0] == "$init":
                        initCommands = splitAdditionalArgs[additionalCommandsIndex]
                        # print(decodedLine)
                        command_send_midprogramm(initCommands)
                        additionalCommandsIndex += 1
                        time.sleep(1)
                    elif splitAdditionalArgs[0] == "$commands":
                        command_sendtime = float(splitAdditionalArgs[additionalCommandsIndex])
                        command_name = (splitAdditionalArgs[additionalCommandsIndex+1],)
                        additionalCommandsIndex += 2
                        timer = Timer(command_sendtime, command_send_midprogramm, args=command_name)
                        timer.start()
                        linestoskip += 1
                additionalArgs = racefile.readline()
            racefile.seek(0)
            for additionalCommandsIndex in range(linestoskip):
                racefile.readline()

            if invalidInput:
                loopIndex = len(flatten_listOfRaces)
            else:
                raceEND = False
                #readline = ser.readline()
                ser.write(b"start" + b"\n")  # \x53\x54\x41\x52\x54\x0a (utf-8)
                time.sleep(0.2)
                stopline = ""
                consoleProgressPrint = "Running race " + raceToRun + "."
                print(consoleProgressPrint, end='')
                progresscounter = 1
                while raceEND != True:
                    print("\r", end='')
                    decodedLine = serialLineDecode()
                    exitfile.write(decodedLine.encode('utf-8') + b'\n')
                    if debugmode:
                        print(decodedLine)
                    else:
                        progresscounter += 1
                    if progresscounter == 5:
                        consoleProgressPrint += "."
                        print(consoleProgressPrint, end='')
                        progresscounter = 0
                    if decodedLine == "RS:  STOP  ":
                        if progresscounter == 0:
                            print("", end='')
                        else:
                            print(consoleProgressPrint, end='')
                        raceEND = True
                        stopline = decodedLine
                        racenumber += 1

                notOKcount = 0
                while "CT" not in stopline:
                    decodedLine = serialLineDecode()
                    stopline = decodedLine
                    #exitfile.write(decodedLine.encode('utf-8'))
                    if ((decodedLine.startswith("Dog ") or decodedLine.startswith(" Team") or decodedLine.startswith("  Net")) and raceEND):
                        #exitfile.write(decodedLine.encode('utf-8') + b'\n')
                        lengthofline = len(decodedLine)
                        normdecodeline = decodedLine
                        for x in range(30-lengthofline):
                            normdecodeline = normdecodeline + " "
                        expectedline = racefile.readline()[:-1]
                        if expectedline.startswith("//RACE") or expectedline.startswith("$"):
                            expectedline = racefile.readline()[:-1]
                        if decodedLine == expectedline:
                            #print(normdecodeline, colored("OK", 'green'))
                            exitfile.write(normdecodeline.encode('utf-8') + b'  OK' + b'\n')
                        else:
                            notOKcount += 1
                            #print(normdecodeline, colored("NOK", 'red'), " Exp: ", Fore.YELLOW + expectedline, Fore.RESET)
                            exitfile.write(normdecodeline.encode('utf-8') + b'  NOK' + b'   Exp: ' + expectedline.encode('utf-8') + b'\n')

                if notOKcount != 0:
                    fileTestsSummary.write(b"Race " + raceToRun.encode('utf-8') + b" FAIL\n")
                    print("FAIL")
                else:
                    fileTestsSummary.write(b"Race " + raceToRun.encode('utf-8') + b" PASSED\n")
                    print("PASSED")

                time.sleep(1.5)
                ser.write(b"reset" + b"\n")
                time.sleep(0.5)
                loopIndex += 1
                racefile.close()
        else:
            print("There are no more testcases")
            loopIndex = len(flatten_listOfRaces)
try:
    exitfile
except NameError:
    print("Invalid parameters")
else:
    exitfile.close()
    fileTestsSummary.close()