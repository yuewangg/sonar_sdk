#include <stdio.h>
#include <string>
#include <cstring>
#include <unistd.h> //read & write g++
#include <arpa/inet.h>
#include <bvt_sdk.h>
#include<iostream>

#define PKG_MAX_LENGTH 65500
#define PACKAGE_HEAD_LENGTH 19 //包头长度
#define MAX_PACKAGE_DATA_NUM (PKG_MAX_LENGTH-PACKAGE_HEAD_LENGTH) //除去包头最大数据量
#define ENABLE_SONAR 1
#define DEST_PORT 8000   //端口号
#define DSET_IP_ADDRESS "127.0.0.1 " // server文件所在PC的IP

using namespace std;

unsigned char pkgData[PKG_MAX_LENGTH];

int UDPWrite(int sock_fd,const void *send_buf,int bufLen,unsigned short port)
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
  addr_serv.sin_port = htons(port);
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


void setPkgHead(unsigned int picIndex,unsigned int pkgIndex, unsigned int width, unsigned int height, unsigned int OriginCol, unsigned int OriginRow){
    pkgData[0] = picIndex&0xff;
    picIndex >>= 8;

    pkgData[1] = picIndex;

    pkgData[2] = pkgIndex;

    pkgData[3] = width&0xff;
    width >>= 8;
    pkgData[4] = width&0xff;

    pkgData[5] = height&0xff;
    height >>= 8;
    pkgData[6] = height&0xff;

    pkgData[7] = OriginCol&0xff;
    OriginCol >>= 8;
    pkgData[8] = OriginCol&0xff;

    pkgData[9] = OriginRow&0xff;
    OriginRow >>= 8;
    pkgData[10] = OriginRow&0xff;
}
char DataFile[] = "/home/yue/workspace/UBM2022/bvtsdk/data/0330Data.son";
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
	BVTSonar_CreateFile(file, "out.son", son, "");
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

	int height, width, picSize,j, row, column;
    int pkgCnt,cnt,sendNum,toSendDataNum;
    //struct ImgDataStr imgData;
    unsigned char *pPkg;
    unsigned short *pImg;

    unsigned short* bitBuffer;
    unsigned short pixel;
    double resolution;
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);


	for (int i = 1; i < pings; i++)
	{
		BVTPing ping = NULL;
		BVTMagImage img;
    
        //unsigned short* bitBuffer;

		ret = BVTHead_GetPing(head, i, &ping);
		
		BVTImageGenerator_GetImageXY(ig, ping, &img);
		
		BVTMagImage_GetHeight(img, &height );

		BVTMagImage_GetWidth(img, &width) ; 

        BVTMagImage_GetBits(img, &bitBuffer);

	    BVTMagImage_GetRangeResolution(img,&resolution);

	    BVTMagImage_GetOriginCol(img,&column);

	    BVTMagImage_GetOriginRow(img,&row);


        
        picSize = height*width;

		for(pkgCnt=0,sendNum=0;sendNum<picSize;pkgCnt++,sendNum += MAX_PACKAGE_DATA_NUM){
            if(sendNum + MAX_PACKAGE_DATA_NUM < picSize){
                toSendDataNum = MAX_PACKAGE_DATA_NUM;
            }
            else toSendDataNum = picSize - sendNum;

            pImg = bitBuffer+MAX_PACKAGE_DATA_NUM*pkgCnt;
            pPkg = &pkgData[PACKAGE_HEAD_LENGTH];
            memcpy(&pkgData[11],&resolution,8);

            for(j=0;j<toSendDataNum;j++)

                pPkg[j] = pImg[j];

            setPkgHead(i, pkgCnt, width,height, column, row);


			cnt = UDPWrite(sock_fd,(const void*)&pkgData,toSendDataNum+PACKAGE_HEAD_LENGTH,DEST_PORT);
		

            //为数据包发送预留时间，减少快速发送大量数据造成网络拥堵
            //可以逐步增大时间到不出现或很少 error frame 为止
            
            usleep(2000);

        


		    
            if (cnt < 0){
                printf("ERROR writing to udp socket:");
                return 1;
            }
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
