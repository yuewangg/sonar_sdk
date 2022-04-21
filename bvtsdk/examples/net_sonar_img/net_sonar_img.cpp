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
#define DEST_PORT 8000   //端口号
#define DSET_IP_ADDRESS  "127.0.0.1 " // server文件所在PC的IP

using namespace std;


int UDPWrite(int sock_fd,char *ip,char *port,void *send_buf,int bufLen)
{
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
  addr_serv.sin_addr.s_addr = inet_addr(ip);
  addr_serv.sin_port = htons((unsigned short)atoi(port));
  len = sizeof(addr_serv);

  int send_num;

  send_num = sendto(sock_fd, send_buf, bufLen, 0, (struct sockaddr *)&addr_serv, len);

  if(send_num < 0)
  {
      perror("sendto error:");
      exit(1);
  }

  if(send_num == bufLen)
  {
    return 1;
  }
  else
  {
  	return 0;
  }
}


int main(int argc, char * argv[])
{
	
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
	if( file == NULL )
	{
		printf("BVTSonar_Create: failed\n");
		return 1;
	}
	
	BVTSonar_CreateFile(file, "son/out.son", son, "");
	BVTSonar_CreateFile(son, "son/work.son", son, "");


	// Request the first head
	BVTHead out_head = NULL;
	BVTSonar_GetHead(file, 0, &out_head);

    BVTImageGenerator ig = BVTImageGenerator_Create();
    BVTImageGenerator_SetHead(ig, head);
	
	////////////////////////////////////////////////
	// Now, let's go get some pings!
	int height;
	int width;
	int pings = -1;
	int oldpings = -1;
	unsigned long ping_time;
	string s = ";"; 
	string row = "row:"; 
	string col = "col"; 
	while(ENABLE_SONAR)
	{
		BVTPing ping = NULL;
		BVTMagImage img;
		ret = BVTHead_GetPing(head, -1, &ping);
		
		if(BVTHead_GetPingCount(head, &pings) != oldpings)
		{

		BVTHead_GetPing(head, pings, &ping);
		BVTImageGenerator_GetImageXY(ig, ping, &img);
		
		BVTMagImage_GetHeight(img, &height );
		BVTMagImage_GetWidth(img, &width) ; 
		stringstream ss;
		struct timeval time;
    	gettimeofday(&time, NULL);

		ping_time = (time.tv_sec*1000 + time.tv_usec/1000);

		ss<<pings<<s<<ping_time<<s<<row<<1<<s;
		char send_buf[65536] = {};
		
		int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
		for ( int i=1; i<height; i++)
		{
			for ( int j=1; j<width; j++)
			{

				unsigned short opixel;
				BVTMagImage_GetPixel(img,i,j,&opixel);
				ss<<opixel<<s;
			}
			if(i % 30 == 0)
			{
				
				strncpy(send_buf, ss.str().c_str(), ss.str().length() + 1);
				if(UDPWrite(sock_fd,"127.0.0.1 ","8000",send_buf,strlen(send_buf)) == 1)
				{
					printf("fasong chenggong");
				}
				
        		ss.str("");
        		if (ss.eof())
        		{
            		ss.clear();
        		}
				ss<<pings<<s<<ping_time<<s<<row<<(i+1)<<s;
			}
			
		}

		strncpy(send_buf, ss.str().c_str(), ss.str().length() + 1);
		if(UDPWrite(sock_fd,"127.0.0.1 ","8000",send_buf,strlen(send_buf)) == 1)
		{
			printf("fasong chenggong");
		}
				
        ss.str("");
        if (ss.eof())
        {
        	ss.clear();
        }	

		close(sock_fd);
		
    	

		BVTHead_PutPing(out_head, ping);
		oldpings = pings;
		}
		
		
		BVTPing_Destroy(ping);
		BVTMagImage_Destroy(img);
	}




	BVTSonar_Destroy(file);
	BVTSonar_Destroy(son);
	return 0;
}
