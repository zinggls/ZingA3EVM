#ifndef __PIB_TEST_H__
#define __PIB_TEST_H__

#include "Application.h"
#include "gpif/PIB.h"

void test_pib_init()
{
	assert("1st PIB Init",PIB_Init()==CY_U3P_SUCCESS);
	assert("2nd PIB Init",PIB_Init()!=CY_U3P_SUCCESS);
}

#endif
