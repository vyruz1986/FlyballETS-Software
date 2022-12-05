# test ESP32
# v1.0.2
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
            for subitem in item:
                yield subitem
        else:
            yield item


def command_send_midprogramm(command):
    ser.write(command.encode('utf-8') + b"\n")


def serialLineDecode():
    readline = ser.readline()[:-2]
    readedLine = readline.decode('utf-8')
    splitdecodeline = readedLine.split("(): ")
    if len(splitdecodeline) > 1:
        readedLine = splitdecodeline[1]
    else:
        readedLine = splitdecodeline[0]
    return readedLine


serialPorts = list(list_ports.comports())
serialPortIndex = 0
for p in serialPorts:
    serialPortIndex += 1
    print(serialPortIndex, ". ", end='')
    print(p)
numberOfSerialPorts = len(serialPorts)
selectedPortNumber = int(input("Select port: "))
print(serialPorts[selectedPortNumber - 1])
ser = serial.Serial(port=serialPorts[selectedPortNumber - 1].device, baudrate=115200, timeout=2)
time.sleep(2)
if ser.isOpen():
    print("SERIAL port is open")

#ser.write(b"reboot" + b"\n")
print("Preparing environment... ")
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
firmware = firmware[18:].decode("utf-8")
print("Firmware: ", firmware)

pathTestsOutputFolder = os.path.join(str(pathOutputRootFolder), str(
    datetime.now().strftime("%Y-%m-%d %H-%M-%S v") + firmware))
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
    raceToRun = str(sys.argv[argument_number])
    if raceToRun[0].isdigit():
        listOfRaces = list(raceToRun.split(","))
        elem_iter = 0
        for elem in listOfRaces:
            if '-' in elem:
                raceRange = elem.split("-")
                firstRace = int(raceRange[0])
                lastRace = int(raceRange[1])
                elem = []
                while firstRace <= lastRace:
                    elem.append(str(firstRace))
                    firstRace += 1
                listOfRaces[elem_iter] = elem
            elem_iter += 1
        flatten_listOfRaces = list(flat2gen(listOfRaces))
    elif raceToRun == "-d":
        debugmode = True
    elif raceToRun == "all":
        initial_count = 0
        for Path in os.listdir(os.getcwd() + "/testcases"):
            if os.path.isfile(os.path.join(os.getcwd() + "/testcases", Path)):
                flatten_listOfRaces.append(str(initial_count))
                initial_count += 1
    elif raceToRun == "loop":
        argument_number += 1
        numberOfStabilityLoops = int(sys.argv[argument_number])
        if numberOfStabilityLoops < 1:
            invalidInput = True
    else:
        invalidInput = True
    argument_number += 1

