#include <stdlib.h>
#include <bvt_sdk.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

#include "opencv2/imgproc/imgproc.hpp"


int pos = 0;
int intensity = 0;
float ggg = 0 ;
int set_gamma = 50 ;
int top = 0 ;
int bottom = 0 ;

float start_range = 1 ;
float stop_range = 100 ;

double maxval = 255 ;


BVTSDK::ImageGenerator img;
BVTSDK::ColorMapper map;
cv::Mat color_img ;

BVTSDK::Sonar sonar;
BVTSDK::Head head = NULL ;
BVTSDK::Ping ping = NULL ;
BVTSDK::ImageGenerator img;
BVTSDK::ColorMapper map;
BVTSDK::ColorMapper automap;

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


int main(int argc, char** argv){
    /// Setup Path ///
	std::string rootPath = "/home/paicaloid/bvtsdk/";
	//std::string dataPath = rootPath + "data/";
	std::string mapperPath = rootPath + "colormaps/jet.cmap";
	std::string fileName = "test_001.son";
	std::string storePath = "/home/paicaloid/catkin_ws/src/cpp_sonar/data" ;
	std::string fullPath = storePath + fileName;

	/// Create Windowname ///
	cv::namedWindow("RawImage" , cv::WINDOW_NORMAL);
	cv::namedWindow("RThetaImage", cv::WINDOW_NORMAL );
	cv::namedWindow("RawImage_Auto", cv::WINDOW_NORMAL );
	sonar.Open("NET" , "192.168.1.45");
	printf("Sonar is Connect\n") ;
	head = sonar.GetHead(0);					// Connect head to sonar, 0 = single-head sonar
	head.SetRange(start_range, stop_range) ;	// Setup range 
	ping = head.GetPing(0);						// Connnect Ping to head with specific ping (Ping #0 or first ping)
	img.SetHead(head);							// Create ImageGenerator
	map.Load(mapperPath);						// Load Colormapper
	map.SetAutoMode(0);							// Disable Auto Parameter, 0 = disable
	automap.Load(mapperPath) ;					// Load Colormapper
	automap.SetAutoMode(1) ;					// Enable Auto Parameter, 1 = enable 
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

    while (1) {
		
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


		/// Chgange Image to grayscale ///
//		cvtColor(color_img, color_img, cv::COLOR_RGB2GRAY);
//		cvtColor(auto_img, auto_img, cv::COLOR_RGB2GRAY);
//		cvtColor(color_rth, color_rth, cv::COLOR_RGB2GRAY);

//      convert cv image to byte
        std::vector<unsigned char> img_encode;
        cv::imencode(".jpg", color_img, img_encode);
        size_t datalen=img_encode.size();
        printf("size = %u\n", datalen) ;
        char *msgImage=new char[datalen];
        for(int i=0;i<datalen;i++)
        {
            msgImage[i]=img_encode[i];
        }
        printf("%d",strlen(msgImage));
        int udptest =sendto(sockfd,msgImage,strlen(msgImage),0,(struct sockaddr*)&addr,len);
//        int udptest =sendto(sockfd,img_encode,img_encode.size(),0,(struct sockaddr*)&addr,len);


		// int row = color_img.rows ;
		// int col = color_img.cols ;

		/// Resize Image for sending through ROS ///
//		cv::resize(color_img, color_img, cv::Size(), 0.5, 0.5, interpolation) ;
//		cv::resize(color_rth, color_rth, cv::Size(), 0.5, 0.5, interpolation) ;
		int roww = color_img.rows ;
		int coll = color_img.cols ;


    }
}