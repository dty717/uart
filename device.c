
#include "header/device.h"
#include "header/crc.h"
#include "header/J212.h"
#include "header/common/handler.h"
#include "config.h"
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "header/flash.h"

uint8_t *flashData;

deviceData_t *new_deviceData(uint16_t poolNums, uint16_t pollutionNums, uint8_t MN_len)
{
	deviceData_t *deviceData;
	deviceData = (deviceData_t *)malloc(sizeof(deviceData_t));
	deviceData->pollutionNums = pollutionNums;
	deviceData->poolNums = poolNums;
	deviceData->MN_len = MN_len;
	deviceData->MN = malloc(MN_MAX_LENGTH);
	memset(deviceData->MN, 0, MN_MAX_LENGTH * sizeof(uint8_t));
	deviceData->PW = malloc(PW_LENGTH);
	memset(deviceData->PW, 0, PW_LENGTH * sizeof(uint8_t));
	deviceData->pollutions = (pollution_t *)malloc((deviceData->pollutionNums + remainingPollutionNums) * sizeof(pollution_t));
	memset(deviceData->pollutions, 0, (deviceData->pollutionNums + remainingPollutionNums) * sizeof(pollution_t));
	return deviceData;
}

void setPollutionNums(deviceData_t *deviceData, uint16_t pollutionNums)
{
	deviceData->pollutionNums = pollutionNums;
	free(deviceData->pollutions);
	deviceData->pollutions = NULL;
	deviceData->pollutions = (pollution_t *)malloc((deviceData->pollutionNums + remainingPollutionNums) * sizeof(pollution_t));
	memset(deviceData->pollutions, 0, (deviceData->pollutionNums + remainingPollutionNums) * sizeof(pollution_t));
}

uint8_t checkMN(uint8_t MN_len, uint16_t *current_MN, uint8_t *deviceData_MN)
{
	for (size_t i = 0; i < MN_len - 1; i += 2)
	{
		if ((deviceData_MN[i] != (uint8_t)(current_MN[MNAddr + i / 2] >> 8)) || (deviceData_MN[i + 1] != (uint8_t)(current_MN[MNAddr + i / 2] & 0x00FF)))
		{
			return 0;
		}
	}
	if (MN_len % 2 != 0)
	{
		if (deviceData_MN[MN_len - 1] != (uint8_t)(current_MN[MNAddr + (MN_len - 1) / 2] >> 8))
		{
			return 0;
		}
	}
	return 1;
}

response_type_t ask_all_devices(modbus_t *ctx, deviceData_t **deviceData)
{
	response_type_t needUpdate = ask_device(ctx, deviceData);
	// needUpdate = 1;
	// (*deviceData)->poolNum = 1;
	// printf("needUpdate:%d\r\n",needUpdate);

	return needUpdate;
}

uint16_t poolNum;
uint16_t poolNums;
uint16_t pollutionNums;
uint8_t MN_len;

float value;
uint8_t year;
uint8_t month;
uint8_t date;
uint8_t hour;
uint8_t minute;
uint8_t second;

uint8_t set_led_values(modbus_t *ctx, uint8_t pool, uint16_t valueNums, uint8_t *valueDatas)
{
	size_t i = 0;
	modbus_set_slave(ctx, ledAddr);

	int rc = 0;
	rc = modbus_write_register(ctx, LED_POOL_ADDRESS, pool);
	// if (rc > 0)
	// {
	// 	printf("set register ok");
	// }
	uint16_t *values;
	values = (uint16_t *)malloc(valueNums * sizeof(uint16_t));
	for (i = 0; i < valueNums; i++)
	{
		values[i] = (valueDatas[i * 2] << 8) + valueDatas[i * 2 + 1];
	}
	sleep_ms(500);
	rc = modbus_write_registers(ctx, LED_VALUE_ADDRESS, valueNums, values);
	sleep_ms(200);
	// if (rc > 0)
	// {
	// 	printf("set registers ok");
	// }
	free(values);
	values = NULL;
	return rc;
}

uint8_t set_led_value(modbus_t *ctx, deviceData_t *deviceData)
{
	size_t i = 0;
	modbus_set_slave(ctx, ledAddr);

	int rc = 0;
	rc = modbus_write_register(ctx, LED_POOL_ADDRESS, deviceData->poolNum);
	// if (rc > 0)
	// {
	// 	printf("set register ok");
	// }
	uint16_t *values;
	values = (uint16_t *)malloc(2 * deviceData->pollutionNums * sizeof(uint16_t));
	for (i = 0; i < deviceData->pollutionNums; i++)
	{
		AppendfloatToU16Array(deviceData->pollutions[i].data, values, i * 2);
	}
	rc = modbus_write_registers(ctx, LED_VALUE_ADDRESS, setLedValueNums * 2, values);
	// if (rc > 0)
	// {
	// 	printf("set registers ok");
	// }
	free(values);
	values = NULL;
	return rc;
}

