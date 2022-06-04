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
    double lonOrigin = 102.0;
    int zoneOrigin = (int)(lonOrigin / 6) + 31;
    printf("(lat, lon) = (%f, %f)  zone = %d\r\n", latOrigin, lonOrigin, zoneOrigin);
    sprintf(buf, "ll_init#%f,%f,%d,%c", latOrigin, lonOrigin, zoneOrigin, 'N');
    udpWrite(fd, BROADCAST_IP, "9999", buf, strlen(buf));
    bzero(buf, sizeof(buf));

    sprintf(buf, "pose_data#%f,%f,%f,%f,%f,%f", 0.3, 0.2, 0.1, -0.1, -0.2, -0.3);
    udpWrite(fd, BROADCAST_IP, "9999", buf, strlen(buf));
    bzero(buf, sizeof(buf));

    // 发送数据
    // data.txt格式：x, y, lat, lon, conf, zone, hemi
    ifstream ifs;
    ifs.open("../data/data.txt", ios::in);
    if(!ifs.is_open())
    {
        printf("打开 data.txt 失败\r\n");
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
        sprintf(buf, "side_data#%f,%f,%f", data[2], data[3], data[4]);
        udpWrite(fd, BROADCAST_IP, "9999", buf, strlen(buf));
        usleep(1 * 1000);
    }
    bzero(buf, 1024);
    sprintf(buf, "end#");
    udpWrite(fd, BROADCAST_IP, "9999", buf, strlen(buf));
    
    ifs.close();
    close(fd);
    return 0;
}

