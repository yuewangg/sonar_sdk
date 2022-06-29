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
#include <vector>
#include <pthread.h>
#include <mutex>
#include <yaml-cpp/yaml.h>
#include <sys/socket.h>

#include "opencv2/imgproc/imgproc.hpp"

#define PKG_MAX_LENGTH 65500
#define PACKAGE_HEAD_LENGTH 16 //包头长度
#define MAX_PACKAGE_SIZE PKG_MAX_LENGTH//最大数据包长度
#define MAX_PACKAGE_DATA_NUM (MAX_PACKAGE_SIZE-PACKAGE_HEAD_LENGTH) //除去包头最大数据量
#define ENABLE_SONAR 1
#define DEST_PORT "8000"   //端口号
#define DSET_IP_ADDRESS "127.0.0.1" // server文件所在PC的IP

unsigned char pkgData[PKG_MAX_LENGTH];

int pos = 0;
int intensity = 0;
float ggg = 0 ;
int set_gamma = 50 ;
int top = 0 ;
int bottom = 0 ;


double maxval = 255 ;


BVTSDK::Sonar sonar;
BVTSDK::Head head = NULL ;
BVTSDK::Ping ping = NULL ;
BVTSDK::ImageGenerator img;
BVTSDK::ColorMapper map;
BVTSDK::ColorMapper automap;
// cv::Mat color_img ;
// cv::Mat auto_img ;

/// Changing Gamma in ColorMapper ///
void Set_Gamma( int , void* ){
	float int2float = set_gamma/100.0 ;
	map.SetGamma(int2float) ;
}

/// Changing Threshold in ColorMapper ///
void SetMapperThreshold( int , void*){
	if(top > bottom){
		map.SetThresholds(top, bottom);
	}
}

/// Changing Sound speed in ImageGenerator ///
void SpeedChange( int , void*){
	img.SetSoundSpeedOverride(pos);
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

int main(int argc, char** argv){
	
	std::string strFileName("../config/config.yaml");
	YAML::Node config;
	config = YAML::LoadFile(strFileName.c_str());  
	/// Setup Path ///
	std::string rootPath = config["SDK_path"].as<std::string>();			// SDK path
	std::string dataPath = rootPath + "data/";					// .son path
	std::string mapperPath = rootPath + "colormaps/jet.cmap";	// Colormapper path
	std::string fileName = "0330Data.son";			// filename of .son
	//std::string fullPath = dataPath + fileName;
	std::string fullPath = config["data_full_path"].as<std::string>();
	float start_range = config["start_range"].as<float>();
	float stop_range = config["stop_range"].as<float>();

	/// Create Windowname ///
	cv::namedWindow("RawImage" , cv::WINDOW_NORMAL);
	cv::namedWindow("RThetaImage", cv::WINDOW_NORMAL );
	cv::namedWindow("RawImage_Auto", cv::WINDOW_NORMAL );
	printf("%s",fullPath.c_str());
	// Open .son file ///
	sonar.Open("FILE" , fullPath.c_str());				// Open .son file
	printf("SDK Ready!!!\n") ;
	head = sonar.GetHead(0);					// Connect head to sonar, 0 = single-head sonar
	head.SetRange(start_range, stop_range) ;	// Setup range 
	
	ping = head.GetPing(0);						// Connnect Ping to head with specific ping (Ping #0 or first ping)
	img.SetHead(head);							// Create ImageGenerator
	
	map.Load(mapperPath);						// Load Colormapper
	map.SetAutoMode(0);							// Disable Auto Parameter, 0 = disable
	automap.Load(mapperPath) ;					// Load Colormapper
	automap.SetAutoMode(1) ;					// Enable Auto Parameter, 1 = enable 
	printf("SDK Ready!!!\n") ;
	Sonar_status() ;

	/// Create Trackbar ///
	pos = 1530;
	top = map.GetTopThreshold() ;
	bottom = map.GetBottomThreshold() ;
	ggg = map.GetGamma() ;
	/// RawImage Trackerbar ///
	cv::createTrackbar("Sound Speed" , "RawImage" , &pos , 2000 , SpeedChange);
	cv::createTrackbar("Top Thres" , "RawImage" , &top , 32700 , SetMapperThreshold);
	cv::createTrackbar("Bottom Thres" , "RawImage" , &bottom , 32700 , SetMapperThreshold);
	cv::createTrackbar("Gamma" , "RawImage" , &set_gamma , 100 , Set_Gamma);
	/// Soundspeed Trackbar ///
	cv::createTrackbar("Sound Speed" , "RThetaImage" , &pos , 2000 , SpeedChange);
	cv::createTrackbar("Sound Speed" , "RawImage_Auto" , &pos , 2000 , SpeedChange);
	
	int p = 1;
	int k = 0;
	int i = 0;

	printf("SDK Ready!!!\n") ;
	int sock_fd = createSocket(DSET_IP_ADDRESS, 8000);
	int PingCount = head.GetPingCount();
	while(1){

		ping = head.GetPing(-1);  	// Getping less than 0 is get next ping

		/// Genarate XY ColorImage ///
		BVTSDK::MagImage mag = img.GetImageXY(ping);			// Get XY image from ping
		BVTSDK::ColorImage cimg = map.MapImage(mag);			// Covert to ColorImage with Colormapping
		BVTSDK::ColorImage cimg_auto = automap.MapImage(mag);	// Covert to ColorImage with Colormapping

		/// Generate RTheta ColorImage ///
		BVTSDK::MagImage rth = img.GetImageRTheta(ping);		// Get RTheta image from ping
		BVTSDK::ColorImage cimgrth = map.MapImage(rth);			// Covert to ColorImage with Colormapping
		
		/// Find XY Image resolution ///
		int height = cimg.GetHeight();
		int width = cimg.GetWidth();
		printf("XY Image resolution : %d x %d\n", width, height) ;

		/// Find RTheta Image resolution ///
		int height_rth = cimgrth.GetHeight();
		int width_rth = cimgrth.GetWidth();
		printf("RTheta Image resolution : %d x %d\n", width_rth, height_rth) ;
		
		/// Convert to Matrix format ///
		/// GetBits() will return pointer of the entrie image ///
		cv::Mat color_img(height , width , CV_8UC4 , cimg.GetBits());
		cv::Mat auto_img(height , width , CV_8UC4 , cimg_auto.GetBits());
		cv::Mat color_rth(height_rth , width_rth , CV_8UC4 , cimgrth.GetBits());

		// Pause by pressing 'P' ///
	 	while(!p){
	 		k = cv::waitKey(30);
	    	if (k == 112){
	    		p = !p;
	    	}
		}
		
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


		/// Chgange Image to grayscale ///
		//cvtColor(color_img, color_img, cv::COLOR_RGB2GRAY);
		//cvtColor(auto_img, auto_img, cv::COLOR_RGB2GRAY);
		//cvtColor(color_rth, color_rth, cv::COLOR_RGB2GRAY);

		/// Show Image ///
		cv::imshow("RawImage", color_img);
		cv::imshow("RawImage_Auto", auto_img) ;
		cv::imshow("RThetaImage", color_rth) ;
        	
		/// unknown ///
		k = cv::waitKey(30);
    	if( k == 27 )
    		break;
    	else if (k == 112){
    		p = !p;
    	}
		i++;
		usleep(800000);
	}
}