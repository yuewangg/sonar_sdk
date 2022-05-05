#include <stdio.h>
#include <string>
#include <cstring>
#include <unistd.h> //read & write g++
#include <arpa/inet.h>
#include <bvt_sdk.h>
#include<iostream>

#define PKG_MAX_LENGTH 65500
#define PACKAGE_HEAD_LENGTH 7 //包头长度
#define MAX_PACKAGE_SIZE (1460*2)//最大数据包长度
#define MAX_PACKAGE_DATA_NUM (PKG_MAX_LENGTH-PACKAGE_HEAD_LENGTH) //除去包头最大数据量
#define ENABLE_SONAR 1
#define DEST_PORT "8000"   //端口号
#define DSET_IP_ADDRESS "127.0.0.1 " // server文件所在PC的IP

using namespace std;

unsigned short pkgData[PKG_MAX_LENGTH];

int UDPWrite(int sock_fd,const void *send_buf,int bufLen)
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
  addr_serv.sin_addr.s_addr = inet_addr(DSET_IP_ADDRESS);
  addr_serv.sin_port = htons((unsigned short)atoi(DEST_PORT));
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
char DataFile[] = "../../data/0330Data.son";
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

	// Open the sonar
    if ( argc == 2 )
        strcpy( DataFile, argv[1] );

	ret = BVTSonar_Open(son, "FILE", DataFile);
	if( ret != 0 )
	{
		printf("BVTSonar_Open: ret=%d\n", ret);
		return 1;
	}
    BVTSonar file = BVTSonar_Create();
	BVTSonar_CreateFile(file, "son/out.son", son, "");
	BVTHead out_head = NULL;
	BVTSonar_GetHead(file, 0, &out_head);
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

	int height, width, picSize;

    int key,pkgCnt,cnt,sendNum,toSendDataNum,test;
    int i,k,sum,sendCnt = 0;;
    //struct ImgDataStr imgData;
    unsigned short *pPkg;
    unsigned short *pImg;
	unsigned long ping_time;

    unsigned short* bitBuffer;
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

	for (int i = 1; i < 10; i++)
	{
		BVTPing ping = NULL;
		BVTMagImage img;
		ret = BVTHead_GetPing(head, i, &ping);
		
		BVTImageGenerator_GetImageXY(ig, ping, &img);
		
		BVTMagImage_GetHeight(img, &height );
		BVTMagImage_GetWidth(img, &width) ; 

        BVTMagImage_GetBits(img, &bitBuffer);
		

        
        picSize = height*width;

		for(pkgCnt=0,sendNum=0;sendNum<picSize;pkgCnt++,sendNum += MAX_PACKAGE_DATA_NUM){
            if(sendNum + MAX_PACKAGE_DATA_NUM < picSize){
                toSendDataNum = MAX_PACKAGE_DATA_NUM;
            }
            else toSendDataNum = picSize - sendNum;

            pImg = bitBuffer+MAX_PACKAGE_DATA_NUM*pkgCnt;
            pPkg = &pkgData[PACKAGE_HEAD_LENGTH];
            memcpy(pPkg,pImg,toSendDataNum);

            setPkgHead(i, pkgCnt, picSize);

            //cout<<"pkgCnt"<<(int)pkgCnt<<"send:" << picSize<<endl;


			cnt = UDPWrite(sock_fd,(const void*)&pkgData,toSendDataNum+PACKAGE_HEAD_LENGTH);
		

            //为数据包发送预留时间，减少快速发送大量数据造成网络拥堵
            //可以逐步增大时间到不出现或很少 error frame 为止
            if(sendCnt%2 == 2)
                usleep(2000);

        


		    
            if (cnt < 0){
                printf("ERROR writing to udp socket:");
                return 1;
            }
        }
        
        if(i==8)
        {
            BVTHead_GetPing(out_head, 5, &ping);
            BVTImageGenerator_GetImageXY(ig, ping, &img);
            double range;
            BVTMagImage_GetPixelRange(img,500,500,&range);
            printf("%f\n",range);
            double bearing;
            BVTMagImage_GetPixelRelativeBearing(img,500,500,&bearing);
            printf("%f",bearing);

        }
        
        BVTHead_PutPing(out_head, ping);
		

		BVTPing_Destroy(ping);
		BVTMagImage_Destroy(img);

        usleep(800000);
    }
    close(sock_fd);
    BVTSonar_Destroy(file);
	BVTSonar_Destroy(son);
	return 0;
}
