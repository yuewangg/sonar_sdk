//============================================================================
// Name        : client.cpp
// Author      : shgued
// Version     : 2.1
// Copyright   : open source
// Description : OpenCV experimentation in C++, transform video with UDP
// !! 标记重点关注
// 修正线程函数中recvfrom返回-1的bug
//============================================================================

#include "client.hpp"
#include <time.h>
#include <sys/time.h>

#define TAG "Client- "
#define  PKG_RANDOM_ORDER_TEST  //测试数据包排序功能，定义则测试  交换 pkgIndex 0和1的发包顺序

using namespace std;
using namespace cv;

//---------客户端服务器端设置要一致
#define PACKAGE_HEAD_LENGTH 6 //包头长度
#define MAX_PACKAGE_SIZE (1460*2)//最大数据包长度
#define MAX_PACKAGE_DATA_NUM (MAX_PACKAGE_SIZE-PACKAGE_HEAD_LENGTH) //除去包头最大数据量

#define MAX_PIC_INDEX 100 //图片索引最大值，不包含
//---------客户端服务器端设置要一致

//数据包头结构
struct PkgHeadStr{
    unsigned char picIndex;//0~99图片索引
    unsigned char pkgIndex;//0～255数据包索引
    unsigned int size;//一幅图片数据量
};


struct PkgHeadStr pkgHead;//包头结构
pthread_mutex_t sendMutex;//UDP发送数据互斥锁

//发送数据暂存
#define PKG_MAX_LENGTH 60012
unsigned char pkgData[PKG_MAX_LENGTH];



//建立socket
int SonarTSClient::makeSocket(char * ip, int port){
    int optVal;
    unsigned int optLen;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

//    memset((void*)&addr,0,addrLen);
//    addr.sin_family = AF_INET;
//    addr.sin_addr.s_addr= htonl(INADDR_ANY);
//    addr.sin_port = htons(port);
//    if(bind(sockfd,(struct sockaddr*)(&addr),addrLen)<0){
//        cout<<TAG<<"bind error!"<<endl;
//        exit(1);
//    }
//    else cout<<TAG<<"bind succesfully!"<<endl;

    memset((void*)&addr,0,addrLen);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr=inet_addr(ip);
    addr.sin_port = htons(port);

//    if(bind(sockfd,(struct sockaddr*)(&addr),addrLen)<0){
//        cout<<TAG<<"bind error!"<<endl;
//        exit(1);
//    }

    //发送缓冲区
    optVal=65536;
    optLen=sizeof(optVal);
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (void*)&optVal, optLen);
    getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &optVal,&optLen);
    cout << "send buf size:" << optVal << endl;

    /*
    if(bind(sockfd,(struct sockaddr*)(&addr),addrLen)<0){
        cout<<TAG<<"bind error!"<<endl;
        exit(1);
    }
    else cout<<TAG<<"bind succesfully!"<<endl;*/

    cout << TAG << "Opened socket seccessfully!"<<endl;
    cout << "server ip:"<<ip<<" server port:"<<port<<" socket id:"<<sockfd<<endl;

    return sockfd;
}

//设置包头 index:包索引 sum：校验和 datasize：发送的数据量
void setPkgHead(unsigned char picIndex,unsigned char pkgIndex, unsigned int dataSize){
    pkgData[0] = picIndex;

    pkgData[1] = pkgIndex;

    pkgData[2] = dataSize&0xff;
    dataSize >>= 8;
    pkgData[3] = dataSize&0xff;
    dataSize >>= 8;
    pkgData[4] = dataSize&0xff;
    dataSize >>= 8;
    pkgData[5] = dataSize&0xff;
}

//解析包头
struct PkgHeadStr getPkgHead(unsigned char *pkgData){
    struct PkgHeadStr ph;
    ph.picIndex = pkgData[0];
    ph.pkgIndex = pkgData[1];
    ph.size = pkgData[2] | (pkgData[3]<<8) | (pkgData[4]<<16) | (pkgData[5]<<24);
    return ph;
}