uint8_t set_led_valueByAddr(modbus_t *ctx, uint16_t led_value_address, float *data, uint16_t dataLen)
{
	size_t i = 0;
	modbus_set_slave(ctx, ledAddr);

	int rc = 0;
	// if (rc > 0)
	// {
	// 	printf("set register ok\r\n");
	// }
	uint16_t *values;
	values = (uint16_t *)malloc(2 * dataLen * sizeof(uint16_t));
	for (i = 0; i < dataLen; i++)
	{
		AppendfloatToU16Array(data[i], values, i * 2);
	}
	rc = modbus_write_registers(ctx, led_value_address, dataLen * 2, values);

	// if (rc > 0)
	// {
	// 	printf("set registers ok\r\n");
	// }
	free(values);
	values = NULL;
	return rc;
}

uint8_t *pollutionCode(uint16_t code)
{
#ifdef UART_KUNSHAN
	switch (code)
	{

	case 1:
		return "001";
	case 2:
		return "002";
	case 3:
		return "003";
	case 10:
		return "010";
	case 11:
		return "011";
	case 15:
		return "015";
	case 21:
		return "021";
	case 22:
		return "022";
	case 23:
		return "023";
	case 24:
		return "024";
	case 25:
		return "025";
	case 26:
		return "026";
	case 27:
		return "027";
	case 28:
		return "028";
	case 29:
		return "029";
	case 30:
		return "030";
	case 31:
		return "031";
	case 32:
		return "032";
	case 33:
		return "033";
	case 34:
		return "034";
	case 35:
		return "035";
	case 36:
		return "036";
	case 37:
		return "037";
	case 38:
		return "038";
	case 39:
		return "039";
	case 40:
		return "040";
	case 41:
		return "041";
	case 60:
		return "060";
	case 61:
		return "061";
	case 65:
		return "065";
	case 80:
		return "080";
	case 101:
		return "101";
		// case 99001:
		// 	return "w99001";
		// break;
	default:
		return "xxx";
		// break;
	}
//todo why not work?
	// uint8_t *_pollutionCode;
	// _pollutionCode = (uint8_t *)malloc(3 * sizeof(uint8_t));
	// _pollutionCode[0] = code/100%10;
	// _pollutionCode[1] = code/10%10;
	// _pollutionCode[2] = code%10;
	// return _pollutionCode;
#else
	switch (code)
	{

	case 0:
		return "w00000";
	case 1001:
		return "w01001";
	case 1002:
		return "w01002";
	case 1003:
		return "w01003";
	case 1006:
		return "w01006";
	case 1009:
		return "w01009";
	case 1010:
		return "w01010";
	case 1012:
		return "w01012";
	case 1014:
		return "w01014";
	case 1017:
		return "w01017";
	case 1018:
		return "w01018";
	case 1019:
		return "w01019";
	case 1020:
		return "w01020";
	case 2003:
		return "w02003";
	case 2006:
		return "w02006";
	case 3001:
		return "w03001";
	case 3002:
		return "w03002";
	case 19001:
		return "w19001";
	case 19002:
		return "w19002";
	case 20012:
		return "w20012";
	case 20023:
		return "w20023";
	case 20038:
		return "w20038";
	case 20061:
		return "w20061";
	case 20089:
		return "w20089";
	case 20092:
		return "w20092";
	case 20111:
		return "w20111";
	case 20113:
		return "w20113";
	case 20115:
		return "w20115";
	case 20116:
		return "w20116";
	case 20117:
		return "w20117";
	case 20119:
		return "w20119";
	case 20120:
		return "w20120";
	case 20121:
		return "w20121";
	case 20122:
		return "w20122";
	case 20123:
		return "w20123";
	case 20124:
		return "w20124";
	case 20125:
		return "w20125";
	case 20126:
		return "w20126";
	case 20127:
		return "w20127";
	case 20128:
		return "w20128";
	case 20138:
		return "w20138";
	case 20139:
		return "w20139";
	case 20140:
		return "w20140";
	case 20141:
		return "w20141";
	case 20142:
		return "w20142";
	case 20143:
		return "w20143";
	case 20144:
		return "w20144";
	case 21001:
		return "w21001";
	case 21003:
		return "w21003";
	case 21004:
		return "w21004";
	case 21006:
		return "w21006";
	case 21007:
		return "w21007";
	case 21011:
		return "w21011";
	case 21016:
		return "w21016";
	case 21017:
		return "w21017";
	case 21019:
		return "w21019";
	case 21022:
		return "w21022";
	case 21038:
		return "w21038";
	case 22001:
		return "w22001";
	case 23002:
		return "w23002";
	case 25043:
		return "w25043";
	case 33001:
		return "w33001";
	case 33007:
		return "w33007";
		// case 99001:
		// 	return "w99001";
		break;
	default:
		break;
	}
	return "";
#endif
}

