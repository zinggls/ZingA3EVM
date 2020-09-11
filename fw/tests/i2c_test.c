#include "i2c_test.h"
#include "i2c.h"

void assert(const char *str,int nExpression)
{
	nExpression? CyU3PDebugPrint(4,"%s: ok\n",str) : CyU3PDebugPrint(4,"%s: fail\n",str);
}

void test_i2c_init()
{
	CyU3PReturnStatus_t Status = I2C_Init();
	assert("I2C Init",Status==CY_U3P_SUCCESS);
	CyU3PDebugPrint(4,"test_i2c_init=%d\n",Status);
}