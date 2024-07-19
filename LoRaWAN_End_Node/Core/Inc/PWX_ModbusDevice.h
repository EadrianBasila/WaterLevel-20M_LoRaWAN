/**
 * @file PWX_ModbusMonitoring.h
 * @brief Modbus Monitoring Header
 * @date August 22, 2023
 * @version 1.0
 * @author Charles Kim Kabiling
 */

#ifndef INC_PWX_MODBUSDEVICE_H_
#define INC_PWX_MODBUSDEVICE_H_

#include <stdint.h>
#include <stdbool.h>

#include "PWX_ST50H_Modbus.h"
#include "PWX_ModbusMonitoring.h"

/* Flash Addresses for Modbus Devices*/
#define MODBUS_DEVICE_BASE_ADDRESS_1   ((void *)0x0803C800UL)
#define MODBUS_DEVICE_BASE_ADDRESS_2   ((void *)(MODBUS_DEVICE_BASE_ADDRESS_1  - 0x800UL))
#define MODBUS_DEVICE_BASE_ADDRESS_3   ((void *)(MODBUS_DEVICE_BASE_ADDRESS_2  - 0x800UL))
#define MODBUS_DEVICE_BASE_ADDRESS_4   ((void *)(MODBUS_DEVICE_BASE_ADDRESS_3  - 0x800UL))
#define MODBUS_DEVICE_BASE_ADDRESS_5   ((void *)(MODBUS_DEVICE_BASE_ADDRESS_4  - 0x800UL))
#define MODBUS_DEVICE_BASE_ADDRESS_6   ((void *)(MODBUS_DEVICE_BASE_ADDRESS_5  - 0x800UL))
#define MODBUS_DEVICE_BASE_ADDRESS_7   ((void *)(MODBUS_DEVICE_BASE_ADDRESS_6  - 0x800UL))
#define MODBUS_DEVICE_BASE_ADDRESS_8   ((void *)(MODBUS_DEVICE_BASE_ADDRESS_7  - 0x800UL))
#define MODBUS_DEVICE_BASE_ADDRESS_9   ((void *)(MODBUS_DEVICE_BASE_ADDRESS_8  - 0x800UL))
#define MODBUS_DEVICE_BASE_ADDRESS_10  ((void *)(MODBUS_DEVICE_BASE_ADDRESS_9  - 0x800UL))
#define MODBUS_DEVICE_BASE_ADDRESS_11  ((void *)(MODBUS_DEVICE_BASE_ADDRESS_10 - 0x800UL))
#define MODBUS_DEVICE_BASE_ADDRESS_12  ((void *)(MODBUS_DEVICE_BASE_ADDRESS_11 - 0x800UL))
#define MODBUS_DEVICE_BASE_ADDRESS_13  ((void *)(MODBUS_DEVICE_BASE_ADDRESS_12 - 0x800UL))
#define MODBUS_DEVICE_BASE_ADDRESS_14  ((void *)(MODBUS_DEVICE_BASE_ADDRESS_13 - 0x800UL))
#define MODBUS_DEVICE_BASE_ADDRESS_15  ((void *)(MODBUS_DEVICE_BASE_ADDRESS_14 - 0x800UL))
#define MODBUS_DEVICE_BASE_ADDRESS_16  ((void *)(MODBUS_DEVICE_BASE_ADDRESS_15 - 0x800UL))

#define NUM_DEV_SEGMENTS 16
#define NUM_DEVICES      16

struct Segment{
	uint8_t cmdSize;
	uint8_t cmdRaw[32];
	uint8_t sendNow;
	uint64_t validAddresses;
	uint8_t enableSegment;
};

struct ModbusDevice{

	uint8_t DeviceActive;

	uint8_t ID;
	uint8_t Baudrate;
	uint8_t Parity;
	uint8_t StopBits;

	bool enableCRCCheck;
	bool standardModbus;

	struct Segment Segment[16];
	ModbusMonitorSlot_t MonitoringSlot;

};

typedef enum{
	PARITY_NONE = 0,
	PARITY_EVEN,
	PARITY_ODD
}Parity_e;

typedef enum{
	BAUD_9600 = 9600,
	BAUD_115200 = 115200
}BaudRate_e;


extern uint32_t ModbusBaudRates[];

extern uint32_t StopBitSettings[];

extern uint32_t ParitySettings[];



typedef enum{
	STOP_BIT_0 = 0,
	STOP_BIT_1 = 1,
	STOP_BIT_2 = 2
}StopBits_e;

typedef enum {
    COIL_STATUS = 1,
    INPUT_STATUS = 2,
    HOLDING_REGISTER = 3,
	DISCRETE_INPUT = 4

} ModbusCommandType_e;

// Example structure for Modbus response
struct ModbusResponse {
    // Add other members as needed
    uint16_t holdingRegisters[64]; // Assuming 64 registers for holding registers
    uint8_t discreteInputs[64];    // Assuming 64 registers for discrete inputs
};

void initModbusDevices();  // init counter for active device and segment.


void scanModbusDevice(struct ModbusDevice ModbusDevice, uint8_t SegmentID, size_t* outputSize, uint8_t* arrFilteredData);


void initModbusParameters(uint32_t baudRate, uint8_t parity, uint8_t stopBits);

size_t buildDataToSend(uint8_t* destination, uint8_t* source, size_t sourceSize, uint8_t desStartIndex);

void initArray(uint32_t *arr, size_t size,uint8_t val);

// Select device to clear from device 1 to 16
void clearModbusParams(uint8_t deviceNum);


void showMonitoringSlotParams(struct ModbusDevice MonitoringSlotNVM);

void initMonitoringSlotParams(uint8_t slotId);

void viewMonitoringSlotParams(uint8_t slotId);



#endif /* INC_PWX_MODBUSDEVICE_H_ */
