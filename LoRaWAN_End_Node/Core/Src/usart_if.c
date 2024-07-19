/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart_if.c
  * @author  MCD Application Team
  * @brief   Configuration of UART driver interface for hyperterminal communication
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usart_if.h"

/* USER CODE BEGIN Includes */
#include "sys_app.h"
#include "project_config.h"
#include <stdbool.h>
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/**
  * @brief DMA handle
  */
extern DMA_HandleTypeDef hdma_usart2_tx;

/**
  * @brief UART handle
  */
extern UART_HandleTypeDef huart2;

/**
  * @brief buffer to receive 1 character
  */
uint8_t charRx;

/* USER CODE BEGIN EV */
#define HEX8(X)   X[0], X[1], X[2], X[3], X[4], X[5], X[6], X[7]
#define HEX16(X)  HEX8(X), X[8], X[9], X[10], X[11], X[12], X[13], X[14], X[15]

bool isConfigMode = false;

extern ModBus_t ModbusResp;
/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/**
  * @brief Trace driver callbacks handler
  */
const UTIL_ADV_TRACE_Driver_s UTIL_TraceDriver =
{
  vcom_Init,
  vcom_DeInit,
  vcom_ReceiveInit,
  vcom_Trace_DMA,
};

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/**
  * @brief  TX complete callback
  * @return none
  */
static void (*TxCpltCallback)(void *);
/**
  * @brief  RX complete callback
  * @param  rxChar ptr of chars buffer sent by user
  * @param  size buffer size
  * @param  error errorcode
  * @return none
  */
static void (*RxCpltCallback)(uint8_t *rxChar, uint16_t size, uint8_t error);

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/

UTIL_ADV_TRACE_Status_t vcom_Init(void (*cb)(void *))
{
  /* USER CODE BEGIN vcom_Init_1 */

  /* USER CODE END vcom_Init_1 */
  TxCpltCallback = cb;
  MX_DMA_Init();
  MX_USART2_UART_Init();
  LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_27);
  return UTIL_ADV_TRACE_OK;
  /* USER CODE BEGIN vcom_Init_2 */

  /* USER CODE END vcom_Init_2 */
}

UTIL_ADV_TRACE_Status_t vcom_DeInit(void)
{
  /* USER CODE BEGIN vcom_DeInit_1 */

  /* USER CODE END vcom_DeInit_1 */
  /* ##-1- Reset peripherals ################################################## */
  __HAL_RCC_USART2_FORCE_RESET();
  __HAL_RCC_USART2_RELEASE_RESET();

  /* ##-2- MspDeInit ################################################## */
  HAL_UART_MspDeInit(&huart2);

  /* ##-3- Disable the NVIC for DMA ########################################### */
  /* USER CODE BEGIN 1 */
  HAL_NVIC_DisableIRQ(DMA1_Channel5_IRQn);

  return UTIL_ADV_TRACE_OK;
  /* USER CODE END 1 */
  /* USER CODE BEGIN vcom_DeInit_2 */

  /* USER CODE END vcom_DeInit_2 */
}

void vcom_Trace(uint8_t *p_data, uint16_t size)
{
  /* USER CODE BEGIN vcom_Trace_1 */

  /* USER CODE END vcom_Trace_1 */
  HAL_UART_Transmit(&huart2, p_data, size, 1000);
  /* USER CODE BEGIN vcom_Trace_2 */

  /* USER CODE END vcom_Trace_2 */
}

UTIL_ADV_TRACE_Status_t vcom_Trace_DMA(uint8_t *p_data, uint16_t size)
{
  /* USER CODE BEGIN vcom_Trace_DMA_1 */

  /* USER CODE END vcom_Trace_DMA_1 */
  HAL_UART_Transmit_DMA(&huart2, p_data, size);
  return UTIL_ADV_TRACE_OK;
  /* USER CODE BEGIN vcom_Trace_DMA_2 */

  /* USER CODE END vcom_Trace_DMA_2 */
}

UTIL_ADV_TRACE_Status_t vcom_ReceiveInit(void (*RxCb)(uint8_t *rxChar, uint16_t size, uint8_t error))
{
  /* USER CODE BEGIN vcom_ReceiveInit_1 */

  /* USER CODE END vcom_ReceiveInit_1 */
  UART_WakeUpTypeDef WakeUpSelection;

  /*record call back*/
  RxCpltCallback = RxCb;

  /*Set wakeUp event on start bit*/
  WakeUpSelection.WakeUpEvent = UART_WAKEUP_ON_STARTBIT;

  HAL_UARTEx_StopModeWakeUpSourceConfig(&huart2, WakeUpSelection);

  /* Make sure that no UART transfer is on-going */
  while (__HAL_UART_GET_FLAG(&huart2, USART_ISR_BUSY) == SET);

  /* Make sure that UART is ready to receive)   */
  while (__HAL_UART_GET_FLAG(&huart2, USART_ISR_REACK) == RESET);

  /* Enable USART interrupt */
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_WUF);

  /*Enable wakeup from stop mode*/
  HAL_UARTEx_EnableStopMode(&huart2);

  /*Start LPUART receive on IT*/
  HAL_UART_Receive_IT(&huart2, &charRx, 1);

  return UTIL_ADV_TRACE_OK;
  /* USER CODE BEGIN vcom_ReceiveInit_2 */

  /* USER CODE END vcom_ReceiveInit_2 */
}

