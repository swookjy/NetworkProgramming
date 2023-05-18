/*
UDP는 TCP와 같이 연결된 상태로 데이터를 송수신하지 않으므로 연결 설정의 과정이 필요 없다.
 -> listen(), accpet()함수가 필요 없음

TCP : 소켓과 소켓의 관계가 일대일
 -> 문지기 역할을 하는 서버 소켓을 제외하고도 클라이언트의 수만큼 소켓이 더 필요함
UDP : 하나의 UDP소켓으로 여러개의 호스트와 통신 가능

UDP소켓은 연결상태를 유지하지 않으므로, 데이터를 전송할 때마다 반드시 주소정보를 추가해야함

전송
ssize_t sendto(int sock, void *buff, size_t nbytes, int flags, struct sockaddr *to, socklen_t addrlen);
 -> to : destination 주소정보를 담고 있는 sockaddr 구조체변수의 주소값
 
수신
ssize_t recvfrom(int sock, void *buff, size_t nbytes, struct sockaddr *from, socklen_t *addrlen);
 -> from : 발신지 정보를 채워 넣을 구조체변수의 주소값

*/


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
    int serv_sock;
    char message[BUF_SIZE];
    int str_len;
    socklen_t clnt_adr_sz;

    struct sockaddr_in serv_adr, clnt_adr;
    if(argc != 2){
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if(serv_sock == -1)
        error_handling("UDP socket creation error");
    
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    while(1){
        clnt_adr_sz = sizeof(clnt_adr);
        str_len = recvfrom(serv_sock, message, BUF_SIZE, 0, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
        sendto(serv_sock, message, str_len, 0, (struct sockaddr*)&clnt_adr, clnt_adr_sz);
    }
    close(serv_sock);
    return 0;

}