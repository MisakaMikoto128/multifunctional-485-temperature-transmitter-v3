/*
 * tmp102.h
 *
 *  Created on: Sep 1, 2021
 *      Author: pawel
 */

#ifndef INC_TMP102_H_
#define INC_TMP102_H_

// Defined by user if float operations are enabled
// Working with float is recommended
#define TMP102_USE_FLOATNUMBERS				1

/*
 * TMP102 specific defines @defines
 */
#define TMP102_RESOLUTION					0.0625
#define TMP102_MIN					0
#define TMP102_MAX					1
#define TMP102_I2C_TIMEOUT			1000

/*
 * TMP102 error @error
 */
#define TMP102_ERR_NOERROR 					0
#define TMP102_ERR_WRITEUNSUCCESSFUL		1
#define TMP102_ERR_WRONGCONFIG				2
#define TMP102_ERR_WRONGREGISTERDEFINED		3
#define TMP102_ERR_WRONGMINMAXVALUES		4
#define TMP102_ERR_TEMPOUTOFLIMITS			5

/*
 * TMP102 adresses @address
 */
#define TMP102_ADDRESS 				0x48
#define TMP102_ADRESS_ALT

/*
 * TMP102 registers @registers
 */
#define TMP102_REG_TEMP				0x00
#define TMP102_REG_CONFIG			0x01
#define TMP102_REG_MINTEMP			0x02
#define TMP102_REG_MAXTEMP			0x03

/*
 * Configurable register values @config
 */

/* @Modes
 * The Shutdown-mode bit saves maximum power by shutting down all device circuitry other than the serial
 * interface, reducing current consumption to typically less than 0.5 μA. Shutdown mode enables when the SD bit is
 * 1; the device shuts down when current conversion is completed. When SD is equal to 0, the device maintains a
 * continuous conversion state
 */
#define TMP102_CR_MODE_CONTINUOS	0
#define TMP102_CR_MODE_SHUTDOWN		1

/* @Conversion rate
 * The conversion rate bits, CR1 and CR0, configure the TMP102 device for conversion rates of 0.25 Hz, 1 Hz, 4
 * Hz, or 8 Hz. The default rate is 4 Hz.
 */
#define TMP102_CR_CONV_RATE_025Hz	0
#define TMP102_CR_CONV_RATE_1Hz		1
#define TMP102_CR_CONV_RATE_4Hz		2
#define TMP102_CR_CONV_RATE_8Hz		3

/* @Fault queue
 * A fault condition exists when the measured temperature exceeds the user-defined limits set in the THIGH and
 * TLOW registers. Additionally, the number of fault conditions required to generate an alert may be programmed
 * using the fault queue. The fault queue is provided to prevent a false alert as a result of environmental noise. The
 * fault queue requires consecutive fault measurements in order to trigger the alert function
 */
#define TMP102_CR_FALUTQUEUE_1F		0
#define TMP102_CR_FALUTQUEUE_2F		1
#define TMP102_CR_FALUTQUEUE_4F		2
#define TMP102_CR_FALUTQUEUE_6F		3

/* @One-shot
 * When the device is in Shutdown Mode, writing a 1 to the OS bit starts a single temperature conversion. During
 * the conversion, the OS bit reads '0'. The device returns to the shutdown state at the completion of the single
 * conversion.
 */
#define TMP102_CR_ONESHOT			1

/* @Extended Mode
 * The Extended-Mode bit configures the device for Normal Mode operation (EM = 0) or Extended Mode operation
 * (EM = 1). In normal mode, the temperature register, high-limit register, and low-limit register use a 12-bit data
 * format.
 */
#define TMP102_CR_EXTENDED_ON		1
#define TMP102_CR_EXTENDED_OFF		0

/* @Polarity
 * The polarity bit allows the user to adjust the polarity of the ALERT pin output. If the POL bit is set to 0 (default),
 * the ALERT pin becomes active low. When the POL bit is set to 1, the ALERT pin becomes active high and the
 * state of the ALERT pin is inverted.
 */
#define TMP102_CR_POLARITY_LOW		0
#define TMP102_CR_POLARITY_HIGH		1
/* @Thermostat mode
 * The Thermostat mode bit indicates to the device whether to operate in Comparator mode (TM = 0) or Interrupt
 * mode (TM = 1).
 */
#define TMP102_CR_THERMOSTAT_DEFAULT 	0
#define TMP102_CR_THERMOSTAT_IT			1

/* @Resolution
 * The converter resolution bits, R1 and R0, are read-only bits. The TMP102 converter resolution is set at device
 * start-up to 11 which sets the temperature register to a 12 bit-resolution.
 */

