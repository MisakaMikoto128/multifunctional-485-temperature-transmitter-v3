/*
 * tmp102.c
 *
 *  Created on: Sep 1, 2021
 *      Author: pawel
 */

#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"
#include "usart.h"
#include "tmp102.h"

// edit register macro
#define TMP102_EDITCONIFG_1BIT(config,value,offset)				(config) &= ~(1 << (offset)); (config) |= ((value) << (offset))
#define TMP102_EDITCONIFG_2BIT(config,value,offset)				(config) &= ~(3 << (offset)); (config) |= ((value) << (offset))

// check if the value defined by user is not to big for register macro
#define TMP102_CHECK_REGISTER_1BIT(value,errorbuffer)			if((value) > 1) { (errorbuffer) = TMP102_ERR_WRONGCONFIG; return (errorbuffer);}
#define TMP102_CHECK_REGISTER_2BIT(value,errorbuffer)			if((value) > 3) { (errorbuffer) = TMP102_ERR_WRONGCONFIG; return (errorbuffer);}

//Convert to 2's complement, since temperature can be negative
#define TMP102_CHECKSIGN_12BIT(value)							if (value > 0x7FF){value |= 0xF000;}
#define TMP102_CHECKSIGN_13BIT(value)							if (value > 0xFFF){value |= 0xE000;}

/*
 * Read 2 bytes from TMP102
 *
 * @param[*tmp102] - TMP102 sensor structure
 * @param[reg] - predefined registers address
 *
 * @return - 16 bit value from register
 */
static uint16_t TMP102_Read16(TMP102_t *tmp102, uint8_t reg)
{

	// buffer for return data
	uint8_t value[2];

	// check if register address is more than 3
	if (reg > TMP102_REG_MAXTEMP)
	{
		tmp102->ErrorCode = TMP102_ERR_WRONGREGISTERDEFINED;
		return 0;
	}

	// address has to be shifted one place left because hal requires left allinged 7bit address
	HAL_I2C_Mem_Read(tmp102->I2CHandle, ((tmp102->DeviceAdress) << 1), reg, 1,
			value, 2, TMP102_I2C_TIMEOUT);

	// write to 16 bits , two 8 bits registers
	// 0000 0000 0000 0000 , first we write value[0] which has 8 significant bits as X << 4 XXXX XXXX
	// 0000 0000 XXXX XXXX // << 4
	// 0000 XXXX XXXX 0000 // then we push value[1] which has only 4 significant bits (YYYY 0000) >> 4
	// 0000 XXXX XXXX YYYY // combined

	// return 16 bit data
	if (reg != TMP102_REG_CONFIG)
	{
		if (tmp102->Configuration.TMP102_EM == 0)
		{
			return (value[0] << 4) | (value[1] >> 4);
		}
		else
		{
			return (value[0] << 5) | (value[1] >> 3);
		}
	}
	else
	{
		// use union structure for config register
		return (value[0]) | (value[1] << 8);
	}
}

/*
 * Write 2 bytes to TMP102
 *
 * @param[*tmp102] - TMP102 sensor structure
 * @param[reg] - predefined registers address
 * @param[value] - value to write
 * @return - 16 bit value from register
 */
static void TMP102_Write16(TMP102_t *tmp102, uint8_t reg, uint16_t value)
{
	// buffer to take 2 bytes
	uint8_t buf[2];

	// check if register address is more than 3
	if (reg > TMP102_REG_MAXTEMP)
	{
		tmp102->ErrorCode = TMP102_ERR_WRONGREGISTERDEFINED;
		return;
	}

	// define bit structure for temp and config
	if (reg != TMP102_REG_CONFIG)
	{
		buf[0] = value >> 4;
		buf[1] = value << 4;
	}
	else
	{
		buf[0] = value;
		buf[1] = value >> 8;
	}

	// write 16 bit data to TMP102
	HAL_I2C_Mem_Write(tmp102->I2CHandle, ((tmp102->DeviceAdress) << 1), reg, 1,
			buf, 2, TMP102_I2C_TIMEOUT);
	tmp102->ErrorCode = TMP102_ERR_NOERROR;
	return;
}

#if (TMP102_USE_FLOATNUMBERS == 1)
/*
 * Calculate temperature and return float value
 *
 * @param[*tmp102] - TMP102 sensor structure
 * @return - temperature calculated from register
 */
