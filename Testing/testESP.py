#test ESP32
import os
import serial
import time
import string
import sys
from termcolor import colored
from colorama import init, Fore

exitfile = open(os.getcwd() + "\\dataESP.txt", "wb")
outputfile = open(os.getcwd() + "\\stabilityLOG.txt", "wb")

ser = serial.Serial('COM7', 115200)
time.sleep(2)
if ser.isOpen():
    print("COM port is available")
else:
    print("COM port is not available")

ser.write(b"reboot" + b"\n")
#ser.write(b"stop" + b"\n")
#ser.write(b"reset" + b"\n")
time.sleep(3)
readline_reboot = ser.readline()[:-2]
while b"ESP log level 4" not in readline_reboot:
    readline_reboot = ser.readline()[:-2]
ammountofraces = 1
racenumber = 0
endless = False


selectedrace = input("Select race: ") # 0 or 1 or 2
#selectedrace = str(sys.argv[1])
#selectedrace = '0'
#print(type(selectedrace))
while selectedrace != "end":
    if selectedrace.isdigit() == True:
        bselectedrace = selectedrace.encode('utf-8')
        ser.write(b"race " + bselectedrace + b"\n")
        racefile = open(os.getcwd() + "\\RACE" + selectedrace + ".txt", "r")
    elif selectedrace == "-all":
        ser.write(b"race 0" + b"\n")
        for a in range(2):
            numofraces = ser.readline()[:-2]
            #print(numofraces)
        splitnumofraces = numofraces.split(b"races: ")
        ammountofraces = int(splitnumofraces[1])
        exitfile.write(b"//race 0" + b'\n')
        racefile = open(os.getcwd() + "\\RACE0.txt", "r")
    elif selectedrace == "-stab":
        endless = True
        ser.write(b"race 0" + b"\n")
        for a in range(2):
            numofraces = ser.readline()[:-2]
        splitnumofraces = numofraces.split(b"races: ")
        #print(splitnumofraces)
        ammountofraces = int(splitnumofraces[1])
        exitfile.write(b"//race 0" + b'\n')
        racefile = open(os.getcwd() + "\\RACE0.txt", "r")
    else:
        print("Error: Invalid input")
        racenumber = ammountofraces
    #print(ammountofraces)
    readline = ser.readline()

    bytetime = b'0'
    raceEND = False
    ser.write(b"start" + b"\n") #\x53\x54\x41\x52\x54\x0a (utf-8)
    '''
    while racenumber < ammountofraces:
        #ser.write(b"reset" + b"\n") #\x52\x45\x53\x45\x54\x0a (utf-8)
        if raceEND == True:
            raceEND = False
            racefile.close()
            if racenumber <= ammountofraces:
                bracenumber = b'%i' % racenumber
                #print(bracenumber)
                strracenumber = str(racenumber)
                racefile = open(os.getcwd() + "\\RACE" + selectedrace + ".txt", "r")
                ser.write(b"race " + bracenumber + b'\n')
                for x in range(3):
                    ser.readline()
                exitfile.write(b"//Race " + bracenumber + b'\n')
    '''
    stopline = ""
    while raceEND != True:
        readline = ser.readline()[:-2]
        #print(readline)
        decodeline = readline.decode('utf-8')
        splitdecodeline = decodeline.split("(): ")
        print(splitdecodeline[1])
        if endless == True:
            outputfile.write(readline + b'\n')
            if splitdecodeline[1] == "RS:  STOP  ":
                outputfile.write(bytetime + b'\n')
        if splitdecodeline[1] == "RS:  STOP  ":
            raceEND = True
            stopline = readline
            racenumber += 1
            if racenumber == ammountofraces and endless == True:
                racefile.seek(0)
                racenumber = 0
        #del splitdecodeline
    exitfile.write(b'### RACE ' + selectedrace.encode('utf-8') + b' ###\n')
    while b"Net" not in stopline:
        readline = ser.readline()[:-2]
        stopline = readline
        decodeline = readline.decode('utf-8')
        splitdecodeline = decodeline.split("(): ")
        if((splitdecodeline[1].startswith("Dog ") or splitdecodeline[1].startswith(" Team") or splitdecodeline[1].startswith("  Net")) and raceEND == True):
            #exitfile.write(splitdecodeline[1].encode('utf-8') + b'\n')
            lengthofline = len(splitdecodeline[1])
            normdecodeline = splitdecodeline[1]
            for x in range(30-lengthofline):
                normdecodeline = normdecodeline + " "
            expectedline = racefile.readline()[:-1]
            if expectedline.startswith("//RACE") == True:
                expectedline = racefile.readline()[:-1]
            if splitdecodeline[1] == expectedline:
                print(normdecodeline, colored("OK", 'green'))                    
                exitfile.write(normdecodeline.encode('utf-8')+ b'  OK' + b'\n')
            else:
                print(normdecodeline, colored("NOK", 'red'), " Exp: ", Fore.YELLOW + expectedline, Fore.RESET)
                exitfile.write(normdecodeline.encode('utf-8')+ b'  NOK' + b'   Exp: ' + expectedline.encode('utf-8') + b'\n')             
                
        if endless == True:
            ser.write(b"time" + b'\n')
            timecheck = ser.readline()[:-2]
            timecheck = ser.readline()[:-2]
            print(timecheck)
            #if timecheck.startswith(b"[I]"):
            splittimecheck = timecheck.split(b" ")
            splittimecheck = splittimecheck[5].split(b".")
            time = int(splittimecheck[0])
            bytetime = bytes(splittimecheck[0])
            exitfile.write(bytetime + b'\n')
            print(time)
            if time > 21600000:
                racenumber = ammountofraces
            #print(racenumber)
    if endless == True:
        print(time)
    ser.write(b"reset" + b"\n")
    time.sleep(3)
    selectedrace = input("Select race: ") # 0 or 1 or 2 or end
racefile.close()
exitfile.close()
outputfile.close()