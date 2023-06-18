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
#define AGE_SIZE 4
#define COUNTRY_SIZE 20

void *handle_in(void *arg);
void *handle_out(void *arg);
void error_handling(char *msg);
void free_naming();
void GiveClientInfo(int requestclntfd, char* name);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
char *clnt_name[MAX_CLNT];
char duplicated_name_msg[] = "A user with the same name already exists. Try another name.\n";

//client로부터 수신한 메세지
char msg[BUF_SIZE];
int str_len = 0;
//client로 전송할 메세지
char echo_msg[BUF_SIZE];

pthread_mutex_t mutx;

int main(int argc, char *argv[]){
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id1;
	pthread_t t_id2;
	FILE * fp;

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

        int name_len, ages_len, country_len, agree_len;
        char name[NAME_SIZE] = {0, };
		char ages[AGE_SIZE] = {0, };
		char country[COUNTRY_SIZE] = {0, };
		char agree[1] = {0, };
        unsigned int dup_flag = 0;

		//client의 이름 수신
        name_len = read(clnt_sock, name, sizeof(name)-1);
        
		pthread_mutex_lock(&mutx);
        
        //1. name 중복 flag 확인
        for(int i=0; i<clnt_cnt; i++){
            if(strcmp(name, clnt_name[i]) == 0){//이름 중복
                dup_flag = 1;
                break;
            }
        }

        //2. name 중복 여부에 따라 추가할지, 강퇴시킬지 결정
        if(dup_flag){
            write(clnt_sock, duplicated_name_msg, sizeof(duplicated_name_msg));
            close(clnt_sock);
			printf("client %d tried duplicated name.\n", clnt_sock);
        }
        else{
            clnt_name[clnt_cnt] = (char*)malloc(sizeof(name));
            sprintf(clnt_name[clnt_cnt], "%s", name);
            write(clnt_sock, "welcome!\n", 10);
			
			fp = fopen("ClientDB.txt", "a");
			if(fp == NULL)
				error_handling("database file open error");
			
			//get client's age
			ages_len = read(clnt_sock, ages, sizeof(ages));

			//get client's country
			country_len = read(clnt_sock, country, sizeof(country));

			//get client's agreement
			agree_len = read(clnt_sock, agree, 1);
			printf("%d\n", agree_len);
			char temp = agree[0];
			agree[0] = '\0';
			agree[0] = temp;
			printf("%c\n", temp);
			          
			clnt_socks[clnt_cnt++] = clnt_sock;

			char client_information[BUF_SIZE] = {0,};
			sprintf(client_information, "%c\t%d\t%s\t%s\t%s\n", temp, clnt_sock, name, ages, country);
			fputs(client_information, fp);
			printf("%s", client_information);

			fclose(fp);

	        //client마다 전송, 수신 thread 생성
			pthread_create(&t_id1, NULL, handle_in, (void*)&clnt_sock);
			pthread_create(&t_id2, NULL, handle_out, (void*)&clnt_sock);

        	//thread 종료시, client 자원 해제
			pthread_detach(t_id1);
			pthread_detach(t_id2);
		
			printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
		}
		pthread_mutex_unlock(&mutx);
	}

	close(serv_sock);
    free_naming();
	return 0;
}

void *handle_in(void *arg){
	int clnt_sock = *((int*)arg);
	int msg_len = 0;
	char request_info[BUF_SIZE+BUF_SIZE];
	
	//semd all
	while((msg_len = read(clnt_sock, msg, sizeof(msg)))!=0){
		if(msg_len == -1)
			return (void*)-1;

		pthread_mutex_lock(&mutx);

		//정보 요청
		if(msg[0] == '[' && msg[msg_len-2] == ']'){
			sprintf(request_info, "%d REQUEST INFORMATION ABOUT %s", clnt_sock ,msg);
			msg[msg_len - 1] = '\0';
			strcpy(echo_msg, request_info);
			GiveClientInfo(clnt_sock, msg);
		}
		
		//일반 chat
		else{
			str_len = msg_len;
			strcpy(echo_msg, msg);
			printf("%s", echo_msg);
		}

		pthread_mutex_unlock(&mutx);
	}
	
	// remove disconnected client
	pthread_mutex_lock(&mutx);	
	for(int i=0; i<clnt_cnt; i++){
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

void *handle_out(void *arg){
	while(1){
		pthread_mutex_lock(&mutx);
		if(str_len != 0){
			for(int i=0; i<clnt_cnt; i++)
				write(clnt_socks[i], echo_msg, str_len);
			str_len = 0;
			memset(msg, 0, sizeof(msg));
		}		
		pthread_mutex_unlock(&mutx);
	}
	return NULL;
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

void GiveClientInfo(int requestclntfd, char* req_name) {
    FILE* fp = fopen("ClientDB.txt", "r");
    if (fp == NULL) {
        printf("Failed to open DB txt file\n");
        return;
    }
	
	char line[BUF_SIZE];
	int found = 0; // flag to find the name

    while (fgets(line, sizeof(line), fp)) {
        int temp;
        sscanf(line, "%d", &temp); // temp 값 추출
        
        int clnt_sock;
        char name[NAME_SIZE];
        char ages[AGE_SIZE];
        char country[COUNTRY_SIZE];      
		char client_information[BUF_SIZE] = {0,};    

		sscanf(line, "%*d\t%d\t%s\t%s\t%s", &clnt_sock, name, ages, country); // 정보 추출
		if (strcmp(name, req_name) == 0) {
            found = 1; // 이름이 일치하는 클라이언트가 발견되었음을 표시
            if(temp == 0){
                sprintf(client_information, "%s refused to disclose personal information\n", name);
            }
            else if (temp == 1) {
                sprintf(client_information, "%c\t%d\t%s\t%s\t%s\n", temp, clnt_sock, name, ages, country);
            }
            write(requestclntfd, client_information, strlen(client_information));
            break; // 해당 이름의 클라이언트 정보를 찾았으므로 반복문을 종료
        }
    }
	if (!found) {
        char not_found_msg[] = "No such user exists\n";
        write(requestclntfd, not_found_msg, sizeof(not_found_msg));
    }
    fclose(fp);
}