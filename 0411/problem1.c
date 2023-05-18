#include <stdio.h>
#include <unistd.h>
int main(){
    if(fork() || fork())
        fork();
    
    // sleep(1);
    printf("1 ");
    // sleep(1);
    return 0;
}