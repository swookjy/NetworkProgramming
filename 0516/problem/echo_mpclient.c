#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handling(char *buf)
{
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(1);
}

int main(int argc, char *argv[])
{
	int sock;
    int len;
	char message[BUF_SIZE];
	int str_len;
	struct sockaddr_in serv_adr;

    FILE *readfp;
    FILE *writefp;
    char pid[BUF_SIZE];
    


	if(argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock = socket(PF_INET, SOCK_STREAM, 0);   
	if(sock == -1)
		error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_adr.sin_port = htons(atoi(argv[2]));
	
	if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("connect() error!");
	else{
		//puts("Connected...........");
    }
    readfp = fdopen(sock, "r");
    writefp = fdopen(sock, "w");
	
	while(1) 
	{
		//fputs("Input message(Q to quit): ", stdout);
		fgets(message, BUF_SIZE, stdin);
		
		if(!strcmp(message,"q\n") || !strcmp(message,"Q\n")){
            strcpy(pid, message);
			fputs(pid, writefp);
			fflush(writefp);

			sprintf(pid, "removed proc id : %d\n", getpid());
			fputs(pid, writefp);                           
    		fflush(writefp);

			shutdown(fileno(writefp), SHUT_WR);
			break;
        }

		write(sock, message, strlen(message));
		str_len=read(sock, message, BUF_SIZE-1);
		message[str_len]=0;
		printf("Message from server: %s", message);
	}

	fclose(readfp);
	fclose(writefp);
	close(sock);
    
	return 0;
}