response_type_t ask_common_device(modbus_t *ctx, deviceData_t **deviceData)
{

	int i = 0;
	uint16_t *tab_rp_registers = NULL;

	tab_rp_registers = (uint16_t *)malloc(COMMON_DEVICE_nb_points * sizeof(uint16_t));
	modbus_set_slave(ctx, deviceAddr);
	int rc = 0;
	rc = modbus_read_registers(ctx, COMMON_DEVICE_REGISTERS_ADDRESS,
							   COMMON_DEVICE_nb_points, tab_rp_registers);

	if (rc > 0)
	{
		// print_buf(tab_rp_registers, 64);

		uint8_t b0 = (uint8_t)(tab_rp_registers[commonPollutionDataAddr] >> 8);
		uint8_t b1 = (uint8_t)(tab_rp_registers[commonPollutionDataAddr] & 0x00FF);
		uint8_t b2 = (uint8_t)(tab_rp_registers[commonPollutionDataAddr + 1] >> 8);
		uint8_t b3 = (uint8_t)(tab_rp_registers[commonPollutionDataAddr + 1] & 0x00FF);
		value = bytesToFloat(b0, b1, b2, b3);

		year = hexToInt(tab_rp_registers[commonYearMonthAddr] >> 8);
		month = hexToInt(tab_rp_registers[commonYearMonthAddr] & 0x00FF);
		date = hexToInt(tab_rp_registers[commonDateHourAddr] >> 8);
		hour = hexToInt(tab_rp_registers[commonDateHourAddr] & 0x00FF);
		minute = hexToInt(tab_rp_registers[commonMinuteSecondAddr] >> 8);
		second = hexToInt(tab_rp_registers[commonMinuteSecondAddr] & 0x00FF);

		if (*deviceData == NULL)
		{
			poolNums = 1;
			pollutionNums = 1;
			MN_len = COMMON_MN_LEN;

			poolNum = 1;
			*deviceData = new_deviceData(poolNums, pollutionNums, MN_len);
			(*deviceData)->poolNum = poolNum;
			(*deviceData)->PW = "123456";
			// printf("in %d %d %d\r\n", (*deviceData)->poolNums, (*deviceData)->pollutionNums, (*deviceData)->MN_len);
			goto addNewCommonDate;
		}
		else
		{
			if ((*deviceData)->year != year ||
				(*deviceData)->month != month ||
				(*deviceData)->date != date ||
				(*deviceData)->hour != hour ||
				(*deviceData)->minute != minute ||
				(*deviceData)->second != second)
			{				
				goto checkPollutionNums;
			}
			printf("deviceData poolNum:%d poolNums:%d pollutionNums:%d MN_len:%d MN:%s PW:%s DataTime:%d-%d-%d %d:%d:%d\r\n", (*deviceData)->poolNum, (*deviceData)->poolNums, (*deviceData)->pollutionNums, (*deviceData)->MN_len, (*deviceData)->MN, (*deviceData)->PW,
				   year + 2000,month,date,hour,minute,second);
		}

		for (i = 0; i < pollutionNums + remainingPollutionNums; i++)
		{
			printf("pollution %d: code=%s,data=%f,state=%d\r\n", i + 1, (*deviceData)->pollutions[i].code, (*deviceData)->pollutions[i].data, (*deviceData)->pollutions[i].state);
		}
		free(tab_rp_registers);
		tab_rp_registers = NULL;
		return normal;
	}
	free(tab_rp_registers);
	tab_rp_registers = NULL;
	return noResponse;
checkPollutionNums:
	printf("checkPollutionNums %d\r\n", (*deviceData)->pollutionNums);
	if (pollutionNums != (*deviceData)->pollutionNums)
	{
		setPollutionNums(*deviceData, pollutionNums);
		(*deviceData)->poolNums = poolNums;
	}
addNewCommonDate:
	printf("addNewDate\r\n");
	addNewCommonDate(*deviceData, tab_rp_registers);

	flashData[0] = '1';
	flashData[1] = '2';
	flashData[2] = '3';
	flashData[2 * poolNumsAddr + configAddr] = 0;
	flashData[2 * poolNumsAddr + configAddr + 1] = 1;

	flashData[2 * poolNumAddr + configAddr] = 0;
	flashData[2 * poolNumAddr + configAddr + 1] = 1;

	flashData[2 * pollutionNumsAddr + configAddr] = 0;
	flashData[2 * pollutionNumsAddr + configAddr + 1] = 1;

	flashData[2 * MN_lenAddr + configAddr] = 0;
	flashData[2 * MN_lenAddr + configAddr + 1] = COMMON_MN_LEN & 0xff;

	strcpy(flashData + 2 * MNAddr + configAddr, COMMON_DEVICE_MN);

	flashData[2 * YearMonthAddr + configAddr] = intToHex(year);
	flashData[2 * YearMonthAddr + configAddr + 1] = intToHex(month);

	flashData[2 * DateHourAddr + configAddr] = intToHex(date);
	flashData[2 * DateHourAddr + configAddr + 1] = intToHex(hour);
	
	flashData[2 * MinuteSecondAddr + configAddr] = intToHex(minute);
	flashData[2 * MinuteSecondAddr + configAddr + 1] = intToHex(second);

	flashData[2 * pollutionCodeAddr + configAddr] = COMMON_DEVICE_CODE >> 8;
	flashData[2 * pollutionCodeAddr + configAddr + 1] = COMMON_DEVICE_CODE & 0xff;

	flashData[2 * pollutionDataAddr + configAddr] = (uint8_t)(tab_rp_registers[commonPollutionDataAddr + 1] >> 8);
	flashData[2 * pollutionDataAddr + configAddr + 1] = (uint8_t)(tab_rp_registers[commonPollutionDataAddr + 1] & 0x00FF);
	flashData[2 * pollutionDataAddr + configAddr + 2] = (uint8_t)(tab_rp_registers[commonPollutionDataAddr] >> 8);
	flashData[2 * pollutionDataAddr + configAddr + 3] = (uint8_t)(tab_rp_registers[commonPollutionDataAddr] & 0x00FF);

	flashData[2 * pollutionStateAddr + configAddr] = 0;
	flashData[2 * pollutionStateAddr + configAddr + 1] = 1;

	// for (i = 0; i < COMMON_DEVICE_nb_points; i++)
	// {
	// 	flashData[i * 2 + configAddr] = tab_rp_registers[i] >> 8;
	// 	flashData[i * 2 + configAddr + 1] = tab_rp_registers[i] & 0xff;
	// }

	flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
	flash_range_program(FLASH_TARGET_OFFSET, flashData, FLASH_PAGE_SIZE);
	free(tab_rp_registers);
	tab_rp_registers = NULL;
	return newData;
}

