/**
 * @file PWX_ModbusMonitoring.h
 * @brief Modbus Monitoring Header
 * @date August 22, 2023
 * @version 1.0
 * @author Charles Kim Kabiling
 */

#ifndef INC_PWX_MODBUSMONITORING_H_
#define INC_PWX_MODBUSMONITORING_H_

#include <stdint.h>
#include <stdbool.h>

#include "PWX_ST50H_Modbus.h"
//#include "PWX_ModbusDevice.h"

#define DEBUG_DATA_CONVERSION // show data conversion per dataType

/**
 * @union VarData_u
 * @brief Union for representing different types of data.
 */


typedef union {
    uint32_t u32;
    uint32_t i32;
    uint16_t u16;
    int16_t  i16;
    float f;
    uint8_t buff[4];
    int8_t i8;
    uint8_t u8;
} VarData_u;

/**
 * @struct ModbusMonitorSlot_t
 * @brief Structure representing a monitoring slot.
 */
typedef struct {
    uint8_t modbusCMD[32];
    uint8_t cmdSize;
    uint8_t dataType;
    uint8_t valueStartIndex; // from index 0
    VarData_u value;
    VarData_u prevValue;
    bool isActive;
    bool alarmState;
    uint16_t ThresholdActive; // [Spike Up, Spike Down, thresholdHigh, thresholdLow, onChange, triggerFlagValue]
    VarData_u SpikeUp;
    VarData_u SpikeDown;
    VarData_u thresholdHigh;
    VarData_u thresholdLow;
    bool onChange;
    bool triggerFlagValue;

} ModbusMonitorSlot_t;

typedef enum {
    Offset_SpikeUp = 15,
    Offset_SpikeDown = 14,
    Offset_ThresholdHigh = 13,
    Offset_ThresholdLow = 12,
    Offset_OnChange = 11,
    Offset_TriggerFlag = 10
} ThresholdOffset_e;

/**
 * @brief Initializes the Modbus monitor with the provided scan interval and active slots.
 *
 * @param scanInterval The scan interval in milliseconds.
 * @param activeSlots Bitfield representing active slots.
 */
void initModbusMonitor(uint16_t scanInterval, uint16_t activeSlots);

/**
 * @brief Initializes a monitoring slot with the provided parameters.
 *
 * @param ID The ID of the monitoring slot (0 to 15).
 * @param modbusCMD Pointer to the Modbus command array.
 * @param ThresholdActive Bitfield representing active thresholds.
 * @param SpikeUp The upward spike threshold.
 * @param SpikeDown The downward spike threshold.
 * @param thresholdHigh The high threshold value.
 * @param thresholdLow The low threshold value.
 * @param triggerFlagValue The trigger flag value.
 */
void initMonitorSlot(uint8_t ID, uint8_t *modbusCMD, uint8_t cmdSize, uint8_t dataType, uint8_t valueStartIndex, uint16_t ThresholdActive, VarData_u SpikeUp, VarData_u SpikeDown, VarData_u thresholdHigh, VarData_u thresholdLow, bool onChange, bool triggerFlagValue);


/**
 * @brief Checks a monitoring slot for alarm conditions and prints the Modbus response.
 *
 * @param ID The ID of the monitoring slot.
 * @param modbusResponse Pointer to the ModBus response buffer.
 */
void checkSlot(uint8_t ID, ModBus_t *modbusResponse);



VarData_u convertBytesToData(uint8_t *bytes, uint8_t dataType);

#endif /* INC_PWX_MODBUSMONITORING_H_ */
