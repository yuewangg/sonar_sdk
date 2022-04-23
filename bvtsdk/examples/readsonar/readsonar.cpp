//============================================================================
// Name        : sonarclient.cpp
// Author      : shgued
// Version     : 1.1
// Copyright   : open source
// Description : OpenCV experimentation in C++, transform video with UDP
// !! 标记重点关注
// 修正线程函数中recvfrom返回-1的bug
//============================================================================

#include <stdio.h>
#include <string>
#include <cstring>
#include <unistd.h> //read & write g++
#include <arpa/inet.h>
#include <bvt_sdk.h>

#define PKG_MAX_LENGTH 60012
#define PACKAGE_HEAD_LENGTH 7 //包头长度
#define MAX_PACKAGE_SIZE (1460*2)//最大数据包长度
#define MAX_PACKAGE_DATA_NUM (MAX_PACKAGE_SIZE-PACKAGE_HEAD_LENGTH) //除去包头最大数据量
#define ENABLE_SONAR 1
#define DEST_PORT "8000"   //端口号
#define DSET_IP_ADDRESS "127.0.0.1 " // server文件所在PC的IP

using namespace std;

unsigned char pkgData[PKG_MAX_LENGTH];

int UDPWrite(int sock_fd,char *ip,char *port,const void *send_buf,int bufLen)
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

void setPkgHead(unsigned int picIndex,unsigned char pkgIndex, unsigned int dataSize){
    pkgData[0] = picIndex&0xff;
    picIndex >>= 8;

    pkgData[1] = picIndex;

    pkgData[2] = pkgIndex;

    pkgData[3] = dataSize&0xff;
    dataSize >>= 8;
    pkgData[4] = dataSize&0xff;
    dataSize >>= 8;
    pkgData[5] = dataSize&0xff;
    dataSize >>= 8;
    pkgData[6] = dataSize&0xff;
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
	int height, width, picSize;

	int pings, oldpings = -1;
    int key,pkgCnt,cnt,sendNum,toSendDataNum,test;
    int i,k,sum,sendCnt = 0;;
    //struct ImgDataStr imgData;
    unsigned char *pPkg;
    unsigned short *pImg;
	unsigned long ping_time;

    unsigned short* bitBuffer;

	while(ENABLE_SONAR)
	{
		BVTPing ping = NULL;
		BVTMagImage img;
		ret = BVTHead_GetPing(head, -1, &ping);
		
		if(BVTHead_GetPingCount(head, &pings) != oldpings)
		{

		BVTImageGenerator_GetImageXY(ig, ping, &img);
		
		BVTMagImage_GetHeight(img, &height );
		BVTMagImage_GetWidth(img, &width) ; 

        BVTMagImage_GetBits(img, &bitBuffer);
		
		int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
        
        picSize = height*width;

		for(pkgCnt=0,sendNum=0;sendNum<picSize;pkgCnt++,sendNum += MAX_PACKAGE_DATA_NUM){
            if(sendNum + MAX_PACKAGE_DATA_NUM < picSize){
                toSendDataNum = MAX_PACKAGE_DATA_NUM;
            }
            else toSendDataNum = picSize - sendNum;

            pImg = bitBuffer+MAX_PACKAGE_DATA_NUM*pkgCnt;
            pPkg = &pkgData[PACKAGE_HEAD_LENGTH];
            memcpy(pPkg,pImg,toSendDataNum);

            setPkgHead(pings, pkgCnt, picSize);

            //cout<<"pkgCnt"<<(int)pkgHead.pkgIndex<<"send:" << pkgHead.size<<endl;


			cnt = UDPWrite(sock_fd,DSET_IP_ADDRESS,DEST_PORT,(const void*)&pkgData,toSendDataNum+PACKAGE_HEAD_LENGTH);
		

            //为数据包发送预留时间，减少快速发送大量数据造成网络拥堵
            //可以逐步增大时间到不出现或很少 error frame 为止
            if(sendCnt%5 == 4)
                usleep(2000);


            if (cnt < 0){
                printf("ERROR writing to udp socket:");
                return;
            }
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
