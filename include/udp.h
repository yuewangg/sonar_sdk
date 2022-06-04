#pragma once

int createSocket(const char* ip, const int port);
int udpWrite(int sock_fd,const char *ip,const char *port,void* buf,int bufLen);
int udpRead(int fd,void* buffer,int bufLen);
