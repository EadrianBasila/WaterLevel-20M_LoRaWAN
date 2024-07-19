/**
 * @file PWX_ModbusMonitoring.c
 * @brief Modbus Monitoring Implementation
 * @date August 22, 2023
 * @version 1.0
 * @author Charles Kim Kabiling
 */

#include "PWX_ModbusMonitoring.h"
#include "project_config.h"
#include <stdio.h>
#include "usart_if.h"
#include "usart.h"

/* Private Variables */
static uint16_t _scanInterval;
static bool _activeSlots[16];

//#define DEBUG_DATA_CONVERSION
/* Modbus Response Handler */
ModBus_t ModbusMonitorResp;

/* Declare Slots */
ModbusMonitorSlot_t MonitoringSlot[16];

/* Private Function Prototypes */
void uint16ToBoolArray(uint16_t value, bool boolArray[16]);
uint16_t bytesToUint16(uint8_t byte1, uint8_t byte2);
void checkThresholdHigh(uint8_t ID, ModbusMonitorSlot_t *slot);
void checkThresholdLow(uint8_t ID, ModbusMonitorSlot_t *slot);
void checkSpikeUp(uint8_t ID, ModbusMonitorSlot_t *slot);
void checkSpikeDown(uint8_t ID, ModbusMonitorSlot_t *slot);
void checkOnChange(uint8_t ID, ModbusMonitorSlot_t *slot);

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}


/**
 * @brief Initializes the Modbus monitor with the provided scan interval and active slots.
 *
 * @param scanInterval The scan interval in milliseconds.
 * @param activeSlots Bitfield representing active slots.
 */
void initModbusMonitor(uint16_t scanInterval, uint16_t activeSlots) {
    _scanInterval = scanInterval;

    // Convert the activeSlots value to a boolean array
    uint16ToBoolArray(activeSlots, _activeSlots);

    // Update active slots for each monitoring slot
    for (uint8_t i = 0; i < 16; i++) {
        MonitoringSlot[i].isActive = _activeSlots[i];
    }
}

/**
 * @brief Initializes a monitoring slot with the provided parameters.
 *
 * @param ID The ID of the monitoring slot (0 to 15).
 * @param modbusCMD Pointer to the Modbus command array.
 * @param dataType The data type of the value in the response.
 * @param valueStartIndex The index of the starting byte of the value in the response.
 * @param order The order configuration (16-bit or 32-bit).
 * @param isLittleEndian Flag indicating if the data is in little-endian format.
 * @param ThresholdActive Bitfield representing active thresholds.
 * @param SpikeUp The upward spike threshold.
 * @param SpikeDown The downward spike threshold.
 * @param thresholdHigh The high threshold value.
 * @param thresholdLow The low threshold value.
 * @param triggerFlagValue The trigger flag value.
 */

void initMonitorSlot(uint8_t ID, uint8_t *modbusCMD, uint8_t cmdSize, uint8_t dataType, uint8_t valueStartIndex, uint16_t ThresholdActive, VarData_u SpikeUp, VarData_u SpikeDown, VarData_u thresholdHigh, VarData_u thresholdLow, bool onChange, bool triggerFlagValue) {
    // Check if ID is within a valid range
    if (ID >= 0 && ID < 16) {
        // Clear the contents of the monitoring slot
        memset(&MonitoringSlot[ID], 0, sizeof(ModbusMonitorSlot_t));

        // Initialize the MonitoringSlot using the provided parameters
        memcpy(MonitoringSlot[ID].modbusCMD, modbusCMD, sizeof(MonitoringSlot[ID].modbusCMD));
        MonitoringSlot[ID].cmdSize = cmdSize;
        MonitoringSlot[ID].dataType = dataType;
        MonitoringSlot[ID].valueStartIndex = valueStartIndex;
        MonitoringSlot[ID].ThresholdActive = ThresholdActive;
        MonitoringSlot[ID].SpikeUp = SpikeUp;
        MonitoringSlot[ID].SpikeDown = SpikeDown;
        MonitoringSlot[ID].thresholdHigh = thresholdHigh;
        MonitoringSlot[ID].thresholdLow = thresholdLow;
        MonitoringSlot[ID].onChange = onChange;
        MonitoringSlot[ID].triggerFlagValue = triggerFlagValue;

    } else {
        // Handle invalid ID (out of range)
        // You can add error handling code here if needed
    }
}

