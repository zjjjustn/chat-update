#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>
#include <sqlite3.h>
#include<pthread.h>
 
#include <sys/stat.h>
#include <fcntl.h>
 
#define PORT  9999
#define SIZE 20
#define CTRL 30
int flag = 0;
int offline = 0; 
struct Usr
{
	char name[SIZE];
	int socket;
	int flag;
};
 
struct Msg
{
	struct Usr usr[SIZE];
	char msg[1024];
	char buf[1024];
	char name[SIZE];
	char fromname[SIZE];
	char toname[SIZE];
	char password[SIZE];
	int cmd;
	int filesize;
	int flag;    //0普通，1超级
};
 
struct Msg tmp;
 

void display_line()
{
	system("clear");
	printf("\n");
	printf("\n");
	printf("\n");
	printf("\n");
	printf("\n");
	printf("\n");
}
 

void maininterface()
{
	display_line();
    printf("\033[%dC_______________________________\n", CTRL);
    printf("\033[%dC|                             |\n", CTRL);
    printf("\033[%dC|                             |\n", CTRL);
    printf("\033[%dC|           1.注册            |\n", CTRL);
    printf("\033[%dC|                             |\n", CTRL);
    printf("\033[%dC|           2.登陆            |\n", CTRL);
    printf("\033[%dC|                             |\n", CTRL);
    printf("\033[%dC|           3.退出            |\n", CTRL);
    printf("\033[%dC|                             |\n", CTRL);
    printf("\033[%dC|           输入指令          |\n", CTRL);
    printf("\033[%dC|_____________________________|\n", CTRL);
}
 

void cominterface(struct Msg *msg)
{
	display_line();
	printf("\t\t-.- %s:\n", msg->name);
	printf("\n");

    printf("\033[%dC_________________________________\n", CTRL);
    printf("\033[%dC|                               |\n", CTRL);
    printf("\033[%dC|           1.查看在线用户      |\n", CTRL);
    printf("\033[%dC|           2.群发消息          |\n", CTRL);
    printf("\033[%dC|           3.私发消息          |\n", CTRL);
    printf("\033[%dC|           4.删除聊天          |\n", CTRL);
    printf("\033[%dC|           5.查看聊天          |\n", CTRL);
    printf("\033[%dC|           6.上传文件          |\n", CTRL);
    printf("\033[%dC|           7.退出账号          |\n", CTRL);
	printf("\033[%dC|           8.下载文件          |\n", CTRL);
    printf("\033[%dC|                               |\n", CTRL);
    printf("\033[%dC|           输入指令            |\n", CTRL);
    printf("\033[%dC|_______________________________|\n", CTRL);
	
}
 

void supinterface(struct Msg *msg)
{
	display_line();
	printf("\t\t-.- %s:\n", msg->name);
	printf("\n");

    printf("\033[%dC_________________________________\n", CTRL);
    printf("\033[%dC|                               |\n", CTRL);
    printf("\033[%dC|           1.查看在线用户      |\n", CTRL);
    printf("\033[%dC|           2.群发消息          |\n", CTRL);
    printf("\033[%dC|           3.私发消息          |\n", CTRL);
    printf("\033[%dC|           4.删除聊天          |\n", CTRL);
    printf("\033[%dC|           5.查看聊天          |\n", CTRL);
    printf("\033[%dC|           6.设置禁言          |\n", CTRL);
    printf("\033[%dC|           7.解除禁言          |\n", CTRL);
    printf("\033[%dC|           8.退出账号          |\n", CTRL);
    printf("\033[%dC|                               |\n", CTRL);
    printf("\033[%dC|           输入指令 :          |\n", CTRL);
    printf("\033[%dC|_______________________________|\n", CTRL);
	
}
 

void see_online(int socketfd, struct Msg *msg)
{
	msg->cmd = 1;
	write(socketfd, msg, sizeof(struct Msg));	
	return;
}
 
