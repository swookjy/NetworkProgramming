/*

IPC : Parent process - Child process
Exchange a string between two relative processes over a total of five times. 
The type of string to be exchanged is desired to be determined by you in the program.
The exchanged string is finally stored in a file. >> I'll save it in parent process

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024


int main(int argc, char *argv[]){
	int fds1[2];
	int fds2[2];
	int i, len;
	
	char str1[] = "aaaaa";
	char str2[] = "bbbbbb";
	char str3[] = "ccccccc";
	char str4[] = "dddddddd";
	char str5[] = "eeeeeeeee";

	char buf[BUF_SIZE];
	pid_t pid;

	char * strs[] = {str1, str2, str3, str4, str5};

	FILE *fp = fopen("prob1.txt", "wt");

    pipe(fds1);
    pipe(fds2);

    pid = fork();

	if (pid == 0){
		for (i=0; i<5; i++){
			write(fds1[1], strs[i], strlen(strs[i])+1);
			sleep(1);
        }
		return 0;
    }

    else{
		for(i = 0; i<5; i++){
			len = read(fds1[0], buf, BUF_SIZE);
            printf("Child => Parent ) message%d : %s\n", i+1, buf);
			fwrite((char*)buf, 1, len-1, fp);
        }
		fclose(fp);
    }
    return 0;
}