#include <sys/socket.h>     // 提供 socket 的建立、綁定和通訊功能
#include <netinet/in.h>     // 定義網絡協議的結構，例如 sockaddr_in
#include <string.h>         // 提供 bzero 函數來清空內存
#include <stdio.h>          // 標準輸入輸出
#include <stdlib.h>         // 提供 exit 函數
#include <arpa/inet.h>      // 提供 inet_addr 函數來處理 IP 地址
#include <unistd.h>
#include <pthread.h>

#define BufSize 256
#define MAX_CLIENTS 2

int client_sockets[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(const char* message, const char* name, int csock){
	pthread_mutex_lock(&clients_mutex);
	for(int i = 0 ; i < MAX_CLIENTS ; i++){
		if(client_sockets[i] != 0 && client_sockets[i] != csock){
			/*char whosay[35] = ""; */
			/*strcat(whosay, name);*/
			/*strcat(whosay, ": ");*/
			/*send(client_sockets[i], whosay, strlen(whosay), 0);*/
			send(client_sockets[i], message, strlen(message), 0);
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *client_socket){
	int csock = *(int *)client_socket;
	char name[16];
	int nameSize = recv(csock, name, sizeof(name), 0);
	char *newline = strchr(name, '\n');
	if(newline){
		*newline = '\0';
	}
	printf("%s enter the chatroom\n", name);

	char buf[BufSize]; 
	recv(csock, buf, sizeof(buf), 0);
	while(1){
		memset(buf, 0, sizeof(buf));
		int readSize = recv(csock, buf, sizeof(buf), 0);
		if(!readSize) break;

		buf[readSize] = '\0';
		printf("Read Message from client(%s): %s\n", name, buf);
		broadcast_message(buf, name, csock);
		
	}

	close(csock);
	pthread_mutex_lock(&clients_mutex);
	for(int i = 0 ; i < MAX_CLIENTS ; i++){
		if(client_sockets[i] == csock){
			client_sockets[i] = 0;
			break;
		}
	}
	pthread_mutex_unlock(&clients_mutex);

	printf("client(%d) %s leave the chatroom\n", csock, name);
	return NULL;
}

int main(void){
	struct sockaddr_in server, client;
	int sock, csock, readSize, addressSize;
    char buf[BufSize];
	bzero(&server, sizeof(server));
			
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(1111);

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if(sock == -1){
		printf("Fail to create a socket\n");
	}

	int err = bind(sock, (struct sockaddr*)&server, sizeof(server));
	if(err == -1){
		printf("Bind error");
	}

	listen(sock, 5);

	printf("server start to listen\n");

	addressSize = sizeof(client);
	/*csock = accept(sock, (struct sockaddr*)&client, &addressSize);*/

	while(1){
		int new_socket = accept(sock, (struct sockaddr*)&client, &addressSize);
		printf("New client connected.\n");

		pthread_mutex_lock(&clients_mutex);
		int add_flag = 0;
		for(int i = 0 ; i < MAX_CLIENTS ; i++){
			if(client_sockets[i] == 0){
				client_sockets[i] = new_socket;
				add_flag = 1;
				break;
			}
		}
		pthread_mutex_unlock(&clients_mutex);
		if(!add_flag){
			printf("Max client reached. Can not add more client\n");
			close(new_socket);
			continue;
		}

		pthread_t thread_id;
		pthread_create(&thread_id, NULL, handle_client, &new_socket);
		pthread_detach(thread_id);
	}

	printf("Client has closed the connection.\n");
	close(sock);

    exit(0);
}
