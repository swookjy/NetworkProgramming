//client sends a hostname entered by the user to the server
//then it receives a host address or an error message from the server
//client keeps working until the user enters 'Q' or 'q'

//client는 server에게 DNS를 전송

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 1024

void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

int main(int argc, char *argv[]){
    int sock;
    char message[BUF_SIZE];
    int str_len, recv_len, recv_cnt;
    struct sockaddr_in serv_adr;

    if(argc != 3){
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1){
        error_handling("socket() error");
    }

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1){
        error_handling("connect() error");
    }

    //요까지 ok

    while(1){
        fputs("Enter a host name(q to quit): ",stdout);
        fgets(message, BUF_SIZE, stdin);

        str_len = write(sock, message, strlen(message)-1);
        
        if(!strcmp(message, "q\n") || !strcmp(message, "Q\n")){//q입력하면, 같으므로 리턴값이 0. 반전하면 1이므로 if문에 들어감.
            break;
        }

        recv_cnt = read(sock, message, sizeof(message));
        if(recv_cnt == -1){
            error_handling("read() error!");
        }
        printf("%s\n", message);
        
    }
    sleep(1);
    close(sock);
    return 0;

}
