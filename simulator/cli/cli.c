/* Mulator - An extensible {ARM} {e,si}mulator
 * Copyright 2011-2012  Pat Pannuto <pat.pannuto@gmail.com>
 *
 * This file is part of Mulator.
 *
 * Mulator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mulator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mulator.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "core/common.h"

// For access to 'raiseonerror' flag
#define PP_STRING "CLI"
#include "core/pretty_print.h"

#include "core/simulator.h"

#include <getopt.h>

static void usage_fail(int retcode) {
	printf("\nUSAGE: ./simulator [OPTS]\n\n");
	printf("\
-- DEBUGGING -------------------------------------------------------------------\n\
\t-g, --gdb [port]\n\
\t\tAct as a remote target for gdb to debug. The optional port\n\
\t\targument specifies the port to listen on, otherwise a random\n\
\t\tport will be selected.\n\
\t-c, --dumpatpc PC_IN_HEX\n\
\t\tExecute until PC reaches the given value. The simulator will\n\
\t\tpause at the given PC and prompt for a new pause PC\n\
\t-y, --dumpatcycle N\n\
\t\tExecute until cycle N and print the current machine state.\n\
\t\tYou will be prompted for a new N at the pause\n\
\t-a, --dumpallcycles\n\
\t\tPrints the machine state every cycle -- This is a lot of text\n\
\t-p, --printcycles\n\
\t\tPrints cycle count and inst to execute every cycle (before exec)\n\
\t\tConceptually, this executes between the intstruction fetch and\n\
\t\tdecode stage, so the PC is already advanced\n\
\t-s, --slowsim\n\
\t\tSlows simulation down, running an instruction every .1s\n\
\t-e, --raiseonerror\n\
\t\tRaises a SIGTRAP for gdb on errors before dying\n\
\t\t(Useful for debugging with gdb)\n\
-- RUN TIME --------------------------------------------------------------------\n\
\t-r, --returnr0\n\
\t\tSets simulator binary return code to the return\n\
\t\tcode of the executed program on simulator exit\n\
\t-m, --limit\n\
\t\tLimit CPU execution to N cycles, returns failure if hit\n\
\t\t(useful for catching runaway test cases)\n\
\t-f, --flash FILE\n\
\t\tFlash FILE into ROM before executing\n\
\t\t(this file is likely somthing.bin)\n\
\t--usetestflash\n\
\t\tFlash the program ROM with a copy of the echo program\n\
\t\tbefore running. Conflicts with -f\n\
\n\
"\
	       );
	exit(retcode);
}

static void usage(void) {
	usage_fail(0);
}

int main(int argc, char **argv) {
	const char *flash_file = NULL;

	// Command line parsing
	while (1) {
		static struct option long_options[] = {
			{"gdb",           optional_argument, 0,              'g'},
			{"dumpatpc",      required_argument, 0,              'c'},
			{"dumpatcycle",   required_argument, 0,              'y'},
			{"dumpallcycles", no_argument,       &dumpallcycles, 'a'},
			{"printcycles",   no_argument,       &printcycles,   'p'},
			{"slowsim",       no_argument,       &slowsim,       's'},
			{"raiseonerror",  no_argument,       &raiseonerror,  'e'},
			{"returnr0",      no_argument,       &returnr0,      'r'},
			{"limit",         required_argument, 0,              'm'},
			{"flash",         required_argument, 0,              'f'},
			{"usetestflash",  no_argument,       &usetestflash,  1},
			{"help",          no_argument,       0,              '?'},
			{0,0,0,0}
		};
		int option_index = 0;
		int c;

		c = getopt_long(argc, argv, "dg::c:y:apserm:f:?", long_options,
				&option_index);

		if (c == -1) break;

		switch (c) {
			case 0:
				// option set a flag
				break;

			case 'g':
				gdb_port = optarg ? atoi(optarg) : 0;
				break;

			case 'c':
				dumpatpc = strtol(optarg, NULL, 16);
				INFO("Simulator will pause at execute of PC %x\n",
						dumpatpc);
				break;

			case 'y':
				dumpatcycle = atoi(optarg);
				INFO("Simulator will pause at cycle %d\n",
						dumpatcycle);
				break;

			case 'a':
				dumpallcycles = true;
				break;

			case 'p':
				printcycles = true;
				break;

			case 's':
				slowsim = true;
				break;

			case 'e':
				raiseonerror = true;
				break;

			case 'r':
				returnr0 = true;
				break;

			case 'm':
				limitcycles = atoi(optarg);
				INFO("Simulator will terminate at cycle %d\n",
						limitcycles);
				break;

			case'f':
				flash_file = optarg;
				INFO("Simulator will use %s as flash\n",
						flash_file);
				break;

			case '?':
			default:
				usage();
				break;
		}
	}

	if (flash_file && usetestflash) {
		ERR(E_BAD_FLASH, "Only one of -f or --usetestflash may be used\n");
	} else if (usetestflash) {
		INFO("Simulator will use internal test flash\n");
	}

	simulator(flash_file);

	ERR(E_UNKNOWN, "Simluator returned to cli main thread?\n");
}
