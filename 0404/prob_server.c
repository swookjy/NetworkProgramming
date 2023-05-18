//server receives a hostname from a client
//then it gets a corresponding host address list and sends the first one in the list back to the client

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
    int serv_sock;
    int clnt_sock;

    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;

    int str_len, i;
    char err_message[16] = {"gethost...error"};

    struct hostent *host;

    if(argc != 2){
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1){
        error_handling("socket() error");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){
        error_handling("bind() error");
    }
    if(listen(serv_sock, 5) == -1){
        error_handling("listen() error");
    }
    clnt_addr_size = sizeof(clnt_addr);
    
    for(i=0; i<5; i++){
        
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        
        if(clnt_sock == -1){
            error_handling("accept() error");
        }
        else{
            printf("new client connected ... \n");
        }
        
        //read()함수 : 성공시 데이터의 크기를 return. 실패시 -1.
        //write()함수 : 성공시 데이터의 크기를 Return. 실패시 -1.

        //while((strcmp(message, "q\n") != 0)){
        while(1){   
            char message[BUF_SIZE] = {};//문자열 초기화 안하니까 계속 오류 발생. 입력한 뒷 부분이 남아있음. ex)www.naver.comcom
            str_len = read(clnt_sock, message, sizeof(message));
            if(!strcmp(message, "q") || !strcmp(message, "Q")) {
                printf("client disconnected......\n");
                break;
            }
            

            host = gethostbyname(message);
            if(!host){
                write(clnt_sock, err_message, sizeof(err_message));//error message 전송
            }
            else{
                write(clnt_sock, inet_ntoa(*(struct in_addr*)host->h_addr_list[0]), BUF_SIZE);
            }
            //inet_nota()함수 리턴값 : 변환된 해당 문자열의 포인터
        }
        
        close(clnt_sock);
        
    }
    close(serv_sock);

    return 0;
}