/**
 * @brief Checks a monitoring slot for alarm conditions and prints the modbus response.
 *
 * @param ID The ID of the monitoring slot.
 * @param modbusResponse Pointer to the ModBus response buffer.
 */
void checkSlot(uint8_t ID, ModBus_t *modbusResponse) {

	ID--;

	struct ModbusDevice MonitoringSlotNVM;
	// Transfer Data to current slot
	if (FLASH_IF_Read(&MonitoringSlotNVM, ModbusDeviceFlashAddresses[ID], sizeof(MonitoringSlotNVM)) == FLASH_IF_OK) {

		MonitoringSlot[ID].cmdSize = MonitoringSlotNVM.MonitoringSlot.cmdSize;


		for (int j = 0; j < MonitoringSlot[ID].cmdSize; j++) {
			MonitoringSlot[ID].modbusCMD[j] = MonitoringSlotNVM.MonitoringSlot.modbusCMD[j];
		}

		MonitoringSlot[ID].dataType = MonitoringSlotNVM.MonitoringSlot.dataType;
		MonitoringSlot[ID].valueStartIndex = MonitoringSlotNVM.MonitoringSlot.valueStartIndex;
		MonitoringSlot[ID].ThresholdActive = MonitoringSlotNVM.MonitoringSlot.ThresholdActive;
		MonitoringSlot[ID].SpikeUp = MonitoringSlotNVM.MonitoringSlot.SpikeUp;
//		MonitoringSlot[ID].SpikeUp[1] = MonitoringSlotNVM.MonitoringSlot.SpikeUp[1];
//		MonitoringSlot[ID].SpikeUp[2] = MonitoringSlotNVM.MonitoringSlot.SpikeUp[2];
//		MonitoringSlot[ID].SpikeUp[3] = MonitoringSlotNVM.MonitoringSlot.SpikeUp[3];
		MonitoringSlot[ID].SpikeDown = MonitoringSlotNVM.MonitoringSlot.SpikeDown;
//		MonitoringSlot[ID].SpikeDown[1] = MonitoringSlotNVM.MonitoringSlot.SpikeDown[1];
//		MonitoringSlot[ID].SpikeDown[2] = MonitoringSlotNVM.MonitoringSlot.SpikeDown[2];
//		MonitoringSlot[ID].SpikeDown[3] = MonitoringSlotNVM.MonitoringSlot.SpikeDown[3];
		MonitoringSlot[ID].thresholdHigh = MonitoringSlotNVM.MonitoringSlot.thresholdHigh;
//		MonitoringSlot[ID].thresholdHigh[1] = MonitoringSlotNVM.MonitoringSlot.thresholdHigh[1];
//		MonitoringSlot[ID].thresholdHigh[2] = MonitoringSlotNVM.MonitoringSlot.thresholdHigh[2];
//		MonitoringSlot[ID].thresholdHigh[3] = MonitoringSlotNVM.MonitoringSlot.thresholdHigh[3];
		MonitoringSlot[ID].thresholdLow = MonitoringSlotNVM.MonitoringSlot.thresholdLow;
//		MonitoringSlot[ID].thresholdLow[1] = MonitoringSlotNVM.MonitoringSlot.thresholdLow[1];
//		MonitoringSlot[ID].thresholdLow[2] = MonitoringSlotNVM.MonitoringSlot.thresholdLow[2];
//		MonitoringSlot[ID].thresholdLow[3] = MonitoringSlotNVM.MonitoringSlot.thresholdLow[3];
		MonitoringSlot[ID].onChange = MonitoringSlotNVM.MonitoringSlot.onChange;
		MonitoringSlot[ID].triggerFlagValue = MonitoringSlotNVM.MonitoringSlot.triggerFlagValue;

		  // Initialize Serial
		  huart1.Instance = USART1;
		  huart1.Init.WordLength = UART_WORDLENGTH_8B;

		  huart1.Init.BaudRate = ModbusBaudRates[MonitoringSlotNVM.Baudrate];
		  huart1.Init.StopBits = StopBitSettings[MonitoringSlotNVM.StopBits];
		  huart1.Init.Parity = ParitySettings[MonitoringSlotNVM.StopBits];

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


		  HAL_Delay(100);

		  HAL_UART_Receive_IT(&huart1, (uint8_t *)ModbusResp.buffer, 1);



	} else {
	  APP_LOG(TS_OFF, VLEVEL_M, "FAILED READING FLASH \r\n");
	}

	if(MonitoringSlotNVM.MonitoringSlot.isActive == 1 && MonitoringSlotNVM.DeviceActive == 1){

		APP_LOG( TS_OFF, VLEVEL_M, " - - - - - - - Scanning Slot %d  - - - - - - - \r\n", ID + 1);
		APP_LOG( TS_OFF, VLEVEL_M, "MODBUS CMD (Hex):  ");
		for (int i = 0; i < MonitoringSlot[ID].cmdSize; i++) {
			APP_LOG( TS_OFF, VLEVEL_M, "%02X ", MonitoringSlot[ID].modbusCMD[i]);
		}
		APP_LOG( TS_OFF, VLEVEL_M, " \r\n");
		// Send the modbus command and receive the response
		sendRaw(MonitoringSlot[ID].modbusCMD, MonitoringSlot[ID].cmdSize, modbusResponse);

		// Delay to allow response to be received
		HAL_Delay(1000);

		APP_LOG( TS_OFF, VLEVEL_M, "MODBUS RESPONSE (Hex): ");

		for (int i = 0; i < modbusResponse->rxIndex; i++) {
			APP_LOG( TS_OFF, VLEVEL_M, "%02X ", modbusResponse->buffer[i]);
		}
		APP_LOG( TS_OFF, VLEVEL_M, " \r\n");

		// Clear the value before updating
		memset(&MonitoringSlot[ID].value, 0, sizeof(VarData_u));

		if(modbusResponse->rxIndex > 1){  // 2 = minimum number of bytes to be received
			// Convert bytes to data based on dataType and print sensor value
			MonitoringSlot[ID].value = convertBytesToData(&modbusResponse->buffer[MonitoringSlot[ID].valueStartIndex], MonitoringSlot[ID].dataType);
			//APP_LOG( TS_OFF, VLEVEL_M, "Converted Value: %d \r\n", MonitoringSlot[ID].value.u16);

			// Check each threshold condition based on ThresholdActive flags
			if (MonitoringSlot[ID].ThresholdActive & (1 << Offset_ThresholdHigh)) {
				checkThresholdHigh(ID, &MonitoringSlot[ID]);
			}
			if (MonitoringSlot[ID].ThresholdActive & (1 << Offset_ThresholdLow)) {
				checkThresholdLow(ID, &MonitoringSlot[ID]);
			}
			if (MonitoringSlot[ID].ThresholdActive & (1 << Offset_SpikeUp)) {
				checkSpikeUp(ID, &MonitoringSlot[ID]);
			}
			if (MonitoringSlot[ID].ThresholdActive & (1 << Offset_SpikeDown)) {
				checkSpikeDown(ID, &MonitoringSlot[ID]);
			}
			if (MonitoringSlot[ID].onChange == true) {
				checkOnChange(ID, &MonitoringSlot[ID]);
			}

			MonitoringSlot[ID].prevValue = MonitoringSlot[ID].value;
		}else{
			APP_LOG( TS_OFF, VLEVEL_M, "ERROR: NO RESPONSE");
		}
	}


}

