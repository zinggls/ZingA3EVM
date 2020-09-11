#include "i2c_test.h"
#include "i2c.h"

void test_i2c_init()
{
	CyU3PReturnStatus_t Status = I2C_Init();
	assert("1st I2C Init",Status==CY_U3P_SUCCESS);

	Status = I2C_Init();
	assert("2nd I2C Init",Status!=CY_U3P_SUCCESS);
}
