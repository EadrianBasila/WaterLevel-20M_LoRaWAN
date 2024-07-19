/*
 * @file PWX_ModbusMonitoring.c
 * @brief Modbus Monitoring Source
 * @date August 22, 2023
 * @version 1.0
 * @author Charles Kim Kabiling
 */

#include "PWX_ModbusDevice.h"
#include "PWX_ST50H_Modbus.h"
#include "lora_app.h"
#include "project_config.h"
#include "usart.h"
#include <stdio.h>



extern ModBus_t ModbusResp;
extern uint8_t activeDeviceIndex;
extern uint8_t activeDeviceSegment;

extern uint32_t ModbusBaudRates[];
extern uint32_t StopBitSettings[];
extern uint32_t ParitySettings[];

const void *ModbusDeviceFlashAddresses[] = {
    MODBUS_DEVICE_BASE_ADDRESS_1,
    MODBUS_DEVICE_BASE_ADDRESS_2,
    MODBUS_DEVICE_BASE_ADDRESS_3,
    MODBUS_DEVICE_BASE_ADDRESS_4,
    MODBUS_DEVICE_BASE_ADDRESS_5,
    MODBUS_DEVICE_BASE_ADDRESS_6,
    MODBUS_DEVICE_BASE_ADDRESS_7,
    MODBUS_DEVICE_BASE_ADDRESS_8,
    MODBUS_DEVICE_BASE_ADDRESS_9,
    MODBUS_DEVICE_BASE_ADDRESS_10,
    MODBUS_DEVICE_BASE_ADDRESS_11,
    MODBUS_DEVICE_BASE_ADDRESS_12,
    MODBUS_DEVICE_BASE_ADDRESS_13,
    MODBUS_DEVICE_BASE_ADDRESS_14,
    MODBUS_DEVICE_BASE_ADDRESS_15,
    MODBUS_DEVICE_BASE_ADDRESS_16
};


uint16_t bytesToUInt16(uint8_t byte1, uint8_t byte2) {
    return (uint16_t)((byte1 << 8) | byte2);
}

void convertUint16ToUint8(uint16_t *input, size_t inputSize, uint8_t *output) {
    for (size_t i = 0; i < inputSize; ++i) {
        output[2 * i] = (uint8_t)(input[i] >> 8);    // Higher byte
        output[2 * i + 1] = (uint8_t)(input[i] & 0xFF);  // Lower byte
    }
}

