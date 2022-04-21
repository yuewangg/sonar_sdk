/*
 * File Sonar Example
 * Demonstrate opening a file, accessing a head, and retriving a ping.
 * The ping is then processed into an image and saved to a file.
 * Finally, a colormap is loaded and the image is colormapped.
 * Note: Copy dll files from lib folder to output folder before running project.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <bvt_sdk.h>
#include <iostream>
#include <sstream>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;


int UDPWrite(int sock_fd,char *ip,char *port,void* buf,int bufLen){
struct sockaddr_in to_addr;

memset(&to_addr,0,sizeof(to_addr));
to_addr.sin_family=AF_INET;
to_addr.sin_port=htons((unsigned short)atoi(port));
if(!inet_aton(ip,&(to_addr.sin_addr)))
return -1;
if(sendto(sock_fd,buf,bufLen,0,(struct sockaddr*)&to_addr,sizeof(to_addr))==bufLen)
return 1;
else
{
// xErrorMsg("UDPWrite:sendto.");
return -1;
}
}

#define DEST_PORT 8000   //端口号
#define DSET_IP_ADDRESS  "127.0.0.1 " // server文件所在PC的IP

char DataFile[] = "../../data/0330Data.son";

int main( int argc, char *argv[] )
{
	int ret;
	// Create a new BVTSonar Object
	BVTSonar son = BVTSonar_Create();
	if( son == NULL )
	{
		printf("BVTSonar_Create: failed\n");
		return 1;
	}

	// Open the sonar
    if ( argc == 2 )
        strcpy( DataFile, argv[1] );

	ret = BVTSonar_Open(son, "FILE", DataFile);
	if( ret != 0 )
	{
		printf("BVTSonar_Open: ret=%d\n", ret);
		return 1;
	}

	// Make sure we have the right number of heads
	int heads = -1;
	BVTSonar_GetHeadCount(son, &heads);
	printf("BVTSonar_GetHeadCount: %d\n", heads);


	// Get the first head
	BVTHead head = NULL;
	ret = BVTSonar_GetHead(son, 0, &head);
	if( ret != 0 )
	{
		printf("BVTSonar_GetHead: ret=%d\n", ret);
		return 1;
	}
	
	// Check the ping count
	int pings = -1;
	BVTHead_GetPingCount(head, &pings);
	printf("BVTHead_GetPingCount: %d\n", pings);

    // Check the min and max range in this file
    float min_range, max_range;
    BVTHead_GetMinimumRange(head, &min_range);
    BVTHead_GetMaximumRange(head, &max_range);
    printf("BVTHead_GetMinimumRange: %0.2f\n", min_range );
    printf("BVTHead_GetMaximumRange: %0.2f\n", max_range );
	

    BVTImageGenerator ig = BVTImageGenerator_Create();
    BVTImageGenerator_SetHead(ig, head);
    // Now, get a ping!
	BVTPing ping = NULL;
	int height;
	int width;
	unsigned long ping_time;
	string s = ";"; 
	string row = "row:"; 
	for(int a = 0; a < 10; a = a + 1)
	{

		BVTMagImage img;
		ret = BVTHead_GetPing(head, a, &ping);

		BVTImageGenerator_GetImageXY(ig, ping, &img);
		
		BVTMagImage_GetHeight(img, &height );
		BVTMagImage_GetWidth(img, &width) ; 
		stringstream ss;
		struct timeval time;
    	gettimeofday(&time, NULL);

		ping_time = (time.tv_sec*1000 + time.tv_usec/1000);

		ss<<pings<<s<<ping_time<<s<<row<<1<<s;

		std::string str;
		int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
		for ( int i=1; i<height; i++)
		{
			for ( int j=1; j<width; j++)
			{

				unsigned short opixel;
				BVTMagImage_GetPixel(img,i,j,&opixel);
				ss<<opixel<<s;
				
			}
				/* socket文件描述符 */
				int sock_fd;

				/* 建立udp socket */
				sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
				if(sock_fd < 0)
				{
					perror("socket");
					exit(1);
				}

				/* 设置address */
				struct sockaddr_in addr_serv;
				int len;
				memset(&addr_serv, 0, sizeof(addr_serv));
				addr_serv.sin_family = AF_INET;
				addr_serv.sin_addr.s_addr = inet_addr(DSET_IP_ADDRESS);
				addr_serv.sin_port = htons(DEST_PORT);
				len = sizeof(addr_serv);
				int send_num;

				char send_buf[65536] = {};

				strncpy(send_buf, ss.str().c_str(), ss.str().length() + 1);

				send_num = sendto(sock_fd, send_buf, strlen(send_buf), 0, (struct sockaddr *)&addr_serv, len);

				if(send_num < 0)
				{
					perror("sendto error:");
					exit(1);
				}


				close(sock_fd);
				
        		ss.str("");
        		if (ss.eof())
        		{
            		ss.clear();
        		}
				ss<<pings<<s<<ping_time<<s<<row<<(i+1)<<s;
			
			
		}


				/* 建立udp socket */
		sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
		if(sock_fd < 0)
		{
			perror("socket");
			exit(1);
		}

				/* 设置address */
		struct sockaddr_in addr_serv;
		int len;
		memset(&addr_serv, 0, sizeof(addr_serv));
		addr_serv.sin_family = AF_INET;
		addr_serv.sin_addr.s_addr = inet_addr(DSET_IP_ADDRESS);
		addr_serv.sin_port = htons(DEST_PORT);
		len = sizeof(addr_serv);
		int send_num;

		char send_buf[65536] = {};

		strncpy(send_buf, ss.str().c_str(), ss.str().length() + 1);

		send_num = sendto(sock_fd, send_buf, strlen(send_buf), 0, (struct sockaddr *)&addr_serv, len);

		if(send_num < 0)
		{
			perror("sendto error:");
			exit(1);
		}


		close(sock_fd);
		BVTMagImage_Destroy(img);

	}
	BVTPing_Destroy(ping);
	BVTPing_Destroy(ping);
	BVTSonar_Destroy(son);
	return 0;
}
