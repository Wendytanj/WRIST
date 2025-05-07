#include "Haptic_Driver.h"
#include "usbd_cdc_if.h"
#include "string.h"

/* Global flag for I2C transfer completion */
volatile uint8_t I2C_Transfer_Complete = 0;

/* Callback provided by HAL that will be called when an interrupt‐based transmit completes */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    I2C_Transfer_Complete = 1;
}

/* Internal helper function prototypes (private to this file) */
static _Bool _writeRegister(Haptic_Driver *hd, uint8_t _wReg, uint8_t _mask, uint8_t _bits, uint8_t _startPosition);
static uint8_t _readRegister(Haptic_Driver *hd, uint8_t _reg);

/* Initializes the Haptic_Driver struct with default values */
void Haptic_Driver_init(Haptic_Driver *hd, uint8_t address)
{
    if (hd == NULL)
        return;
    hd->_address = address;
    hd->_i2cPort = NULL;
    hd->lastPosWritten = 0;
    /* Zero out the memory copy buffer and settings */
    for (int i = 0; i < 100; i++) {
        hd->snpMemCopy[i] = 0;
    }
    hd->sparkSettings.motorType = LRA_TYPE;
    hd->sparkSettings.nomVolt = 0.0f;
    hd->sparkSettings.absVolt = 0.0f;
    hd->sparkSettings.currMax = 0.0f;
    hd->sparkSettings.impedance = 0.0f;
    hd->sparkSettings.lraFreq = 0.0f;
}

/* Begins communication with the device.
   Reads the chip revision register and returns true if the value is correct. */
_Bool Haptic_Driver_begin(Haptic_Driver *hd, I2C_HandleTypeDef *hi2c)
{
    if (hd == NULL || hi2c == NULL)
        return 0;
    hd->_i2cPort = hi2c;
    uint8_t tempRegVal = _readRegister(hd, CHIP_REV_REG);

    // Return true if the chip revision read is non-zero,
    // otherwise return false.
    return (tempRegVal != 0);
}


/* Sets the actuator type to LRA or ERM. */
_Bool Haptic_Driver_setActuatorType(Haptic_Driver *hd, uint8_t actuator)
{
    if (actuator != LRA_TYPE && actuator != ERM_TYPE)
        return 0;
    return _writeRegister(hd, TOP_CFG1, 0xDF, actuator, 5);
}

/* Sets the operation mode (0 to 3, corresponding to DRO_MODE, PWM_MODE, etc.). */
_Bool Haptic_Driver_setOperationMode(Haptic_Driver *hd, uint8_t mode)
{
    if (mode > 3)
        return 0;
    return _writeRegister(hd, TOP_CTL1, 0xF8, mode, 0);
}

/* Reads and returns the current operation mode. */
uint8_t Haptic_Driver_getOperationMode(Haptic_Driver *hd)
{
    return _readRegister(hd, TOP_CTL1) & 0x07;
}

/* Sets up default motor parameters.
   Returns true only if all configuration calls succeed. */
_Bool Haptic_Driver_defaultMotor(Haptic_Driver *hd)
{
    hd->sparkSettings.motorType = LRA_TYPE;
    hd->sparkSettings.nomVolt = 2.106f;
    hd->sparkSettings.absVolt = 2.26f;
    hd->sparkSettings.currMax = 165.4f;
    hd->sparkSettings.impedance = 13.8f;
    hd->sparkSettings.lraFreq = 170.0f;

    if ( Haptic_Driver_setActuatorType(hd, hd->sparkSettings.motorType) &&
         Haptic_Driver_setActuatorABSVolt(hd, hd->sparkSettings.absVolt) &&
         Haptic_Driver_setActuatorNOMVolt(hd, hd->sparkSettings.nomVolt) &&
         Haptic_Driver_setActuatorIMAX(hd, hd->sparkSettings.currMax) &&
         Haptic_Driver_setActuatorImpedance(hd, hd->sparkSettings.impedance) &&
         Haptic_Driver_setActuatorLRAfreq(hd, hd->sparkSettings.lraFreq) )
    {
        return 1;
    }
    else
        return 0;
}

/* Reads various motor settings and returns them in a hapticSettings struct */
hapticSettings Haptic_Driver_getSettings(Haptic_Driver *hd)
{
    hapticSettings temp;
    temp.nomVolt = _readRegister(hd, ACTUATOR1) * (23.4e-3f);
    temp.absVolt = _readRegister(hd, ACTUATOR2) * (23.4e-3f);
    temp.currMax = (_readRegister(hd, ACTUATOR3) * 7.2f) + 28.6f;
    uint16_t v2i_factor = (((uint16_t)_readRegister(hd, CALIB_V2I_H)) << 8) | _readRegister(hd, CALIB_IMP_L);
    temp.impedance = (v2i_factor * 1.6104f) / (_readRegister(hd, ACTUATOR3) + 4);
    /* lraFreq is not read from the IC; we return the previously stored value */
    temp.lraFreq = hd->sparkSettings.lraFreq;
    return temp;
}

