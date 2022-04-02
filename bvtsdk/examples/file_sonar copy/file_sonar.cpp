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

char DataFile[] = "../../data/20m.son";

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

    BVTImageGenerator ig = BVTImageGenerator_Create();
    BVTImageGenerator_SetHead(ig, head);

    // Now, get a ping!
	BVTPing ping = NULL;
	ret = BVTHead_GetPing(head, 200, &ping);
	if( ret != 0 )
	{
		printf("BVTHead_GetPing: ret=%d\n", ret);
		return 1;
	}
	
	// Generate an RangeProfile from the ping
	BVTRangeProfile rgp;
	
	ret = BVTImageGenerator_GetRangeProfile(ig, ping, &rgp);
	if( ret != 0 )
	{
		printf("BVTHead_GetRangeCount: ret=%d\n", ret);
		return 1;
	}

	float * range;
	unsigned short * intensity;
	int * count;
	ret = BVTRangeProfile_GetCount(rgp,count);
	if( ret != 0 )
	{
		printf("BVTHead_GetRangeCount: ret=%d\n", ret);
		return 1;
	}

	BVTRangeProfile_GetIntensityValue(rgp,0,intensity);


	ret = BVTRangeProfile_GetRangeValue(rgp,0,range);
	if( ret != 0 )
	{
		printf("BVTHead_GetRangeValue: ret=%d\n", ret);
		return 1;
	}

	ret = BVTRangeProfile_GetBearingValue(rgp,0,range);
	if( ret != 0 )
	{
		printf("BVTHead_GetRangeValue: ret=%d\n", ret);
		return 1;
	}
	
	// Clean up
	BVTRangeProfile_Destroy(rgp);
	BVTPing_Destroy(ping);
	BVTSonar_Destroy(son);
	return 0;
}
