#include "i2c_test.h"
#include "i2c.h"
#include "ZingHw.h"

void test_i2c_init()
{
	CyU3PReturnStatus_t Status = I2C_Init();
	assert("1st I2C Init",Status==CY_U3P_SUCCESS);

	Status = I2C_Init();
	assert("2nd I2C Init",Status!=CY_U3P_SUCCESS);
}

void test_i2c_write_read()
{
	CyU3PReturnStatus_t Status;
	uint8_t buffer[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10,0x10};
	uint8_t rxBuffer[11] = {0,};
	uint8_t devAddr = I2C_DeviceAddress;
	uint8_t regAddr = 0x00;

	Status = I2C_Write(devAddr,regAddr,buffer,sizeof(buffer));
	assert("I2C Write",Status==CY_U3P_SUCCESS);

	Status = I2C_Read(devAddr,regAddr,rxBuffer,sizeof(rxBuffer));
	assert("I2C Read",memcmp(rxBuffer,buffer,sizeof(buffer))==0);
}
