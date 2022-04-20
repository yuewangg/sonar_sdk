/*
 * NET Sonar Example
 * Connect to a networked device, do a ping, and save it to a .son file
 * Note: Copy dll files from lib folder to output folder before running project.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <bvt_sdk.h>
#include <iostream>
#include <sstream>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ENABLE_SONAR 1

using namespace std;

std::string num2str(int i)
{
    std::stringstream ss;
    ss<<i;
    return ss.str();
}



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

    char buf[2048] = {0};
	
	int ret;
	
	// Create a new BVTSonar Object
	BVTSonar son = BVTSonar_Create();
	if( son == NULL )
	{
		printf("BVTSonar_Create: failed\n");
		return 1;
	}

	// Open the first sonar
	ret = BVTSonar_Open(son, "NET", "192.168.1.45"); // default ip address
	if( ret != 0 )
	{
		printf("BVTSonar_Open: ret=%d\n", ret);
		return 1;
	}

	// Make sure we have the right number of heads
	int heads;
	BVTSonar_GetHeadCount(son, &heads);
	printf("BVTSonar_GetHeadCount: %d\n", heads);

	// Get the first head
	BVTHead head = NULL;
	ret = BVTSonar_GetHead(son, 0, &head);
	if( ret != 0 )
	{
        // some sonars start at head 1 instead of zero...
        ret = BVTSonar_GetHead(son, 1, &head);
        if( ret != 0 )
        {
            printf("BVTSonar_GetHead: ret=%d\n", ret);
            return 1;

        }
	}
	
	// SetuP
	BVTHead_SetRange(head, 1, 40);
	//BVTHead_SetSoundSpeed(head,1500);
	//BVTHead_SetGainAdjustment(head,0);
	//BVTHead_SetTVGSlope(head,0);
	////////////////////////////////////////////////
	// Now, Create a file to save some pings to
	BVTSonar file = BVTSonar_Create();
	BVTSonar feedback = BVTSonar_Create();
	if( file == NULL )
	{
		printf("BVTSonar_Create: failed\n");
		return 1;
	}
		if( feedback == NULL )
	{
		printf("BVTSonar_Create: failed\n");
		return 1;
	}
	
	BVTSonar_CreateFile(file, "son/out.son", son, "");
	BVTSonar_CreateFile(feedback, "son/reget.son", son, "");
	BVTSonar_CreateFile(son, "son/work.son", son, "");


	// Request the first head
	BVTHead out_head = NULL;
	BVTHead re_head = NULL;
	BVTSonar_GetHead(file, 0, &out_head);
	BVTSonar_GetHead(feedback, 0, &re_head); 

    BVTImageGenerator ig = BVTImageGenerator_Create();
    BVTImageGenerator_SetHead(ig, head);
	
	////////////////////////////////////////////////
	// Now, let's go get some pings!
	const char* img_filename=NULL;
	BVTPing ping = NULL;
	BVTPing reping = NULL;
	BVTMagImage img;
	BVTMagImage reimg;
	double bearing;
	double range;
	while(ENABLE_SONAR)
	{
		BVTHead_GetPing(head, -1, &ping);
		BVTImageGenerator_GetImageXY(ig, ping, &img);

		int pings = -1;
		BVTHead_GetPingCount(head, &pings);
		std::string num;
		num=num2str(pings);
		std::string img_name = "img/0_" + num + ".pgm";
		img_filename = img_name.c_str();
		// Save it to a PGM (PortableGreyMap)
		BVTMagImage_SavePGM(img, img_filename);
		BVTHead_PutPing(out_head, ping);
		if(0)
		{
			BVTHead_GetPing(head, -1, &reping);   //-1重新设置为反馈ping值
			BVTImageGenerator_GetImageXY(ig, reping, &reimg);
			BVTMagImage_GetPixelRelativeBearing ( img, 222, 222, &bearing); //222重新设置为反馈像素值
			BVTMagImage_GetPixelRange ( img, 222, 222, &range);
			BVTHead_PutPing(re_head, reping);
		}
		
        sendto(udp_socket_fd, buf, strlen(buf), 0, (struct sockaddr *)&dest_addr,sizeof(dest_addr)); 
		memset(buf,0,sizeof(buf));//清空存留消息		
	}

    close(udp_socket_fd);
	BVTPing_Destroy(reping);
	BVTPing_Destroy(ping);
	BVTSonar_Destroy(feedback);
	BVTSonar_Destroy(file);
	BVTSonar_Destroy(son);
	return 0;
}
