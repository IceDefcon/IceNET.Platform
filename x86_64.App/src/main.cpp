//
// Author: Ice.Marek
// 2023 IceNET Technology
//

using namespace std;

#include <stdio.h>
#include <iostream>

#include "hack.h"

int main(void)
{

	int * pX;
	static int * pY;

	int X = 10;
	static int Y = 20;

	pX = &X;
	pY = &Y;
;
	printf(" ---==[ Quick Address check ]==--- \n");
	printf("  X ---> %x\n", X);
	printf("  Y ---> %x\n", Y);
	printf(" pX ---> %p\n", pX);
	printf(" pY ---> %p\n", pY);

	/* code */
	return 0;
}
