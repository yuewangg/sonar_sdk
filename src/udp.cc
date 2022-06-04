#include <udp.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define EOK 0x0000
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int createSocket(const char* ip, const int port)
{
    int fd;
    struct sockaddr_in serveraddr;

    /* 构造用于UDP通信的套接字 */
    fd = socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;                        /* IPv4 */
    //serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);         /* 本地任意IP INADDR_ANY = 0 */
    // 上面注释可以替代下面这一句
    //inet_pton(AF_INET, "127.0.0.1" , &serveraddr.sin_addr.s_addr);
    inet_pton(AF_INET, ip, &serveraddr.sin_addr.s_addr);
    serveraddr.sin_port = htons(port);

    bind(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    return fd;
}

int udpWrite(int sock_fd,const char *ip,const char *port,void* buf,int bufLen)
{
  struct sockaddr_in to_addr;

  bzero(&to_addr, sizeof(to_addr));
  to_addr.sin_family=AF_INET;
  to_addr.sin_port=htons((unsigned short)atoi(port));
 
  if(!inet_pton(AF_INET, ip,&(to_addr.sin_addr.s_addr)))
  {
    printf("udpWrite: inet_aton.\r\n");
    return -1;
  }
  
  sendto(sock_fd,buf,bufLen,0,(struct sockaddr*)&to_addr,sizeof(to_addr));
  
  return EOK; 
}

int udpRead(int fd,void* buffer,int bufLen){
  int bytes_read;
  struct sockaddr_in sin;
  socklen_t sin_len=sizeof(sin);

  bytes_read=recvfrom(fd, buffer, bufLen, MSG_DONTWAIT, (struct sockaddr*)&sin, &sin_len);
  
  if(bytes_read<0)
  {
    if(errno==EINTR)          
#ifdef DEBUG    
      printf("udpRead:recvfrom.'errno=EINTR'.\r\n");
    else
      //printf("udpRead:recvfrom.\r\n");
#endif      
    return -1;
  }
  if(bytes_read>bufLen)
  {
#ifdef DEBUG    
    printf("FdRead:buffer is too small.\r\n");
#endif      
    return -2;
  }
 return (bytes_read);
}