float TMP102GetTempFloat(TMP102_t *tmp102)
{
	// define variables
	int16_t val = 0;
	float temp_c = 0;

	// check configuration
	TMP102GetConfiguration(tmp102);

	// read temp data from register
	val = (int16_t) TMP102_Read16(tmp102, TMP102_REG_TEMP);

	// 12 bit mode - normal
	if (tmp102->Configuration.TMP102_EM == 0)
	{
		// check if value is negative
		TMP102_CHECKSIGN_12BIT(val);
	}
	else
	//13 bit mode - extended
	{
		TMP102_CHECKSIGN_13BIT(val);
	}

	// Convert to float temperature value (Celsius)
	temp_c = (float) (val * 0.0625);

	return temp_c;
}
#endif

/*
 * Calculate temperature and return integer/decimal parts
 *
 * @param[*tmp102] - TMP102 sensor structure
 * @param[*value] - array of 2 bytes, [0] integer part , [1] decimal part
 * @return - void
 */
void TMP102GetTempInt(TMP102_t *tmp102, int8_t *value)
{
	// define variables
	int16_t val;
	// read temp data from register
	val = (int16_t) TMP102_Read16(tmp102, TMP102_REG_TEMP);

	// 12 bit mode - normal
	if (tmp102->Configuration.TMP102_EM == 0)
	{
		// check if value is negative
		TMP102_CHECKSIGN_12BIT(val);
	}
	else
	//13 bit mode - extended
	{
		TMP102_CHECKSIGN_13BIT(val);
	}

	// Convert to float temperature value (Celsius)
	value[0] = val * TMP102_RESOLUTION; // // integer part
	value[1] = ((val * 100) * TMP102_RESOLUTION) - (value[0] * 100); // decimal part
}

/*
 * Save configuration in TMP102 struct
 *
 * @param[*tmp102] - TMP102 sensor structure
 * @return - void
 */
void TMP102GetConfiguration(TMP102_t *tmp102)
{
	// read uint16 config register value and convert it to bitfield
	configConverter tempConfig;
	tempConfig.i = TMP102_Read16(tmp102, TMP102_REG_CONFIG);
	tmp102->Configuration = tempConfig.conf;
}

/*
 * Change configuration register of TMP102
 *
 * @param[*tmp102] - TMP102 sensor structure
 * @param[command] - predefined command TMP102_WRITE_XXX @commands
 * @param[value] - predefined register values TMP102_CR_XXX @config
 * @return - status msg
 */
uint8_t TMP102WriteConfig(TMP102_t *tmp102, TMP102writeConfig command,
		uint16_t value)
{
	// read raw config value
	uint16_t config;
	config = TMP102_Read16(tmp102, TMP102_REG_CONFIG);

	// CONTROL REGISTER :
	// MSB [CR1][CR0][AL][EM][0][0][0][0][OS][R1][R0][F1][F0][POL][TM][SD] LSB
	//     [7]  [6]  [5] [4] [3][2][1][0] [7] [6] [5] [4] [3] [2] [1] [0]

	TMP102_CHECK_REGISTER_2BIT(value, tmp102->ErrorCode);
	// select command
	switch (command)
	{

	case TMP102_WRITE_SHUTDOWN:

		TMP102_CHECK_REGISTER_1BIT(value, tmp102->ErrorCode)
		;
		TMP102_EDITCONIFG_1BIT(config, value, TMP102_CR_OFFSET_SD)
		;
		break;

	case TMP102_WRITE_THERMOSTATMODE:

		TMP102_CHECK_REGISTER_1BIT(value, tmp102->ErrorCode)
		;
		TMP102_EDITCONIFG_1BIT(config, value, TMP102_CR_OFFSET_TM)
		;
		break;

	case TMP102_WRITE_POLARITY:

		TMP102_CHECK_REGISTER_1BIT(value, tmp102->ErrorCode)
		;
		TMP102_EDITCONIFG_1BIT(config, value, TMP102_CR_OFFSET_POL)
		;
		break;

	case TMP102_WRITE_FALUTQUEUE:

		TMP102_EDITCONIFG_2BIT(config, value, TMP102_CR_OFFSET_FQ)
		;
		break;

	case TMP102_WRITE_EXTENDEDMODE:

		TMP102_CHECK_REGISTER_1BIT(value, tmp102->ErrorCode)
		;
		TMP102_EDITCONIFG_1BIT(config, value, TMP102_CR_OFFSET_EM)
		;
		break;

	case TMP102_WRITE_CONV_RATE:

		TMP102_EDITCONIFG_2BIT(config, value, TMP102_CR_OFFSET_CR)
		;
		break;
	}

	// write new config to register
	TMP102_Write16(tmp102, TMP102_REG_CONFIG, config);

	// read updated config to structure
	TMP102GetConfiguration(tmp102);
	return tmp102->ErrorCode;
}