void scanModbusDevice(struct ModbusDevice ModbusDevice, uint8_t SegmentID, size_t* outputSize, uint8_t* arrFilteredData){
	// Set Device Setting - Baud rate, Parity, Stop Bit
	initModbusParameters(ModbusDevice.Baudrate, ModbusDevice.Parity, ModbusDevice.StopBits);



	HAL_Delay(1000);


    // Send Commands to Modbus Device
	APP_LOG(TS_OFF, VLEVEL_M, "SENDING DEVICE MODBUS COMMAND \r\n");
	//printf("SENDING DEVICE MODBUS COMMAND \r\n");

	//	data = DeviceSample.Segment[1].cmd;
	//	sendRaw_CRC(data, getArrayLength(data), &ModbusResp);
	//	OS_Delay(1000);

	APP_LOG(TS_OFF, VLEVEL_M, "MODBUS COMMAND (Hex):  \r\n");

	for (int x = 0; x < ModbusDevice.Segment[SegmentID].cmdSize; x++){
		APP_LOG(TS_OFF, VLEVEL_M, "%02X ", ModbusDevice.Segment[SegmentID].cmdRaw[x]);
	}

	APP_LOG(TS_OFF, VLEVEL_M, "\r\n");

	sendRaw(ModbusDevice.Segment[SegmentID].cmdRaw, (uint16_t)ModbusDevice.Segment[SegmentID].cmdSize, &ModbusResp);
	HAL_Delay(2000);


//    // Parse Response
	APP_LOG(TS_OFF, VLEVEL_M, "MMODBUS RESPONSE (Hex):   \r\n");

	for (int x = 0; x < ModbusResp.rxIndex; x++){
		APP_LOG(TS_OFF, VLEVEL_M, "%02X \r\n", ModbusResp.buffer[x]);

	}
	APP_LOG(TS_OFF, VLEVEL_M, "\r\n");

	uint16_t bytesToSend[64];     // this will hold registers to send
	uint8_t bytesToSendIndex = 0;

	uint8_t cmdType = ModbusResp.buffer[1]; // Determine Modbus Command - Read or Write Coil or Registers

	if(cmdType == HOLDING_REGISTER || cmdType ==  DISCRETE_INPUT){
		printf("Parsing Data \n");
		for (uint8_t i = 0; i < 32; i++) { // 8 is used for testing instead of 64 for 64 bit value
			if ((ModbusDevice.Segment[SegmentID].validAddresses & (1 << i)) != 0) {
				uint8_t offset = (i * 2) + 3;
				uint16_t value = bytesToUInt16(ModbusResp.buffer[offset], ModbusResp.buffer[offset + 1]);
			    printf("Save Holding Register %d: Value = %04X\n", i, value);
				 // Store the value in the array
				if (bytesToSendIndex < sizeof(bytesToSend) / sizeof(bytesToSend[0])) {
					bytesToSend[bytesToSendIndex++] = value;
				} else {
					printf("Warning: Sending array is full.\n");
				}
			}
		}

		size_t _outputBytesSize = bytesToSendIndex;
		uint8_t outputBytes[_outputBytesSize * 2];

		convertUint16ToUint8(bytesToSend, _outputBytesSize, outputBytes);

		printf("Register to Send: ");
		for(uint8_t i = 0; i < _outputBytesSize * 2; i++){
			printf("%02X, ", outputBytes[i]);
		}
		printf("\n");


		for (size_t i = 0; i < _outputBytesSize * 2; i++) {
			arrFilteredData[i] = outputBytes[i];
		}


//		printf("Segment Data to Send:\n");
//		for (uint8_t i = 0; i < _outputBytesSize * 2; i++) {
//			printf("%02X ", arrFilteredData[i]);
//		}
//		printf("\n");

		*outputSize = _outputBytesSize * 2;

	}
	//else if(cmdType == COIL_STATUS || cmdType ==  INPUT_STATUS){
//		for (uint8_t i = 0; i < 8; i++) { // 8 is used for testing instead of 64 for 64 bit value
//			if ((ModbusDevice.Segment[0].validAddresses & (1 << i)) != 0) {
//				uint8_t offset = i + 3;
//				uint16_t value = ModbusResp.buffer[offset];
//				// printf("Save Holding Register %d: Value = %04X\n", i, value);
//				 // Store the value in the array
//				if (bytesToSendIndex < sizeof(bytesToSend) / sizeof(bytesToSend[0])) {
//					bytesToSend[bytesToSendIndex++] = value;
//				} else {
//					printf("Warning: Sending array is full.\n");
//				}
//
//			}
//
//		}
//		printf("Register to Send: ");
//		for(uint8_t i = 0; i < bytesToSendIndex; i++){
//			printf("%04X, ", bytesToSend[i]);
//		}
//		printf("\n");
	//}

	else{

		for (size_t i = 0; i < ModbusResp.rxIndex; i++) {
			arrFilteredData[i] = ModbusResp.buffer[i];
		}
		*outputSize = ModbusResp.rxIndex;
	}

}


void initModbusParameters(uint32_t baudRate, uint8_t parity, uint8_t stopBits) {
    // Deinitialize USART1 to reconfigure it
	  huart1.Instance = USART1;
	  huart1.Init.WordLength = UART_WORDLENGTH_8B;

//	  huart1.Init.BaudRate = 9600;
//	  huart1.Init.StopBits = UART_STOPBITS_1;
//	  huart1.Init.Parity = UART_PARITY_NONE;

	  huart1.Init.BaudRate = ModbusBaudRates[baudRate];
	  huart1.Init.StopBits = StopBitSettings[stopBits];
	  huart1.Init.Parity = ParitySettings[parity];

	  huart1.Init.Mode = UART_MODE_TX_RX;
	  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	  if (HAL_UART_Init(&huart1) != HAL_OK)
	  {
	    Error_Handler();
	  }
//	  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
//	  {
//	    Error_Handler();
//	  }
//	  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
//	  {
//	    Error_Handler();
//	  }
//	  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
//	  {
//	    Error_Handler();
//	  }

	  HAL_Delay(100);

	  HAL_UART_Receive_IT(&huart1, (uint8_t *)ModbusResp.buffer, 1);

}

size_t buildDataToSend(uint8_t* destination, uint8_t* source, size_t sourceSize, uint8_t desStartIndex) {

	// Append data from the source array to the destination array
	size_t destIndex = 0;
	for (size_t i = 0; i < sourceSize; ++i) {
		destIndex = desStartIndex + i;
		destination[destIndex] = source[i];

	}

	return destIndex + 1;
}

void initArray(uint32_t *arr, size_t size,uint8_t val) {
    for (size_t i = 0; i < size; ++i) {
        arr[i] = val;
    }
}


