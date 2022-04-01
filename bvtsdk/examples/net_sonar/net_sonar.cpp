/*
 * NET Sonar Example
 * Connect to a networked device, do a ping, and save it to a .son file
 * Note: Copy dll files from lib folder to output folder before running project.
 */

#include <stdio.h>

#include <bvt_sdk.h>

int main( void )
{
	int ret;
	
	// Create a new BVTSonar Object
	BVTSonar son = BVTSonar_Create();
	if( son == NULL )
	{
		printf("BVTSonar_Create: failed\n");
		return 1;
	}

	// Open the first sonar
	ret = BVTSonar_Open(son, "NET", "192.168.1.45"); // default ip address
	if( ret != 0 )
	{
		printf("BVTSonar_Open: ret=%d\n", ret);
		return 1;
	}

	// Make sure we have the right number of heads
	int heads;
	BVTSonar_GetHeadCount(son, &heads); //Return the number of heads on this sonar
	printf("BVTSonar_GetHeadCount: %d\n", heads);

	// Get the first head
	BVTHead head = NULL;
	ret = BVTSonar_GetHead(son, 0, &head);
	if( ret != 0 )
	{
        // some sonars start at head 1 instead of zero...
        ret = BVTSonar_GetHead(son, 1, &head);  //Retrieve a Head object from the sonar
        if( ret != 0 )
        {
            printf("BVTSonar_GetHead: ret=%d\n", ret);
            return 1;

        }
	}
	
	// Set the range window to be 1m to 40m
	BVTHead_SetRange(head, 1, 40);  //Set the range to be acquired.
	
	////////////////////////////////////////////////
	// Now, Create a file to save some pings to
	BVTSonar file = BVTSonar_Create();
	if( file == NULL )
	{
		printf("BVTSonar_Create: failed\n");
		return 1;
	}
	
	ret = BVTSonar_CreateFile(file, "out.son", son, "");  //Create a new data file. 
	if( ret != 0 )
	{
		printf("BVTSonar_CreateFile: ret=%d\n", ret);
		return 1;
	}

	// Request the first head
	BVTHead out_head = NULL;
	ret = BVTSonar_GetHead(file, 0, &out_head);
	if( ret != 0 )
	{
		printf("BVTSonar_GetHead: ret=%d\n", ret);
		return 1;
	}

	
	////////////////////////////////////////////////
	// Now, let's go get some pings!
    int num_pings = 10;
	for (int i=0; i < num_pings; i++)
	{
		BVTPing ping = NULL;
		ret = BVTHead_GetPing(head, -1, &ping);
		if( ret != 0 )
		{
			printf("BVTHead_GetPing: ret=%d\n", ret);
			return 1;
		}

		ret = BVTHead_PutPing(out_head, ping);  //Write a ping to a file.
		if( ret != 0 )
		{
			printf("BVTHead_PutPing: ret=%d\n", ret);
			return 1;
		}
		BVTPing_Destroy(ping);  //Destroy a BVTPing object
	}

    printf("Saved %d pings to out.son file\n", num_pings);

	BVTSonar_Destroy(file);  //Destroy a BVTSonar object
	BVTSonar_Destroy(son);  //Destroy a BVTSonar object
	return 0;
}