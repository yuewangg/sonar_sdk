/*
 * File Sonar Example
 * Demonstrate opening a file, accessing a head, and retriving a ping.
 * The ping is then processed into an image and saved to a file.
 * Finally, a colormap is loaded and the image is colormapped.
 * Note: Copy dll files from lib folder to output folder before running project.
 */
#include <stdio.h>
#include <string.h>

#include <bvt_sdk.h>

char DataFile[] = "../../data/0330Data.son";

int main( int argc, char *argv[] )
{
	int ret;
	// Create a new BVTSonar Object
	BVTSonar son = BVTSonar_Create();
	if( son == NULL )
	{
		printf("BVTSonar_Create: failed\n");
		return 1;
	}

	// Open the sonar
    if ( argc == 2 )
        strcpy( DataFile, argv[1] );

	ret = BVTSonar_Open(son, "FILE", DataFile);
	if( ret != 0 )
	{
		printf("BVTSonar_Open: ret=%d\n", ret);
		return 1;
	}

	// Make sure we have the right number of heads
	int heads = -1;
	BVTSonar_GetHeadCount(son, &heads);
	printf("BVTSonar_GetHeadCount: %d\n", heads);


	// Get the first head
	BVTHead head = NULL;
	ret = BVTSonar_GetHead(son, 0, &head);
	if( ret != 0 )
	{
		printf("BVTSonar_GetHead: ret=%d\n", ret);
		return 1;
	}
	
	// Check the ping count
	int pings = -1;
	BVTHead_GetPingCount(head, &pings);
	printf("BVTHead_GetPingCount: %d\n", pings);

    // Check the min and max range in this file
    float min_range, max_range;
    BVTHead_GetMinimumRange(head, &min_range);
    BVTHead_GetMaximumRange(head, &max_range);
    printf("BVTHead_GetMinimumRange: %0.2f\n", min_range );
    printf("BVTHead_GetMaximumRange: %0.2f\n", max_range );

    BVTImageGenerator ig = BVTImageGenerator_Create();
    BVTImageGenerator_SetHead(ig, head);

    // Now, get a ping!
	BVTPing ping = NULL;
	ret = BVTHead_GetPing(head, 20, &ping);
	if( ret != 0 )
	{
		printf("BVTHead_GetPing: ret=%d\n", ret);
		return 1;
	}
	BVTEventMark a = BVTEventMark_Create();
	double timetamp;
	BVTEventMark_GetTimestamp(a,&timetamp);
	printf("%f\n",timetamp);
	int number;
	BVTEventMark_GetPingNumber(a,&number);
	printf("%d\n",number);	
	// Generate an image from the ping
	BVTMagImage img;
	ret = BVTImageGenerator_GetImageXY(ig, ping, &img);
	if( ret != 0 )
	{
		printf("BVTImageGenerator_GetImageXY: ret=%d\n", ret);
		return 1;
	}

	printf("\n");

	/////////////////////////////////////////////////////////
	
	// Check the image height and width out
	int height;
	BVTMagImage_GetHeight(img, &height);
	printf("BVTMagImage_GetHeight: %d\n", height);
	int width;
	BVTMagImage_GetWidth(img, &width);
	printf("BVTMagImage_GetWidth: %d\n", width);
	unsigned short pixel = 0;

	BVTMagImage_GetPixel 	(img,100,100, &pixel); 		
	printf("MagImage_GetPixel: %d\n", pixel);


	
	// Save it to a PGM (PortableGreyMap)
	ret = BVTMagImage_SavePGM(img, "img.pgm");
	if( ret != 0 )
	{
		printf("BVTMagImage_SavePGM: ret=%d\n", ret);
		return 1;
	}

	/////////////////////////////////////////////////////////
	
	// Build a color mapper
	BVTColorMapper mapper;
	mapper = BVTColorMapper_Create();
	if( mapper == NULL )
	{
		printf("BVTColorMapper_Create: failed\n");
		return 1;
	}
	
	// Load the bone colormap
	ret = BVTColorMapper_Load(mapper, "bone.cmap");
	if( ret != 0 )
	{
		printf("BVTColorMapper_Load: ret=%d\n", ret);
		return 1;
	}

	
	// Perform the colormapping
	BVTColorImage cimg;
	ret = BVTColorMapper_MapImage(mapper, img, &cimg);
	if( ret != 0 )
	{
		printf("BVTColorMapper_MapImage: ret=%d\n", ret);
		return 1;
	}
	printf("\n");
	
	/////////////////////////////////////////////////////////
	// Check the image height and width out
	BVTColorImage_GetHeight(cimg, &height);
	printf("BVTColorImage_GetHeight: %d\n", height);
	BVTColorImage_GetWidth(cimg, &width);
	printf("BVTColorImage_GetWidth: %d\n", width);

	
	// Save it to a PPM (PortablePixMap)
	ret = BVTColorImage_SavePPM(cimg, "cimg.ppm");
	if( ret != 0 )
	{
		printf("BVTColorImage_SavePPM: ret=%d\n", ret);
		return 1;
	}

	// Clean up
	BVTEventMark_Destroy(a); 	
	BVTColorImage_Destroy(cimg);
	BVTMagImage_Destroy(img);
	BVTColorMapper_Destroy(mapper);
	BVTPing_Destroy(ping);
	BVTSonar_Destroy(son);
	return 0;
}