void vcom_Resume(void)
{
  /* USER CODE BEGIN vcom_Resume_1 */

  /* USER CODE END vcom_Resume_1 */
  /*to re-enable lost UART settings*/
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }

  /*to re-enable lost DMA settings*/
  if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN vcom_Resume_2 */

  /* USER CODE END vcom_Resume_2 */
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  /* USER CODE BEGIN HAL_UART_TxCpltCallback_1 */

  /* USER CODE END HAL_UART_TxCpltCallback_1 */
  /* buffer transmission complete*/
  if (huart->Instance == USART2)
  {
    TxCpltCallback(NULL);
  }
  /* USER CODE BEGIN HAL_UART_TxCpltCallback_2 */

  /* USER CODE END HAL_UART_TxCpltCallback_2 */
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* USER CODE BEGIN HAL_UART_RxCpltCallback_1 */
  static char buffer[MAX_UART_BUFFER_SIZE];  // Static buffer for command storage
  static uint8_t bufferIndex = 0;
  static uint8_t devEui[SE_EUI_SIZE]; // Array to store DevEUI bytes
  static uint8_t _devEui[8]; // Array to store DevEUI bytes
  static uint8_t appEui[8]; // Array to store DevEUI bytes
  static uint8_t appKey[16]; // Array to store DevEUI bytes
  static uint64_t txInterval;
  static uint64_t hbInterval;

  SecureElementNvmData_t FlashNVM; //

  /* USER CODE END HAL_UART_RxCpltCallback_1 */
  if (huart->Instance == USART2)
  {
    if ((NULL != RxCpltCallback) && (HAL_UART_ERROR_NONE == huart->ErrorCode))
    {
      RxCpltCallback(&charRx, 1, 0);
    }

    /* Check for newline character */
	if (charRx == '\n') {
		/* Terminate buffer with null character for string processing */
		buffer[bufferIndex] = '\0';

		/* Enter Device to Config Mode */
		if (strncmp(buffer, "set config on", 13) == 0) {
			APP_LOG(TS_OFF, VLEVEL_M, "###### Configuration Mode: ON \r\n");
			isConfigMode = true;
		}

		/* Exit Device Config Mode */
		else if (strncmp(buffer, "set config off", 13) == 0) {
			APP_LOG(TS_OFF, VLEVEL_M, "###### Configuration Mode: OFF \r\n");
			isConfigMode = false; // wont work, so just restart it
			//HAL_NVIC_SystemReset();
		}



		/* Enter Lora Config Mode */
		else if (strncmp(buffer, "set lora-config on", 18) == 0) {
			APP_LOG(TS_OFF, VLEVEL_M, "###### Lora-Configuration Mode: ON \r\n");
			if (FLASH_IF_Read(&FlashNVM, LORAWAN_NVM_BASE_ADDRESS, sizeof(FlashNVM)) == FLASH_IF_OK) {
				memcpy1( ( uint8_t * )&_devEui, ( uint8_t * )FlashNVM.SeNvmDevJoinKey.DevEui, sizeof(_devEui));
				memcpy1( ( uint8_t * )&appEui, ( uint8_t * )FlashNVM.SeNvmDevJoinKey.JoinEui, sizeof(appEui));
				memcpy1( ( uint8_t * )&appKey, ( uint8_t * )FlashNVM.KeyList[0].KeyValue,    sizeof(appKey));
				txInterval = FlashNVM.pwxTxInterval;
				hbInterval = FlashNVM.pwxHeartbeatInterval;
			} else {
				APP_LOG(TS_OFF, VLEVEL_M, "FAILED READING FLASH \r\n");
			}
		}

		/* View Lora Config Settings */
		else if (strncmp(buffer, "get lora-config", 15) == 0) {
			if (FLASH_IF_Read(&FlashNVM, LORAWAN_NVM_BASE_ADDRESS, sizeof(FlashNVM)) == FLASH_IF_OK) {
				APP_LOG( TS_OFF, VLEVEL_M, "###### Lora-Configuration: \r\n");
				APP_LOG( TS_OFF, VLEVEL_M, "###### DevEUI:      %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", HEX8( FlashNVM.SeNvmDevJoinKey.DevEui ) );
				APP_LOG( TS_OFF, VLEVEL_M, "###### AppEUI:      %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", HEX8( FlashNVM.SeNvmDevJoinKey.JoinEui ) );
				APP_LOG( TS_OFF, VLEVEL_M, "###### APPKEY:      %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", HEX16(FlashNVM.KeyList[0].KeyValue));
				APP_LOG( TS_OFF, VLEVEL_M, "###### ModBus Burst Interval: %u \r\n", FlashNVM.pwxTxInterval);
				APP_LOG( TS_OFF, VLEVEL_M, "###### ModBus HeartBeat Interval: %u \r\n", FlashNVM.pwxHeartbeatInterval);
				APP_LOG( TS_OFF, VLEVEL_M, "###### ADR: On \r\n");
				APP_LOG( TS_OFF, VLEVEL_M, "###### SF: \r\n");
				/* TODO: Add Interval Here */
				/* TODO: Add ADR State Here: Default is ADR ON */
				/* TODO: Add SF Here */


			} else {
				APP_LOG(TS_OFF, VLEVEL_M, "FAILED READING FLASH \r\n");
			}


		}

		/* Set DEV EUI */
		else if (strncmp(buffer, "set deveui ", 11) == 0) {
			/* Parse and store hexadecimal bytes */
			for (int i = 0; i < 8; i++) {
			  sscanf(&buffer[11 + i * 2], "%02x", &_devEui[i]);
			}
			APP_LOG( TS_OFF, VLEVEL_M, "###### DevEUI:      %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", HEX8(_devEui));

		}

		/* Set APP EUI */
		else if (strncmp(buffer, "set appeui ", 11) == 0) {
			/* Parse and store hexadecimal bytes */
			for (int i = 0; i < 8; i++) {
			  sscanf(&buffer[11 + i * 2], "%02x", &appEui[i]);
			}
			APP_LOG( TS_OFF, VLEVEL_M, "###### AppEUI:      %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", HEX8(appEui));

		}

		/* Set APP KEY */
		else if (strncmp(buffer, "set appkey ", 11) == 0) {
			/* Parse and store hexadecimal bytes */
			for (int i = 0; i < 16; i++) {
			  sscanf(&buffer[11 + i * 2], "%02x", &appKey[i]);
			}
			APP_LOG( TS_OFF, VLEVEL_M, "###### APPKEY:      %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", HEX16(appKey));
		}

		/* Set Burst mode TX Interval */
		else if (strncmp(buffer, "set lora-tx interval ", 21) == 0) {
			// Add code for APP INTERVAL

			sscanf(&buffer[21], "%u", &txInterval);

			APP_LOG( TS_OFF, VLEVEL_M, "###### ModBus Burst Interval: %u \r\n", txInterval);

		}
		/* Set ModBus HeartBeat TX Interval */
		else if (strncmp(buffer, "set modbus-hb interval ", 23) == 0) {
			// Add code for APP INTERVAL

			sscanf(&buffer[23], "%u", &hbInterval);

			APP_LOG( TS_OFF, VLEVEL_M, "###### ModBus Heartbeat Interval: %u \r\n", hbInterval);

		}


		/* View Current User Config before saving*/
		else if (strncmp(buffer, "view config", 11) == 0) {
			APP_LOG( TS_OFF, VLEVEL_M, "###### Current Lora-Configuration: \r\n");
			APP_LOG( TS_OFF, VLEVEL_M, "###### DevEUI:      %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", HEX8(_devEui) );
			APP_LOG( TS_OFF, VLEVEL_M, "###### AppEUI:      %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", HEX8(appEui) );
			APP_LOG( TS_OFF, VLEVEL_M, "###### APPKEY:      %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", HEX16(appKey));
			APP_LOG( TS_OFF, VLEVEL_M, "###### ModBus Burst Interval: %u \r\n", txInterval);
			APP_LOG( TS_OFF, VLEVEL_M, "###### ModBus HeartBeat Interval: %u \r\n", hbInterval);
			APP_LOG( TS_OFF, VLEVEL_M, "###### ADR: On \r\n");
			APP_LOG( TS_OFF, VLEVEL_M, "###### SF: \r\n");
		}

		/* Save Current User Config to NVM */
		else if (strncmp(buffer, "save lora-config", 16) == 0) {
			APP_LOG(TS_OFF, VLEVEL_M, "###### Saving Lora Configuration \r\n");
			// save data to flash
			memcpy1( ( uint8_t * )FlashNVM.SeNvmDevJoinKey.DevEui,  ( uint8_t * )&_devEui, sizeof(_devEui));
			memcpy1( ( uint8_t * )FlashNVM.SeNvmDevJoinKey.JoinEui, ( uint8_t * )&appEui, sizeof(appEui));
			memcpy1( ( uint8_t * )FlashNVM.KeyList[0].KeyValue,     ( uint8_t * )&appKey, sizeof(appKey));
			memcpy1( ( uint8_t * )FlashNVM.KeyList[1].KeyValue,     ( uint8_t * )&appKey, sizeof(appKey));
			memcpy1( ( uint8_t * )FlashNVM.KeyList[2].KeyValue,     ( uint8_t * )&appKey, sizeof(appKey));
			memcpy1( ( uint8_t * )FlashNVM.KeyList[3].KeyValue,     ( uint8_t * )&appKey, sizeof(appKey));

			FlashNVM.pwxTxInterval = txInterval;
			FlashNVM.pwxHeartbeatInterval  = hbInterval;

			if (FLASH_IF_Erase(LORAWAN_NVM_BASE_ADDRESS, FLASH_PAGE_SIZE) == FLASH_IF_OK){
				if(FLASH_IF_Write(LORAWAN_NVM_BASE_ADDRESS, &FlashNVM, sizeof(FlashNVM)) == FLASH_IF_OK){
					APP_LOG(TS_OFF, VLEVEL_M, "###### Success Saving to Flash \r\n");

					// Read saved parameters
					if (FLASH_IF_Read(&FlashNVM, LORAWAN_NVM_BASE_ADDRESS, sizeof(FlashNVM)) == FLASH_IF_OK) {
						APP_LOG( TS_OFF, VLEVEL_M, "###### Lora-Configuration: \r\n");
						APP_LOG( TS_OFF, VLEVEL_M, "###### DevEUI:      %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", HEX8( FlashNVM.SeNvmDevJoinKey.DevEui ) );
						APP_LOG( TS_OFF, VLEVEL_M, "###### AppEUI:      %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", HEX8( FlashNVM.SeNvmDevJoinKey.JoinEui ) );
						APP_LOG( TS_OFF, VLEVEL_M, "###### APPKEY:      %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", HEX16(FlashNVM.KeyList[0].KeyValue));
						APP_LOG( TS_OFF, VLEVEL_M, "###### ModBus Burst Interval: %u \r\n", FlashNVM.pwxTxInterval);
						APP_LOG( TS_OFF, VLEVEL_M, "###### ModBus HeartBeat Interval: %u \r\n", FlashNVM.pwxHeartbeatInterval);
						APP_LOG( TS_OFF, VLEVEL_M, "###### ADR: On \r\n");
						APP_LOG( TS_OFF, VLEVEL_M, "###### SF: \r\n");
						/* TODO: Add Interval Here */
						/* TODO: Add ADR State Here: Default is ADR ON */
						/* TODO: Add SF Here */
					} else {
						APP_LOG(TS_OFF, VLEVEL_M, "FAILED READING FLASH \r\n");
					}
				}else{
					APP_LOG(TS_OFF, VLEVEL_M, "###### Error Saving to Flash \r\n");
				}
			}else{
				APP_LOG(TS_OFF, VLEVEL_M, "###### Error Erasing Flash \r\n");
			}
		}

		/* View Device and Segment Settings */
		else if (strncmp(buffer, "get modbus-params dev ", 22) == 0) {
		    uint8_t devID, segID;
		    if (sscanf(&buffer[22], "%u seg %u", &devID, &segID) == 2) {

		    	if(devID >= 1 && devID <= 16 && segID >= 1 && segID <= 16){

					struct ModbusDevice _modbusDevice;

					if (FLASH_IF_Read(&_modbusDevice, ModbusDeviceFlashAddresses[devID - 1], sizeof(struct ModbusDevice)) == FLASH_IF_OK) {
						APP_LOG(TS_OFF, VLEVEL_M, "READ OK\r\n");
					} else {
						APP_LOG(TS_OFF, VLEVEL_M, "READ ERROR\r\n");
					}
					APP_LOG(TS_OFF, VLEVEL_M, "###### Configuration for Dev ID: %u, Seg ID: %u \r\n", devID, segID);
					APP_LOG(TS_OFF, VLEVEL_M, "Baudrate: %u \r\n", _modbusDevice.Baudrate);
					APP_LOG(TS_OFF, VLEVEL_M, "Parity: %d   \r\n", _modbusDevice.Parity);
					APP_LOG(TS_OFF, VLEVEL_M, "StopBits: %d \r\n", _modbusDevice.StopBits);
					APP_LOG(TS_OFF, VLEVEL_M, "Device Enable: %s \r\n", _modbusDevice.DeviceActive == 1 ? "true" : "false");


					APP_LOG(TS_OFF, VLEVEL_M, "\r");
					APP_LOG(TS_OFF, VLEVEL_M, "Segment %d Configuration: \r\n", segID);
					APP_LOG(TS_OFF, VLEVEL_M, "enableSegment: %s \r\n", (uint8_t)_modbusDevice.Segment[segID - 1].enableSegment == 1? "true" : "false");
					APP_LOG(TS_OFF, VLEVEL_M, "cmdRaw: ");

					// Determine the size of cmdRaw for the current segment
					size_t cmdSize = _modbusDevice.Segment[segID - 1].cmdSize;

					// Print each byte of cmdRaw
					for (int j = 0; j < cmdSize; j++) {
						APP_LOG(TS_OFF, VLEVEL_M, "%02X ", _modbusDevice.Segment[segID - 1].cmdRaw[j]);
					}

					APP_LOG(TS_OFF, VLEVEL_M, "\r\n");
					APP_LOG(TS_OFF, VLEVEL_M, "validAddresses: %08X \r\n", (uint32_t)_modbusDevice.Segment[segID - 1].validAddresses);
					APP_LOG(TS_OFF, VLEVEL_M, "sendNow: %s \r\n\n", _modbusDevice.Segment[segID - 1].sendNow ? "true" : "false");


		    	}else{
		    		APP_LOG(TS_OFF, VLEVEL_M, "Parameters out of range \r\n");
		    	}


		    } else {

		        APP_LOG(TS_OFF, VLEVEL_M, "Invalid input format\r\n");
		    }
		}

		/* Set Device Settings */
		else if (strncmp(buffer, "set device-params ", 18) == 0) {
			uint8_t cmdID, devId, baudRate, parity, stopBit, isActive;

			// Parse the input string
			int parsed = sscanf(buffer, "set device-params %u %u %u %u %u %u", &cmdID, &devId, &baudRate, &parity, &stopBit, &isActive);

			// Check if all values were successfully parsed
			if (parsed == 6) {
				APP_LOG(TS_OFF, VLEVEL_M, "cmdID: %u, DevId: %u, Baudrate: %u, Parity: %u, StopBit: %u, Active: %u \r\n", cmdID, devId, baudRate, parity, stopBit, isActive);
				struct ModbusDevice _modbusDevice;

				if (FLASH_IF_Read(&_modbusDevice, ModbusDeviceFlashAddresses[devId - 1], sizeof(struct ModbusDevice)) == FLASH_IF_OK) {
					APP_LOG(TS_OFF, VLEVEL_M, "READ OK\r\n");
				} else {
					APP_LOG(TS_OFF, VLEVEL_M, "READ ERROR\r\n");
				}
				if(cmdID == 0x01){
					_modbusDevice.Baudrate = baudRate;
					_modbusDevice.Parity   = parity;
					_modbusDevice.StopBits = stopBit;
					_modbusDevice.DeviceActive = isActive == 0 ? 0 : 1;
				}

				if (FLASH_IF_Erase((void *)ModbusDeviceFlashAddresses[devId - 1], FLASH_PAGE_SIZE) == FLASH_IF_OK){
					APP_LOG(TS_OFF, VLEVEL_M, "ERASE OK");
					if(FLASH_IF_Write((void *)ModbusDeviceFlashAddresses[devId - 1], (void *)&_modbusDevice, sizeof(struct ModbusDevice)) == FLASH_IF_OK){
						APP_LOG(TS_OFF, VLEVEL_M, "WRITE OK");
					}
					else{
						APP_LOG(TS_OFF, VLEVEL_M, "WRITE ERROR");
					}
				}
				else{
					APP_LOG(TS_OFF, VLEVEL_M, "ERASE ERROR");
				}


			} else {
				APP_LOG(TS_OFF, VLEVEL_M, "Error: Failed to parse input string \r\n");
			}

		}

		/* Set Segment Settings */
		else if (strncmp(buffer, "set segment-params ", 19) == 0) {
			uint8_t cmdID, devId, segId, enSegment, sendNow, validAdd_1, validAdd_2, validAdd_3, validAdd_4, cmdSize;

			int parsed = sscanf(buffer, "set segment-params %u %u %u %u %u %x %x %x %x %x ", &cmdID, &devId, &segId, &enSegment, &sendNow, &validAdd_1, &validAdd_2, &validAdd_3, &validAdd_4, &cmdSize);

			uint32_t _validAddr = 0;

			_validAddr |= ((uint32_t)validAdd_1 << 24);
			_validAddr |= ((uint32_t)validAdd_2 << 16);
			_validAddr |= ((uint32_t)validAdd_3 << 8);
			_validAddr |= validAdd_4;

			if(parsed == 10){
				APP_LOG(TS_OFF, VLEVEL_M, "cmdID: %u, devId: %u, segId: %u, enSegment: %u, sendNow: %u, validAdd_1: %02X, validAdd_2: %02X, validAdd_3: %02X, validAdd_4: %02X, cmdSize: %u \r\n", cmdID, devId, segId, enSegment, sendNow, sendNow, validAdd_1, validAdd_2, validAdd_3, validAdd_4, cmdSize);

				uint8_t *cmdRaw = (uint8_t*)malloc(cmdSize * sizeof(uint8_t));


				for(uint8_t i = 0; i < cmdSize; i++){
					sscanf(&buffer[49 + (i * 3)], "%x ", (unsigned int *)&cmdRaw[i]);
				}


				APP_LOG(TS_OFF, VLEVEL_M, "CMD RAW: ");
				for (int j = 0; j < cmdSize; j++) {
					APP_LOG(TS_OFF, VLEVEL_M, "%02X ", cmdRaw[j]);
				}
				APP_LOG(TS_OFF, VLEVEL_M, "\r\n");

				struct ModbusDevice _modbusDevice;

				if (FLASH_IF_Read(&_modbusDevice, ModbusDeviceFlashAddresses[devId - 1], sizeof(struct ModbusDevice)) == FLASH_IF_OK) {
					APP_LOG(TS_OFF, VLEVEL_M, "READ OK\r\n");
				} else {
					APP_LOG(TS_OFF, VLEVEL_M, "READ ERROR\r\n");
				}

				for(uint8_t i = 0; i < cmdSize; i++){
					_modbusDevice.Segment[segId - 1].cmdRaw[i] = cmdRaw[i];
				}

				free(cmdRaw);

				_modbusDevice.Segment[segId - 1].cmdSize        = cmdSize;
				_modbusDevice.Segment[segId - 1].sendNow        = sendNow == 0 ? 0 : 1;
				_modbusDevice.Segment[segId - 1].validAddresses = _validAddr;
				_modbusDevice.Segment[segId - 1].enableSegment  = enSegment == 0 ? 0 : 1;

				if (FLASH_IF_Erase((void *)ModbusDeviceFlashAddresses[devId - 1], FLASH_PAGE_SIZE) == FLASH_IF_OK){
					APP_LOG(TS_OFF, VLEVEL_M, "ERASE OK");
					if(FLASH_IF_Write((void *)ModbusDeviceFlashAddresses[devId - 1], (void *)&_modbusDevice, sizeof(struct ModbusDevice)) == FLASH_IF_OK){
						APP_LOG(TS_OFF, VLEVEL_M, "WRITE OK");
					}
					else{
						APP_LOG(TS_OFF, VLEVEL_M, "WRITE ERROR");
					}
				}
				else{
					APP_LOG(TS_OFF, VLEVEL_M, "ERASE ERROR");
				}

			}
			else{
				APP_LOG(TS_OFF, VLEVEL_M, "Error: Failed to parse input string \r\n");
			}


		}

		/* Clear Device Settings */
		else if (strncmp(buffer, "clear modbus-params dev ", 24) == 0) {
			uint8_t devID;
			sscanf(&buffer[24], "%u ", &devID);
			clearModbusParams(devID);
		}

		/* Restart Device */
		else if (strncmp(buffer, "set device restart", 18) == 0) {
			HAL_NVIC_SystemReset();
		}


		// Flags Monitoring Section
		else if (strncmp(buffer, "set slot-params ", 16) == 0) {
			uint8_t cmdID, slotId, dataType, valueStartIndex, THActive_1, THActive_2, SpikeUp_1, SpikeUp_2, SpikeUp_3, SpikeUp_4, SpikeDown_1, SpikeDown_2, SpikeDown_3, SpikeDown_4, THHigh_1, THHigh_2, THHigh_3, THHigh_4, THLow_1, THLow_2, THLow_3, THLow_4, onChange, cmdSize;

			int parsed = sscanf(buffer, "set slot-params %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x ", &cmdID, &slotId, &dataType, &valueStartIndex, &THActive_1, &THActive_2, &SpikeUp_1, &SpikeUp_2, &SpikeUp_3, &SpikeUp_4, &SpikeDown_1, &SpikeDown_2, &SpikeDown_3, &SpikeDown_4, &THHigh_1, &THHigh_2, &THHigh_3, &THHigh_4, &THLow_1, &THLow_2, &THLow_3, &THLow_4, &onChange, &cmdSize);
			if(parsed == 24){
				APP_LOG(TS_OFF, VLEVEL_M, "NUM PARSED DATA: %d \r\n", parsed);
				APP_LOG(TS_OFF, VLEVEL_M, "SLOT ID: %d \r\n", slotId);
				APP_LOG(TS_OFF, VLEVEL_M, "CMD ID: %02X \r\n", cmdID);
				APP_LOG(TS_OFF, VLEVEL_M, "CMD SIZE: %d \r\n", cmdSize);
				APP_LOG(TS_OFF, VLEVEL_M, "DataType: %d \r\n", dataType);
				APP_LOG(TS_OFF, VLEVEL_M, "Data Start Index: %d \r\n", valueStartIndex);
				APP_LOG(TS_OFF, VLEVEL_M, "Threshold Active: %02X %02X \r\n", THActive_1, THActive_2);
				APP_LOG(TS_OFF, VLEVEL_M, "Spike Up TH: %02X %02X %02X %02X \r\n", SpikeUp_1, SpikeUp_2, SpikeUp_3, SpikeUp_4);
				APP_LOG(TS_OFF, VLEVEL_M, "Spike Down TH: %02X %02X %02X %02X \r\n", SpikeDown_1, SpikeDown_2, SpikeDown_3, SpikeDown_4);
				APP_LOG(TS_OFF, VLEVEL_M, "Threshold High TH: %02X %02X %02X %02X \r\n", THHigh_1, THHigh_2, THHigh_3, THHigh_4);
				APP_LOG(TS_OFF, VLEVEL_M, "Threshold Low TH: %02X %02X %02X %02X \r\n", THLow_1, THLow_2, THLow_3, THLow_4);
				APP_LOG(TS_OFF, VLEVEL_M, "On Change: %s \r\n", onChange == 1 ? "true" : "false");

				uint8_t *cmdRaw = (uint8_t*)malloc(cmdSize * sizeof(uint8_t));

				if (cmdRaw != NULL) {
				    memset(cmdRaw, 0, cmdSize * sizeof(uint8_t));
				}


				for(uint8_t i = 0; i < cmdSize; i++){
					sscanf(&buffer[88 + (i * 3)], "%x ", (unsigned int *)&cmdRaw[i]);
				}


				APP_LOG(TS_OFF, VLEVEL_M, "CMD RAW: ");
				for (int j = 0; j < cmdSize; j++) {
					APP_LOG(TS_OFF, VLEVEL_M, "%02X ", cmdRaw[j]);
				}
				APP_LOG(TS_OFF, VLEVEL_M, "\r\n");

				// Write to NVM
				struct ModbusDevice MonitoringSlotNVM;

				if (FLASH_IF_Read(&MonitoringSlotNVM, ModbusDeviceFlashAddresses[slotId - 1], sizeof(MonitoringSlotNVM)) == FLASH_IF_OK) {
				  MonitoringSlotNVM.MonitoringSlot.cmdSize = cmdSize;


				  for(uint8_t i = 0; i < cmdSize; i++){
					  MonitoringSlotNVM.MonitoringSlot.modbusCMD[i] = cmdRaw[i];
				  }

				  free(cmdRaw);

				  MonitoringSlotNVM.MonitoringSlot.dataType = dataType;
				  MonitoringSlotNVM.MonitoringSlot.valueStartIndex = valueStartIndex;
				  MonitoringSlotNVM.MonitoringSlot.ThresholdActive = (THActive_1 << 8) | THActive_2;

				  VarData_u TH_High, TH_Low, TH_SpikeUp, TH_SpikeDown;

				  TH_High.buff[0] = THHigh_1;            // 30.00 degrees celsius
				  TH_High.buff[1] = THHigh_2;
				  TH_High.buff[2] = THHigh_3;
				  TH_High.buff[3] = THHigh_4;

				  TH_Low.buff[0] = THLow_1;              // 21.50 degrees celsius
				  TH_Low.buff[1] = THLow_2;
				  TH_Low.buff[2] = THLow_3;
				  TH_Low.buff[3] = THLow_4;

				  TH_SpikeUp.buff[0] = SpikeUp_1;        // +3 degree Celcius
				  TH_SpikeUp.buff[1] = SpikeUp_2;
				  TH_SpikeUp.buff[2] = SpikeUp_3;
				  TH_SpikeUp.buff[3] = SpikeUp_4;

				  TH_SpikeDown.buff[0] = SpikeDown_1;    // -3 degree Celcius
				  TH_SpikeDown.buff[1] = SpikeDown_2;
				  TH_SpikeDown.buff[2] = SpikeDown_3;
				  TH_SpikeDown.buff[3] = SpikeDown_4;

				  MonitoringSlotNVM.MonitoringSlot.SpikeUp = TH_SpikeUp;
				  MonitoringSlotNVM.MonitoringSlot.SpikeDown = TH_SpikeDown;
				  MonitoringSlotNVM.MonitoringSlot.thresholdHigh = TH_High;
				  MonitoringSlotNVM.MonitoringSlot.thresholdLow = TH_Low;
				  MonitoringSlotNVM.MonitoringSlot.onChange = onChange;
				  MonitoringSlotNVM.MonitoringSlot.triggerFlagValue = false;

					if (FLASH_IF_Erase((void *)ModbusDeviceFlashAddresses[slotId - 1], FLASH_PAGE_SIZE) == FLASH_IF_OK){
						APP_LOG(TS_OFF, VLEVEL_M, "ERASE OK");
						if(FLASH_IF_Write((void *)ModbusDeviceFlashAddresses[slotId - 1], (void *)&MonitoringSlotNVM, sizeof(struct ModbusDevice)) == FLASH_IF_OK){
							APP_LOG(TS_OFF, VLEVEL_M, "WRITE OK");
						}
						else{
							APP_LOG(TS_OFF, VLEVEL_M, "WRITE ERROR");
						}
					}
					else{
						APP_LOG(TS_OFF, VLEVEL_M, "ERASE ERROR");
					}

					initMonitorSlot(slotId - 1, MonitoringSlotNVM.MonitoringSlot.modbusCMD, MonitoringSlotNVM.MonitoringSlot.cmdSize, MonitoringSlotNVM.MonitoringSlot.dataType, MonitoringSlotNVM.MonitoringSlot.valueStartIndex, MonitoringSlotNVM.MonitoringSlot.ThresholdActive, MonitoringSlotNVM.MonitoringSlot.SpikeUp, MonitoringSlotNVM.MonitoringSlot.SpikeDown,  MonitoringSlotNVM.MonitoringSlot.thresholdHigh, MonitoringSlotNVM.MonitoringSlot.thresholdLow, MonitoringSlotNVM.MonitoringSlot.onChange, MonitoringSlotNVM.MonitoringSlot.triggerFlagValue);

					viewMonitoringSlotParams(slotId - 1);




				} else {
				  APP_LOG(TS_OFF, VLEVEL_M, "FAILED READING FLASH \r\n");
				}

			}
			else{
				APP_LOG(TS_OFF, VLEVEL_M, "Invalid Input \r\n", parsed);
			}

		}

		else if (strncmp(buffer, "view slot-params ", 17) == 0) {
			uint8_t slotId;
			sscanf(&buffer[17], "%u", &slotId);
			viewMonitoringSlotParams(slotId - 1);
		}

		else if (strncmp(buffer, "clear slot-params ", 18) == 0) {
			uint8_t slotId;
			sscanf(&buffer[18], "%u", &slotId);
			initMonitoringSlotParams(slotId - 1);
			viewMonitoringSlotParams(slotId - 1);
		}

		else if (strncmp(buffer, "clear all slots-params", 22) == 0) {
			for(uint8_t i = 0; i < 16; i++){
				initMonitoringSlotParams(i);
			}
		}


		else if (strncmp(buffer, "set slot-active ", 16) == 0) {
			uint8_t slotId, state;
			sscanf(&buffer[16], "%u %u", &slotId, &state);

			struct ModbusDevice MonitoringSlotNVM;

			if (FLASH_IF_Read(&MonitoringSlotNVM, ModbusDeviceFlashAddresses[slotId - 1], sizeof(MonitoringSlotNVM)) == FLASH_IF_OK) {

				MonitoringSlotNVM.MonitoringSlot.isActive = state == 1 ? 1: 0;
				if (FLASH_IF_Erase((void *)ModbusDeviceFlashAddresses[slotId - 1], FLASH_PAGE_SIZE) == FLASH_IF_OK){
					APP_LOG(TS_OFF, VLEVEL_M, "ERASE OK");
					if(FLASH_IF_Write((void *)ModbusDeviceFlashAddresses[slotId - 1], (void *)&MonitoringSlotNVM, sizeof(struct ModbusDevice)) == FLASH_IF_OK){
						APP_LOG(TS_OFF, VLEVEL_M, "WRITE OK");
					}
					else{
						APP_LOG(TS_OFF, VLEVEL_M, "WRITE ERROR");
					}
				}
				else{
					APP_LOG(TS_OFF, VLEVEL_M, "ERASE ERROR");
				}


			} else {
			  APP_LOG(TS_OFF, VLEVEL_M, "FAILED READING FLASH \r\n");
			}

		}


		/* Invalid Command */
		else {
			APP_LOG(TS_OFF, VLEVEL_M, "###### Invalid Command! \r\n");
		}

		/* Reset buffer index and prepare for next command */
		bufferIndex = 0;
	} else {
		/* Store received byte in buffer (handle buffer overflow) */
		if (bufferIndex < MAX_UART_BUFFER_SIZE - 1) {
			buffer[bufferIndex++] = charRx;
		} else {
			// Handle buffer overflow (e.g., print error message)
		}
	}

    HAL_UART_Receive_IT(huart, &charRx, 1);
  }
  /* USER CODE BEGIN HAL_UART_RxCpltCallback_2 */
  if (huart->Instance == USART1) {

 	Modbus_RxCallback(&ModbusResp);
 	HAL_UART_Receive_IT(&huart1, (uint8_t *)(ModbusResp.buffer + ModbusResp.rxIndex), 1);

   }
  /* USER CODE END HAL_UART_RxCpltCallback_2 */
}


/* USER CODE BEGIN EF */

/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/

/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */
