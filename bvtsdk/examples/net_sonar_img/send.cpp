#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
int main(int argc, char * argv[])
{
	//检查命令行参数是否匹配
	if(argc != 3)
	{
		printf("请传递对方的ip和端口号");
		return -1;
	}
	
	int port = atoi(argv[2]);//从命令行获取端口号
	if( port<1025 || port>65535 )//0~1024一般给系统使用，一共可以分配到65535
	{
		printf("端口号范围应为1025~65535");
		return -1;
	}
	
	//1 创建udp通信socket
	int udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(udp_socket_fd == -1)
	{
		perror("socket failed!\n");
		return -1;
	}
    
	//设置目的IP地址
    struct sockaddr_in dest_addr = {0};
    dest_addr.sin_family = AF_INET;//使用IPv4协议
    dest_addr.sin_port = htons(port);//设置接收方端口号
    dest_addr.sin_addr.s_addr = inet_addr(argv[1]); //设置接收方IP 
	
	
    char buf[1024] = {0};
	//2 循环发送数据
    while (1)
    {
		printf("Please input msg:");
		scanf("%s",buf);//从键盘输入获取要发送的消息
        sendto(udp_socket_fd, buf, strlen(buf), 0, (struct sockaddr *)&dest_addr,sizeof(dest_addr)); 
        if(strcmp(buf, "exit") == 0)
		{
			break;//退出循环
		}
		memset(buf,0,sizeof(buf));//清空存留消息
    }
	
	//3 关闭通信socket
    close(udp_socket_fd);
    return 0;
}