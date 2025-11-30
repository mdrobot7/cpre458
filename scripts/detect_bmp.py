import os
import platform
from serial.tools.list_ports_common import ListPortInfo
from serial.tools.list_ports import comports

def detect_bmp():
    for port in comports():
        if platform.system() == 'Darwin':
            if port.description.find('Black Magic Probe') != -1 and port.device.endswith('1'):
                return port
        elif platform.system() == 'Windows':
            if port.vid == 7504 and port.pid == 24600 and port.location is None:
                return port
        elif platform.system() == 'Linux':
            if port.vid == 7504 and port.pid == 24600 and 'GDB' in port.interface:
                return port
    return None

if __name__ == "__main__":
    print(str(detect_bmp()).split()[0])