/* @Alert
 * The AL bit is a read-only function. Reading the AL bit provides information about the comparator mode status.
 * The state of the POL bit inverts the polarity of data returned from the AL bit. When the POL bit equals 0, the AL
 * bit reads as 1 until the temperature equals or exceeds T(HIGH) for the programmed number of consecutive faults,
 * causing the AL bit to read as 0. The AL bit continues to read as 0 until the temperature falls below T(LOW) for the
 * programmed number of consecutive faults, when it again reads as 1. The status of the TM bit does not affect the
 * status of the AL bit..

 */


/*
 * Register offset @offset
 */
#define TMP102_CR_OFFSET_SD			0
#define TMP102_CR_OFFSET_TM			1
#define TMP102_CR_OFFSET_POL		2
#define TMP102_CR_OFFSET_FQ			3
#define TMP102_CR_OFFSET_R			5
#define TMP102_CR_OFFSET_OS			7
#define TMP102_CR_OFFSET_EM			12
#define TMP102_CR_OFFSET_AL			13
#define TMP102_CR_OFFSET_CR			14

/*
 * Configuration register structure
 */
typedef struct
{

	// STRUCTURE :
	// MSB [CR1][CR0][AL][EM][0][0][0][0][OS][R1][R0][F1][F0][POL][TM][SD] LSB
	//     [7]  [6]  [5] [4] [3][2][1][0] [7] [6] [5] [4] [3] [2] [1] [0]

	uint16_t TMP102_SD:1; 	// @Modes											// R/W
	uint16_t TMP102_TM:1;	// @Thermostat mode									// R/W
	uint16_t TMP102_POL:1;	// @Polarity										// R/W
	uint16_t TMP102_FQ:2;	// @Fault queue										// R/W
	uint16_t TMP102_R:2;	// @Resolution										// R
	uint16_t TMP102_OS:1;	// @One-shot										// R/W
	uint16_t TMP102_NOTUSED4:4;
	uint16_t TMP102_EM:1; 	// @Extended Mode									// R/W
	uint16_t TMP102_AL:1;	// @Alert											// R
	uint16_t TMP102_CR:2; 	// @Conversion rate									// R/W
}TMP102config_t;

/*
 * Union to make uint16 -> bitfield conversion
 */
typedef union
{
	TMP102config_t conf;
	uint16_t i;
}configConverter;

/*
 * Write commands @commands
 */
typedef enum
{
	TMP102_WRITE_SHUTDOWN = 0,
	TMP102_WRITE_THERMOSTATMODE,
	TMP102_WRITE_POLARITY,
	TMP102_WRITE_FALUTQUEUE,
	TMP102_WRITE_EXTENDEDMODE,
	TMP102_WRITE_CONV_RATE

}TMP102writeConfig;

/*
 * TMP102 structure variable
 */
typedef struct
{
	I2C_HandleTypeDef* 	I2CHandle; // pointer to i2c line
	uint8_t     	DeviceAdress;  // device addres
#if (TMP102_USE_FLOATNUMBERS == 1)
	float			MaxTemperature;  // min temp
	float			MinTemperature;	 // max temp
#else
	int8_t			MaxTemperatureIntegerPart;  // min temp integer part
	uint8_t			MaxTemperatureDecimalPart;  // min temp decimal part
	int8_t			MinTemperatureIntegerPart;	// max temp integer part
	uint8_t			MinTemperatureDecimalPart;	// max temp decimal
#endif
	TMP102config_t	Configuration;   // configuration
	uint8_t			ErrorCode;

}TMP102_t;


/*
 * TMP102 functions
 */
void TMP102Init(TMP102_t *DeviceStruct, I2C_HandleTypeDef *initI2CHandle,uint8_t initDeviceAddress);
#if (TMP102_USE_FLOATNUMBERS == 1)
float TMP102GetTempFloat(TMP102_t *tmp102);
uint8_t TMP102WriteMinMaxTempFloat(TMP102_t *tmp102, float temp, uint8_t MinOrMax);
#else
uint8_t TMP102WriteMinMaxTempInt(TMP102_t *tmp102, int8_t IntegerPart, uint8_t DecimalPart, uint8_t MinOrMax);
#endif
void TMP102GetTempInt(TMP102_t *tmp102,int8_t* value);
void TMP102GetConfiguration(TMP102_t *tmp102);
void TMP102GetMinMaxTemp(TMP102_t *tmp102);



#endif /* INC_TMP102_H_ */
