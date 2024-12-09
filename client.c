#include <sys/socket.h>     // 提供 socket 函數
#include <netinet/in.h>     // 提供 sockaddr_in 結構
#include <string.h>         // 提供字符串處理函數
#include <stdio.h>          // 提供標準輸入輸出
#include <stdlib.h>         // 提供標準庫函數
#include <arpa/inet.h>      // 提供 inet_addr 函數
#include <unistd.h>
#include <pthread.h>
#include <gtk/gtk.h>

#define BufSize 256

void *read_message(void *server_socket){
	int sock = *(int *)server_socket;
	char buf[BufSize];
	while(1){
		memset(buf, 0, sizeof(buf));
		recv(sock, buf, sizeof(buf), 0);
		printf("%s\n", buf);
	}
	return NULL;
}

int main(void){
    struct sockaddr_in server; // 定義服務器的地址結構
    int sock, readSize;
    char buf[BufSize] = "TEST TCP\n";
    bzero(&server, sizeof(server)); // 清空服務器地址結構

    // 設置服務器的地址屬性
    server.sin_family = AF_INET;            // 指定地址族為 IPv4
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // 指定服務器的 IP 地址
    server.sin_port = htons(1111);          // 指定端口，使用 htons 轉換字節序

    // 創建 Socket
    sock = socket(PF_INET, SOCK_STREAM, 0);

    // 連接到服務器
	int connect_status = connect(sock, (struct sockaddr*)&server, sizeof(server));
	printf("%d\n", connect_status);
    if(connect_status < 0){
		printf("connect error!\n");
		close(sock);
		return 0;
	}

	printf("Please enter yout name:");
	char name[16];
	fgets(name, sizeof(name), stdin);
	send(sock, name, sizeof(name), 0);
	send(sock, buf, sizeof(buf), 0);

	pthread_t thread_id;
	pthread_create(&thread_id, NULL, read_message, &sock);
	pthread_detach(thread_id);
    
    // 進入通信循環，與服務器交互
	while(fgets(buf, 255, stdin)){
        send(sock, buf, sizeof(buf), 0); // 將消息發送到服務器
        printf("Send Message: %s", buf);
        /*readSize = recv(sock, buf, sizeof(buf), 0); // 從服務器接收消息*/
        /*printf("%s\n", buf);*/
	}

    /*buf[0] = '\0'; // 發送空消息以通知服務器關閉連接*/
    /*send(sock, buf, 0, 0);*/
    printf("Close connection!\n");
    close(sock); // 關閉客戶端的 Socket
}
