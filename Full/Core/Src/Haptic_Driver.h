#ifndef _SPARKFUN_HAPTIC_DRIVER_DA7280_
#define _SPARKFUN_HAPTIC_DRIVER_DA7280_

#include "stm32g0xx_hal.h"
#include "stm32g0xx_hal_i2c.h"
#include <math.h>
#include <stdint.h>

/* Definitions and Constants */
#define DEF_ADDR         0x48
#define CHIP_REV         0xCA
#define ENABLE           0x01
#define UNLOCKED         0x01
#define DISABLE          0x00
#define LOCKED           0x00
#define LRA_TYPE         0x00
#define ERM_TYPE         0x01
#define RAMP             0x01
#define STEP             0x00

/* Structure for motor settings */
typedef struct
{
    uint8_t motorType;
    float nomVolt;
    float absVolt;
    float currMax;
    float impedance;
    float lraFreq;
} hapticSettings;

/* Enumeration types */
typedef enum
{
    HAPTIC_SUCCESS,
    E_SEQ_CONTINUE = 0x01,
    E_UVLO = 0x02,
    HAPTIC_HW_ERROR,
    E_SEQ_DONE = 0x04,
    HAPTIC_INCORR_PARAM,
    HAPTIC_UNKNOWN_ERROR,
    E_OVERTEMP_CRIT = 0x08,
    E_SEQ_FAULT = 0x10,
    E_WARNING = 0x20,
    E_ACTUATOR_FAULT = 0x40,
    E_OC_FAULT = 0x80
} event_t;

typedef enum
{
    NO_DIAG = 0x00,
    E_PWM_FAULT = 0x20,
    E_MEM_FAULT = 0x40,
    E_SEQ_ID_FAULT = 0x80
} diag_status_t;

typedef enum
{
    STATUS_NOM = 0x00,
    STA_SEQ_CONTINUE = 0x01,
    STA_UVLO_VBAT_OK = 0x02,
    STA_PAT_DONE = 0x04,
    STA_OVERTEMP_CRIT = 0x08,
    STA_PAT_FAULT = 0x10,
    STA_WARNING = 0x20,
    STA_ACTUATOR = 0x40,
    STA_OC = 0x80
} status_t;

typedef enum
{
    INACTIVE = 0x00,
    DRO_MODE,
    PWM_MODE,
    RTWM_MODE,
    ETWM_MODE
} OPERATION_MODES;

typedef enum
{
    BEGIN_SNP_MEM   = 0x00,
    NUM_SNIPPETS    = 0x00,
    NUM_SEQUENCES   = 0x01,
    ENDPOINTERS     = 0x02,
    SNP_ENDPOINTERS = 0x02,
    TOTAL_MEM_REGISTERS = 0x64,
} SNPMEM_ARRAY_POS;

typedef enum
{
    NUM_SNIPPETS_REG    = 0x84,
    NUM_SEQUENCES_REG   = 0x85,
    SNP_ENDPOINTERS_REGS = 0x88,
    END_OF_MEM          = 0xE7
} SNPMEM_REGS;

typedef enum
{
    CHIP_REV_REG = 0x00,
    IRQ_EVENT1   = 0x03,
    IRQ_EVENT_WARN_DIAG,
    IRQ_EVENT_SEQ_DIAG,
    IRQ_STATUS1,
    IRQ_MASK1,
    CIF_I2C1,
    FRQ_LRA_PER_H = 0x0A,
    FRQ_LRA_PER_L,
    ACTUATOR1,
    ACTUATOR2,
    ACTUATOR3,
    CALIB_V2I_H,
    CALIB_V2I_L = 0x10,
    CALIB_IMP_H,
    CALIB_IMP_L,
    TOP_CFG1,
    TOP_CFG2,
    TOP_CFG3,
    TOP_CFG4,
    TOP_INT_CFG1,
    TOP_INT_CFG6_H = 0x1C,
    TOP_INT_CFG6_L,
    TOP_INT_CFG7_H,
    TOP_INT_CFG7_L,
    TOP_INT_CFG8 = 0x20,
    TOP_CTL1 = 0x22,
    TOP_CTL2,
    SEQ_CTL1,
    SWG_C1,
    SWG_C2,
    SWG_C3,
    SEQ_CTL2,
    GPI_0_CTL,
    GPI_1_CTL,
    GPI_2_CTL,
    MEM_CTL1,
    MEM_CTL2,
    ADC_DATA_H1,
    ADC_DATA_L1,
    POLARITY = 0x43,
    LRA_AVR_H,
    LRA_AVR_L,
    FRQ_LRA_PER_ACT_H,
    FRQ_LRA_PER_ACT_L,
    FRQ_PHASE_H,
    FRQ_PHASE_L,
    FRQ_CTL = 0x4C,
    TRIM3 = 0x5F,
    TRIM4,
    TRIM6 = 0x62,
    TOP_CFG5 = 0x6E,
    IRQ_EVENT_ACTUATOR_FAULT = 0x81,
    IRQ_STATUS2,
    IRQ_MASK2,
    SNP_MEM_X
} REGISTERS;

