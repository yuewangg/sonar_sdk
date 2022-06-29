#include<iostream>
#include<fstream>
#include <stdlib.h>
#include <bvt_sdk.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <udp.h>
#include <stdio.h>
#include <cmath>
#include <cstring>
#include <unistd.h> //read & write g++
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <vector>
#include <pthread.h>
#include <mutex>
#include <yaml-cpp/yaml.h>
#include <sys/socket.h>
#include <sys/time.h>


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
	real = 80* 3.1415926 / 180 - an;
	w_height = maxleng * cos(real);		
	printf("%f\n",w_height);

}

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

// get now time 2 str
string get_now_time(){
	time_t set_time;
	time(&set_time);
	tm* ptm = localtime(&set_time);
	string time = to_string(ptm->tm_year + 1900)
					   + "_"
					   + to_string(ptm->tm_mon + 1)
					   + "_"
					   + to_string(ptm->tm_mday)
					   + "_"
					   + to_string(ptm->tm_hour)
					   + "_"
					   + to_string(ptm->tm_min)
					   + "_"
					   + to_string(ptm->tm_sec);
	return time;

}

int64_t getTimeStamp()
{
//毫秒数
    int mSecond = 0; 
#if defined(WIN32)
    SYSTEMTIME sys;
    GetLocalTime(&sys);
    mSecond = sys.wMilliseconds;
#else
//linux 下gettimeofday
    struct timeval    tv;
    struct timezone tz;
    struct tm* p;
    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);
    mSecond = tv.tv_usec / 1000;
#endif
    int64_t timeStamp = ((int64_t)time(NULL)) * 1000 + mSecond;
    return timeStamp;
}



int main(int argc, char** argv){

	string strFileName("../config/config.yaml");
	/// Setup Path ///
	YAML::Node config;
	config = YAML::LoadFile(strFileName.c_str());  
	unsigned int pos = config["soundspeed"].as<unsigned int>();
	unsigned int record_enable = config["record_enable"].as<unsigned int>();
	string sonar_ip = config["sonar_ip"].as<string>();
	float start_range = config["start_range"].as<float>();
	float stop_range = config["stop_range"].as<float>();
	string str_time = get_now_time();
	string fileName = str_time + ".son";			// filename of .son
	string rootPath = "/root/Documents/bvtsdk/";			// SDKs path
	string dataPath = rootPath + "data/";					// .son path
	string mapperPath = rootPath + "colormaps/jet.cmap";	// Colormapper path
	string fullPath = dataPath + fileName;
	
	// Open .son file ///
	sonar.Open("NET" , sonar_ip);				// 
	printf("SDK Ready!!!\n") ;
	head = sonar.GetHead(0);					// Connect head to sonar, 0 = single-head sonar
	head.SetRange(start_range, stop_range) ;	// Setup range 
	
	ping = head.GetPing(0);						// Connnect Ping to head with specific ping (Ping #0 or first ping)
	img.SetHead(head);							// Create ImageGenerator
	
	mapp.Load(mapperPath);						// Load Colormapper
	mapp.SetAutoMode(1);							// Disable Auto Parameter, 0 = disable
	
	//write .txt file
	ofstream time_txt;
	string txtname = str_time + ".txt";	
    time_txt.open(txtname,ios::out );

	//.son file	
	if (record_enable == 1)
	{
		file.CreateFile(fileName, sonar, "") ;
		file_head = file.GetHead(0) ;

		printf("Record Enable!!!\n");
	}
	else{
		remove(txtname.c_str());
		printf("Record Disable!!!\n");
	} 
	
	

	/// Create Trackbar ///
	img.SetSoundSpeedOverride(pos);

	printf("SDK Ready!!!\n") ;		

	int sock_fd = createSocket(DSET_IP_ADDRESS, 8000);

	while(1){
		ping = head.GetPing(-1);  	// Getping less than 0 is get next ping

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
		vector<unsigned char> img_encode;
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

		//get_depth();
		if (record_enable == 1)
		{
			file_head.PutPing(ping);

			int64_t timeStamp = getTimeStamp();
			
			
			time_txt << "Ping:" << "\t" << i << "\t" "Time:" << "\t" << timeStamp << "\t" << endl; 
		
		}
		i++;
	}
	time_txt.close();
    return 0;
}