//保存一条聊天记录
void insert_mysql(struct Msg *msg)
{
	sqlite3 * database;
	int ret = sqlite3_open("person.db", &database);
	if (ret != SQLITE_OK)
	{
		printf ("打开数据库失败\n");
		return ;
	}
	
	//获取系统当前时间
	time_t timep;  
    char s[30];   
    time(&timep);  
    strcpy(s,ctime(&timep));
	int count = strlen(s);
	s[count-1] = '\0';
	
	char *errmsg = NULL;
	char *sql = "create table if not exists person(time TEXT,fromname TEXT,toname TEXT,word TEXT)";
	ret = sqlite3_exec(database, sql, NULL, NULL, &errmsg);
	if (ret != SQLITE_OK)
	{
		printf ("用户表创建失败：%s\n", errmsg);
		return;
	}
	
	char buf[10000];
	sprintf(buf, "insert into person values('%s','%s','%s','%s')",s,msg->fromname, msg->toname,msg->msg);
	ret = sqlite3_exec(database, buf, NULL, NULL, &errmsg);
	if (ret != SQLITE_OK)
	{
		printf ("添加聊天记录失败：%s\n", errmsg);
		return ;
	}
	
	sqlite3_close(database);
	return ;
}
 
//群聊
void chat_group(int socketfd, struct Msg *msg)
{
	msg->cmd = 2;
	printf ("\t\t请输入要发送的内容: ");
	fgets(msg->msg, 1024, stdin);
	int len = strlen(msg->msg);
	msg->msg[len-1] = '\0';
	strcpy(msg->fromname, msg->name);
	strcpy(msg->toname, "all");
	insert_mysql(msg);
	
	write (socketfd, msg, sizeof(struct Msg));

}
 
//私聊
void  chat_private(int socketfd, struct Msg *msg)
{
	msg->cmd = 3;
	
	printf ("\t\t请输入要发送的对象名称: \n");
	fgets(msg->toname, SIZE, stdin);
	int len = strlen(msg->toname);
	msg->toname[len-1] = '\0';
	printf ("\t\t请输入要发送的内容: \n");
	fgets(msg->msg, 1024, stdin);
	len = strlen(msg->msg);
	msg->msg[len-1] = '\0';
	strcpy(msg->fromname, msg->name);
	
	insert_mysql(msg);
	
	write(socketfd, msg, sizeof(struct Msg));
}
 
//删除聊天记录
void del_personsql()
{
	sqlite3 * database;
	int ret = sqlite3_open("person.db", &database);
	if (ret != SQLITE_OK)
	{
		printf ("打开数据库失败\n");
		return;
	}
	
	char *errmsg = NULL;
	char *sql = "create table if not exists person(time TEXT,fromname TEXT,toname TEXT,word TEXT)";
	ret = sqlite3_exec(database, sql, NULL, NULL, &errmsg);
	if (ret != SQLITE_OK)
	{
		printf ("用户表创建失败：%s\n", errmsg);
		return;
	}
	
	char *buf = "drop table person";
	ret = sqlite3_exec(database, buf, NULL, NULL, &errmsg);
	if (ret != SQLITE_OK)
	{
		printf ("删除聊天记录失败：%s\n", errmsg);
		return;
	}
	
	sqlite3_close(database);
	display_line();
	printf("\t\t删除成功\n");
	printf("\t\t按ENTER键返回\n");
	while(getchar() != '\n');
	return;
}
 
