/*
  This is a library for...
  By: Elias Santistevan
  Date:
  License: This code is public domain but you buy me a beer if you use this and
  we meet someday (Beerware license).

  Feel like supporting our work? Buy a board from SparkFun!
 */

#include "Haptic_Driver.h"
#include "math.h"

Haptic_Driver::Haptic_Driver(uint8_t address)
{
    _address = address;

} // Constructor for I2C

// Address: 0x00 , bit[7:0]: default value is 0xBA.
// Checks the WHOAMI value in the CHIP_REV register.
bool Haptic_Driver::begin(TwoWire &wirePort)
{

    delay(2);
    _i2cPort = &wirePort;
    uint8_t chipRev;

    uint8_t tempRegVal = _readRegister(CHIP_REV_REG);
    chipRev |= tempRegVal << 8;
    chipRev |= tempRegVal;

    if (chipRev != CHIP_REV)
        return false;
    else
        return true;
}

// Address: 0x13 , bit[5]: default value is 0x00.
// This sets the "actuator" (motor) type, which is either an LRA_MOTOR
// or is an ERM_MOTOR.
bool Haptic_Driver::setActuatorType(uint8_t actuator)
{

    if (actuator != LRA_TYPE && actuator != ERM_TYPE)
        return false;

    if (_writeRegister(TOP_CFG1, 0xDF, actuator, 5))
        return true;
    else
        return false;
}

// Address: 0x22, bits[2:0]
// Sets how the user will be operating the motor, which is one of four modes:
// PWM_MODE, DRO_MODE (I2C contorl), RTWM_MODE (Register-Triggered-Waveform-Memory
// Mode), or ETWM_MODE (Edge-Triggered-Waveform-Memory Mode).
bool Haptic_Driver::setOperationMode(uint8_t mode)
{

    if (mode < 0 || mode > 3)
        return false;

    if (_writeRegister(TOP_CTL1, 0xF8, mode, 0))
        return true;
    else
        return false;

    delay(1);
}

// Address: 0x22, bits[2:0]
// Returns one of the four operation modes.
// PWM_MODE, DRO_MODE (I2C contorl), RTWM_MODE (Register-Triggered-Waveform-Memory
// Mode), or ETWM_MODE (Edge-Triggered-Waveform-Memory Mode).
uint8_t Haptic_Driver::getOperationMode()
{

    uint8_t mode = _readRegister(TOP_CTL1);
    return (mode & 0x07);
}

// This function calls a number of other functions to set the following
// electrical characteristics of the motor that comes with the SparkFun Haptic
// Motor Driver. This can be set individually by the user, using the individual
// function calls - see datasheet on the motor being used.
bool Haptic_Driver::defaultMotor()
{

    sparkSettings.motorType = LRA_TYPE;
    sparkSettings.nomVolt = 2.106;  // volts - set to 2.5 when acceleration is ON
    sparkSettings.absVolt = 2.26;   // volts - when acceleration is off, set this only
    sparkSettings.currMax = 165.4;  // milliamps -
    sparkSettings.impedance = 13.8; // ohms
    sparkSettings.lraFreq = 170;    // hertz

    if (setActuatorType(sparkSettings.motorType) && setActuatorABSVolt(sparkSettings.absVolt) &&
        setActuatorNOMVolt(sparkSettings.nomVolt) && setActuatorIMAX(sparkSettings.currMax) &&
        setActuatorImpedance(sparkSettings.impedance) && setActuatorLRAfreq(sparkSettings.lraFreq))
        return true;
    else
        return false;
}

// This function returns a struct of the motor's settings set by the user.
hapticSettings Haptic_Driver::getSettings()
{

    hapticSettings temp;
    uint16_t v2i_factor;
    temp.nomVolt = _readRegister(ACTUATOR1) * (23.4 * pow(10, -3));
    temp.absVolt = _readRegister(ACTUATOR2) * (23.4 * pow(10, -3));
    temp.currMax = (_readRegister(ACTUATOR3) * 7.2) + 28.6;
    v2i_factor = (_readRegister(CALIB_V2I_H) << 8) | _readRegister(CALIB_IMP_L);
    temp.impedance = (v2i_factor * 1.6104) / (_readRegister(ACTUATOR3) + 4);
    return temp;
}

