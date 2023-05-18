//data의 경계가 UDP엔 존재하므로, 5초간의 delay를 삽입해도 총 3개의 메시지를 3번의 recvfrom()으로 수신
//data 전송에 있어 TCP와의 유일한 차이점은, 사용하는 함수가 다르고, 목적지 정보를 매 호출시마다 지정한다

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

int main(int argc, char *argv[]){
    int sock;
    char message[BUF_SIZE];
    struct sockaddr_in my_adr;
    struct sockaddr_in your_adr;
    socklen_t adr_sz;
    int str_len;
    int i;

    if(argc != 2){
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if(sock == -1)
        error_handling("socket() error");

    memset(&my_adr, 0, sizeof(my_adr));
    my_adr.sin_family = AF_INET;
    my_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_adr.sin_port = htons(atoi(argv[1]));

    if(bind(sock, (struct sockaddr*)&my_adr, sizeof(my_adr)) == -1)
        error_handling("bind() error");
    
    for(i=0; i<3; i++){
        sleep(5);
        
        adr_sz = sizeof(your_adr);
        str_len = recvfrom(sock, message, BUF_SIZE, 0, (struct sockaddr*)&your_adr, &adr_sz);

        printf("Message %d : %s\n", i+1, message);
    }

    close(sock);
    return 0;
}