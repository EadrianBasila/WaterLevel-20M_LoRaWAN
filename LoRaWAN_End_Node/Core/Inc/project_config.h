/*
 * project_config.h
 *
 *  Created on: May 2, 2024
 *      Author: Owner
 */

#ifndef APPLICATION_USER_CORE_PROJECT_CONFIG_H_
#define APPLICATION_USER_CORE_PROJECT_CONFIG_H_

#include "secure-element-nvm.h";
#include "flash_if.h"
#include "stm32wlxx_hal.h"
#include <stdbool.h>
#include "sys_app.h"
#include "lora_app.h"
#include "PWX_ST50H_Modbus.h"
#include "PWX_ModbusDevice.h"
#include "PWX_ModbusMonitoring.h"

//#define LORA_UART_CONFIG

#define MAIN_VERSION   02
#define MEDIUM_VERSION 05
#define MINOR_VERSION  04

#define MAX_UART_BUFFER_SIZE 200

#define CLI_WAIT_TIME 5000

#define LORAWAN_NVM_BASE_ADDRESS                    ((void *)0x0803F000UL)


#define LORA_CONFIG_PARAMS_PORT 	55
#define CONFIG_DEVEUI_ID     		0x01
#define CONFIG_APPEUI_ID 			0x02
#define CONFIG_APPKEY_ID 			0x03
#define CONFIG_INTERVAL_ID 			0x04
#define CONFIG_ADR_ID 				0x05
#define CONFIG_SF_ID 				0x06
#define CONFIG_CONF_UPLINK_ID		0x07
#define CONFIG_LEVEL_THRESHOLD_ID	0x08
#define CONFIG_SENSING_MODE_ID		0x09
#define CONFIG_SAMPLING_COUNT_ID	0x0A

#define CONFIG_SAVE_REBOOT      0xAA

#define LORAWAN_MODBUS_DOWNLINK_PORT 25
//#define LORAWAN_MODBUS_DEVICE_CONFIG_ID             1
//#define LORAWAN_MODBUS_SEGMENT_CONFIG_ID            2
//#define LORAWAN_BYPASS_CMD_PORT                     3



#define LORAWAN_PARAM_CHECK_PORT                    20
#define LORAWAN_PARAM_CHECK_REPLY_PORT              23
#define PARAM_CHECK_FW_VERSION                      1

#define LORAWAN_EVENT_PORT                          21


#define BOARD_DIAGNOSTIC_PORT 22


extern const void *ModbusDeviceFlashAddresses[];

extern int MAX_UPLINK_BEFORE_CONFIRMED;
extern int MAX_WATER_LEVEL_SAMPLES;
extern int SAMPLE_INTERVAL_MS;
extern int TRANSMIT_INTERVAL_MS;
extern float thresholdLevel;
extern uint16_t confUplinkCounter;

extern bool isConfigMode;
extern int PWX_TX_INTERVAL;
extern bool justTransmitted;
extern ModBus_t ModbusResp;
extern uint8_t activeDeviceIndex;
extern uint8_t activeDeviceSegment;
extern bool hasConnectedOnce;
extern bool doneScanningDevices;
extern bool isTxSuccess;
extern bool isDiagnosticsSent;

extern bool DC1State;
extern bool DC2State;

extern uint8_t SECTION_TO_SEND;

#define SENDING_IDLE         0
#define BOARD_DIAGNOSTICS    1
#define MODBUS_DEVICES       2
#define EXTERNAL_PERIPHERALS 3

extern uint32_t doneScanningDevicesMillis;

extern SecureElementNvmData_t DeviceParamsNVM;


#endif /* APPLICATION_USER_CORE_PROJECT_CONFIG_H_ */
