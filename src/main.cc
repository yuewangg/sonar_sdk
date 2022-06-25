#include <stdlib.h>
#include <bvt_sdk.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <udp.h>
#include <stdio.h>
#include <cmath>
#include <cstring>
#include <fstream>
#include <unistd.h> //read & write g++
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <target_pos_fusion.h>
#include <vector>
#include <pthread.h>
#include <mutex>
#include <utm.h>
#include <sys/socket.h>


//#include "ros/ros.h"
//#include "sensor_msgs/CompressedImage.h"
//#include "image_transport/image_transport.h"
//#include "cv_bridge/cv_bridge.h"
//#include "sensor_msgs/image_encodings.h"
#include "opencv2/imgproc/imgproc.hpp"

#define PKG_MAX_LENGTH 65500
#define PACKAGE_HEAD_LENGTH 16 //包头长度
#define MAX_PACKAGE_SIZE PKG_MAX_LENGTH//最大数据包长度
#define MAX_PACKAGE_DATA_NUM (MAX_PACKAGE_SIZE-PACKAGE_HEAD_LENGTH) //除去包头最大数据量
#define ENABLE_SONAR 1
#define DEST_PORT "8000"   //端口号
#define DSET_IP_ADDRESS "127.0.0.1" // server文件所在PC的IP

using namespace std;



unsigned char pkgData[PKG_MAX_LENGTH];
int i = 0;
cv::Mat color_img ;

//int thres_type = CV_THRESH_BINARY ;
//int interpolation = CV_INTER_LINEAR ;

BVTSDK::Sonar sonar;
BVTSDK::Head head = NULL ;
BVTSDK::Head file_head = NULL;
BVTSDK::Ping ping = NULL ;
BVTSDK::ImageGenerator img;
BVTSDK::ColorMapper mapp;
BVTSDK::Sonar file;


			///depth match
void get_depth(){
	BVTSDK::RangeProfile rg = img.GetRangeProfile(ping);
	int rgcount = rg.GetValidCount() ;
	unsigned short intensity;
	unsigned short intensity1;
	unsigned short max;
	for(int j=1;j<rgcount;j++){
		intensity = rg.GetIntensityValue(j);
		if (intensity > max){	
			max = intensity;
		}		
	}

	int imax;
	for(int k=1;k<rgcount;k = k+1){
		intensity = rg.GetIntensityValue(k);
		intensity1 = rg.GetIntensityValue(k+1);
		if (intensity ==  max && intensity ==intensity1)
			imax = k;
	}

   //get height
	float maxleng;
	float x;
	float length;
	double an;
	double real;
	float  w_height;
	maxleng = rg.GetRangeValue(imax);
	x = rg.Get_X_Value(imax);
	an = acos(x/maxleng);
	real = 80* PI / 180 - an;
	w_height = maxleng * cos(real);		
	printf("%f\n",w_height);

}
// cv::Mat color_img ;
// cv::Mat auto_img ;

// setpkghead
void setPkgHead(unsigned int picIndex,unsigned int pkgIndex, unsigned int sendIndex, unsigned int OriginCol, unsigned int OriginRow){
    pkgData[0] = picIndex&0xff;
    picIndex >>= 8;

    pkgData[1] = picIndex;

    pkgData[2] = pkgIndex;

    pkgData[3] = sendIndex;

    pkgData[4] = OriginCol&0xff;
    OriginCol >>= 8;
    pkgData[5] = OriginCol&0xff;

    pkgData[6] = OriginRow&0xff;
    OriginRow >>= 8;
    pkgData[7] = OriginRow&0xff;
}

