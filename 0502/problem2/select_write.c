#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#include <string.h>
#define BUF_SIZE 30

int main(int argc, char *argv[])
{
	fd_set reads, temps, writes;
	int result, str_len;
	char buf[BUF_SIZE];
	
    struct timeval timeout;
	FD_ZERO(&reads);
	FD_SET(0, &reads);
    FD_ZERO(&writes);
    FD_SET(1, &writes);// stdio file descriptor

    int count = 0;
	timeout.tv_sec = 5;
	timeout.tv_usec = 5000;

	while(1)
	{
		temps = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		//result=select(1, &temps, 0, 0, &timeout);
		result = select(2, &temps, &writes, 0, &timeout);

        
        if(result == -1)    //error
		{
			puts("select() error!");
			break;
		}
		else if(result == 0)    //time out
		{
			puts("Time-out!");
		}
		else 
		{
			if(FD_ISSET(0, &temps)) //console
			{
				str_len = read(0, buf, BUF_SIZE);
				buf[str_len] = 0;
				printf("message from console: %s", buf);
			}

            if(FD_ISSET(1, &writes))  // 출력용 파일 디스크립터
            {
                char message[BUF_SIZE];
                sprintf(message, "message %d\n", count++);
                write(1, message, strlen(message));
                sleep(5);
            }
		}
	}
	return 0;
}