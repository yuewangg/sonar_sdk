#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <udp.h>
#include <string>
#include <config.h>

using namespace std;

#define MAXLINE 4096
#define CLIENT_PORT 9996


#define RECV_PORT 9996  // recv report data 

int recvData(int fd, double* data, int& dataType)
{
    char buf[MAXLINE];
    int cnt;

    while(1) 
    {
        bzero(buf, sizeof(buf));
        cnt = udpRead(fd, buf, sizeof(buf));
        if(cnt <= 0)
        {
            cout << "udpRead error return cnt <= 0" << endl;
            continue ;
        }

        cout << "recv: "<< buf << endl;        
        
        // 截取头部
        string strBuf(buf);
        size_t index = strBuf.find('#');
        if(index == string::npos)
        {
            printf("index == string::npos\r\n");
            continue;
        }
        string head;
        head = strBuf.substr(0, index);
        strBuf.erase(0, index + 1);
        
        // 截取数据
        string item;
        cnt = 0;
        while((index = strBuf.find(',')) != string::npos)
        {            
            item = strBuf.substr(0, index);
            data[cnt++] = stod(item);
            strBuf.erase(0, index + 1);
        }
        item = strBuf.substr(0, index);
        data[cnt++] = stod(item);
        //解析buf
        if(!strncmp(head.c_str(), "forward_data", strlen("forward_data")))
        {
            dataType = FORWARD_LOOKING_SONAR;
        }
        else if(!strncmp(head.c_str(), "side_data", strlen("side_data")))
        {
            dataType = SIDE_SCAN_SONAR;
        }
        else if(!strncmp(head.c_str(), "pose_data", strlen("pose_data")))
        {
            dataType = UUV_POSE;
        }
        else
        {

        }
        break;
    }    
    return cnt;
}

int main(int argc, char *argv[])
{
    struct sockaddr_in localaddr;
    int fd;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&localaddr, sizeof(localaddr));
    localaddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1" , &localaddr.sin_addr.s_addr);
    localaddr.sin_port = htons(RECV_PORT);
    int ret = bind(fd, (struct sockaddr *)&localaddr, sizeof(localaddr)); 

    while(1)
    {
        char buf[1024];
        int cnt = udpRead(fd, buf, sizeof(buf));
        if(cnt <= 0)
            continue;

        printf("recv %s from %d\r\n", buf, CLIENT_PORT);
    }

    #if 0

    struct sockaddr_in localaddr;
    int fd;
    ssize_t len;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&localaddr, sizeof(localaddr));
    localaddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1" , &localaddr.sin_addr.s_addr);
    localaddr.sin_port = htons(CLIENT_PORT);

    int ret = bind(fd, (struct sockaddr *)&localaddr, sizeof(localaddr)); 

    // 前视声呐：forward_data#0,0,0    （长度，角度，置信度）
    // 侧扫声呐：side_data#0,0,0 (纬度，经度，置信度)
    // 位姿：pose_data#0,0,0,0,0,0
    double data[6];
    int dataType;
    
    while(1)
    {
        int len = recvData(fd, data, dataType);
        if(len != 3 && len != 6)
        {
            cout << "len != 3 && len != 6" << endl;
            continue;
        }

        if(dataType == SIDE_SCAN_SONAR)
        {
            //声呐数据转换为帧
            Vector2d sideData(data[0], data[1]);
            double conf = data[2];
            ///////////////////////////
        }
        else if(dataType == FORWARD_LOOKING_SONAR)
        {
            Vector2d forwardData(data[0], data[1]);
            double conf = data[2];
            //等位姿数据
            while(dataType != UUV_POSE)
            {
                len = recvData(fd, data, dataType);
                if(len != 3 && len != 6)
                {
                    cout << "len != 3 && len != 6" << endl;
                    continue;
                }
                if(dataType == UUV_POSE)
                {
                    // 前视声呐数据转帧
                    //Vector6d pose(data[0], data[1], data[2], data[3], data[4], data[5]);
                    
                }
                else
                {
                    // 前视声呐数据丢弃，侧扫声呐数据存起来
                }
            }
        }
        cout << "process" << endl;
    }

        #endif








    close(fd);

    return 0;
}