void SonarTSClient::sendFramToserver(){
    int key,pkgCnt,cnt,sendNum,toSendDataNum,test;
    int i,k,sum,sendCnt = 0;;
    //struct ImgDataStr imgData;
    unsigned char *pImg,*pPkg;
    int picSize;

    time_t t;
    struct tm *lt;

    static int us;
    struct timeval tv;
    struct timezone tz;
    int us1,us2;

    for(;;)
    {
        gettimeofday (&tv , &tz); us1 = tv.tv_usec;
        cap >> frame; //采集图片等待时间有时可达0.1s以上

        gettimeofday (&tv , &tz); us2 = tv.tv_usec;//获取us级时间
        us = us2 - us1; if(us < 0)  us += 1000000;

        resendPos++;        
        if(resendPos >= PIC_RESEND_BKP_NUM) resendPos = 0;
        //resendPicBuf[resendPos].picIndex = MAX_PIC_INDEX;
        imencode(".jpg", frame, resendPicBuf[resendPos].pic);//7～8ms

        picSize = resendPicBuf[resendPos].pic.size();
        resendPicBuf[resendPos].size = picSize;
        picIndex++;
        if(picIndex >= MAX_PIC_INDEX) picIndex = 0;
        resendPicBuf[resendPos].picIndex = picIndex;


        time (&t);//获取Unix时间戳。
        lt = localtime (&t);//转为时间结构。
        if(sendCnt%100 == 0)
            printf ( "sendPicCnt:%d clock:%d time_s:%d scnt:%d smpus:%d\n",sendCnt,clock(),lt->tm_sec,cnt,us);//输出结果
        sendCnt++;

        for(pkgCnt=0,sendNum=0;sendNum<picSize;pkgCnt++,sendNum += MAX_PACKAGE_DATA_NUM){
            if(sendNum + MAX_PACKAGE_DATA_NUM < picSize){
                toSendDataNum = MAX_PACKAGE_DATA_NUM;
            }
            else toSendDataNum = picSize - sendNum;

            #ifdef PKG_RANDOM_ORDER_TEST
                        if(pkgCnt == 0) test = 1;
                        else if(pkgCnt == 1) test = 0;
                        else test = pkgCnt;
            #else
                        test = pkgCnt;
            #endif

            pthread_mutex_lock(sendMutex);//UDP发送 互斥
            pImg = resendPicBuf[resendPos].pic.data()+MAX_PACKAGE_DATA_NUM*test;
            pPkg = &pkgData[PACKAGE_HEAD_LENGTH];
            memcpy(pPkg,pImg,toSendDataNum);

            setPkgHead(resendPicBuf[resendPos].picIndex, test, resendPicBuf[resendPos].size);
            pkgHead = getPkgHead(pkgData);

            time (&t);//获取Unix时间戳。
            lt = localtime (&t);//转为时间结构。
            //printf ( "1:%d/%d/%d %d:%d:%d\n",lt->tm_year+1900, lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);//输出结果

            clock_t ct1 = clock();

            //30～110us
            cnt = sendto(sockfd, (const void*)&pkgData, \
                  toSendDataNum+PACKAGE_HEAD_LENGTH, 0, (struct sockaddr*)&addr,addrLen);

            pthread_mutex_unlock(sendMutex);

            //为数据包发送预留时间，减少快速发送大量数据造成网络拥堵
            //可以逐步增大时间到不出现或很少 error frame 为止
            if(sendCnt%5 == 4)
                usleep(2000);

            clock_t ct2 = clock();
            //printf("ct2:%d\n",ct2-ct1);

            time (&t);//获取Unix时间戳。
            lt = localtime (&t);//转为时间结构。
            //printf ( "2:%d/%d/%d %d:%d:%d\n",lt->tm_year+1900, lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);//输出结果

            if (cnt < 0){
                cerr<<TAG<<"ERROR writing to udp socket:"<<cnt;
                return;
            }
        }
    }
}


//默认地址和端口
char server_ip[16] = "127.0.0.1"; //default server ip
int server_port = 8000; //default server port

int udpSockfd = -1;//多线程使用
short lrStatus;//左右转状态 >0 右
short gbStatus;//前进后退状态 >0 前
const short LR_STATUS_MAX_VALUE = 2;
const short GB_STATUS_MAX_VALUE = 2;
const int CAR_CTL_CMD_ID = 101;//控制车命令ID
const int PKG_RESEND_CMD_ID = 102;//包重传命令ID
int recvFps;//服务器端的每秒接收图片数量
int sendFps;//客户端端的每秒发送图片数量

SonarTSClient vc(&sendMutex);

int main(int argc,char **argv){    

    if(vc.makeSocket(server_ip,server_port) < 0){
        cerr<<TAG<<"ERROR opening socket";
    }

    vc.sendFramToserver();

    return 0;
}
