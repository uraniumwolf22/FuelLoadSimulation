# This Python file uses the following encoding: utf-8
import sys
import ctypes
import subprocess
import time
import posix_ipc
from multiprocessing import shared_memory
from PySide6.QtWidgets import QApplication, QWidget

from ui_form import Ui_ECUGUI

# Define the engine struct modeled after tables.h
class Engine(ctypes.Structure):
    _fields_ = [
        # Static engine variables
        ("displacementPerRev", ctypes.c_int),
        ("coldCoolant", ctypes.c_int),

        # Sensors
        ("TPS", ctypes.c_uint16),
        ("RPM", ctypes.c_uint16),
        ("MAP", ctypes.c_uint16),
        ("AAP", ctypes.c_uint16),
        ("IAT", ctypes.c_uint16),
        ("OXVoltage", ctypes.c_uint16),
        ("COOLANT", ctypes.c_uint16),
        ("fuelTrim", ctypes.c_uint16),

        # Calculated Values
        ("fuelLoad", ctypes.c_uint16),
        ("VE", ctypes.c_uint16),
        ("STFTCorrection", ctypes.c_uint16),
        ("LTFTCorrection", ctypes.c_uint16),
        ("REALAFR", ctypes.c_uint16),
        ("AFR_TARGET", ctypes.c_float),
        ("toeEnrichmentMultiplier", ctypes.c_float),

        # Engine flags
        ("EngineCranking", ctypes.c_bool),
        ("Coldstart", ctypes.c_bool),

        # Utility Variables
        ("lastTPSValue", ctypes.c_uint16),
        ("TIFE", ctypes.c_uint16),
        ("AFRIntigralAccumulator", ctypes.c_uint16)
    ]


class ECUGUI(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.ui = Ui_ECUGUI()
        self.ui.setupUi(self)

        self.ui.coolantSlider.valueChanged.connect(self.updateCoolantTemp)


    def updateCoolantTemp(self):
        current_value = self.ui.coolantSlider.value()
        sem.acquire()
        engineStatus.COOLANT = current_value
        sem.release()

        print(f"Engine Coolant is at {engineStatus.COOLANT}")


if __name__ == "__main__":
    ECUPROC = subprocess.Popen(".././ECU")             # Define the process to open (ECU Simulator)
    time.sleep(0.1)                                 # Delay for process to init

    sem = posix_ipc.Semaphore("/engineSemaphore")   # define semophore.  Created by ECUSimulator
    enginedata = shared_memory.SharedMemory(name="engineStateMemory", create=False) # Define shared memory object

    engineStatus = Engine.from_buffer(enginedata.buf)                               # Update engine status struct

    app = QApplication(sys.argv)
    widget = ECUGUI()
    widget.show()
    sys.exit(app.exec())

    ECUPROC.terminate()
    enginedata.close()
    sem.close()