//查看聊天记录
void see_record()
{
	sqlite3 * database;
	int ret = sqlite3_open("person.db", &database);
	if (ret != SQLITE_OK)
	{
		printf ("打开数据库失败\n");
		return ;
	}
	
	char *errmsg = NULL;
	char *sql = "create table if not exists person(time TEXT,fromname TEXT,toname TEXT,word TEXT)";
	ret = sqlite3_exec(database, sql, NULL, NULL, &errmsg);
	if (ret != SQLITE_OK)
	{
		printf ("用户表创建失败：%s\n", errmsg);
		return;
	}
	
	char **resultp = NULL;
	int nrow, ncolumn;
	char *buf = "select * from person";
	ret = sqlite3_get_table(database, buf, &resultp, &nrow, &ncolumn, &errmsg);
	if (ret != SQLITE_OK)
	{
		printf ("数据库操作失败：%s\n", errmsg);
		return;
	}

	if(nrow == 0)
	{
		display_line();
		printf("\t\t没有聊天记录\n");
	}
	else
	{
		int i;
		system("clear");
		printf ("nrow = %d, ncolumn = %d\n", nrow, ncolumn);
		printf("\t\t%s",resultp[0]);
		printf("\t%12s",resultp[1]);
		printf("\t%8s",resultp[2]);
		printf("\t%s",resultp[3]);
		for (i = 4; i < (nrow+1)*ncolumn; i++)
		{
			if (i % ncolumn == 0)
				printf ("\n");
			printf ("%12s", resultp[i]);
		}
		printf ("\n");	
		
	}
		
	
	printf("\t\t按ENTER键返回\n");
	while(getchar() != '\n');
	
	sqlite3_free_table(resultp);
	
	sqlite3_close(database);
	return;
	
}
 

int file_size(char* filename)  
{  
    struct stat statbuf;  
    stat(filename,&statbuf);  
    int size=statbuf.st_size;  
  
    return size;  
}

//上传文件
void send_file(int socketfd, struct Msg *msg)
{
	msg->cmd = 6;
	
	strcpy(msg->fromname, msg->name);
	
	printf("\t\t请输入要发送的文件名：\n");
	fgets(msg->msg, 1024, stdin);
	int len = strlen(msg->msg);
	msg->msg[len-1] = '\0';
	
	int size = file_size(msg->msg);
	msg->filesize = size;
	write(socketfd, msg, sizeof(struct Msg));
	printf("\t\t正在上传，请稍后...\n");
	sleep(1);
	
	int fd = open(msg->msg, O_RDONLY, 0777);
	if(fd == -1)
	{
		perror("open");
		printf("文件传输失败\n");
	}
	
	char buf[65535];
	memset(buf, 0, 65535);
	
	int ret = read(fd, buf, size);
	if(ret == -1)
	{
		perror("read");
		return;
	}	
	write(socketfd, buf, ret);
	close(fd);
	printf("\t\t上传文件成功\n");
	sleep(1);
 
}
 
 
//设置禁言
void forbid_speak(int socketfd, struct Msg *msg)
{
	msg->cmd = 6;
	printf("\t\t请输入要禁言的对象:\n");
	fgets(msg->msg, SIZE, stdin);
	int len = strlen(msg->msg);
	msg->msg[len-1] = '\0';
	
	write(socketfd, msg, sizeof(struct Msg));
	
}
 
//解除禁言
void relieve_speak(int socketfd, struct Msg *msg)
{
	msg->cmd = 7;
	printf("\t\t请输入要解除禁言的对象:\n");
	fgets(msg->msg, SIZE, stdin);
	int len = strlen(msg->msg);
	msg->msg[len-1] = '\0';
	
	write(socketfd, msg, sizeof(struct Msg));
}
 
//下线
void off_line(int socketfd, struct Msg *msg)
{
	msg->cmd = 8;
	write(socketfd, msg, sizeof(struct Msg));
	display_line();
	printf("\t\t正在退出...\n");
}
 
//超级用户
void superusr(int socketfd, struct Msg *msg)
{
	char ch[SIZE];
	int x;
	while (1)
	{
		supinterface(msg);
		scanf("%s",ch);
		while(getchar() != '\n');
		
		switch(ch[0])
		{
			case '1':                   // 查看在线用户
				see_online(socketfd, msg);
				break;
			case '2':                   // 群发消息
				chat_group(socketfd, msg);
				break;
			case '3':                   // 发送私聊
				chat_private(socketfd, msg);
				break;
			case '4':                   // 删除聊天记录
				del_personsql();
				break;
			case '5':                   // 查看聊天记录
				see_record();
				break;
			case '6':                   // 设置禁言
				forbid_speak(socketfd, msg);
				break;
			case '7':                   // 解除禁言
				relieve_speak(socketfd, msg);
				break;
			case '8':                   // 退出当前账号
				off_line(socketfd, msg);
				return;			
			default: 
				display_line();
				printf("错误指令，请重新输入\n");
				sleep(1);
				break;
		}	
		
		
	}
}
 
