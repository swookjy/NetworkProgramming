#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

/*
클라이언트 프로그램에서는 두개의 과정만 존재
1. 소켓 생성          :   socket()함수
2. 서버로 연결 요청     :   connect()함수
*/

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    char message[30];
    int str_len;

    if(argc != 3){
        printf("Usage : %s <IP> <port>\n", argv[0]);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1){
        error_handling("socket() error");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));


    // 서버의 listen()함수가 호출되어야 클라이언트가 연결요청을 할 수 있음
    // listen()함수 호출 이전에 connect()함수 호출시 오류 발생
    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){
        error_handling("connect() error");
    }
    // connect() 함수호출 시 자동으로 소켓에 IP, PORT가 할당됨.
    // 따라서 클라이언트 프로그램 구현시 bind()함수를 명시적으로 호출할 필요 없음

    // read(file descriptor, buffer 주소값, 수신할 최대 바이트수)
    // 성공시 수신한바이트수, 실패시 -1 반환
    str_len = read(sock, message, sizeof(message)-1);
    if(str_len == -1){
        error_handling("read() error");
    }

    printf("Message from server : %s \n", message);
    close(sock);
    return 0;
}