#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "common.h"
#include "nlist.h"
#include "dlink.h"
#include "flink.h"
#include "pnode.h"
#include "pcoder.h"
#include "prescan.h"
#include "display.h"
#include "popt.h"
#include "./p16e/pic16e.h"

int main(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++)
	{
		std::string str = "cpp1 "; 
		str += argv[i];
	
		printf("parse ... %s\n", argv[i]);
		if ( system(str.c_str()) == 0 )
		{
			str = argv[i]; str += "_";
			
			int rtcode = _main((char*)str.c_str());
//			remove(str.c_str());
			
//			display(progUnit, 0);
			if ( rtcode == 0 )
			{
				Nlist nlist;
				PreScan preScan(&nlist);
				progUnit = preScan.scan(progUnit);
				
				Pcoder pcoder;			// P-code generation
				pcoder.run(progUnit);	// run it now.
//				display(pcoder.mainPcode); printf("\n");

				if ( pcoder.errorCount == 0 )
				{
					Optimizer opt(&pcoder);
					opt.run();
					display(pcoder.mainPcode);

					str.replace(str.length()-3, 3, ".asm");
					PIC16E asm_gen((char*)str.c_str(), &nlist, &pcoder);
					asm_gen.run();
				}
			}
		}
	}
	return 0;
}