/*
 * Read from min and max temp registers and save it to struct
 *
 * @param[*tmp102] - TMP102 sensor structure
 * @return - void
 */
void TMP102GetMinMaxTemp(TMP102_t *tmp102)
{
	// define variables
	int16_t val_max, val_min;

	// read temp data from register
	val_max = (int16_t) TMP102_Read16(tmp102, TMP102_REG_MAXTEMP);
	val_min = (int16_t) TMP102_Read16(tmp102, TMP102_REG_MINTEMP);

	// Convert to 2's complement, since temperature can be negative

	// 12 bit mode - normal
	if (tmp102->Configuration.TMP102_EM == 0)
	{
		TMP102_CHECKSIGN_12BIT(val_min);
		TMP102_CHECKSIGN_12BIT(val_max);
	}
	else
	//13 bit mode - extended
	{
		TMP102_CHECKSIGN_13BIT(val_min);
		TMP102_CHECKSIGN_13BIT(val_max);
	}

#if(TMP102_USE_FLOATNUMBERS == 1)
	// Convert to float temperature value (Celsius)
	tmp102->MaxTemperature = (float) (val_max * TMP102_RESOLUTION);
	tmp102->MinTemperature = (float) (val_min * TMP102_RESOLUTION);
#else
	tmp102->MaxTemperatureIntegerPart = (val_max * TMP102_RESOLUTION);
	tmp102->MaxTemperatureDecimalPart = abs(((val_max - (tmp102->MaxTemperatureIntegerPart / TMP102_RESOLUTION)) * 100) * TMP102_RESOLUTION);
	tmp102->MinTemperatureIntegerPart = (val_min * TMP102_RESOLUTION);
	tmp102->MinTemperatureDecimalPart = abs(((val_min - (tmp102->MinTemperatureIntegerPart / TMP102_RESOLUTION)) * 100) * TMP102_RESOLUTION);
#endif

}

/*
 * Write new maximum and minimum temperature that will set the alarm
 * using floats
 *
 * @param[*tmp102] - TMP102 sensor structure
 * @param[temp] - temperature value to edit
 * @param[MinOrMax] - TMP102_MIN or TMP102_MAX @defines
 * @return - status
 */
#if (TMP102_USE_FLOATNUMBERS == 1)
uint8_t TMP102WriteMinMaxTempFloat(TMP102_t *tmp102, float temp,
		uint8_t MinOrMax)
{

	// value send to function is too high or too low
	// limits : -128 - +128 C
	if (fabs(temp) > 128)
	{
		tmp102->ErrorCode = TMP102_ERR_TEMPOUTOFLIMITS;
		return tmp102->ErrorCode;
	}

	uint16_t reg_value;

	// count from float to digital min temperature
	if (temp > 0)
	{
		reg_value = (uint16_t) (temp / TMP102_RESOLUTION);
	}
	else
	{
		reg_value = (uint16_t) (fabs(temp) / TMP102_RESOLUTION);
		reg_value = ~reg_value;
		reg_value += 1;
	}

	// write to min or max register
	if (MinOrMax == TMP102_MIN)
	{
		TMP102_Write16(tmp102, TMP102_REG_MINTEMP, reg_value);
	}
	else if (MinOrMax == TMP102_MAX)
	{
		TMP102_Write16(tmp102, TMP102_REG_MAXTEMP, reg_value);
	}

	// read values in the registers
	TMP102GetMinMaxTemp(tmp102);

	// compare boundries
	if (tmp102->MinTemperature > tmp102->MaxTemperature)
	{
		tmp102->ErrorCode = TMP102_ERR_WRONGMINMAXVALUES;
		return tmp102->ErrorCode;
	}

	// check if the value written match value in the registers
	// if value returned is not bigger/smaller than 0.0625 (resolution) then operation was successful
	if (MinOrMax == TMP102_MIN)
	{
		if (fabs(tmp102->MinTemperature - temp) > TMP102_RESOLUTION)
		{
			tmp102->ErrorCode = TMP102_ERR_WRITEUNSUCCESSFUL;
			return tmp102->ErrorCode;
		}

	}
	else if (MinOrMax == TMP102_MAX)

	{
		if (fabs(tmp102->MaxTemperature - temp) > TMP102_RESOLUTION)
		{
			tmp102->ErrorCode = TMP102_ERR_WRITEUNSUCCESSFUL;
			return tmp102->ErrorCode;
		}
	}

	tmp102->ErrorCode = TMP102_ERR_NOERROR;
	return tmp102->ErrorCode;

}
#else

