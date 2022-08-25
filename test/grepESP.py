#grep from log

log = open("D:\\Users\\konri\\Kurs\\ESP32\\log.txt", 'r')
interestingdata = open("D:\\Users\\konri\\Kurs\\ESP32\\interestingDATA.txt", 'wb')

readline = log.readline()
interestingdata.write(b"####################### RACE 0" + b'\n')
race = 0
while readline != "":
    if readline.startswith("D0") or readline.startswith("D1") or readline.startswith("D2") or readline.startswith("D3") or readline.startswith("RT") or readline.startswith("RS") or readline.startswith("Dog "):
        encodeline = readline.encode('utf-8')
        interestingdata.write(encodeline)
        if readline.startswith("RS:  STOP") == True:
            race += 1
            byterace = b'%i' % race
            interestingdata.write(b"####################### RACE " + byterace + b'\n')
    readline = log.readline()
log.close()
interestingdata.close()