/// Show Sonar information ///
void Sonar_status(){
	std::string model = sonar.GetSonarModelName() ;
	int fileSize = sonar.GetFileSize() ;
	// int timeIndex = sonar.GetTimeIndexedPingCount() ;	// useless
	// float temp = sonar.GetTemperature() ;				// coredump
	// int NavData = sonar.GetNavDataCount() ; 				// Navdata = 0, need find more information about NavData

	printf("Sonar model : %s\n", model.c_str()) ;
	printf("File size : %d bytes\n", fileSize) ;
	// printf("Time index : %d\n", timeIndex) ;
	// printf("Temperature : %f\n", temp) ;
	// printf("NavData count : %d\n", NavData) ;

	if(sonar.SupportsMulticast()){
		printf("SUPPORTTTTTTT\n") ;
	}

	float gain = head.GetGainAdjustment() ;
	int freq = head.GetCenterFreq() ;
	int PingCount = head.GetPingCount();
	printf("Gain : %f dB\n", gain) ;
	printf("Freq : %d Hz\n", freq) ;
	printf("Number of Ping = %d\n",PingCount);

	double timeStamp = ping.GetTimestamp() ;
	int timeZone = ping.GetTimeZoneOffset() ;
	printf("timeStamp : %d\n", timeZone) ;

	if(ping.HasNavData()){
		printf("NAVDATA\n") ;
	}

}
// get now time 2 str
std::string get_now_time(){
	time_t set_time;
	time(&set_time);
	tm* ptm = localtime(&set_time);
	std::string time = std::to_string(ptm->tm_year + 1900)
					   + "_"
					   + std::to_string(ptm->tm_mon + 1)
					   + "_"
					   + std::to_string(ptm->tm_mday)
					   + "_"
					   + std::to_string(ptm->tm_hour)
					   + "_"
					   + std::to_string(ptm->tm_min)
					   + "_"
					   + std::to_string(ptm->tm_sec);
	return time;

}
// send sonar img
int send_img(int sock_fd){

	/// Genarate XY ColorImage ///
	BVTSDK::MagImage mag = img.GetImageXY(ping);			// Get XY image from ping
	BVTSDK::ColorImage cimg = mapp.MapImage(mag);			// Covert to ColorImage with Colormapping
		
	/// Find XY Image resolution ///
	int height = cimg.GetHeight();
	int width = cimg.GetWidth();

	/// Convert to Matrix format ///
	/// GetBits() will return pointer of the entrie image ///
	cv::Mat color_img(height , width , CV_8UC4 , cimg.GetBits());
	int pkgCnt,cnt,sendNum,toSendDataNum;
	unsigned char *pPkg, *pImg;
	int column = cimg.GetOriginCol();
	int row = cimg.GetOriginRow();
	double resolution = cimg.GetRangeResolution();
    std::vector<unsigned char> img_encode;
    cv::imencode(".jpg", color_img, img_encode);
    unsigned int datalen=img_encode.size();

	int sendCnt = datalen / MAX_PACKAGE_DATA_NUM;

	for(pkgCnt=0,sendNum=0;sendNum<datalen;pkgCnt++,sendNum += MAX_PACKAGE_DATA_NUM){
        if(sendNum + MAX_PACKAGE_DATA_NUM < datalen){
            toSendDataNum = MAX_PACKAGE_DATA_NUM;
        }
        else toSendDataNum = datalen - sendNum;

        pImg = img_encode.data()+MAX_PACKAGE_DATA_NUM*pkgCnt;
        pPkg = &pkgData[PACKAGE_HEAD_LENGTH];

        memcpy(&pkgData[8],&resolution,8);

		memcpy(pPkg,pImg,toSendDataNum);

        setPkgHead(i, pkgCnt,sendCnt, column, row);

		cnt = udpWrite(sock_fd, DSET_IP_ADDRESS, DEST_PORT, (void*)&pkgData,toSendDataNum+PACKAGE_HEAD_LENGTH);

            //为数据包发送预留时间，减少快速发送大量数据造成网络拥堵
            //可以逐步增大时间到不出现或很少 error frame 为止            
        usleep(2000);
        }
	return cnt;
}
// sonar setup and open
void sonar_set(string strYamlFileName){
	/// Setup Path ///
	YAML::Node config;
	config = YAML::LoadFile(strYamlFileName.c_str());  
	unsigned int pos = config["sundspeed"].as<unsigned int>();
	string sonar_ip = config["sonar_ip"].as<string>();
	float start_range = config["start_range"].as<float>();
	float stop_range = config["stop_range"].as<float>();
	std::string str_time = get_now_time();
	std::string fileName = str_time + ".son";			// filename of .son
	std::string rootPath = "/root/Documents/bvtsdk/";			// SDKs path
	std::string dataPath = rootPath + "data/";					// .son path
	std::string mapperPath = rootPath + "colormaps/jet.cmap";	// Colormapper path
	std::string fullPath = dataPath + fileName;
	
	// Open .son file ///
	sonar.Open("NET" , sonar_ip);				// 
	printf("SDK Ready!!!\n") ;
	head = sonar.GetHead(0);					// Connect head to sonar, 0 = single-head sonar
	head.SetRange(start_range, stop_range) ;	// Setup range 
	
	ping = head.GetPing(0);						// Connnect Ping to head with specific ping (Ping #0 or first ping)
	img.SetHead(head);							// Create ImageGenerator
	
	mapp.Load(mapperPath);						// Load Colormapper
	mapp.SetAutoMode(1);							// Disable Auto Parameter, 0 = disable
	
	//.son file	
    file.CreateFile(fileName, sonar, "") ;
	file_head = file.GetHead(0) ;
	/// Create Trackbar ///
	img.SetSoundSpeedOverride(pos);

	printf("SDK Ready!!!\n") ;	
}


int main(int argc, char** argv){


	//#ifdef DEBUG
    //	cout.precision(12);
	//#endif
    //bool bEndFlag = false;
	string strFileName("../config/config.yaml");
	sonar_set(strFileName);

	int sock_fd = createSocket(DSET_IP_ADDRESS, 8000);

    //TargetPosFusion* fusion = new TargetPosFusion(strFileName.c_str());
    //if(!(fusion->getInitFlag()))
    //{
	//#ifdef DEBUG
    //    cout << "TargetPosFunsion init error" << endl;
	//#endif
    //    return -1;
    //}

	//while(!(bEndFlag = fusion->getEndFlag())){
	while(1){
		ping = head.GetPing(-1);  	// Getping less than 0 is get next ping

		send_img(sock_fd);

		get_depth();

		file_head.PutPing(ping) ;    

		i++;

		//fusion->run();
	}
	
//#ifdef DEBUG
//    fusion->write2File();
//#endif
//    fusion->report();

    return 0;
}