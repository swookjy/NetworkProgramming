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
네트워크 프로그래밍에서 연결요청을 허용하는 소켓의 생성 과정
1. 소켓 생성                :   socket()함수 호출
2. IP주소, port번호 할당     :      bind()함수 호출
3. 연결요청 가능상태로 변경     :   listen()함수 호출
4. 연결요청에 대한 수락        :   accept()함수 호출
*/

int main(int argc, char *argv[])
{
    int serv_sock;
    int clnt_sock;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;

    socklen_t clnt_addr_size;

    char message[] = "hello world!";

    if(argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1){
        error_handling("bind() error");
    }

    if(listen(serv_sock, 5) == -1){
        error_handling("listen() error");
    }

    // listen 함수 호출 이후 클라이언트의 연결요청이 들어오면, 들어온 순서대로 연결요청을 수락해야함
    // 연결요청 수락 = 클라이언트와 데이터를 주고받을 수 있는 상태가 됨
    // 즉, listen()함수 호출 이후 서버는 연결요청 대기 큐를 만듬.

    // accept()함수 : 연결요청대기큐 에서 대기중인 클라이언트의 연결요청을 수락하는 기능
    // accpet()함수 호출성공시, 내부적으로 데이터 입출력에 사용할 소켓을 생성함
    // 소켓의 file descriptor를 return
    // 즉, accept()함수를 정리하면
    // 1. 소켓이 자동으로 생성됨
    // 2. 연결요청을 한 클라이언트 소켓에 연결까지 이뤄짐.


    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    if(clnt_sock == -1){
        error_handling("accept() error");
    }

    write(clnt_sock, message, sizeof(message));
    // write(file descriptor, buffer주소, 데이터의 바이트수);
    // 성공시 전달한바이트수, 실패시 -1 반환
    close(clnt_sock);
    // close(file descriptor); 
    // 성공시0, 실패시-1 반환
    close(serv_sock);
    return 0;
}