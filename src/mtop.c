#define _GNU_SOURCE

#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

#include "../include/mt_type_defs.h"
#include "../include/startup.h"

int main(int argc, char **argv) 
{
    int option_index = 0;
    u8 transparencyEnabled = 0;

    static struct option long_options[] = 
    {
	{ "transparent", no_argument, NULL, 't' },
	{ NULL, no_argument, NULL, 0 }
    };

    u8 arg = getopt_long(argc, argv, "t", long_options, &option_index);

    switch (arg) 
    {
	case 't':
	    transparencyEnabled = 1;
	    break;
	default:
	    break;
    }

    atexit(cleanup);
    run(transparencyEnabled);
    
    return 0;
}
