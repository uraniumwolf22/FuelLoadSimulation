import ctypes
from multiprocessing import shared_memory
import subprocess
import time
from pathlib import Path
import posix_ipc

SIMULATOR_DIR = Path(__file__).resolve().parent
ECU_EMULATOR_PATH = SIMULATOR_DIR / "ECU"
SEM_NAME = "/engineSemaphore_local"
MEMORY_NAME = "engineStateMemory_local"

# Define C types

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


def main():

    ECUPROC = subprocess.Popen([str(ECU_EMULATOR_PATH)], cwd=str(SIMULATOR_DIR))  # Define the process to open (ECU Simulator)
    time.sleep(0.1)                                 # Delay for process to init

    sem = posix_ipc.Semaphore(SEM_NAME)   # define semophore.  Created by ECUSimulator
    enginedata = shared_memory.SharedMemory(name=MEMORY_NAME, create=False) # Define shared memory object

    engineStatus = Engine.from_buffer(enginedata.buf)                               # Update engine status struct

    try:
        sem.acquire()                   # Acquire sem lock
        engineStatus.COOLANT = 50       # Update coolant temp
        sem.release()                   # Release sem lock

        print(engineStatus.COOLANT)     # Print engine coolant temp

    finally:
        enginedata.close()
        sem.close()
        ECUPROC.terminate()


if __name__ == "__main__":
    main()