/* Main structure that “holds” the driver state */
typedef struct
{
    uint8_t snpMemCopy[100];
    uint8_t lastPosWritten;
    hapticSettings sparkSettings;
    uint8_t _address;
    I2C_HandleTypeDef* _i2cPort;
} Haptic_Driver;

#ifdef __cplusplus
extern "C" {
#endif

/* API function prototypes */
void Haptic_Driver_init(Haptic_Driver *hd, uint8_t address);
_Bool Haptic_Driver_begin(Haptic_Driver *hd, I2C_HandleTypeDef *hi2c);
_Bool Haptic_Driver_setActuatorType(Haptic_Driver *hd, uint8_t actuator);
_Bool Haptic_Driver_setOperationMode(Haptic_Driver *hd, uint8_t mode);
uint8_t Haptic_Driver_getOperationMode(Haptic_Driver *hd);
_Bool Haptic_Driver_defaultMotor(Haptic_Driver *hd);
hapticSettings Haptic_Driver_getSettings(Haptic_Driver *hd);
_Bool Haptic_Driver_setActuatorABSVolt(Haptic_Driver *hd, float absVolt);
float Haptic_Driver_getActuatorABSVolt(Haptic_Driver *hd);
_Bool Haptic_Driver_setActuatorNOMVolt(Haptic_Driver *hd, float rmsVolt);
float Haptic_Driver_getActuatorNOMVolt(Haptic_Driver *hd);
_Bool Haptic_Driver_setActuatorIMAX(Haptic_Driver *hd, float maxCurr);
uint16_t Haptic_Driver_getActuatorIMAX(Haptic_Driver *hd);
_Bool Haptic_Driver_setActuatorImpedance(Haptic_Driver *hd, float motorImpedance);
uint16_t Haptic_Driver_getActuatorImpedance(Haptic_Driver *hd);
uint16_t Haptic_Driver_readImpAdjus(Haptic_Driver *hd);
_Bool Haptic_Driver_setActuatorLRAfreq(Haptic_Driver *hd, float frequency);
_Bool Haptic_Driver_enableAcceleration(Haptic_Driver *hd, _Bool enable);
_Bool Haptic_Driver_enableRapidStop(Haptic_Driver *hd, _Bool enable);
_Bool Haptic_Driver_enableFreqTrack(Haptic_Driver *hd, _Bool enable);
_Bool Haptic_Driver_setBemfFaultLimit(Haptic_Driver *hd, _Bool enable);
_Bool Haptic_Driver_calibrateImpedanceDistance(Haptic_Driver *hd, _Bool enable);
_Bool Haptic_Driver_setVibrate(Haptic_Driver *hd, uint8_t val);
uint8_t Haptic_Driver_getVibrate(Haptic_Driver *hd);
_Bool Haptic_Driver_setMask(Haptic_Driver *hd, uint8_t mask);
uint8_t Haptic_Driver_getMask(Haptic_Driver *hd);
_Bool Haptic_Driver_setBemf(Haptic_Driver *hd, uint8_t val);
float Haptic_Driver_getBemf(Haptic_Driver *hd);
uint8_t Haptic_Driver_getAccelState(Haptic_Driver *hd);

#ifdef __cplusplus
}
#endif

#endif // _SPARKFUN_HAPTIC_DRIVER_DA7280_