response_type_t ask_device(modbus_t *ctx, deviceData_t **deviceData)
{
	int i = 0;
	uint16_t *tab_rp_registers = NULL;

	tab_rp_registers = (uint16_t *)malloc(nb_points * sizeof(uint16_t));
	modbus_set_slave(ctx, deviceAddr);
	int rc = 0;
	rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
							   nb_points, tab_rp_registers);

	if (rc > 0)
	{
		poolNum = tab_rp_registers[poolNumAddr];
		poolNums = tab_rp_registers[poolNumsAddr];
		pollutionNums = tab_rp_registers[pollutionNumsAddr];
		MN_len = tab_rp_registers[MN_lenAddr];
		if (*deviceData == NULL)
		{
			if (MN_len == 0 || MN_len > 26)
			{
				free(tab_rp_registers);
				tab_rp_registers = NULL;
				return MN_len_erro;
			}
			if (poolNums == 0)
			{
				free(tab_rp_registers);
				tab_rp_registers = NULL;
				return poolNums_erro;
			}
			printf("NULL\r\n");
			*deviceData = new_deviceData(poolNums, pollutionNums, MN_len);
			(*deviceData)->poolNum = poolNum;
			(*deviceData)->PW = "123456";
			// printf("in %d %d %d\r\n", (*deviceData)->poolNums, (*deviceData)->pollutionNums, (*deviceData)->MN_len);
			goto addNewDate;
		}
		else
		{
			if ((*deviceData)->year != hexToInt(tab_rp_registers[YearMonthAddr] >> 8) ||
				(*deviceData)->month != hexToInt(tab_rp_registers[YearMonthAddr] & 0x00FF) ||
				(*deviceData)->date != hexToInt(tab_rp_registers[DateHourAddr] >> 8) ||
				(*deviceData)->hour != hexToInt(tab_rp_registers[DateHourAddr] & 0x00FF) ||
				(*deviceData)->minute != hexToInt(tab_rp_registers[MinuteSecondAddr] >> 8) ||
				(*deviceData)->second != hexToInt(tab_rp_registers[MinuteSecondAddr] & 0x00FF))
			{
				goto checkPollutionNums;
			}
			printf("deviceData poolNum:%d poolNums:%d pollutionNums:%d MN_len:%d MN:%s PW:%s DataTime:%d-%d-%d %d:%d:%d\r\n", (*deviceData)->poolNum, (*deviceData)->poolNums, (*deviceData)->pollutionNums, (*deviceData)->MN_len, (*deviceData)->MN, (*deviceData)->PW,
				   hexToInt(tab_rp_registers[YearMonthAddr] >> 8) + 2000,
				   hexToInt(tab_rp_registers[YearMonthAddr] & 0x00FF),
				   hexToInt(tab_rp_registers[DateHourAddr] >> 8),
				   hexToInt(tab_rp_registers[DateHourAddr] & 0x00FF),
				   hexToInt(tab_rp_registers[MinuteSecondAddr] >> 8),
				   hexToInt(tab_rp_registers[MinuteSecondAddr] & 0x00FF));
		}
		for (i = 0; i < pollutionNums + remainingPollutionNums; i++)
		{
			printf("pollution %d: code=%s,data=%f,state=%d\r\n", i + 1, (*deviceData)->pollutions[i].code, (*deviceData)->pollutions[i].data, (*deviceData)->pollutions[i].state);
		}
		free(tab_rp_registers);
		tab_rp_registers = NULL;
		return normal;
	}
	free(tab_rp_registers);
	tab_rp_registers = NULL;
	return noResponse;
