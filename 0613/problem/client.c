#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
	
#define BUF_SIZE 100
#define NAME_SIZE 20
#define AGE_SIZE 4
#define COUNTRY_SIZE 20
	
void *send_msg(void *arg);
void *recv_msg(void *arg);
void error_handling(char *msg);

char name[NAME_SIZE] = "[DEFAULT]";
char ages[AGE_SIZE] = {0, };
char country[COUNTRY_SIZE] = {0, };
char msg[BUF_SIZE];
	
int main(int argc, char *argv[]){
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void *thread_return;
    char buf[BUF_SIZE];
	int age;

	if(argc != 6) {
		printf("Usage : %s <IP> <port> <name> <age> <country>\n", argv[0]);
		exit(1);
	 }
	
    //user name
	sprintf(name, "[%s]", argv[3]);

	//user age
	sprintf(ages, "%s", argv[4]);
	if((age=atoi(ages)) == 0 ){
		error_handling("Age input error");
	}

	//user country
	sprintf(country, "%s", argv[5]);

    //check user name unique
	sock = socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));
	
    //연결 요청
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");
	
    //name을 전송
    write(sock, name, strlen(name));
    
    //name 중복 검사 결과 수신
    int k = read(sock, buf, sizeof(buf)-1);
    printf("%s", buf);
    if(strcmp(buf, "welcome!\n") != 0){
        close(sock);
        exit(1);
    }

	//부가 정보 전송(age, country)
	write(sock, ages, strlen(ages));
	write(sock, country, strlen(country));
    
	//thread management : 
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);

	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);

	close(sock);  
	return 0;
}

// send thread main
void *send_msg(void *arg){
	int sock = *((int*)arg);
	char name_msg[NAME_SIZE + BUF_SIZE];
	
	while(1) {
		fgets(msg, BUF_SIZE, stdin);
		if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n")){
			close(sock);
			exit(0);
		}
		sprintf(name_msg,"%s %s", name, msg);
        
		write(sock, name_msg, strlen(name_msg));
	}
	return NULL;
}


// read thread main
void *recv_msg(void *arg){
	int sock = *((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE];
	int str_len;
	
	while(1){
		str_len = read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
		if(str_len == -1) 
			return (void*)-1;
		name_msg[str_len] = 0;
		fputs(name_msg, stdout);
	}
	return NULL;
}
	
void error_handling(char *msg){
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