/**
 * @brief Converts a uint16 value to a boolean array.
 *
 * @param value The uint16 value to convert.
 * @param boolArray The boolean array to store the converted bits.
 */
void uint16ToBoolArray(uint16_t value, bool boolArray[16]) {
    for (int i = 0; i < 16; ++i) {
        boolArray[i] = (value >> i) & 0x01;
    }
}

/**
 * @brief Converts two bytes to a uint16 value.
 *
 * @param byte1 The first byte.
 * @param byte2 The second byte.
 * @return The uint16 value.
 */
uint16_t bytesToUint16(uint8_t byte1, uint8_t byte2) {
    uint16_t result = (uint16_t)byte1 << 8 | byte2;
    return result;
}

/**
* @brief Converts two bytes to a uint16 value.
*
* @param byte1 The first byte.
* @param byte2 The second byte.
* @return The uint16 value.
*/
int16_t bytesToInt16(uint8_t byte1, uint8_t byte2) {
    int16_t result = (int16_t)((int16_t)byte1 << 8) | byte2;
    return result;
}

uint32_t bytesToUint32(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4) {
    uint32_t result = ((uint32_t)byte1 << 24) | ((uint32_t)byte2 << 16) | ((uint32_t)byte3 << 8) | byte4;
    return result;
}

