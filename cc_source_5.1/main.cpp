#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "common.h"
#include "display.h"
#include "nlist.h"

int main(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++)
	{
		std::string buffer = "cpp1 "; 
		buffer += argv[i];
		
		if ( system(buffer.c_str()) == 0 )
		{
			buffer = argv[i]; buffer += "_";
			
			_main((char*)buffer.c_str());
			display(progUnit, 0);
		}
	}
	return 0;
}