print("Starting execution... ")
for stabilityLoopIndex in range(numberOfStabilityLoops):
    if numberOfStabilityLoops > 1:
        print("Stability loop " + str(stabilityLoopIndex+1) + ":")
        if fileTestsSummary._checkClosed:
            fileTestsSummary = open(pathTestsOutputFolder + "/!summary.txt", "ab")
        fileTestsSummary.write(b"Stability loop " + str(stabilityLoopIndex+1).encode('utf-8') + b":\n")
    loopIndex = 0
    while loopIndex < len(flatten_listOfRaces):
        raceToRun = str(flatten_listOfRaces[loopIndex])
        ser.write(b"uptime" + b"\n")
        time.sleep(0.2)
        ser.write(b"reset" + b"\n")
        time.sleep(0.2)
        ser.write(b"race " + raceToRun.encode('utf-8') + b"\n")
        time.sleep(0.2)

        racefilePATH = os.getcwd() + "/testcases" + "/RACE" + raceToRun + ".txt"
        if os.path.isfile(racefilePATH):
            testcaseInputFile = open(racefilePATH, "r")
            if numberOfStabilityLoops > 1:
                testcaseOutputFile = open(pathTestsOutputFolder + "/" + str(stabilityLoopIndex+1) + "_" + "race" + raceToRun + ".txt", "wb")
            else:
                testcaseOutputFile = open(pathTestsOutputFolder + "/race" + raceToRun + ".txt", "wb")
            time.sleep(1)

            linestoskip = 0
            additionalArgs = testcaseInputFile.readline()
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
                        command_sendtime = float(
                            splitAdditionalArgs[additionalCommandsIndex])
                        command_name = (
                            splitAdditionalArgs[additionalCommandsIndex+1],)
                        additionalCommandsIndex += 2
                        inRaceCommandTimer = Timer(command_sendtime, command_send_midprogramm, args=command_name)
                        inRaceCommandTimer.start()
                        linestoskip += 1
                additionalArgs = testcaseInputFile.readline()
            testcaseInputFile.seek(0)
            for additionalCommandsIndex in range(linestoskip):
                testcaseInputFile.readline()

            if invalidInput:
                loopIndex = len(flatten_listOfRaces)
            else:
                raceEND = False
                #ser.flushInput()
                ser.write(b"start" + b"\n")
                raceStartTime = int(time.time())
                time.sleep(0.1)
                stopline = ""
                consoleProgressPrint = "Running race " + raceToRun + "."
                print(consoleProgressPrint, end='')
                progresscounter = 1
                notOKcount = 0
                while (not raceEND) & (notOKcount == 0):
                    print("\r", end='')
                    decodedLine = serialLineDecode()
                    testcaseOutputFile.write(decodedLine.encode('utf-8') + b'\n')
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
                    if ((time.time() - raceStartTime) > 90):
                        if progresscounter == 0:
                            print("", end='')
                        else:
                            print(consoleProgressPrint, end='')
                        notOKcount = 10
                        testcaseOutputFile.write(decodedLine.encode('utf-8') + b'  TIMEOUT\n')
                if debugmode:
                    print("\r", end='')

                while ("CT" not in stopline) & (notOKcount < 10):
                    decodedLine = serialLineDecode()
                    if debugmode:
                        print(decodedLine)
                    stopline = decodedLine
                    if ((decodedLine.startswith("Dog ") or decodedLine.startswith(" Team") or decodedLine.startswith("   CT")) and raceEND):
                        #exitfile.write(decodedLine.encode('utf-8') + b'\n')
                        lengthofline = len(decodedLine)
                        normdecodeline = decodedLine
                        for x in range(30-lengthofline):
                            normdecodeline = normdecodeline + " "
                        expectedline = testcaseInputFile.readline()[:-1]
                        if expectedline.startswith("//RACE") or expectedline.startswith("$"):
                            expectedline = testcaseInputFile.readline()[:-1]
                        if decodedLine == expectedline:
                            #print(normdecodeline, colored("OK", 'green'))
                            testcaseOutputFile.write(normdecodeline.encode('utf-8') + b'  OK' + b'\n')
                        else:
                            notOKcount += 1
                            #print(normdecodeline, colored("NOK", 'red'), " Exp: ", Fore.YELLOW + expectedline, Fore.RESET)
                            testcaseOutputFile.write(normdecodeline.encode('utf-8') + b'  NOK' + b'   Exp: ' + expectedline.encode('utf-8') + b'\n')
                    if ((time.time() - raceStartTime) > 110):
                        notOKcount = 11
                        testcaseOutputFile.write(normdecodeline.encode('utf-8') + b'  TIMEOUT\n')

                if fileTestsSummary._checkClosed:
                    fileTestsSummary = open(pathTestsOutputFolder + "/!summary.txt", "ab")
                if notOKcount == 0:
                    fileTestsSummary.write(b"Race " + raceToRun.encode('utf-8') + b" passed\n")
                    print("passed")
                elif notOKcount < 10:
                    fileTestsSummary.write(b"Race " + raceToRun.encode('utf-8') + b" FAIL\n")
                    print("FAIL")
                else:
                    fileTestsSummary.write(b"Race " + raceToRun.encode('utf-8') + b" TIMEOUT\n")
                    print("TIMEOUT")
                    ser.write(b"stop" + b"\n")
                    time.sleep(2)

                time.sleep(1.5)
                ser.flushInput()
                loopIndex += 1
                testcaseInputFile.close()
                testcaseOutputFile.close()
                fileTestsSummary.close()
        else:
            print("There are no more testcases")
            loopIndex = len(flatten_listOfRaces)
try:
    testcaseOutputFile
except NameError:
    print("Invalid parameters")
else:
    fileTestsSummary.close()
time.sleep(0.2)
ser.write(b"reset" + b"\n")