checkPollutionNums:
	printf("checkPollutionNums %d\r\n", (*deviceData)->pollutionNums);
	if (pollutionNums != (*deviceData)->pollutionNums)
	{
		setPollutionNums(*deviceData, pollutionNums);
		(*deviceData)->poolNums = poolNums;
	}
addNewDate:
	printf("addNewDate\r\n");
	addNewDate(*deviceData, tab_rp_registers);

	flashData[0] = '1';
	flashData[1] = '2';
	flashData[2] = '3';
	for (i = 0; i < nb_points; i++)
	{
		flashData[i * 2 + configAddr] = tab_rp_registers[i] >> 8;
		flashData[i * 2 + configAddr + 1] = tab_rp_registers[i] & 0xff;
	}

	flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
	flash_range_program(FLASH_TARGET_OFFSET, flashData, FLASH_PAGE_SIZE);
	free(tab_rp_registers);
	tab_rp_registers = NULL;
	return newData;
}

void addNewCommonDate(deviceData_t *deviceData, uint16_t *tab_rp_registers)
{
	// uint16_t MN_len = 24;
	size_t i;
	deviceData->MN = COMMON_DEVICE_MN;
	deviceData->MN_len = COMMON_MN_LEN;
	deviceData->poolNum = poolNum;

	deviceData->year = hexToInt(tab_rp_registers[commonYearMonthAddr] >> 8);
	deviceData->month = hexToInt(tab_rp_registers[commonYearMonthAddr] & 0x00FF);
	deviceData->date = hexToInt(tab_rp_registers[commonDateHourAddr] >> 8);
	deviceData->hour = hexToInt(tab_rp_registers[commonDateHourAddr] & 0x00FF);
	deviceData->minute = hexToInt(tab_rp_registers[commonMinuteSecondAddr] >> 8);
	deviceData->second = hexToInt(tab_rp_registers[commonMinuteSecondAddr] & 0x00FF);

	for (i = 0; i < pollutionNums; i++)
	{
		deviceData->pollutions[i].code = pollutionCode(COMMON_DEVICE_CODE);
		// printf("%d",sizeof(pollutionCode(tab_rp_registers[pollutionCodeAddr + PollutionDataLen * i])));
		uint8_t b0 = (uint8_t)(tab_rp_registers[commonPollutionDataAddr + PollutionDataLen * i] >> 8);
		uint8_t b1 = (uint8_t)(tab_rp_registers[commonPollutionDataAddr + PollutionDataLen * i] & 0x00FF);
		uint8_t b2 = (uint8_t)(tab_rp_registers[commonPollutionDataAddr + 1 + PollutionDataLen * i] >> 8);
		uint8_t b3 = (uint8_t)(tab_rp_registers[commonPollutionDataAddr + 1 + PollutionDataLen * i] & 0x00FF);
		deviceData->pollutions[i].data = bytesToFloat(b0, b1, b2, b3);
		// printf("data  %.2X  %.2X  %.2X  %.2X\r\n",b0, b1, b2, b3);
		deviceData->pollutions[i].state = 1;
	}
}

