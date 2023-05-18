#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void timeout(int sig){
    if(sig == SIGALRM)
        puts("\n<<<Time out!>>>\n");
    alarm(2);
}

void keycontrol(int sig){
    if(sig == SIGINT)
        puts("\n<<<CTRL + C pressed!>>>\n");
}

void mychild(int sig){
    if(sig == SIGCHLD)
        puts("\n<<<Child process termination>>>\n");
}

int main(int argc, char *argv[]){
    int i;
    signal(SIGALRM, timeout);
    signal(SIGINT, keycontrol);
    signal(SIGCHLD, mychild);
    alarm(2);

    pid_t pid = fork();
    if(pid == 0){
        exit(7);
    }

    for(i=0; i<5; i++){
        puts("wait...");
        sleep(5);
    }

    return 0;
}