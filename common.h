#ifndef COMMON_H
#define COMMON_H
#include <time.h>
int timeMesure(clock_t start){
    return(int)(((float)(clock()-start)/CLOCKS_PER_SEC)*1000);
}
#endif // COMMON_H
