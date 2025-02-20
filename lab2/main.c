
#include <stdio.h>

int send_email(char* account, char* password, char* from_email, char* to_email, 
        char* subject, char* body, char* attachment_path);

int main(int argc, char** argv){
	char username[20];
	char password[50];
	char from_email[50];
	char to_email[50];
	printf("please input your email username (with \"@stu.pku.edu.cn\")\n");
	scanf("%s", username);
	printf("please input your email password \n");
	scanf("%s", password);
	printf("please input your email account (with \"@stu.pku.edu.cn\")\n");
	scanf("%s", from_email);
	printf("please input your target email account (with \"@yyyy.xxx\")\n");
	scanf("%s", to_email);
	send_email(username, password, from_email, to_email, "homework 2", "hello, world", "logo.jpg");	
	return 0;
}