//下载文件
void download_file(int socketfd, struct Msg *msg)
{
	msg->cmd = 9;
 
	printf("\t\t请输入要下载的文件名：\n");
	fgets(msg->msg, 1024, stdin);
	int len = strlen(msg->msg);
	msg->msg[len-1] = '\0';
 
	write(socketfd, msg, sizeof(struct Msg));
 
}
void fun(int socketfd, struct Msg *msg)
{
	printf("\n\t\t正在下载,请稍后...\n");
	int fd = open(msg->msg, O_RDWR | O_CREAT, 0777);
	if(fd == -1)
	{
		perror("open");
		printf("文件下载失败了\n");
	}
	
	int size = msg->filesize;
	char buf[65535];
	memset(buf, 0, 65535);
	
	int ret = read(socketfd, buf, size);
	if(ret == -1)
	{
		perror("read");
		return;
	}
	write(fd, buf, ret);
	close(fd);
	sleep(1);
	printf("\t\t文件下载完成\n");
}

//普通用户
void commonusr(int socketfd, struct Msg *msg)
{
	char ch[SIZE];
	int x;
	while (1)
	{
		cominterface(msg);
		fgets(ch, SIZE, stdin);
		if(offline == 1)
		{
			display_line();
			printf("\n\t\t请稍后...\n");
			off_line(socketfd, msg);
			return;
		}
		else
		{
			switch(ch[0])
			{
				case '1':                   // 查看在线用户
					see_online(socketfd, msg);
					break;
				case '2':                   // 群聊
					chat_group(socketfd, msg);
					break;
				case '3':                   // 私聊
					chat_private(socketfd, msg);
					break;
				case '4':                   // 删除聊天记录
					del_personsql();
					break;
				case '5':                   // 查看聊天记录
					see_record();
					break;
				case '6':                   // 上传文件
					send_file(socketfd, msg);
					break;
				case '7':                   // 退出当前账号
					off_line(socketfd, msg);
					return;
				case '8':
					download_file(socketfd,msg);
					break;
				default: 
					display_line();
					printf("错误指令，请重新输入\n");
					sleep(1);
					break;
			}
		}			
	}
}
 

void set_mysql()
{
	sqlite3 * database;
	
	int ret = sqlite3_open("person.db", &database);
	if (ret != SQLITE_OK)
	{
		printf ("打开数据库失败\n");
		return;
	}
 
	char *errmsg = NULL;
	char *sql = "create table if not exists person(time TEXT,fromname TEXT,toname TEXT,word TEXT)";
	ret = sqlite3_exec(database, sql, NULL, NULL, &errmsg);
	if (ret != SQLITE_OK)
	{
		printf ("聊天记录表创建失败：%s\n", errmsg);
		return;
	}
	sqlite3_close(database);
	return;
}
 

void reg(int socketfd)
{
	struct Msg msg;
	memset(&msg, 0, sizeof(msg));
	msg.cmd = 1;
	
	display_line();
	printf("\t\t请输入姓名：");
	scanf("%s",msg.name);
	while(getchar() != '\n');
 
	printf("\n");
	printf("\t\t请输入密码：");
	scanf("%s",msg.password);
	while(getchar() != '\n');
 
	write(socketfd, &msg, sizeof(msg));
	
	read(socketfd, &msg, sizeof(msg));
	if(msg.cmd == 0)
	{
		display_line();
		printf("\t\t注册失败，该用户已被注册\n");
	}
	if(msg.cmd == 1)
	{
		display_line();
		
		set_mysql();     
		printf("\t\t注册成功\n");
	}
	sleep(2);
	return;
}
 

