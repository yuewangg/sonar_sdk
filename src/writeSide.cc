#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <udp.h>
#include <fstream>
using namespace std;

#define SERVER_PORT 8000       
#define MAXLINE 1024

#define BROADCAST_IP "127.0.0.1" 

int main(void)
{
    int fd;
    struct sockaddr_in serveraddr;
    char buf[MAXLINE];


    fd = createSocket(BROADCAST_IP, SERVER_PORT);

    //给当前的socket赋予广播权限
    //int flag = 1;
    //setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &flag, sizeof(flag));
    
    int i = 0;
    // 前视声呐：forward_data#0,0,0    （长度，角度，置信度）
    // 侧扫声呐：side_data#0,0,0 (纬度，经度，置信度)
    // 位姿：pose_data#0,0,0,0,0,0
    // 经纬度信息初始化：ll_init#lat,lon,zone,hemi(type = char)
    // 结束信号：end#
    // 发送初始化信息
    double latOrigin = 26.0;
    double lonOrigin = 103.0;
    int zoneOrigin = (int)(lonOrigin / 6) + 31;
    
    //sprintf(buf, "pose_data#%.12f,%.12f,%.12f,%.12f,%.12f,%.12f", 2877215.83578, 299819.459818, 0.0, 0.0, 0.0, 0.0);
    sprintf(buf, "pose_data#%.12f,%.12f,%.12f,%.12f,%.12f,%.12f", lonOrigin, latOrigin, 0.0, 0.0, 0.0, 0.0);
    udpWrite(fd, BROADCAST_IP, "9999", buf, strlen(buf));
    bzero(buf, sizeof(buf));

    // 发送数据    
    // dataSide:    x, y, lat, lon  , conf, zone, hemi
    ifstream ifs;
    ifs.open("../data/dataSide.txt", ios::in);
    if(!ifs.is_open())
    {
        printf("打开 dataSide.txt 失败\r\n");
        return -1;
    }

    double data[5];
    int j = 1;
    while (!ifs.eof()) 
    {
        // 读文本
        bzero(buf, 1024);
        ifs.getline(buf, 100);
        if(strlen(buf) == 0)    break;
        string line(buf);
        int index;
        string item;
        int cnt = 0;
        //while((index = line.find(' ')) != string::npos)
        while(cnt < 5)
        {
            index = line.find(' ');
            item = line.substr(0, index);
            data[cnt++] = stod(item);
            line.erase(0, index+1);
        }
        index = line.find(' ');
        item = line.substr(0, index);
        int zone = stoi(item);
        line.erase(0, index+1);
        index = line.find(' ');
        item = line.substr(0, index);
        char hemi = item.c_str()[0];
        printf("%03d: %.12f %.12f %.12f %.12f %.12f %d %c\r\n", j++, data[0], data[1], data[2], data[3], data[4], zone, hemi);
        
        // 发送
        bzero(buf, 1024);
        sprintf(buf, "side_data#%.12f,%.12f,%.12f", data[2], data[3], data[4]);
        udpWrite(fd, BROADCAST_IP, "9999", buf, strlen(buf));
        usleep(5 * 1000);
    }
    bzero(buf, 1024);
    sprintf(buf, "end#");
    udpWrite(fd, BROADCAST_IP, "9999", buf, strlen(buf));
    
    ifs.close();
    close(fd);
    return 0;
}