// Address: 0x0D , bit[7:0]: default value is: 0x78 (2.808 Volts)
// Function takes the absolute maximum voltage of the motor intended to
// be paired with the motor driver IC. Argument is of float type, and in volts.
bool Haptic_Driver::setActuatorABSVolt(float absVolt)
{

    if (absVolt < 0 || absVolt > 6.0)
        return false;

    absVolt = absVolt / (23.4 * pow(10, -3));

    if (_writeRegister(ACTUATOR2, 0x00, static_cast<uint8_t>(absVolt), 0))
        return true;
    else
        return false;
}

// Address: 0x0D , bit[7:0]: default value is: 0x78 (2.808 Volts)
// Function returns the value in the register below.
float Haptic_Driver::getActuatorABSVolt()
{

    uint8_t regVal = _readRegister(ACTUATOR2);

    return (regVal * (23.4 * pow(10, -3)));
}

// Address: 0x0C , bit[7:0]: default value is: 0x5A (2.106 Volts)
// Function takes the nominal voltage range of the motor intended to
// be paired with the motor driver IC. Argument is of float type, and in volts.
bool Haptic_Driver::setActuatorNOMVolt(float rmsVolt)
{

    if (rmsVolt < 0 || rmsVolt > 3.3)
        return false;

    rmsVolt = rmsVolt / (23.4 * pow(10, -3));

    if (_writeRegister(ACTUATOR1, 0x00, static_cast<uint8_t>(rmsVolt), 0))
        return true;
    else
        return false;
}

// Address: 0x0C , bit[7:0]: default value is: 0x5A (2.106 Volts)
// Function returns the value in the register below.
float Haptic_Driver::getActuatorNOMVolt()
{

    uint8_t regVal = _readRegister(ACTUATOR1);

    return (regVal * (23.4 * pow(10, -3)));
}

// Address: 0x0E , bit[4:0]: default value is: 0x17 (198mA)
// Function takes the max current rating of the motor intended to
// be paired with the motor driver IC. Argument is of float type, and in
// milliamps.
bool Haptic_Driver::setActuatorIMAX(float maxCurr)
{

    if (maxCurr < 0 || maxCurr > 300.0) // Random upper limit
        return false;

    maxCurr = (maxCurr - 28.6) / 7.2;

    if (_writeRegister(ACTUATOR3, 0xE0, static_cast<uint8_t>(maxCurr), 0))
        return true;
    else
        return false;
}

// Address: 0x0E , bit[4:0]: default value is: 0x17 (198mA)
// Function returns the value in the register below.
uint16_t Haptic_Driver::getActuatorIMAX()
{

    uint8_t regVal = _readRegister(ACTUATOR3);
    regVal &= 0x1F;

    return (regVal * 7.2) + 28.6;
}

// Address: 0x0F and 0x10 , bits[7:0]: default value is: 0x01 and 0x0D
// respectively.
// Function takes the impedance of the motor intended to
// be paired with the motor driver IC.The value is dependent on the max current
// set in 0x0E (ACTUATOR3) so be sure to set that first. Argument is of float type, and in
// ohms.
bool Haptic_Driver::setActuatorImpedance(float motorImpedance)
{

    if (motorImpedance < 0 || motorImpedance > 50.0)
        return false;

    uint8_t msbImpedance;
    uint8_t lsbImpedance;
    uint16_t v2iFactor;
    uint8_t maxCurr = _readRegister(ACTUATOR3) | 0x1F;

    v2iFactor = (motorImpedance * (maxCurr + 4)) / 1.6104;
    msbImpedance = (v2iFactor - (v2iFactor & 0x00FF)) / 256;
    lsbImpedance = (v2iFactor - (256 * (v2iFactor & 0x00FF)));

    if (_writeRegister(CALIB_V2I_L, 0x00, lsbImpedance, 0) && _writeRegister(CALIB_V2I_H, 0x00, msbImpedance, 0))
        return true;
    else
        return false;
}