/*
 * Write new maximum and minimum temperature that will set the alarm
 * using int numbers
 * i have my doubts about using this function, using floats seems more user-friendly
 *
 * @param[*tmp102] - TMP102 sensor structure
 * @param[IntegerPart] - Integer part of temperature [-128 to 128]
 * @param[DecimalPart] - Decimal part of temperature [0 to 99]
 * @param[MinOrMax] - TMP102_MIN or TMP102_MAX (which boundary we edit) @defines
 * @return - status
 */
uint8_t TMP102WriteMinMaxTempInt(TMP102_t *tmp102, int8_t IntegerPart, uint8_t DecimalPart, uint8_t MinOrMax)
{
	// putting arguments to this function can be confusing, i want user to define if sign is positive/negative in integer part
	// but to make it look more "human friendly" i leave decimal number as always positive
	// so now user can put in the function something like TMP102WriteMinMaxTempInt(tmp102, -20,65 , TEMP102_MIN)
	// which make it look that the value is -20.65 (not like -20,-65 which would be true because decimal part is also negative in this case)


	// lets work on 2 decimal digits
	if (DecimalPart > 99)
		{
		DecimalPart /= 10;
		}


	uint16_t reg_value;

	// calculate user value to TMP102 value
	if (IntegerPart > 0)
	{
		reg_value = (IntegerPart / TMP102_RESOLUTION) + (DecimalPart / (100 * TMP102_RESOLUTION));
	}
	else
	{
		reg_value = (abs(IntegerPart) / TMP102_RESOLUTION) + (DecimalPart / (100 * TMP102_RESOLUTION)) ;
		reg_value = ~reg_value;
		reg_value += 1;
	}

	// write to min or max register
	if (MinOrMax == TMP102_MIN)
	{
		TMP102_Write16(tmp102, TMP102_REG_MINTEMP, reg_value);
	}
	else if (MinOrMax == TMP102_MAX)
	{
		TMP102_Write16(tmp102, TMP102_REG_MAXTEMP, reg_value);
	}

	// read values in the registers to check
	TMP102GetMinMaxTemp(tmp102);


	// compare boundries
		if (tmp102->MinTemperature > tmp102->MaxTemperature)
		{
			tmp102->ErrorCode = TMP102_ERR_WRONGMINMAXVALUES;
			return tmp102->ErrorCode;
		}

	// check if the value written match value in the registers
	if (MinOrMax == TMP102_MIN)
	{
		if (tmp102->MinTemperatureIntegerPart != IntegerPart || abs(tmp102->MinTemperatureDecimalPart - DecimalPart) > (TMP102_RESOLUTION * 100))
		{
			tmp102->ErrorCode = TMP102_ERR_WRITEUNSUCCESSFUL;
			return tmp102->ErrorCode;
		}

	}
	else if (MinOrMax == TMP102_MAX)

	{
		if (tmp102->MaxTemperatureIntegerPart != IntegerPart || abs(tmp102->MaxTemperatureDecimalPart - DecimalPart) > (TMP102_RESOLUTION * 100))
		{
			tmp102->ErrorCode = TMP102_ERR_WRITEUNSUCCESSFUL;
			return tmp102->ErrorCode;
		}
	}

	tmp102->ErrorCode = TMP102_ERR_NOERROR;
	return tmp102->ErrorCode;
}
#endif

/*
 * Assign i2c handler and device address. Read starting configuration and Min/Max temperature from registers and save it in structure.
 * More configuration options defined by user.
 *
 * @param[*tmp102] - TMP102 sensor structure
 * @param[*initI2CHandle] - i2c handler
 * @param[initDeviceAddress] - device address @address
 * @return - void
 */
void TMP102Init(TMP102_t *tmp102, I2C_HandleTypeDef *initI2CHandle,
		uint8_t initDeviceAddress)
{

	// Read basic information
	tmp102->I2CHandle = initI2CHandle;
	tmp102->DeviceAdress = initDeviceAddress;

	// Write new config - defined by user
	TMP102WriteConfig(tmp102, TMP102_WRITE_CONV_RATE, TMP102_CR_CONV_RATE_8Hz);
	TMP102WriteConfig(tmp102, TMP102_WRITE_SHUTDOWN, TMP102_CR_MODE_CONTINUOS);
	TMP102WriteConfig(tmp102, TMP102_WRITE_EXTENDEDMODE,
	TMP102_CR_EXTENDED_ON);

	TMP102GetConfiguration(tmp102);
	TMP102GetMinMaxTemp(tmp102);
}