/* Sets the absolute voltage in the range 0 – 6.0 V */
_Bool Haptic_Driver_setActuatorABSVolt(Haptic_Driver *hd, float absVolt)
{
    if (absVolt < 0.0f || absVolt > 6.0f)
        return 0;
    absVolt = absVolt / (23.4e-3f);
    return _writeRegister(hd, ACTUATOR2, 0x00, (uint8_t)absVolt, 0);
}

/* Returns the absolute voltage (in volts) by reading the register value */
float Haptic_Driver_getActuatorABSVolt(Haptic_Driver *hd)
{
    uint8_t regVal = _readRegister(hd, ACTUATOR2);
    return regVal * (23.4e-3f);
}

/* Sets the nominal voltage (in volts) for the motor */
_Bool Haptic_Driver_setActuatorNOMVolt(Haptic_Driver *hd, float rmsVolt)
{
    if (rmsVolt < 0.0f || rmsVolt > 3.3f)
        return 0;
    rmsVolt = rmsVolt / (23.4e-3f);
    return _writeRegister(hd, ACTUATOR1, 0x00, (uint8_t)rmsVolt, 0);
}

/* Returns the nominal voltage (in volts) */
float Haptic_Driver_getActuatorNOMVolt(Haptic_Driver *hd)
{
    uint8_t regVal = _readRegister(hd, ACTUATOR1);
    return regVal * (23.4e-3f);
}

/* Configures the maximum current (in milliamps) */
_Bool Haptic_Driver_setActuatorIMAX(Haptic_Driver *hd, float maxCurr)
{
    if (maxCurr < 0.0f || maxCurr > 300.0f)
        return 0;
    maxCurr = (maxCurr - 28.6f) / 7.2f;
    return _writeRegister(hd, ACTUATOR3, 0xE0, (uint8_t)maxCurr, 0);
}

/* Returns the maximum current (in milliamps) */
uint16_t Haptic_Driver_getActuatorIMAX(Haptic_Driver *hd)
{
    uint8_t regVal = _readRegister(hd, ACTUATOR3) & 0x1F;
    float value = regVal * 7.2f + 28.6f;
    return (uint16_t)value;
}

/* Configures the actuator impedance (in ohms) */
_Bool Haptic_Driver_setActuatorImpedance(Haptic_Driver *hd, float motorImpedance)
{
    if (motorImpedance < 0.0f || motorImpedance > 50.0f)
        return 0;
    uint8_t msbImpedance, lsbImpedance;
    uint16_t v2iFactor;
    /* Read the current maximum current (typically masked to the lower 5 bits) */
    uint8_t maxCurr = _readRegister(hd, ACTUATOR3) & 0x1F;
    v2iFactor = (uint16_t)((motorImpedance * (maxCurr + 4)) / 1.6104f);
    msbImpedance = v2iFactor >> 8;
    lsbImpedance = v2iFactor & 0xFF;
    if (_writeRegister(hd, CALIB_V2I_L, 0x00, lsbImpedance, 0) &&
        _writeRegister(hd, CALIB_V2I_H, 0x00, msbImpedance, 0))
    {
        return 1;
    }
    else
        return 0;
}

/* Returns the actuator impedance (in ohms) */
uint16_t Haptic_Driver_getActuatorImpedance(Haptic_Driver *hd)
{
    uint16_t regValMSB = _readRegister(hd, CALIB_V2I_H);
    uint8_t regValLSB = _readRegister(hd, CALIB_V2I_L);
    uint8_t currVal = _readRegister(hd, ACTUATOR3) & 0x1F;
    uint16_t v2iFactor = (((uint16_t)regValMSB) << 8) | regValLSB;
    float impedance = (v2iFactor * 1.6104f) / (currVal + 4);
    return (uint16_t)impedance;
}

/* Reads an impedance adjustment value (calculated by the device) */
uint16_t Haptic_Driver_readImpAdjus(Haptic_Driver *hd)
{
    uint8_t tempMSB = _readRegister(hd, CALIB_IMP_H);
    uint8_t tempLSB = _readRegister(hd, CALIB_IMP_L);
    float totalImp = (4 * 62.5e-3f * tempMSB) + (62.5e-3f * tempLSB);
    return (uint16_t)totalImp;
}

/* Configures the LRA (Linear Resonant Actuator) frequency.
   Note: The computation for lraPeriod uses the factor 1333.32e-9 as in the original code. */
_Bool Haptic_Driver_setActuatorLRAfreq(Haptic_Driver *hd, float frequency)
{
    if (frequency < 0.0f || frequency > 500.0f)
        return 0;
    uint8_t msbFrequency, lsbFrequency;
    uint16_t lraPeriod;
    lraPeriod = (uint16_t)(1.0f / (frequency * (1333.32e-9f)));
    msbFrequency = lraPeriod >> 7;    /* equivalent to division by 128 */
    lsbFrequency = lraPeriod & 0x7F;    /* remainder after division */
    if (_writeRegister(hd, FRQ_LRA_PER_H, 0x00, msbFrequency, 0) &&
        _writeRegister(hd, FRQ_LRA_PER_L, 0x80, lsbFrequency, 0))
    {
        return 1;
    }
    else
        return 0;
}

