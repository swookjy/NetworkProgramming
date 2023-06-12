#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 256
#define NAME_SIZE 20

void check_name(int clnt_sock);
void *handle_clnt(void *arg);
void send_msg(char *msg, int len);
void error_handling(char *msg);
void free_naming();

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
char *clnt_name[MAX_CLNT];
char duplicated_name_msg[] = "A user with the same name already exists.";

pthread_mutex_t mutx;

int main(int argc, char *argv[]){
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	if(argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
  
	pthread_mutex_init(&mutx, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET; 
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");

	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	while(1){
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);

        int name_len;
        char name[NAME_SIZE];
        unsigned int dup_flag = 0;

        name_len = read(clnt_sock, name, sizeof(name)-1);
        printf("name : %s\n", name);

        pthread_mutex_lock(&mutx);
        
        //1. name 중복 flag 확인
        for(int i=0; i<clnt_cnt; i++){
            if(strcmp(name, clnt_name[i]) == 0){//이름 중복
                dup_flag = 1;
                break;
            }
        }

        //2. 중복 여부에 따라 추가할지, 강퇴시킬지 결정
        if(dup_flag){
            write(clnt_sock, duplicated_name_msg, sizeof(duplicated_name_msg));
            close(clnt_sock);
            continue;
        }
        else{
            clnt_name[clnt_cnt] = (char*)malloc(sizeof(name));
            sprintf(clnt_name[clnt_cnt], "%s", name);
            write(clnt_sock, "welcome!", 9);
            printf("client %d : %s\n", clnt_sock, clnt_name[clnt_cnt]);
        }
        
        clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutx);
	
        //client마다 thread 생성 : handle_clnt()함수로.
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);

        //thread 종료시, client 자원 해제
		pthread_detach(t_id);
		
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
	}

	close(serv_sock);
    free_naming();
	return 0;
}

void check_name(int clnt_sock){
    //name 수신
    int name_len;
    char name[NAME_SIZE];
    name_len = read(clnt_sock, name, sizeof(name)-1);
    printf("name : %s\n", name);

    //name 중복 확인
    for(int i=0; i<clnt_cnt; i++){
        if(strcmp(name, clnt_name[i]) == 0){//이름 중복
            write(clnt_sock, duplicated_name_msg, sizeof(duplicated_name_msg));
            close(clnt_sock);
        }
        else{
            clnt_name[clnt_cnt] = (char*)malloc(sizeof(name));
            strcpy(clnt_name[clnt_cnt], name);
        }
    }
    printf("client %d : %s\n", clnt_sock, clnt_name[clnt_cnt]);
}
	
void *handle_clnt(void *arg){
	int clnt_sock = *((int*)arg);
	int str_len = 0;
	int i;
	char msg[BUF_SIZE];
	
	while((str_len = read(clnt_sock, msg, sizeof(msg)))!=0)
		send_msg(msg, str_len);
	
	pthread_mutex_lock(&mutx);
	
	// remove disconnected client
	for(i=0; i<clnt_cnt; i++){
		if(clnt_sock == clnt_socks[i]){
			while(i++ < clnt_cnt-1)
				clnt_socks[i] = clnt_socks[i+1];
			break;
		}
	}
	clnt_cnt--;
	
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}

// send to all
void send_msg(char *msg, int len){
	int i;
	
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)
		write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);

}


void error_handling(char *msg){
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}


void free_naming(){
    for(int i=0; i<clnt_cnt; i++){
        free(clnt_name[i]);
    }
}