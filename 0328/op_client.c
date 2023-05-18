#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
#define RLT_SIZE 4
#define OPSZ 4

void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

int main(int argc, char *argv[]){
    int sock;
    char opmsg[BUF_SIZE];
    int result, opnd_cnt, i;
    struct sockaddr_in serv_adr;
    char message[BUF_SIZE];

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

    else{
        puts("Connected......");
    }


    while(1){
        
        fputs("Operant count(-1 to quit) : ", stdout);
        scanf("%d", &opnd_cnt);

        if(opnd_cnt == -1)
            break;
        
        opmsg[0] = (char)opnd_cnt;
        for(i=0; i<opnd_cnt; i++){
            printf("Operant %d : ", i+1);
            scanf("%d", (int*)&opmsg[i*OPSZ + 1]);
        }
        fgetc(stdin);
        fputs("Operator: ", stdout);
        scanf("%c", &opmsg[opnd_cnt * OPSZ + 1]);
        write(sock, opmsg, opnd_cnt * OPSZ + 2);
        read(sock, &result, RLT_SIZE);

        printf("Operation result: %d\n", result);
        
        
    }
    close(sock);
    return 0;
    

/*
    while(1){
        fputs("Input message(Q to quit): ",stdout);
        fgets(message, BUF_SIZE, stdin);

        if(!strcmp(message, "q\n") || !strcmp(message, "Q\n")){
            break;
        }

        str_len = write(sock, message, strlen(message));
        recv_len = 0;

        while(recv_len < str_len){
            recv_cnt = read(sock, &message[recv_len], BUF_SIZE -1);
            if(recv_cnt == -1){
                error_handling("read() error!");
            }
            recv_len += recv_cnt;
        }
        message[recv_len] = 0;
        printf("Message from server: %s", message);
    }
    close(sock);
    return 0;
*/

}