int32_t bytesToInt32(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4) {
    int32_t result = ((int32_t)byte1 << 24) | ((int32_t)byte2 << 16) | ((int32_t)byte3 << 8) | byte4;
    return result;
}

float bytesToFloat(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4) {
    union {
        uint32_t u32;
        float f;
    } data;
    data.u32 = (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
    return data.f;
}

uint8_t bytesToUint8(uint8_t byte1) {
    return byte1;
}

int8_t bytesToInt8(uint8_t byte1) {
    // Since int8_t can represent both positive and negative values
    // directly from 0 to 255, we can simply return the byte value as int8_t.
    return (int8_t)byte1;
}

void hexToByteArray(uint32_t value, uint8_t *byteArray) {
    byteArray[0] = (value >> 24) & 0xFF;
    byteArray[1] = (value >> 16) & 0xFF;
    byteArray[2] = (value >> 8) & 0xFF;
    byteArray[3] = value & 0xFF;
}

void printTHHighAlarmMessage(){
	APP_LOG( TS_OFF, VLEVEL_M, " #### ALARM HIGH DETECTED ##### \r\n");
}void printTHLowAlarmMessage(){
	APP_LOG( TS_OFF, VLEVEL_M, " #### ALARM LOW DETECTED ##### \r\n");
}
void printSpikeUpAlarmMessage(){
	APP_LOG( TS_OFF, VLEVEL_M, "#### SPIKE UP ALARM DETECTED ####  \r\n");
}
void printSpikeDownAlarmMessage(){
	APP_LOG( TS_OFF, VLEVEL_M, "#### SPIKE DOWN ALARM DETECTED ####  \r\n");
}

void printOnChangeAlarmMessage(){
	APP_LOG( TS_OFF, VLEVEL_M, "#### ON CHANGE ALARM DETECTED ####  \r\n");
}
/**
 * @brief Converts raw bytes to various data types based on configuration.
 *
 * @param bytes Raw bytes received from the Modbus response.
 * @param length Number of bytes to process.
 * @param dataType The data type to convert to (uint16, uint32, float).
 * @param order The order configuration (16-bit or 32-bit).
 * @return Converted data value.
 */

VarData_u convertBytesToData(uint8_t *bytes, uint8_t dataType) {
    VarData_u result;

    switch (dataType) {
        case Modbus_uInt16_AB:
            result.u16 = bytesToUint16(bytes[0], bytes[1]);
#ifdef DEBUG_DATA_CONVERSION
            printf("uInt16_AB: %d \r\n", result.u16);
#endif
            break;
        case Modbus_uInt16_BA:
		   result.u16 = bytesToUint16(bytes[1], bytes[0]);
#ifdef DEBUG_DATA_CONVERSION
		   printf("uInt16_BA: %d \r\n", result.u16);
#endif
		   break;
        case Modbus_Int16_AB:
		   result.i16 = bytesToInt16(bytes[0], bytes[1]);
#ifdef DEBUG_DATA_CONVERSION
		   printf("Int16_AB: %d \r\n", result.i16);
#endif
		   break;
        case Modbus_Int16_BA:
		   result.i16 = bytesToInt16(bytes[1], bytes[0]);
#ifdef DEBUG_DATA_CONVERSION
		   printf("Int16_BA: %d \r\n", result.i16);
#endif
		   break;
        case Modbus_uInt32_ABCD:
		   result.u32 = bytesToUint32(bytes[0], bytes[1], bytes[2], bytes[3]);
#ifdef DEBUG_DATA_CONVERSION
		   printf("uInt32_ABCD: %lu \r\n", result.u32);
#endif
		   break;
        case Modbus_uInt32_DCBA:
		   result.u32 = bytesToUint32(bytes[3], bytes[2], bytes[1], bytes[0]);
#ifdef DEBUG_DATA_CONVERSION
		   printf("uInt32_ABCD: %lu \r\n", result.u32);
#endif
		   break;
        case Modbus_uInt32_BADC:
		   result.u32 = bytesToUint32(bytes[1], bytes[0], bytes[3], bytes[2]);
#ifdef DEBUG_DATA_CONVERSION
		   printf("uInt32_ABCD: %lu \r\n", result.u32);
#endif
		   break;
        case Modbus_uInt32_CDAB:
		   result.u32 = bytesToUint32(bytes[2], bytes[3], bytes[0], bytes[1]);
#ifdef DEBUG_DATA_CONVERSION
		   printf("uInt32_ABCD: %lu \r\n", result.u32);
#endif
		   break;
        case Modbus_Int32_ABCD:
		   result.i32 = bytesToInt32(bytes[0], bytes[1], bytes[2], bytes[3]);
#ifdef DEBUG_DATA_CONVERSION
		   printf("Int32_ABCD: %ld \r\n", result.i32);
#endif
		   break;
        case Modbus_Int32_DCBA:
		   result.i32 = bytesToInt32(bytes[3], bytes[2], bytes[1], bytes[0]);
#ifdef DEBUG_DATA_CONVERSION
		   printf("Int32_ABCD: %ld \r\n", result.i32);
#endif
		   break;
		case Modbus_Int32_BADC:
		   result.i32 = bytesToInt32(bytes[1], bytes[0], bytes[3], bytes[2]);
#ifdef DEBUG_DATA_CONVERSION
		   printf("Int32_ABCD: %ld \r\n", result.i32);
#endif
		   break;
		case Modbus_Int32_CDAB:
		   result.i32 = bytesToInt32(bytes[2], bytes[3], bytes[0], bytes[1]);
#ifdef DEBUG_DATA_CONVERSION
		   printf("Int32_ABCD: %ld \r\n", result.i32);
#endif
		   break;
		case Modbus_Float_ABCD:
		   result.f = bytesToFloat(bytes[0], bytes[1], bytes[2], bytes[3]);
#ifdef DEBUG_DATA_CONVERSION
		   printf("Float_ABCD: %02f \r\n", result.f);
#endif
		   break;
		case Modbus_Float_DCBA:
		   result.f = bytesToFloat(bytes[3], bytes[2], bytes[1], bytes[0]);
#ifdef DEBUG_DATA_CONVERSION
		   printf("Float_DCBA: %02f \r\n", result.f);
#endif
		   break;
		case Modbus_Float_BADC:
		   result.f = bytesToFloat(bytes[1], bytes[0], bytes[3], bytes[2]);
#ifdef DEBUG_DATA_CONVERSION
		   printf("Float_BADC: %02f \r\n", result.f);
#endif
		   break;
		case Modbus_Float_CDAB:
		   result.f = bytesToFloat(bytes[2], bytes[3], bytes[0], bytes[1]);
#ifdef DEBUG_DATA_CONVERSION
		   printf("Float_CDAB: %02f \r\n", result.f);
#endif
		   break;
		case Modbus_uInt8:
		   result.u8 = bytesToUint8(bytes[0]);
#ifdef DEBUG_DATA_CONVERSION
		   printf("uInt8: %u \r\n", result.u8);
#endif
		   break;
		case Modbus_Int8:
		   result.i8 = bytesToInt8(bytes[0]);
#ifdef DEBUG_DATA_CONVERSION
		   printf("Int8: %d \r\n", result.i8);
#endif
		   break;
        default:
            // Handle unsupported data type
            break;
    }

    return result;
}


void checkThresholdHigh(uint8_t ID, ModbusMonitorSlot_t *slot){



	 VarData_u tempThreshold  = convertBytesToData(slot->thresholdHigh.buff, slot->dataType);
	 //printf("Threshold High %d \r\n", tempThreshold.u16);

	 switch (slot->dataType) {
	 	case Modbus_Float_ABCD:
	 	case Modbus_Float_DCBA:
	 	case Modbus_Float_BADC:
		case Modbus_Float_CDAB:
			if (slot->value.f > tempThreshold.f) {
				printTHHighAlarmMessage();
			}
			break;
		case Modbus_uInt32_ABCD:
		case Modbus_uInt32_DCBA:
		case Modbus_uInt32_BADC:
		case Modbus_uInt32_CDAB:
			if (slot->value.u32 > tempThreshold.u32) {
				printTHHighAlarmMessage();
			}
			break;
		case Modbus_Int32_ABCD:
		case Modbus_Int32_DCBA:
		case Modbus_Int32_BADC:
		case Modbus_Int32_CDAB:
			if (slot->value.i32 > tempThreshold.i32) {
				printTHHighAlarmMessage();
			}
			break;
		case Modbus_Int16_AB:
		case Modbus_Int16_BA:
			if (slot->value.i16 > tempThreshold.i16) {
				printTHHighAlarmMessage();
			}
			break;
		case Modbus_uInt16_AB:
		case Modbus_uInt16_BA:
			if (slot->value.u16 > tempThreshold.u16) {
				printTHHighAlarmMessage();
			}
			break;
		case Modbus_Int8:
			if (slot->value.i8 > tempThreshold.i8) {
				printTHHighAlarmMessage();
			}
			break;
		case Modbus_uInt8:
			if (slot->value.u8 > tempThreshold.u8) {
				printTHHighAlarmMessage();
			}
			break;

		default:
			// Handle unsupported data type
			break;
	}


}



void checkThresholdLow(uint8_t ID, ModbusMonitorSlot_t *slot){


	 VarData_u tempThreshold  = convertBytesToData(slot->thresholdLow.buff, slot->dataType);
	 //printf("Threshold Low %d \r\n", tempThreshold.u16);

	 switch (slot->dataType) {
	 	case Modbus_Float_ABCD:
	 	case Modbus_Float_DCBA:
	 	case Modbus_Float_BADC:
		case Modbus_Float_CDAB:
			if (slot->value.f < tempThreshold.f) {
				printTHLowAlarmMessage();
			}
			break;
		case Modbus_uInt32_ABCD:
		case Modbus_uInt32_DCBA:
		case Modbus_uInt32_BADC:
		case Modbus_uInt32_CDAB:
			if (slot->value.u32 < tempThreshold.u32) {
				printTHLowAlarmMessage();
			}
			break;
		case Modbus_Int32_ABCD:
		case Modbus_Int32_DCBA:
		case Modbus_Int32_BADC:
		case Modbus_Int32_CDAB:
			if (slot->value.i32 < tempThreshold.i32) {
				printTHLowAlarmMessage();
			}
			break;
		case Modbus_Int16_AB:
		case Modbus_Int16_BA:
			if (slot->value.i16 < tempThreshold.i16) {
				printTHLowAlarmMessage();
			}
			break;
		case Modbus_uInt16_AB:
		case Modbus_uInt16_BA:
			if (slot->value.u16 < tempThreshold.u16) {
				printTHLowAlarmMessage();
			}
			break;
		case Modbus_Int8:
			if (slot->value.i8 < tempThreshold.i8) {
				printTHLowAlarmMessage();
			}
			break;
		case Modbus_uInt8:
			if (slot->value.u8 < tempThreshold.u8) {
				printTHLowAlarmMessage();
			}
			break;

		default:
			// Handle unsupported data type
			break;
	}


}



void checkSpikeUp(uint8_t ID, ModbusMonitorSlot_t *slot){


	 VarData_u tempThreshold  = convertBytesToData(slot->SpikeUp.buff, slot->dataType);
	 //printf("Threshold Spike Up %d \r\n", tempThreshold.u16);

	 switch (slot->dataType) {
	 	case Modbus_Float_ABCD:
	 	case Modbus_Float_DCBA:
	 	case Modbus_Float_BADC:
		case Modbus_Float_CDAB:
			if ((slot->value.f - slot->prevValue.f) > tempThreshold.f) {
				printSpikeUpAlarmMessage();
			}
			break;
		case Modbus_uInt32_ABCD:
		case Modbus_uInt32_DCBA:
		case Modbus_uInt32_BADC:
		case Modbus_uInt32_CDAB:
			if ((slot->value.u32 - slot->prevValue.u32) > tempThreshold.u32) {
				printSpikeUpAlarmMessage();
			}
			break;
		case Modbus_Int32_ABCD:
		case Modbus_Int32_DCBA:
		case Modbus_Int32_BADC:
		case Modbus_Int32_CDAB:
			if ((slot->value.i32 - slot->prevValue.i32) > tempThreshold.i32) {
				printSpikeUpAlarmMessage();
			}
			break;
		case Modbus_Int16_AB:
		case Modbus_Int16_BA:
			if ((slot->value.i16 - slot->prevValue.i16) > tempThreshold.i16) {
				printSpikeUpAlarmMessage();
			}
			break;
		case Modbus_uInt16_AB:
		case Modbus_uInt16_BA:
			if ((slot->value.u16 - slot->prevValue.u16) > tempThreshold.u16) {
				printSpikeUpAlarmMessage();
			}
			break;
		case Modbus_Int8:
			if ((slot->value.i8 - slot->prevValue.i8) > tempThreshold.i8) {
				printSpikeUpAlarmMessage();
			}
			break;
		case Modbus_uInt8:
			if ((slot->value.u8 - slot->prevValue.u8) > tempThreshold.u8) {
				printSpikeUpAlarmMessage();
			}
			break;

		default:
			// Handle unsupported data type
			break;
	}


}


void checkSpikeDown(uint8_t ID, ModbusMonitorSlot_t *slot){


	 VarData_u tempThreshold  = convertBytesToData(slot->SpikeDown.buff, slot->dataType);
	 //printf("Threshold Down Up %d \r\n", tempThreshold.u16);

	 switch (slot->dataType) {
	 	case Modbus_Float_ABCD:
	 	case Modbus_Float_DCBA:
	 	case Modbus_Float_BADC:
		case Modbus_Float_CDAB:
			if ((slot->value.f - slot->prevValue.f) < tempThreshold.f) {
				printSpikeDownAlarmMessage();
			}
			break;
		case Modbus_uInt32_ABCD:
		case Modbus_uInt32_DCBA:
		case Modbus_uInt32_BADC:
		case Modbus_uInt32_CDAB:
			if ((slot->prevValue.u32 - slot->value.u32) > tempThreshold.u32) {
				printSpikeDownAlarmMessage();
			}
			break;
		case Modbus_Int32_ABCD:
		case Modbus_Int32_DCBA:
		case Modbus_Int32_BADC:
		case Modbus_Int32_CDAB:
			if ((slot->prevValue.i32 - slot->value.i32) > tempThreshold.i32) {
				printSpikeDownAlarmMessage();
			}
			break;
		case Modbus_Int16_AB:
		case Modbus_Int16_BA:
			if ((slot->prevValue.i16 - slot->value.i16) > tempThreshold.i16) {
				printSpikeDownAlarmMessage();
			}
			break;
		case Modbus_uInt16_AB:
		case Modbus_uInt16_BA:
			if ((slot->prevValue.u16 - slot->value.u16) < tempThreshold.u16) {
				printSpikeDownAlarmMessage();
			}
			break;
		case Modbus_Int8:
			if ((slot->prevValue.i8 - slot->value.i8)   > tempThreshold.i8) {
				printSpikeDownAlarmMessage();
			}
			break;
		case Modbus_uInt8:
			if ((slot->prevValue.u8 - slot->value.u8)   > tempThreshold.u8) {
				printSpikeDownAlarmMessage();
			}
			break;

		default:
			// Handle unsupported data type
			break;
	}


}

void checkOnChange(uint8_t ID, ModbusMonitorSlot_t *slot){
	switch (slot->dataType) {
	case Modbus_Float_ABCD:
	case Modbus_Float_DCBA:
	case Modbus_Float_BADC:
	case Modbus_Float_CDAB:
		if (slot->prevValue.f != slot->value.f) {
			printOnChangeAlarmMessage();
		}
		break;
	case Modbus_uInt32_ABCD:
	case Modbus_uInt32_DCBA:
	case Modbus_uInt32_BADC:
	case Modbus_uInt32_CDAB:
		if (slot->prevValue.u32 != slot->value.u32) {
			printOnChangeAlarmMessage();
		}
		break;
	case Modbus_Int32_ABCD:
	case Modbus_Int32_DCBA:
	case Modbus_Int32_BADC:
	case Modbus_Int32_CDAB:
		if (slot->prevValue.i32 - slot->value.i32) {
			printOnChangeAlarmMessage();
		}
		break;
	case Modbus_Int16_AB:
	case Modbus_Int16_BA:
		if (slot->prevValue.i16 - slot->value.i16) {
			printOnChangeAlarmMessage();
		}
		break;
	case Modbus_uInt16_AB:
	case Modbus_uInt16_BA:
		if (slot->prevValue.u16 - slot->value.u16) {
			printOnChangeAlarmMessage();
		}
		break;
	case Modbus_Int8:
		if (slot->prevValue.i8 - slot->value.i8) {
			printOnChangeAlarmMessage();
		}
		break;
	case Modbus_uInt8:
		if (slot->prevValue.u8 - slot->value.u8) {
			printOnChangeAlarmMessage();
		}
		break;

	default:
		// Handle unsupported data type
		break;
	}
}

