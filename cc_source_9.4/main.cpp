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
#include "option.h"

int main(int argc, char *argv[])
{
	option = new Option();
	for (int i = 1; i < argc; i++)
	{
		char *p = argv[i];
		int   l = strlen(p);

		if ( p[0] == '-' )	// compiling option...
		{
			if ( !option->add(&p[1]) ) break;
			continue;
		}

		if ( !(l > 2 && p[l-2] == '.' && toupper(p[l-1]) == 'C') )
			continue;

		printf("parse ... %s\n", p);

		std::string str = "cpp1 "; str += p;
		if ( system(str.c_str()) == 0 )
		{
			str = p; str += "_";	// output file of 'cpp1'

			if ( option->mcuFile )	// merge files
			{
				char buf[l + 32];
				sprintf(buf, "cat %s %s > ~.c_", option->mcuFile, str.c_str());
				system(buf);
				remove(str.c_str());
				rename("~.c_", str.c_str());
			}

			int rtcode = _main((char*)str.c_str());
			remove(str.c_str());	// delete output file of 'cpp1'

			if ( rtcode == 0 )
			{
				Nlist nlist;
				for (int i = 0;;)
				{
					Nnode *np = option->get('D', i++);
					if ( np == NULL ) break;
					nlist.add(np->name, DEFINE, cloneNode(np->np[0]));
				}

				PreScan preScan(&nlist);
				progUnit = preScan.scan(progUnit);

				Pcoder pcoder;			// P-code generation
				pcoder.run(progUnit);	// run it now.

				if ( pcoder.errorCount == 0 )
				{
					if ( option->level > '0' )
					{
						Optimizer opt(&pcoder);
						opt.run();
					}

					if ( option->debug )
						display(pcoder.mainPcode);

					str.replace(str.length()-3, 3, ".asm");
					PIC16E asm_gen((char*)str.c_str(), &nlist, &pcoder, option->asmopt &&
																		option->level > '0');
					asm_gen.run();
				}
			}
		}
	}
	delete option;
	return 0;
}