void clearModbusParams(uint8_t deviceNum){  // Device 1 to 16
	struct ModbusDevice _modbusInitParams;

	  _modbusInitParams.DeviceActive = 0;
	  _modbusInitParams.ID = 0;
	  _modbusInitParams.Parity = 0;
	  _modbusInitParams.StopBits = 0;
	  _modbusInitParams.Baudrate = 0;

	  for (int i = 0; i < NUM_DEV_SEGMENTS; i++) {
		  memset(_modbusInitParams.Segment[i].cmdRaw, 0, sizeof(_modbusInitParams.Segment[i].cmdRaw));
		  _modbusInitParams.Segment[i].cmdSize = 8;
		  _modbusInitParams.Segment[i].sendNow = 0;
		  _modbusInitParams.Segment[i].sendNow = 0;
		  _modbusInitParams.Segment[i].validAddresses = 0x00000000;
		  _modbusInitParams.Segment[i].enableSegment = 0;

		  HAL_Delay(10);
	  }

	  /* Write New Segment Configuration for Selected Device */
		if (FLASH_IF_Erase((void *)ModbusDeviceFlashAddresses[deviceNum - 1], FLASH_PAGE_SIZE) == FLASH_IF_OK){
			APP_LOG(TS_OFF, VLEVEL_M, "ERASE OK");
			if(FLASH_IF_Write((void *)ModbusDeviceFlashAddresses[deviceNum - 1], (void *)&_modbusInitParams, sizeof(struct ModbusDevice)) == FLASH_IF_OK){
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

void showMonitoringSlotParams(struct ModbusDevice MonitoringSlotNVM){
	uint8_t cmdSize = MonitoringSlotNVM.MonitoringSlot.cmdSize;

	APP_LOG(TS_OFF, VLEVEL_M, "ModBus Command: ");
	for (int j = 0; j < cmdSize; j++) {
		APP_LOG(TS_OFF, VLEVEL_M, "%02X ", MonitoringSlotNVM.MonitoringSlot.modbusCMD[j]);
	}
	APP_LOG(TS_OFF, VLEVEL_M, "\n");
	APP_LOG(TS_OFF, VLEVEL_M, "Baudrate: %u \r\n", MonitoringSlotNVM.Baudrate);
	APP_LOG(TS_OFF, VLEVEL_M, "Parity: %d   \r\n", MonitoringSlotNVM.Parity);
	APP_LOG(TS_OFF, VLEVEL_M, "StopBits: %d \r\n", MonitoringSlotNVM.StopBits);
	APP_LOG(TS_OFF, VLEVEL_M, "Device Enable: %s \r\n", MonitoringSlotNVM.DeviceActive == 1 ? "true" : "false");

	APP_LOG(TS_OFF, VLEVEL_M, "DataType: %d \r\n", MonitoringSlotNVM.MonitoringSlot.dataType);
	APP_LOG(TS_OFF, VLEVEL_M, "Data Start Index: %d \r\n", MonitoringSlotNVM.MonitoringSlot.valueStartIndex);
	APP_LOG(TS_OFF, VLEVEL_M, "Threshold Active: %04X \r\n", MonitoringSlotNVM.MonitoringSlot.ThresholdActive);
	APP_LOG(TS_OFF, VLEVEL_M, "Spike Up TH: %02X %02X %02X %02X \r\n", MonitoringSlotNVM.MonitoringSlot.SpikeUp.buff[0], MonitoringSlotNVM.MonitoringSlot.SpikeUp.buff[1], MonitoringSlotNVM.MonitoringSlot.SpikeUp.buff[2], MonitoringSlotNVM.MonitoringSlot.SpikeUp.buff[3]);
	APP_LOG(TS_OFF, VLEVEL_M, "Spike Down TH: %02X %02X %02X %02X \r\n", MonitoringSlotNVM.MonitoringSlot.SpikeDown.buff[0], MonitoringSlotNVM.MonitoringSlot.SpikeDown.buff[1], MonitoringSlotNVM.MonitoringSlot.SpikeDown.buff[2], MonitoringSlotNVM.MonitoringSlot.SpikeDown.buff[3]);
	APP_LOG(TS_OFF, VLEVEL_M, "Threshold High TH: %02X %02X %02X %02X \r\n", MonitoringSlotNVM.MonitoringSlot.thresholdHigh.buff[0], MonitoringSlotNVM.MonitoringSlot.thresholdHigh.buff[1], MonitoringSlotNVM.MonitoringSlot.thresholdHigh.buff[2], MonitoringSlotNVM.MonitoringSlot.thresholdHigh.buff[3]);
	APP_LOG(TS_OFF, VLEVEL_M, "Threshold Low TH: %02X %02X %02X %02X \r\n", MonitoringSlotNVM.MonitoringSlot.thresholdLow.buff[0], MonitoringSlotNVM.MonitoringSlot.thresholdLow.buff[1], MonitoringSlotNVM.MonitoringSlot.thresholdLow.buff[2], MonitoringSlotNVM.MonitoringSlot.thresholdLow.buff[3]);
	APP_LOG(TS_OFF, VLEVEL_M, "On Change: %s \r\n", MonitoringSlotNVM.MonitoringSlot.onChange == 1 ? "true" : "false");
	APP_LOG(TS_OFF, VLEVEL_M, "Active State : %s \r\n", MonitoringSlotNVM.MonitoringSlot.isActive == 1 ? "true" : "false");

}

void initMonitoringSlotParams(uint8_t slotId){

	struct ModbusDevice MonitoringSlotNVM;

	if (FLASH_IF_Read(&MonitoringSlotNVM, ModbusDeviceFlashAddresses[slotId], sizeof(MonitoringSlotNVM)) == FLASH_IF_OK) {
	  MonitoringSlotNVM.MonitoringSlot.cmdSize = 6;

	  uint8_t testCommand[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	  for(uint8_t i = 0; i < MonitoringSlotNVM.MonitoringSlot.cmdSize; i++){
		  MonitoringSlotNVM.MonitoringSlot.modbusCMD[i] = testCommand[i];
	  }
	  MonitoringSlotNVM.MonitoringSlot.dataType = Modbus_Float_ABCD;
	  MonitoringSlotNVM.MonitoringSlot.valueStartIndex = 0;
	  MonitoringSlotNVM.MonitoringSlot.ThresholdActive = 0x0000;

	  VarData_u TH_High, TH_Low, TH_SpikeUp, TH_SpikeDown;

	  TH_High.buff[0] = 0X00;       // 30.00 degrees celsius
	  TH_High.buff[1] = 0X00;
	  TH_High.buff[2] = 0X00;       // 30.00 degrees celsius
	  TH_High.buff[3] = 0X00;

	  TH_Low.buff[0] = 0X00;        // 21.50 degrees celsius
	  TH_Low.buff[1] = 0X00;
	  TH_Low.buff[2] = 0X00;
	  TH_Low.buff[3] = 0X00;

	  TH_SpikeUp.buff[0] = 0X00;    // +3 degree Celcius
	  TH_SpikeUp.buff[1] = 0X00;
	  TH_SpikeUp.buff[2] = 0X00;
	  TH_SpikeUp.buff[3] = 0X00;

	  TH_SpikeDown.buff[0] = 0X00;    // -3 degree Celcius
	  TH_SpikeDown.buff[1] = 0X00;
	  TH_SpikeDown.buff[2] = 0X00;
	  TH_SpikeDown.buff[3] = 0X00;

	  MonitoringSlotNVM.MonitoringSlot.SpikeUp = TH_SpikeUp;
	  MonitoringSlotNVM.MonitoringSlot.SpikeDown = TH_SpikeDown;
	  MonitoringSlotNVM.MonitoringSlot.thresholdHigh = TH_High;
	  MonitoringSlotNVM.MonitoringSlot.thresholdLow = TH_Low;
	  MonitoringSlotNVM.MonitoringSlot.onChange = false;
	  MonitoringSlotNVM.MonitoringSlot.triggerFlagValue = false;
	  MonitoringSlotNVM.MonitoringSlot.isActive = false;


		if (FLASH_IF_Erase((void *)ModbusDeviceFlashAddresses[slotId], FLASH_PAGE_SIZE) == FLASH_IF_OK){
			APP_LOG(TS_OFF, VLEVEL_M, "ERASE OK");
			if(FLASH_IF_Write((void *)ModbusDeviceFlashAddresses[slotId], (void *)&MonitoringSlotNVM, sizeof(struct ModbusDevice)) == FLASH_IF_OK){
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

void viewMonitoringSlotParams(uint8_t slotId){
	struct ModbusDevice MonitoringSlotNVM;

	if (FLASH_IF_Read(&MonitoringSlotNVM, ModbusDeviceFlashAddresses[slotId], sizeof(MonitoringSlotNVM)) == FLASH_IF_OK) {
		APP_LOG( TS_OFF, VLEVEL_M, "MONITORING SLOT %d PARAMETERS: \r\n", slotId);

		showMonitoringSlotParams(MonitoringSlotNVM);

	} else {
	  APP_LOG(TS_OFF, VLEVEL_M, "FAILED READING FLASH \r\n");
	}
}
