"""
Date: 11/21/2024
Author: Wendy
Purpose: Communicate with DA7280 haptic driver via I²C using Analog Discovery 3 and WaveForms SDK.
"""

from ctypes import *
import sys
import time

# Define DA7280 I2C address (7-bit)
DA7280_I2C_ADDRESS = 0x4A

# Define DA7280 Registers (Example - Replace with actual from datasheet)
REGISTER_OPERATION_MODE = 0x00
REGISTER_VIBRATION_AMPLITUDE = 0x01
# Add other register addresses as needed

# Define I2C Parameters
I2C_SPEED = 100000  # 100 kHz (Standard Mode)

# Define DIO Pins
DIO_SCL = 7  # DIO 7
DIO_SDA = 6  # DIO 6

def load_waveforms_sdk():
    """Load the WaveForms SDK based on the operating system."""
    if sys.platform.startswith("win"):
        dwf = cdll.LoadLibrary("dwf.dll")
    elif sys.platform.startswith("darwin"):
        dwf = cdll.LoadLibrary("/Library/Frameworks/dwf.framework/dwf")
    else:
        dwf = cdll.LoadLibrary("libdwf.so")
    return dwf

def open_device(dwf):
    """Open the first available Analog Discovery device."""
    hdwf = c_int()
    dwf.FDwfDeviceOpen(c_int(-1), byref(hdwf))
    if hdwf.value == 0:
        szerr = create_string_buffer(512)
        dwf.FDwfGetLastErrorMsg(szerr)
        print("Failed to open device:")
        print(szerr.value.decode())
        sys.exit(1)
    print("Analog Discovery device opened successfully.")
    return hdwf

def configure_power_supply(dwf, hdwf):
    """Enable and set the power supply if required."""
    print("Configuring Power Supply...")
    dwf.FDwfDigitalIOOutputEnableSet(hdwf, c_int(5), c_int(1))
    dwf.FDwfDigitalIOOutputSet(hdwf, c_int(5), c_int(1))
    print("Power Supply configured to 3.3V.")
    time.sleep(5)  # Wait for power to stabilize

def configure_i2c(dwf, hdwf):
    """Configure the I²C bus."""
    print("Configuring I²C Bus...")
    iNak = c_int()

    # Set I2C speed
    dwf.FDwfDigitalI2cRateSet(hdwf, c_double(I2C_SPEED))
    print(f"I²C Speed set to {I2C_SPEED} Hz.")

    # Set SDA and SCL pins
    dwf.FDwfDigitalI2cSclSet(hdwf, c_int(DIO_SCL))
    dwf.FDwfDigitalI2cSdaSet(hdwf, c_int(DIO_SDA))
    print(f"I²C SDA set to DIO {DIO_SDA}, SCL set to DIO {DIO_SCL}.")

    # Initialize I2C (Clear bus)
    dwf.FDwfDigitalI2cClear(hdwf, byref(iNak))
    if iNak.value == 0:
        print("I²C bus error. Check the pull-up resistors and connections.")
        sys.exit(1)
    print("I²C Bus initialized successfully.")
    time.sleep(0.1)  # Wait for bus to stabilize

def write_i2c_register(dwf, hdwf, register_address, data_value):
    """
    Write a byte to a specific register of DA7280 via I²C.
    
    Args:
        dwf: WaveForms SDK library.
        hdwf: Handle to the device.
        register_address (int): Register address to write to.
        data_value (int): Data byte to write.
    
    Returns:
        bool: True if write was acknowledged, False otherwise.
    """
    # Start Condition
    dwf.FDwfDigitalI2cStartSet(hdwf, c_int(1))
    time.sleep(0.001)  # Small delay

    # Send Address with Write Bit (0)
    addr_byte = (DA7280_I2C_ADDRESS << 1) | 0
    dwf.FDwfDigitalI2cDataSet(hdwf, c_int(addr_byte))
    dwf.FDwfDigitalI2cStartSet(hdwf, c_int(1))  # Trigger sending
    time.sleep(0.001)  # Wait for transmission

    # Check for ACK
    ack = dwf.FDwfDigitalI2cAckGet(hdwf)
    if ack == 0:
        print("No ACK received after address byte.")
        dwf.FDwfDigitalI2cStopSet(hdwf, c_int(1))
        return False

    # Send Register Address
    dwf.FDwfDigitalI2cDataSet(hdwf, c_int(register_address))
    dwf.FDwfDigitalI2cStartSet(hdwf, c_int(1))  # Trigger sending
    time.sleep(0.001)  # Wait for transmission

    # Check for ACK
    ack = dwf.FDwfDigitalI2cAckGet(hdwf)
    if ack == 0:
        print("No ACK received after register address.")
        dwf.FDwfDigitalI2cStopSet(hdwf, c_int(1))
        return False

    # Send Data Byte
    dwf.FDwfDigitalI2cDataSet(hdwf, c_int(data_value))
    dwf.FDwfDigitalI2cStartSet(hdwf, c_int(1))  # Trigger sending
    time.sleep(0.001)  # Wait for transmission

    # Check for ACK
    ack = dwf.FDwfDigitalI2cAckGet(hdwf)
    if ack == 0:
        print("No ACK received after data byte.")
        dwf.FDwfDigitalI2cStopSet(hdwf, c_int(1))
        return False

    # Stop Condition
    dwf.FDwfDigitalI2cStopSet(hdwf, c_int(1))
    time.sleep(0.001)  # Small delay

    print(f"Successfully wrote 0x{data_value:02X} to register 0x{register_address:02X}.")
    return True

def main():
    # Load WaveForms SDK
    dwf = load_waveforms_sdk()

    # Open Device
    hdwf = open_device(dwf)

    # Configure Power Supply (if needed)
    configure_power_supply(dwf, hdwf)

    # Configure I2C
    configure_i2c(dwf, hdwf)

    # Example: Write to DA7280 Operation Mode Register
    # Replace REGISTER_OPERATION_MODE and desired mode with actual values
    success = write_i2c_register(dwf, hdwf, REGISTER_OPERATION_MODE, 0x01)  # Example data
    if not success:
        print("Failed to write Operation Mode.")
    
    # Example: Write to DA7280 Vibration Amplitude Register
    # Replace REGISTER_VIBRATION_AMPLITUDE and desired amplitude with actual values
    success = write_i2c_register(dwf, hdwf, REGISTER_VIBRATION_AMPLITUDE, 0x7F)  # Max amplitude
    if not success:
        print("Failed to write Vibration Amplitude.")
    
    # Additional I2C commands can be added here as needed
    
    # Optional: Read back register values for verification (Implement as needed)
    
    # Keep the script running if continuous operation is needed
    try:
        while True:
            # Example: Toggle vibration on and off
            write_i2c_register(dwf, hdwf, REGISTER_VIBRATION_AMPLITUDE, 0x7F)  # Vibration ON
            time.sleep(1)  # Vibration duration
            write_i2c_register(dwf, hdwf, REGISTER_VIBRATION_AMPLITUDE, 0x00)  # Vibration OFF
            time.sleep(1)  # Pause duration
    except KeyboardInterrupt:
        print("Interrupted by user. Closing device.")
    
    # Disable Power Supply
    print("Disabling Power Supply...")
    dwf.FDwfAnalogIOEnableSet(hdwf, c_int(0))  # Disable Analog IO
    
    # Close Device
    print("Closing device...")
    dwf.FDwfDeviceCloseAll()
    print("Device closed.")

if __name__ == "__main__":
    main()
    print("Script Ended.")