void addNewDate(deviceData_t *deviceData, uint16_t *tab_rp_registers)
{
	uint16_t MN_len = tab_rp_registers[MN_lenAddr];
	size_t i;
	for (i = 0; i < MN_len - 1; i += 2)
	{
		deviceData->MN[i] = (uint8_t)(tab_rp_registers[MNAddr + i / 2] >> 8);
		deviceData->MN[i + 1] = (uint8_t)(tab_rp_registers[MNAddr + i / 2] & 0x00FF);
	}
	if (MN_len % 2 != 0)
	{
		deviceData->MN[MN_len - 1] = (uint8_t)(tab_rp_registers[MNAddr + (MN_len - 1) / 2] >> 8);
	}
	deviceData->MN[MN_len] = 0;
	deviceData->MN_len = MN_len;
	deviceData->poolNum = poolNum;

	deviceData->year = hexToInt(tab_rp_registers[YearMonthAddr] >> 8);
	deviceData->month = hexToInt(tab_rp_registers[YearMonthAddr] & 0x00FF);
	deviceData->date = hexToInt(tab_rp_registers[DateHourAddr] >> 8);
	deviceData->hour = hexToInt(tab_rp_registers[DateHourAddr] & 0x00FF);
	deviceData->minute = hexToInt(tab_rp_registers[MinuteSecondAddr] >> 8);
	deviceData->second = hexToInt(tab_rp_registers[MinuteSecondAddr] & 0x00FF);

	pollutionNums = tab_rp_registers[pollutionNumsAddr];
#if defined(UART_SUZHOU) && defined(usingMultiDevice)
	uint16_t shiftHistoryAddr = 4 * (pollutionNums + remainingPollutionNums) * (deviceData->poolNum - 1);
#endif
	for (i = 0; i < pollutionNums; i++)
	{
		deviceData->pollutions[i].code = pollutionCode(tab_rp_registers[pollutionCodeAddr + PollutionDataLen * i]);
		// printf("%d",sizeof(pollutionCode(tab_rp_registers[pollutionCodeAddr + PollutionDataLen * i])));
		uint8_t b0 = (uint8_t)(tab_rp_registers[pollutionDataAddr + PollutionDataLen * i] >> 8);
		uint8_t b1 = (uint8_t)(tab_rp_registers[pollutionDataAddr + PollutionDataLen * i] & 0x00FF);
		uint8_t b2 = (uint8_t)(tab_rp_registers[pollutionDataAddr + 1 + PollutionDataLen * i] >> 8);
		uint8_t b3 = (uint8_t)(tab_rp_registers[pollutionDataAddr + 1 + PollutionDataLen * i] & 0x00FF);
		#ifdef UART_KUNSHAN
			deviceData->pollutions[i].data = bytesToFloat(b2, b3, b0, b1);;
		#else
			deviceData->pollutions[i].data = bytesToFloat(b0, b1, b2, b3);
		#endif
		// printf("data  %.2X  %.2X  %.2X  %.2X\r\n",b0, b1, b2, b3);
		deviceData->pollutions[i].state = tab_rp_registers[pollutionStateAddr + PollutionDataLen * i];
#if defined(UART_SUZHOU) && defined(usingMultiDevice)
		flashData[HistroySaveAddr + shiftHistoryAddr + 4 * i] = b0;
		flashData[HistroySaveAddr + shiftHistoryAddr + 1 + 4 * i] = b1;
		flashData[HistroySaveAddr + shiftHistoryAddr + 2 + 4 * i] = b2;
		flashData[HistroySaveAddr + shiftHistoryAddr + 3 + 4 * i] = b3;
#endif
	}
}