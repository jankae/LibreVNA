import serial
import serial.tools.list_ports
from enum import Enum

class libreCAL:
    def __init__(self, serialnum = ''):
        self.ser = None
        for p in serial.tools.list_ports.comports():
            if (p.vid == 0x0483 and p.pid == 0x4122) or (p.vid == 0x1209 and p.pid == 0x4122):
                self.ser = serial.Serial(p.device, timeout = 1)
                idn = self.SCPICommand("*IDN?").split(",")
                if idn[0] != "LibreCAL":
                    self.ser = None
                    continue
                self.serial = idn[2]
                if len(serialnum) > 0:
                    # serial number specified, compare
                    if self.serial != serialnum:
                        self.ser = None
                        continue
                break
        if self.ser == None:
            if len(serialnum) > 0:
                raise Exception("LibreCAL with serial number '"+serialnum+"' not detected")
            else:
                raise Exception("No LibreCAL device detected")
            
    def getSerial(self) -> str:
        return self.serial
            
    class Standard(Enum):
        NONE = 0,
        OPEN = 1,
        SHORT = 2,
        LOAD = 3,
        THROUGH = 4
        
    def reset(self):
        self.SCPICommand(":PORT 1 NONE")
        self.SCPICommand(":PORT 2 NONE")
        self.SCPICommand(":PORT 3 NONE")
        self.SCPICommand(":PORT 4 NONE")
        
    def setPort(self, s : Standard, port, port2 = -1):
        if s == self.Standard.THROUGH and port2 == -1:
            raise Exception("When setting a port to THROUGH, the destination port must also be specified")
        cmd = ":PORT "+str(port)+" "+s.name
        if s == self.Standard.THROUGH:
            cmd += " "+str(port2)
        if len(self.SCPICommand(cmd)) == 0:
            return True
        else:
            return False
    
    def getPort(self, port):
        resp = self.SCPICommand(":PORT? "+str(port)).split(" ")[0]
        try:
            s = self.Standard[resp]
            return s
        except:
            raise Exception("LibreCAL reported unknown standard '"+resp+"'")
            
    def getTemperature(self):
        return float(self.SCPICommand(":TEMP?"))
    
    def isStable(self) -> bool:
        if self.SCPICommand(":TEMP:STABLE?") == "TRUE":
            return True
        else:
            return False
        
    def getHeaterPower(self):
        return float(self.SCPICommand(":HEAT:POW?"))

    def getDateTimeUTC(self):
        return self.SCPICommand(":DATE_TIME?")

    def setDateTimeUTC(self, date_time_utc):
        return self.SCPICommand(":DATE_TIME "+ date_time_utc)

    def SCPICommand(self, cmd: str) -> str:
        self.ser.write((cmd+"\r\n").encode())
        resp = self.ser.readline().decode("ascii")
        if len(resp) == 0:
            raise Exception("Timeout occurred in communication with LibreCAL")
        if resp.strip() == "ERROR":
            raise Exception("LibreCAL returned 'ERROR' for command '"+cmd+"'")
        return resp.strip()
