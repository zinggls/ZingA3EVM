#include "i2c_test.h"
#include "i2c.h"

void test_i2c_init()
{
	CyU3PReturnStatus_t Status = I2C_Init();
	assert("I2C Init",Status==CY_U3P_SUCCESS);
}