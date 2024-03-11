#include <stdio.h>
#include <string>
#include "cpp1.h"

int main(int argc, char *argv[], char *env[])
{
	if ( argc <= 1 )
	{
		printf("missing input file!\n");
		return -1;
	}

	std::string output = argv[1];
	output += "_";

	return cpp1(env, argv[1], (char*)output.c_str());
}