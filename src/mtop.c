#include <stdlib.h>

#include "../include/startup.h"

int main(int argc, char **argv) 
{
    atexit(cleanup);
    run(argc, argv);
    
    return 0;
}