void * readMsg(void *v)
 {
	 long socketfd = (long)v;
	 struct Msg msg;
	 while (1)
	 {
		 int i;
		 
		 int ret = read (socketfd, &msg, sizeof(msg));
		 switch (msg.cmd)
		 {
			 case 1:  
				
				printf("\n\t\t当前在线用户：\n");
				printf("\t\t");
				for(i=0; i<SIZE; i++)
				{
					printf("%-5s",msg.usr[i].name);
				}
				printf("\n");	
				break;
			 case 2:   
				printf ("\n%s给大家发了一条消息： %s\n", msg.name, msg.msg);
				insert_mysql(&msg);      
				break;
			 case 3:   
				printf ("\n%s给你发一条消息：%s\n", msg.fromname, msg.msg);
				insert_mysql(&msg);
				break;
			 case 6:   			
				printf("\n用户%s上传了一份文件%s\n", msg.fromname, msg.msg);	
				break;
			 case 7:
				break;
			 case 8:
				break;
			 case 1003:
				printf("\n\t\t您已被管理员禁言\n");	
			    break;
			 case 1004:
				printf("\n您已被管理员解除禁言\n");
			    break;
		 }	 
	 }
 }
 

void login(int socketfd)
{
	struct Msg msg;
	memset(&msg, 0, sizeof(msg));
	msg.cmd = 2;
	
	display_line();
	printf("\t\t请输入姓名：");
	scanf("%s",msg.name);
	while(getchar() != '\n');
	printf("\n");
	printf("\t\t请输入密码：");
	scanf("%s",msg.password);
	while(getchar() != '\n');
	write(socketfd, &msg, sizeof(msg));
	
	read(socketfd, &msg, sizeof(msg));
	
	printf("%d\n",msg.cmd);
	if(msg.cmd == 0)
	{
		
		display_line();
		printf("\t\t正在登陆...\n");
		sleep(2);
		display_line();
		printf("\t\t登陆成功\n");
		
		//启动一个线程专门用来接受聊天信息
		pthread_t id;
		pthread_create(&id, NULL, readMsg,  (void *)(intptr_t)socketfd);
        
		pthread_detach(id);                 
		
		offline = 0;
		if(msg.flag == 1)                   //超级用户
			superusr(socketfd, &msg);
		if(msg.flag == 0)                   //普通用户
			commonusr(socketfd, &msg);		
	}
	if(msg.cmd == 1)
	{
		display_line();
		printf("\t\t正在登陆...\n");
		sleep(2);
		display_line();
		printf("\t\t密码错误\n");
	}
	if(msg.cmd == 2)
	{
		display_line();
		printf("\t\t正在登陆...\n");
		sleep(2);
		display_line();
		printf("\t\t用户不存在，请先注册\n");
	}
	if(msg.cmd == 3)
	{
		display_line();
		printf("\t\t正在登陆...\n");
		sleep(2);
		display_line();
		printf("\t\t该用户已经登陆，请勿重复登陆\n");
	}
		
	sleep(1);
	return;
}
 

void ask_server(int socketfd)
{
	int x;
	while (1)
	{
		maininterface();
		scanf("%d",&x);
		while(getchar() != '\n');
		switch(x)
		{
			case 1:     
				reg(socketfd);
				break;
			case 2:    
				login(socketfd);
				break;
			case 3:     
				system("clear");
				return;
			default: 
				display_line();
				printf("\t\t错误指令，请重新输入\n");
				sleep(1);
				break;
		}	
	}
}
 
int main()
{	

	int socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketfd == -1)
	{
		perror ("socket");
		return -1;
	}
	
	
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family  = AF_INET;     
	addr.sin_port    = htons(PORT); 
	inet_aton("127.0.0.1",&(addr.sin_addr));
	

	int ret = connect(socketfd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret == -1)
	{
		perror ("connect");
		return -1;
	}
 
	printf ("成功连上服务器\n");
	
	ask_server(socketfd);
	
	close(socketfd);
	
	return 0;
}