// Address: 0x0F and 0x10 , bits[7:0]: default value is: 0x01 and 0x0D
// Function returns the value in the register below.
uint16_t Haptic_Driver::getActuatorImpedance()
{

    uint16_t regValMSB = _readRegister(CALIB_V2I_H);
    uint8_t regValLSB = _readRegister(CALIB_V2I_L);
    uint8_t currVal = _readRegister(ACTUATOR3) & 0x1F;

    uint16_t v2iFactor = (regValMSB << 8) | regValLSB;

    return (v2iFactor * 1.6104) / (currVal + 4);
}

// Address: 0x0F and 0x10 , bits[7:0]: default value is: 0x01 and 0x0D
// respectively.
// Function takes the impedance of the motor intended to
// be paired with the motor driver IC.The value is dependent on the max current
// set in 0x0E (ACTUATOR3) so be sure to set that first. Argument is of float type, and in
// ohms.
bool Haptic_Driver::setActuatorLRAfreq(float frequency)
{

    if (frequency < 0 || frequency > 500.0)
        return false;

    uint8_t msbFrequency;
    uint8_t lsbFrequency;
    uint16_t lraPeriod;

    lraPeriod = 1 / (frequency * (1333.32 * pow(10, -9)));
    msbFrequency = (lraPeriod - (lraPeriod & 0x007F)) / 128;
    lsbFrequency = (lraPeriod - 128 * (lraPeriod & 0xFF00));

    if (_writeRegister(FRQ_LRA_PER_H, 0x00, msbFrequency, 0) && _writeRegister(FRQ_LRA_PER_L, 0x80, lsbFrequency, 0))
    {
        return true;
    }
    else
        return false;
}

// Address: 0x11 and 0x12, bits[7:0]: default value is 0x00 for both (22 Ohms)
// This function returns the adjusted impedance of the motor, calculated by the
// IC on playback. This is meant to adjust for the motor's variation in
// impedance given manufacturing tolerances. Value is in Ohms.
uint16_t Haptic_Driver::readImpAdjus()
{

    uint8_t tempMSB = _readRegister(CALIB_IMP_H);
    uint8_t tempLSB = _readRegister(CALIB_IMP_L);

    uint16_t totalImp = (4 * 62.5 * pow(10, -3) * tempMSB) + (62.5 * pow(10, -3) * tempLSB);
    return totalImp;
}

// Address: 0x13, bit[2]: default value is 0x1
// Enables or disables active acceleration.
bool Haptic_Driver::enableAcceleration(bool enable)
{

    if (_writeRegister(TOP_CFG1, 0xFB, enable, 2))
        return true;
    else
        return false;
}

// Address: 0x13, bit[1]: default value is 0x1
// Enables or disables the "rapid stop" technology.
bool Haptic_Driver::enableRapidStop(bool enable)
{

    if (_writeRegister(TOP_CFG1, 0xFD, enable, 1))
        return true;
    else
        return false;
}


// Address: 0x13, bit[0]: default value is 0x0
// Enables or disables the "frequency tracking" technology.
bool Haptic_Driver::enableFreqTrack(bool enable)
{

    if (_writeRegister(TOP_CFG1, 0xF7, enable, 3))
        return true;
    else
        return false;
}

// Address: 0x13, bit[0]: default value is 0x1
// Enables or disables internal loop computations, which should only be
// disabled when using custom waveform or wideband operation.
bool Haptic_Driver::setBemfFaultLimit(bool enable)
{

    if (_writeRegister(TOP_CFG1, 0xEF, enable, 4))
        return true;
    else
        return false;
}

// Address: 0x16 , bit[6]: default value is: 0x1 (disabled)
// Enables or disables automatic updates to impedance value.
bool Haptic_Driver::calibrateImpedanceDistance(bool enable)
{

    if (_writeRegister(TOP_CFG4, 0xBF, enable, 6))
        return true;
    else
        return false;
}

uint8_t Haptic_Driver::getAccelState() {
  // Read TOP_CFG1 and extract bit 2
  uint8_t state = _readRegister(TOP_CFG1);
  return (state & 0x04) >> 2;
}


