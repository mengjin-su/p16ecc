#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <string>
extern "C" {
#include "common.h"
#include "cc.h"
}
#include "option.h"
#include "nlist.h"
#include "prescan.h"
#include "display.h"
#include "popt.h"
#include "./stm8/stm8s.h"

int main(int argc, char *argv[])
{
	option = new Option();

	for (int i = 1; i < argc; i++)
	{
		char *p = argv[i];
		int   l = strlen(p);

		if ( p[0] == '-' ) 	// compiling option
		{
            if ( !option->add(&p[1]) ) break;
			continue;
		}

		if ( !(l > 2 && p[l-2] == '.' && toupper(p[l-1]) == 'C') )
		{
			printf("wrong source file! - %s\n", p);
			continue;
		}

		printf("parsing '%s' ...\n", p);
		std::string str = "st8cpp1 "; str += p;
		if ( system(str.c_str()) == 0 )
		{
			str = p; str += "_";

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

				Pcoder pcoder(&nlist);
				pcoder.run(progUnit);

				if ( pcoder.errCount == 0 )
				{
					if ( option->level > '0' )
					{
						Optimizer opt(&pcoder);
						opt.run();
					}

					// debug purpose - show P-code
					if ( option->debug )
						display(pcoder.mainPcode);

					str.replace(str.length()-3, 3, ".asm");
					STM8S stm8s((char*)str.c_str(), &nlist, &pcoder);
					stm8s.run();
				}
				delNode(progUnit);
			}
		}
	}

	delete option;
	return 0;
}