/* Enable or disable active acceleration */
_Bool Haptic_Driver_enableAcceleration(Haptic_Driver *hd, _Bool enable)
{
    return _writeRegister(hd, TOP_CFG1, 0xFB, enable ? 1 : 0, 2);
}

/* Enable or disable rapid stop */
_Bool Haptic_Driver_enableRapidStop(Haptic_Driver *hd, _Bool enable)
{
    return _writeRegister(hd, TOP_CFG1, 0xFD, enable ? 1 : 0, 1);
}

/* Enable or disable frequency tracking */
_Bool Haptic_Driver_enableFreqTrack(Haptic_Driver *hd, _Bool enable)
{
    return _writeRegister(hd, TOP_CFG1, 0xF7, enable ? 1 : 0, 3);
}

/* Enable or disable the BEMF fault limit */
_Bool Haptic_Driver_setBemfFaultLimit(Haptic_Driver *hd, _Bool enable)
{
    return _writeRegister(hd, TOP_CFG1, 0xEF, enable ? 1 : 0, 4);
}

/* Enable or disable impedance calibration */
_Bool Haptic_Driver_calibrateImpedanceDistance(Haptic_Driver *hd, _Bool enable)
{
    return _writeRegister(hd, TOP_CFG4, 0xBF, enable ? 1 : 0, 6);
}

/* Returns the current acceleration state (bit 2 of TOP_CFG1) */
uint8_t Haptic_Driver_getAccelState(Haptic_Driver *hd)
{
    uint8_t state = _readRegister(hd, TOP_CFG1);
    return (state & 0x04) >> 2;
}

/* Configures the vibration strength.
   If acceleration mode is enabled the value is clamped to a lower maximum. */
_Bool Haptic_Driver_setVibrate(Haptic_Driver *hd, uint8_t val)
{
    uint8_t accelState = (_readRegister(hd, TOP_CFG1) & 0x04) >> 2;
    if (accelState == ENABLE)
    {
        if (val > 0x7F)
            val = 0x7F;
    }
    return _writeRegister(hd, TOP_CTL2, 0x00, val, 0);
}

/* Returns the vibration value from the device */
uint8_t Haptic_Driver_getVibrate(Haptic_Driver *hd)
{
    return _readRegister(hd, TOP_CTL2);
}

/* Sets the IRQ event mask */
_Bool Haptic_Driver_setMask(Haptic_Driver *hd, uint8_t mask)
{
    return _writeRegister(hd, IRQ_MASK1, 0x00, mask, 0);
}

/* Returns the IRQ event mask value */
uint8_t Haptic_Driver_getMask(Haptic_Driver *hd)
{
    return _readRegister(hd, IRQ_MASK1);
}

/* Configures the BEMF voltage limit (0–3 corresponds to predetermined millivolt levels) */
_Bool Haptic_Driver_setBemf(Haptic_Driver *hd, uint8_t val)
{
    if (val > 3)
        return 0;
    return _writeRegister(hd, TOP_INT_CFG1, 0xFC, val, 0);
}

/* Returns the BEMF threshold value (in mV) */
float Haptic_Driver_getBemf(Haptic_Driver *hd)
{
    uint8_t bemf = _readRegister(hd, TOP_INT_CFG1);
    switch (bemf)
    {
        case 0x00: return 0.0f;
        case 0x01: return 4.9f;
        case 0x02: return 27.9f;
        case 0x03: return 49.9f;
        default:   return 0.0f;
    }
}

/* =======================================================================
   Private Helper Functions
   ======================================================================= */

/* Writes to a register:
   - Reads the current value,
   - Modifies only the requested bit-field,
   - Transmits the new value using an interrupt-based I2C transmit.
   Returns true if the operation is successful. */
static _Bool _writeRegister(Haptic_Driver *hd, uint8_t reg, uint8_t mask, uint8_t bits, uint8_t pos)
{
    // Read current register value
    uint8_t currentValue = _readRegister(hd, reg);

    // Apply mask and set bits
    currentValue &= mask;
    currentValue |= (bits << pos);

    // Use the blocking HAL_I2C_Mem_Write function to write the data
    if (HAL_I2C_Mem_Write(hd->_i2cPort, hd->_address << 1, reg, I2C_MEMADD_SIZE_8BIT, &currentValue, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        CDC_Transmit_FS((uint8_t*)"Mem write error\r\n", strlen("Mem write error\r\n"));
        return 0;
    }
    return 1;
}


/* Reads a register value by first transmitting the register address, then reading one byte. */
static uint8_t _readRegister(Haptic_Driver *hd, uint8_t reg)
{
    uint8_t data = 0;
    if (HAL_I2C_Mem_Read(hd->_i2cPort, hd->_address << 1, reg,
                         I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        // Debug message (if needed) to indicate failure
        // CDC_Transmit_FS((uint8_t*)"Mem read error\r\n", strlen("Mem read error\r\n"));
        return 0;
    }
    return data;
}

