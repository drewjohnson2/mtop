#include <stdlib.h>

#include "../include/startup.h"
#include "../include/task.h"

int main(int argc, char **argv) 
{
    atexit(cleanup);
	atexit(broker_cleanup);
    run(argc, argv);
    
    return 0;
}