// Address: 0x23, bit[7:0]
// Applies the argument "wave" to the register that controls the strength of
// the vibration. The function first checks if acceleration mode is enabled
// which limits the maximum value that can be written to the register.
bool Haptic_Driver::setVibrate(uint8_t val)
{
    // Remove the unnecessary check for val < 0 since uint8_t is always >= 0.

    // Read the current configuration to check acceleration state.
    uint8_t accelState = _readRegister(TOP_CFG1);
    accelState = (accelState & 0x04) >> 2;
    
    // Clamp the amplitude based on the acceleration state.
    if (accelState == ENABLE)
    {
        if (val > 0x7F)
            val = 0x7F; // Limit for accelerated mode.
    }
    else
    {
        if (val > 0xFF)
          val = 0xFF;
    }
    
    // Write the clamped value to the vibration control register (TOP_CTL2 = 0x23).
    return _writeRegister(TOP_CTL2, 0x00, val, 0);
}

// Address: 0x23, bit[7:0]
// Reads the vibration value. This will have a result when the user writes to
// this register and also when a PWM_MODE is enabled and a duty cycle is
// applied to the GPI0/PWM pin.
uint8_t Haptic_Driver::getVibrate()
{

    uint8_t vibVal = _readRegister(TOP_CTL2);
    return vibVal;
}

// Address: 0x07, bit[7:0]: default value is: 0x0
// Function sets the register to ignore the given "mask" i.e. irq event.
bool Haptic_Driver::setMask(uint8_t mask)
{

    if (_writeRegister(IRQ_MASK1, 0x00, mask, 0))
        return true;
    else
        return false;
}

// Address: 0x07, bit[7:0]: default value is: 0x0
// Function returns the event ignoring register.
uint8_t Haptic_Driver::getMask()
{

    uint8_t regVal = _readRegister(IRQ_MASK1);
    return regVal;
}

// Address: 0x17, bit[1:0]: default value is: 0x01 (4.9mV)
// Limits the voltage that triggers a BEMF fault (E_ACTUATOR_FAULT)
// A value of zero disable tracking.
bool Haptic_Driver::setBemf(uint8_t val)
{

    if (val < 0 || val > 3)
        return false;

    if (_writeRegister(TOP_INT_CFG1, 0xFC, val, 0))
        return true;
    else
        return false;
}

// Address: 0x17, bit[1:0]: default value is: 0x01 (4.9mV)
// Returns the BEMF value in millivolts (mV).
float Haptic_Driver::getBemf()
{

    int bemf = _readRegister(TOP_INT_CFG1);

    switch (bemf)
    {
    case 0x00:
        return 0.0;
    case 0x01:
        return 4.9;
    case 0x02:
        return 27.9;
    case 0x03:
        return 49.9;
    }
}


// This generic function handles I2C write commands for modifying individual
// bits in an eight bit register. Paramaters include the register's address, a mask
// for bits that are ignored, the bits to write, and the bits' starting
// position.
bool Haptic_Driver::_writeRegister(uint8_t _wReg, uint8_t _mask, uint8_t _bits, uint8_t _startPosition)
{

    uint8_t _i2cWrite;
    _i2cWrite = _readRegister(_wReg);       // Get the current value of the register
    _i2cWrite &= (_mask);                   // Mask the position we want to write to.
    _i2cWrite |= (_bits << _startPosition); // Write the given bits to the variable
    _i2cPort->beginTransmission(_address);  // Start communication.
    _i2cPort->write(_wReg);                 // at register....
    _i2cPort->write(_i2cWrite);             // Write register...

    if (!_i2cPort->endTransmission()) // End communcation.
        return true;
    else
        return false;
}

// This generic function reads an eight bit register. It takes the register's
// address as its' parameter.
uint8_t Haptic_Driver::_readRegister(uint8_t _reg)
{

    _i2cPort->beginTransmission(_address);
    _i2cPort->write(_reg);            // Moves pointer to register.
    _i2cPort->endTransmission(false); // 'False' here sends a re-"start" message so that bus is not released
    _i2cPort->requestFrom(static_cast<uint8_t>(_address),
                          static_cast<uint8_t>(1)); // Read the register, only ever once.
    uint8_t _regValue = _i2cPort->read();
    return _regValue;
}

