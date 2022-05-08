# -*- coding: utf-8 -*-
from re import M
import socket  # 导入socket模块
import time  # 导入time模块
 
import cv2 as cv
import numpy as np
from scipy.fftpack import ss_diff

# server 接收端
# 设置服务器默认端口号
PORT = 8000
PKG_MAX_LENGTH = 65500
PACKAGE_HEAD_LENGTH = 7 
# 创建一个套接字socket对象，用于进行通讯
# socket.AF_INET 指明使用INET地址集，进行网间通讯
# socket.SOCK_DGRAM 指明使用数据协议，即使用传输层的udp协议
server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
address = ("127.0.0.1", PORT)
server_socket.bind(address)  # 为服务器绑定一个固定的地址，ip和端口
server_socket.settimeout(1)  # 设置一个时间提示，如果10秒钟没接到数据进行提示
ByteArray = bytearray()
old_picIndex = 1

while True:
    # 正常情况下接收数据并且显示，如果10秒钟没有接收数据进行提示（打印 "time out"）
    # 当然可以不要这个提示，那样的话把"try:" 以及 "except"后的语句删掉就可以了
    try:
        now = time.time()  # 获取当前时间

        # 接收客户端传来的数据 recvfrom接收客户端的数据，默认是阻塞的，直到有客户端传来数据
        # recvfrom 参数的意义，表示最大能接收多少数据，单位是字节
        # recvfrom返回值说明
        # receive_data表示接受到的传来的数据,是bytes类型
        # client  表示传来数据的客户端的身份信息，客户端的ip和端口，元组
        receive_data, client = server_socket.recvfrom(PKG_MAX_LENGTH)
        # print(receive_data[5])  # 以指定格式显示时间
        widht = receive_data[4]*256 + receive_data[3]
        height = receive_data[6]*256 + receive_data[5]
        print(widht,height)
        imgsize = widht * height
        picIndex  = receive_data[1] * 256 + receive_data[0]
        pkgIndex  = receive_data[2]
        print(picIndex, pkgIndex)
        maxpkg = int(imgsize / (PKG_MAX_LENGTH - PACKAGE_HEAD_LENGTH))
        print(maxpkg)

        if picIndex == old_picIndex: pass            
        else: ByteArray.clear() 
        ByteArray+=receive_data[7:]

        if pkgIndex == maxpkg:
            flatNumpyArray = np.array(ByteArray)
            grayImage = flatNumpyArray.reshape(height, widht)
            cv.imwrite("img/"+str(picIndex)+"GrayImage.jpg", grayImage)
        

        old_picIndex = picIndex
        
        

    except socket.timeout:  # 如果10秒钟没有接收数据进行提示（打印 "time out"）
        print("time out")


