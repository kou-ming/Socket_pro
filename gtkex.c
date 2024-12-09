#include<gtk/gtk.h>
#include <sys/socket.h>     // 提供 socket 函數
#include <netinet/in.h>     // 提供 sockaddr_in 結構
#include <string.h>         // 提供字符串處理函數
#include <stdio.h>          // 提供標準輸入輸出
#include <stdlib.h>         // 提供標準庫函數
#include <arpa/inet.h>      // 提供 inet_addr 函數
#include <unistd.h>
#include <pthread.h>
#include<stdio.h>
#include<stdint.h>

#define BufSize 256

const double BUTTON_OFSX=100;
const double BUTTON_OFSY=42;

GtkWidget *chat_box;
int ssock;

static char *wrap_text(const char *text, int line_length) {
    int len = strlen(text);
    char *wrapped_text = g_malloc(len + len / line_length + 2);
    int j = 0;

    for (int i = 0; i < len; i++) {
        wrapped_text[j++] = text[i];
        if ((i + 1) % line_length == 0 && (i + 1) < len) {
            wrapped_text[j++] = '\n'; // 插入換行符號
        }
    }
    wrapped_text[j] = '\0';
    return wrapped_text;
}

void *read_message(void *server_socket){
	int sock = *(int *)server_socket;
	char buf[BufSize];
	while(1){
		memset(buf, 0, sizeof(buf));
		/*char whosay[35];*/
		/*recv(sock, whosay, sizeof(whosay), 0);*/
		/*printf("%s\n", whosay);*/

		recv(sock, buf, sizeof(buf), 0);
		printf("%s\n", buf);

		char all_line[BufSize + 35] = "";
		/*strcat(all_line, whosay);*/
		strcat(all_line, buf);
		char *formatted_text = wrap_text(all_line, 35);

		GtkWidget *label_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_widget_set_halign(label_container, GTK_ALIGN_START); // 靠右對齊
		gtk_widget_set_margin_bottom(label_container, 5);
		gtk_widget_set_margin_top(label_container, 5);

		GtkStyleContext *context = gtk_widget_get_style_context(label_container);
		gtk_style_context_add_class(context, "message-container");

		GtkWidget *new_label = gtk_label_new(formatted_text);
		gtk_widget_set_halign(new_label, GTK_ALIGN_END); // 靠右對齊
		gtk_label_set_wrap_mode(GTK_LABEL(new_label), PANGO_WRAP_WORD_CHAR);

		gtk_box_append(GTK_BOX(label_container), new_label);
		gtk_box_append(GTK_BOX(chat_box), label_container);

		gtk_widget_show(new_label);
	}
	return NULL;
}


void *send_message(void *server_socket){
	int sock = *(int *)server_socket;
	char buf[BufSize];
	while(fgets(buf, 255, stdin)){
        send(sock, buf, sizeof(buf), 0); // 將消息發送到服務器
        printf("Send Message: %s", buf);
	}
	return NULL;
}


//callback
static void buttonClicked(GtkButton* button, intptr_t buttonID){
	printf("button %ld clicked\n", buttonID);
	GtkWidget *entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "entry"));

	const gchar *text = gtk_editable_get_text(GTK_EDITABLE(entry));
	char buf[BufSize];
	snprintf(buf, sizeof(buf), "%s", text);
	/*scanf(text, "%s", buf);*/
	send(ssock, buf, sizeof(buf), 0); // 將消息發送到服務器
	printf("Send Message: %s", buf);

	char *formatted_text = wrap_text(text, 35);

	if(g_strcmp0(text, "") == 0){
		return;
	}

	// 創建一個容器用於包裹標籤
    GtkWidget *label_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(label_container, GTK_ALIGN_END); // 靠右對齊
    gtk_widget_set_margin_bottom(label_container, 5);
    gtk_widget_set_margin_top(label_container, 5);

    // 為容器添加 CSS 樣式
    GtkStyleContext *context = gtk_widget_get_style_context(label_container);
    gtk_style_context_add_class(context, "message-container");

	GtkWidget *new_label = gtk_label_new(formatted_text);
	gtk_widget_set_halign(new_label, GTK_ALIGN_END); // 靠右對齊
    /*gtk_label_set_wrap(GTK_LABEL(new_label), TRUE);  // 啟用換行*/
	gtk_label_set_wrap_mode(GTK_LABEL(new_label), PANGO_WRAP_WORD_CHAR);
    /*gtk_widget_set_size_request(new_label, 100, -1); // 設置寬度限制，無高度限制*/

	gtk_box_append(GTK_BOX(label_container), new_label);
	gtk_box_append(GTK_BOX(chat_box), label_container);


	gtk_editable_set_text(GTK_EDITABLE(entry), "");

	/*gtk_entry_set_text(GTK_ENTRY(entry), "");*/
	gtk_widget_show(new_label);
}

static void windowClose(GtkWidget* self, int* isEndPtr){
	*isEndPtr=1;
}

//----
//main
int main(){
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
	ssock = sock;

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
	gtk_init();
	GtkWidget* window= gtk_window_new();
	gtk_window_set_title(GTK_WINDOW(window), name);
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 750);
	gtk_window_set_resizable(GTK_WINDOW(window), 0); //讓視窗不可變更大小
	int isEnd=0;
	g_signal_connect(window, "destroy", G_CALLBACK(windowClose), &isEnd);

	GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        ".message-container {"
        "  background-color: #e0f7fa;"
        "  border-radius: 10px;"
        "  padding: 5px;"
        "}",
        -1);

    // 將 CSS 提供者應用到畫面
    GtkStyleContext *screen_context = gtk_widget_get_style_context(window);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

	/*GtkWidget* fixedContainer= gtk_fixed_new();*/
	/*gtk_window_set_child(GTK_WINDOW(window), fixedContainer);*/
	GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	gtk_window_set_child(GTK_WINDOW(window), main_box);

	GtkWidget *scroll_window = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll_window, TRUE);
    gtk_box_append(GTK_BOX(main_box), scroll_window);

	chat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_widget_set_margin_start(chat_box, 10);
    gtk_widget_set_margin_end(chat_box, 10);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_window), chat_box);


	// Create bottom_box
	GtkWidget *bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_margin_bottom(bottom_box, 10);
    gtk_box_append(GTK_BOX(main_box), bottom_box);

	GtkWidget *entry = gtk_entry_new();
	gtk_widget_set_hexpand(entry, TRUE);
    gtk_box_append(GTK_BOX(bottom_box), entry);

	//建立各種按鈕
	GtkWidget* SendButton= gtk_button_new_with_label("Send");
	g_signal_connect(SendButton, "clicked", G_CALLBACK(buttonClicked), (void*)1);
	gtk_box_append(GTK_BOX(bottom_box), SendButton);

	g_object_set_data(G_OBJECT(SendButton), "entry", entry);
    g_object_set_data(G_OBJECT(SendButton), "chat_box", chat_box);
	/*gtk_fixed_put(GTK_FIXED(fixedContainer), pushButton, 320,660);*/

	gtk_window_present(GTK_WINDOW(window));

	send(sock, buf, sizeof(buf), 0);

	pthread_t read_thread_id;
	pthread_create(&read_thread_id, NULL, read_message, &sock);
	pthread_detach(read_thread_id);

	/*pthread_t send_thread_id;*/
	/*pthread_create(&send_thread_id, NULL, send_message, &sock);*/
	/*pthread_detach(send_thread_id);*/

	while(!isEnd){
		g_main_context_iteration(NULL,TRUE);
	}

    printf("Close connection!\n");
    close(sock); // 關閉客戶端的 Socket
	return 0;
}
