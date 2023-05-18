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

    //ch1의 hello_client에서 추가된 부분
    int idx = 0;
    int read_len = 0;

    if(argc != 3){
        printf("Usage : %s <IP> <port>\n", argv[0]);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1){
        error_handling("socket() error");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = PF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    
    //inet_addr(const char *string) : include <arpa/inet.h>
    //점이 찍힌 10진수 문자열을 전달하면, 32Bit 정수형으로 반환

    //int atoi (const char* cStr) : include <stdlib.h>
    //chAr TO Int : 문자열을 정수 타입으로

    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){
        error_handling("connect() error");
    }

    //read(file descriptor, buffer 주소값, 수신할 최대 바이트수)
    //성공시 수신한바이트수, 실패시 -1 return
    while(read_len = read(sock, &message[idx++], 1)){
        if(read_len == -1){
            error_handling("read() error");
        }

        str_len += read_len;
    }

    printf("Message from server : %s \n", message);
    printf("Function read call count : %d\n", str_len);
    close(sock);
    return 0;
}