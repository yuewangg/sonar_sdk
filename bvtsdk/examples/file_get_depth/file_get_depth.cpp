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
	ret = BVTSonar_GetHead(son, 1000, &head);
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
	ret = BVTHead_GetPing(head, 0, &ping);
	if( ret != 0 )
	{
		printf("BVTHead_GetPing: ret=%d\n", ret);
		return 1;
	}
	
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
	float height;
	BVTMagImage_GetMaxRangeOfPixel(img, &height);
	printf("BVTMagImage_GetMaxRangeOfPixel: %d\n", height);



	// Clean up

	BVTMagImage_Destroy(img);
	BVTPing_Destroy(ping);
	BVTSonar_Destroy(son);
	return 0;
}
