#include <stdlib.h>

#include "../include/startup.h"
#include "../include/startup.h"

int main() 
{
    atexit(cleanup);
    run();
    
    return 0;
}
