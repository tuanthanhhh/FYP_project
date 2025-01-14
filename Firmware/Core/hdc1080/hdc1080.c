#include "hdc1080.h"
#include "stm32f1xx_hal_i2c.h"

void hdc1080_init(I2C_HandleTypeDef* hi2c_x,Temp_Reso Temperature_Resolution_x_bit,Humi_Reso Humidity_Resolution_x_bit)
{
	/* Temperature and Humidity are acquired in sequence, Temperature first
	 * Default:   Temperature resolution = 14 bit,
	 *            Humidity resolution = 14 bit
	 */

	/* Set the acquisition mode to measure both temperature and humidity by setting Bit[12] to 1 */
	uint16_t config_reg_value=0x1000;
	uint8_t data_send[2];

	if(Temperature_Resolution_x_bit == Temperature_Resolution_11_bit)
	{
		config_reg_value |= (1<<10); //11 bit
	}

	switch(Humidity_Resolution_x_bit)
	{
	case Humidity_Resolution_11_bit:
		config_reg_value|= (1<<8);
		break;
	case Humidity_Resolution_8_bit:
		config_reg_value|= (1<<9);
		break;
	}

	data_send[0]= (config_reg_value>>8);
	data_send[1]= (config_reg_value&0x00ff);

	HAL_I2C_Mem_Write(hi2c_x,HDC_1080_ADD<<1,Configuration_register_add,I2C_MEMADD_SIZE_8BIT,data_send,2,1000);
}

uint8_t hdc1080_reset(I2C_HandleTypeDef* hi2c_x)
{
    /* Reset the HDC1080 by setting bit 15 (RST) in the Configuration register */
    uint16_t reset_command = 0x8000;
    uint8_t data_send[2];

    data_send[0] = (reset_command >> 8); // MSB
    data_send[1] = (reset_command & 0x00ff); // LSB

    HAL_I2C_Mem_Write(hi2c_x, HDC_1080_ADD << 1, Configuration_register_add, I2C_MEMADD_SIZE_8BIT, data_send, 2, 1000);

    // Optional delay to let the sensor complete the reset process
    HAL_Delay(15); // Datasheet suggests a short delay for initialization after reset

    return 0; // Return success
}

uint8_t hdc1080_start_measurement(I2C_HandleTypeDef* hi2c_x,float* temperature, uint8_t* humidity)
{
	uint8_t receive_data[4];
	uint16_t temp_x,humi_x;
	uint8_t send_data = Temperature_register_add;

	HAL_I2C_Master_Transmit(hi2c_x,HDC_1080_ADD<<1,&send_data,1,1000);

	/* Delay here 15ms for conversion compelete.
	 * Note: datasheet say maximum is 7ms, but when delay=7ms, the read value is not correct
	 */
	HAL_Delay(15);

	/* Read temperature and humidity */
	HAL_I2C_Master_Receive(hi2c_x,HDC_1080_ADD<<1,receive_data,4,1000);


	temp_x =((receive_data[0]<<8)|receive_data[1]);
	humi_x =((receive_data[2]<<8)|receive_data[3]);

	*temperature=((temp_x/65536.0)*165.0)-40.0;
	*humidity=(uint8_t)((humi_x/65536.0)*100.0);

	return 0;

}
