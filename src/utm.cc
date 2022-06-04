#include <math.h>
#include "utm.h"

#define UTM_NO_ERROR							0x0000
#define UTM_LAT_ERROR							0x0001
#define UTM_LON_ERROR							0x0002
#define UTM_EASTING_ERROR						0x0004
#define UTM_NORTHING_ERROR      				0x0008
#define UTM_ZONE_ERROR          				0x0010
#define UTM_HEMISPHERE_ERROR    				0x0020
#define UTM_ZONE_OVERRIDE_ERROR 				0x0040
#define UTM_A_ERROR             				0x0080
#define UTM_INV_F_ERROR         				0x0100

#define TRANMERC_NO_ERROR           			0x0000
#define TRANMERC_LAT_ERROR          			0x0001
#define TRANMERC_LON_ERROR          			0x0002
#define TRANMERC_EASTING_ERROR      			0x0004
#define TRANMERC_NORTHING_ERROR     			0x0008
#define TRANMERC_ORIGIN_LAT_ERROR   			0x0010
#define TRANMERC_CENT_MER_ERROR     			0x0020
#define TRANMERC_A_ERROR            			0x0040
#define TRANMERC_INV_F_ERROR        			0x0080
#define TRANMERC_SCALE_FACTOR_ERROR 			0x0100
#define TRANMERC_LON_WARNING       				0x0200

#define PI           							3.14159265358979323e0    /* PI                        */
#define MIN_LAT      							( (-80.5 * PI) / 180.0 ) /* -80.5 degrees in radians    */
#define MAX_LAT      							( (84.5 * PI) / 180.0 )  /* 84.5 degrees in radians     */
#define MIN_EASTING  							100000
#define MAX_EASTING  							900000
#define MIN_NORTHING 							0
#define MAX_NORTHING 							10000000

#define PI_OVER_2         						(PI/2.0e0)            /* PI over 2 */
#define MAX_DELTA_LONG  						((PI * 90)/180.0)       /* 90 degrees in radians */
#define MIN_SCALE_FACTOR  						0.3
#define MAX_SCALE_FACTOR  						3.0
#define D2R   									0.017453293

#define SPHTMD(Latitude) ((double) (TranMerc_ap * Latitude \
      - TranMerc_bp * sin(2.e0 * Latitude) + TranMerc_cp * sin(4.e0 * Latitude) \
      - TranMerc_dp * sin(6.e0 * Latitude) + TranMerc_ep * sin(8.e0 * Latitude) ) )

#define SPHSN(Latitude) ((double) (TranMerc_a / sqrt( 1.e0 - TranMerc_es * \
      pow(sin(Latitude), 2))))

#define SPHSR(Latitude) ((double) (TranMerc_a * (1.e0 - TranMerc_es) / \
    pow(DENOM(Latitude), 3)))

#define DENOM(Latitude) ((double) (sqrt(1.e0 - TranMerc_es * pow(sin(Latitude),2))))

static double UTM_a = 6378137.0; /* Semi-major axis of ellipsoid in meters  */
static double UTM_f = 1 / 298.257223563; /* Flattening of ellipsoid                 */
static long UTM_Override = 0; /* Zone override flag                      */

/* Ellipsoid Parameters, default to WGS 84  */
static double TranMerc_a = 6378137.0; /* Semi-major axis of ellipsoid in meters */
static double TranMerc_f = 1 / 298.257223563; /* Flattening of ellipsoid  */
static double TranMerc_es = 0.0066943799901413800; /* Eccentricity (0.08181919084262188000) squared */
static double TranMerc_ebs = 0.0067394967565869; /* Second Eccentricity squared */

/* Transverse_Mercator projection Parameters */
static double TranMerc_Origin_Lat = 0.0; /* Latitude of origin in radians */
static double TranMerc_Origin_Long = 0.0; /* Longitude of origin in radians */
static double TranMerc_False_Northing = 0.0; /* False northing in meters */
static double TranMerc_False_Easting = 0.0; /* False easting in meters */
static double TranMerc_Scale_Factor = 1.0; /* Scale factor  */

/* Isometeric to geodetic latitude parameters, default to WGS 84 */
static double TranMerc_ap = 6367449.1458008;
static double TranMerc_bp = 16038.508696861;
static double TranMerc_cp = 16.832613334334;
static double TranMerc_dp = 0.021984404273757;
static double TranMerc_ep = 3.1148371319283e-005;

/* Maximum variance for easting and northing values for WGS 84. */
static double TranMerc_Delta_Easting = 40000000.0;
static double TranMerc_Delta_Northing = 40000000.0;

double GetDistanceLonLat(double long1, double lat1, double long2, double lat2)
{
	double distance = 0;
	long zone1 = 0;
	long zone2 = 0;
	char hemisphere = 0;
	double north1 = 0;
	double east1 = 0;
	double north2 = 0;
	double east2 = 0;

	LonLat2UTM(lat1, long1, &zone1, &hemisphere, &east1, &north1);
	LonLat2UTM(lat2, long2, &zone2, &hemisphere, &east2, &north2);
	if(zone1 == zone2){
		distance = sqrt((north1 - north2) * (north1 - north2) + (east1 - east2) * (east1 - east2));
	}
	else{
		distance = sqrt((lat1-lat2)*60*1852*(lat1-lat2)*60*1852+(long1-long2)*(cos(lat1*D2R)+cos(lat2*D2R))*0.5*60*1852*(long1-long2)*(cos(lat1*D2R)+cos(lat2*D2R))*0.5*60*1852);
	}
	return distance;
}

double GetErrorLon(double long1, double lat1, double long2, double lat2)
{
	double errorLon = 0;
	long zone1 = 0;
	long zone2 = 0;
	char hemisphere = 0;
	double north1 = 0;
	double east1 = 0;
	double north2 = 0;
	double east2 = 0;

	LonLat2UTM(lat1, long1, &zone1, &hemisphere, &east1, &north1);
	LonLat2UTM(lat2, long2, &zone2, &hemisphere, &east2, &north2);

	if(zone1 == zone2){
		errorLon = east1 - east2;
	}
	else{
		errorLon = (long1-long2)*(cos(lat1*D2R)+cos(lat2*D2R))*0.5*60*1852;
	}
	return errorLon;
}

double GetErrorLat(double long1, double lat1, double long2, double lat2)
{
	double errorLat = 0;
	long zone = 0;
	char hemisphere = 0;
	double north1 = 0;
	double east1 = 0;
	double north2 = 0;
	double east2 = 0;

	LonLat2UTM(lat1, long1, &zone, &hemisphere, &east1, &north1);
	LonLat2UTM(lat2, long2, &zone, &hemisphere, &east2, &north2);

	errorLat = north1 - north2;

	return errorLat;
}

//void GetDiffLat(double oriLon, double oriLat, double *aimLon, double *aimLat, double eastDis, double northDis) //Do not Use for Now
//{
//	long zone = 0;
//	char hemisphere = 0;
//	double north = 0;
//	double east = 0;
//
//	LonLat2UTM(oriLat, oriLon, &zone, &hemisphere, &east, &north);
//	east += eastDis;
//	north += northDis;
//	UTM2LonLat(zone, hemisphere, east, north, aimLat, aimLon);
//}

float GetHeadingLonLat(double startLon, double startLat, double endLon, double endLat)
{
	float heading = 0;
	long zone1 = 0;
	long zone2 = 0;
	char hemisphere = 0;
	double startNorth = 0;
	double startEast = 0;
	double endNorth = 0;
	double endEast = 0;

	LonLat2UTM(startLat, startLon, &zone1, &hemisphere, &startEast, &startNorth);
	LonLat2UTM(endLat, endLon, &zone2, &hemisphere, &endEast, &endNorth);

	if(zone1 == zone2){
		heading = 180 / PI * (float) atan2((endEast - startEast), (endNorth - startNorth));
		if (heading < 0)
		{
			heading += 360;
		}
	}
	else{
		heading = 180 / PI * (float) atan2((endLon-startLon)*(cos(startLat*D2R)+cos(endLat*D2R))*0.5*60*1852, (endLat-startLat)*60*1852);
		if (heading < 0)
		{
			heading += 360;
		}
	}

	return heading;
}

double GetDistanceEastNoth(double east1, double north1, double east2, double north2)
{
	double distance = 0;
	distance = sqrt((north1 - north2) * (north1 - north2) + (east1 - east2) * (east1 - east2));
	return distance;
}

float GetHeadingEastNoth(double startEast, double startNorth, double endEast, double endNorth)
{
	float heading = 0;

	heading = 180 / PI * (float) atan2((endEast - startEast), (endNorth - startNorth));
	if (heading < 0)
	{
		heading += 360;
	}
	return heading;
}

//long LonLat2UTM(double Latitude, double Longitude, long *Zone, char *Hemisphere, double *Northing, double *Easting)
long LonLat2UTM(double Longitude, double Latitude, long *Zone, char *Hemisphere, double *Easting, double *Northing)
{
	long Lat_Degrees;
	long Long_Degrees;
	long temp_zone;
	long Error_Code = UTM_NO_ERROR;
	double Origin_Latitude = 0;
	double Central_Meridian = 0;
	double False_Easting = 500000;
	double False_Northing = 0;
	double Scale = 0.9996;
//	char north = 'N';
//	char south = 'S';

	Latitude = Latitude * PI / 180.0;
	Longitude = Longitude * PI / 180.0;
	if ((Latitude < MIN_LAT) || (Latitude > MAX_LAT))
	{ /* Latitude out of range */
		Error_Code |= UTM_LAT_ERROR;
	}
	if ((Longitude < -PI) || (Longitude > (2 * PI)))
	{ /* Longitude out of range */
		Error_Code |= UTM_LON_ERROR;
	}
	if (!Error_Code)
	{ /* no errors */
		if ((Latitude > -1.0e-9) && (Latitude < 0))
			Latitude = 0.0;
		if (Longitude < 0)
			Longitude += (2 * PI) + 1.0e-10;

		Lat_Degrees = (long) (Latitude * 180.0 / PI);
		Long_Degrees = (long) (Longitude * 180.0 / PI);

		if (Longitude < PI)
			temp_zone = (long) (31 + ((Longitude * 180.0 / PI) / 6.0));
		else
			temp_zone = (long) (((Longitude * 180.0 / PI) / 6.0) - 29);
		if (temp_zone > 60)
			temp_zone = 1;

		/* UTM special cases */
		if ((Lat_Degrees > 55) && (Lat_Degrees < 64) && (Long_Degrees > -1) && (Long_Degrees < 3))
			temp_zone = 31;
		if ((Lat_Degrees > 55) && (Lat_Degrees < 64) && (Long_Degrees > 2) && (Long_Degrees < 12))
			temp_zone = 32;
		if ((Lat_Degrees > 71) && (Long_Degrees > -1) && (Long_Degrees < 9))
			temp_zone = 31;
		if ((Lat_Degrees > 71) && (Long_Degrees > 8) && (Long_Degrees < 21))
			temp_zone = 33;
		if ((Lat_Degrees > 71) && (Long_Degrees > 20) && (Long_Degrees < 33))
			temp_zone = 35;
		if ((Lat_Degrees > 71) && (Long_Degrees > 32) && (Long_Degrees < 42))
			temp_zone = 37;

		if (UTM_Override)
		{
			if ((temp_zone == 1) && (UTM_Override == 60))
				temp_zone = UTM_Override;
			else if ((temp_zone == 60) && (UTM_Override == 1))
				temp_zone = UTM_Override;
			else if ((Lat_Degrees > 71) && (Long_Degrees > -1) && (Long_Degrees < 42))
			{
				if (((temp_zone - 2) <= UTM_Override) && (UTM_Override <= (temp_zone + 2)))
					temp_zone = UTM_Override;
				else
					Error_Code = UTM_ZONE_OVERRIDE_ERROR;
			}
			else if (((temp_zone - 1) <= UTM_Override) && (UTM_Override <= (temp_zone + 1)))
				temp_zone = UTM_Override;
			else
				Error_Code = UTM_ZONE_OVERRIDE_ERROR;
		}
		if (!Error_Code)
		{
			if (temp_zone >= 31)
				Central_Meridian = (6 * temp_zone - 183) * PI / 180.0;
			else
				Central_Meridian = (6 * temp_zone + 177) * PI / 180.0;
			*Zone = temp_zone;
			if (Latitude < 0)
			{
				False_Northing = 10000000;
				*Hemisphere = 'S';
//				memmove(Hemisphere,&south,1); //may cause core dump but useless now
			}
			else
				*Hemisphere = 'N';
//				memmove(Hemisphere,&north,1);//may cause core dump but useless now
			SetTransverseMercatorPara(UTM_a, UTM_f, Origin_Latitude, Central_Meridian, False_Easting, False_Northing, Scale);
			Geodetic2TransverseMercator(Latitude, Longitude, Easting, Northing);
			if ((*Easting < MIN_EASTING) || (*Easting > MAX_EASTING))
				Error_Code = UTM_EASTING_ERROR;
			if ((*Northing < MIN_NORTHING) || (*Northing > MAX_NORTHING))
				Error_Code |= UTM_NORTHING_ERROR;
		}
	} /* END OF if (!Error_Code) */
	return (Error_Code);
} /* END OF LonLat2UTM */

long UTM2LonLat(long Zone, char Hemisphere, double Easting, double Northing, double *Longitude, double *Latitude)
{
	long Error_Code = UTM_NO_ERROR;
	long tm_error_code = UTM_NO_ERROR;
	double Origin_Latitude = 0;
	double Central_Meridian = 0;
	double False_Easting = 500000;
	double False_Northing = 0;
	double Scale = 0.9996;

	if ((Zone < 1) || (Zone > 60))
		Error_Code |= UTM_ZONE_ERROR;
	if ((Hemisphere != 'S') && (Hemisphere != 'N'))
		Error_Code |= UTM_HEMISPHERE_ERROR;
	if ((Easting < MIN_EASTING) || (Easting > MAX_EASTING))
		Error_Code |= UTM_EASTING_ERROR;
	if ((Northing < MIN_NORTHING) || (Northing > MAX_NORTHING))
		Error_Code |= UTM_NORTHING_ERROR;
	if (!Error_Code)
	{ /* no errors */
		if (Zone >= 31)
			Central_Meridian = ((6 * Zone - 183) * PI / 180.0 /*+ 0.00000005*/);
		else
			Central_Meridian = ((6 * Zone + 177) * PI / 180.0 /*+ 0.00000005*/);
		if (Hemisphere == 'S')
			False_Northing = 10000000;
		SetTransverseMercatorPara(UTM_a, UTM_f, Origin_Latitude, Central_Meridian, False_Easting, False_Northing, Scale);

		tm_error_code = TransverseMercator2Geodetic(Easting, Northing, Latitude, Longitude);
		if (tm_error_code)
		{
			if (tm_error_code & TRANMERC_EASTING_ERROR)
				Error_Code |= UTM_EASTING_ERROR;
			if (tm_error_code & TRANMERC_NORTHING_ERROR)
				Error_Code |= UTM_NORTHING_ERROR;
		}

		if ((*Latitude < MIN_LAT) || (*Latitude > MAX_LAT))
		{ /* Latitude out of range */
			Error_Code |= UTM_NORTHING_ERROR;
		}
	}
	*Latitude = (*Latitude) * 180.0 / PI;
	*Longitude = (*Longitude) * 180.0 / PI;
	return (Error_Code);
} /* END OF UTM2LonLat */

long SetTransverseMercatorPara(double a, double f, double Origin_Latitude, double Central_Meridian, double False_Easting, double False_Northing, double Scale_Factor)
{
	double tn; /* True Meridianal distance constant  */
	double tn2;
	double tn3;
	double tn4;
	double tn5;
	double dummy_northing;
	double TranMerc_b; /* Semi-minor axis of ellipsoid, in meters */
	double inv_f = 1 / f;
	long Error_Code = TRANMERC_NO_ERROR;

	if (a <= 0.0)
	{ /* Semi-major axis must be greater than zero */
		Error_Code |= TRANMERC_A_ERROR;
	}
	if ((inv_f < 250) || (inv_f > 350))
	{ /* Inverse flattening must be between 250 and 350 */
		Error_Code |= TRANMERC_INV_F_ERROR;
	}
	if ((Origin_Latitude < -PI_OVER_2) || (Origin_Latitude > PI_OVER_2))
	{ /* origin latitude out of range */
		Error_Code |= TRANMERC_ORIGIN_LAT_ERROR;
	}
	if ((Central_Meridian < -PI) || (Central_Meridian > (2 * PI)))
	{ /* origin longitude out of range */
		Error_Code |= TRANMERC_CENT_MER_ERROR;
	}
	if ((Scale_Factor < MIN_SCALE_FACTOR) || (Scale_Factor > MAX_SCALE_FACTOR))
	{
		Error_Code |= TRANMERC_SCALE_FACTOR_ERROR;
	}
	if (!Error_Code)
	{ /* no errors */
		TranMerc_a = a;
		TranMerc_f = f;
		TranMerc_Origin_Lat = Origin_Latitude;
		if (Central_Meridian > PI)
			Central_Meridian -= (2 * PI);
		TranMerc_Origin_Long = Central_Meridian;
		TranMerc_False_Northing = False_Northing;
		TranMerc_False_Easting = False_Easting;
		TranMerc_Scale_Factor = Scale_Factor;

		/* Eccentricity Squared */
		TranMerc_es = 2 * TranMerc_f - TranMerc_f * TranMerc_f;
		/* Second Eccentricity Squared */
		TranMerc_ebs = (1 / (1 - TranMerc_es)) - 1;

		TranMerc_b = TranMerc_a * (1 - TranMerc_f);
		/*True meridianal constants  */
		tn = (TranMerc_a - TranMerc_b) / (TranMerc_a + TranMerc_b);
		tn2 = tn * tn;
		tn3 = tn2 * tn;
		tn4 = tn3 * tn;
		tn5 = tn4 * tn;

		TranMerc_ap = TranMerc_a * (1.e0 - tn + 5.e0 * (tn2 - tn3) / 4.e0 + 81.e0 * (tn4 - tn5) / 64.e0);
		TranMerc_bp = 3.e0 * TranMerc_a * (tn - tn2 + 7.e0 * (tn3 - tn4) / 8.e0 + 55.e0 * tn5 / 64.e0) / 2.e0;
		TranMerc_cp = 15.e0 * TranMerc_a * (tn2 - tn3 + 3.e0 * (tn4 - tn5) / 4.e0) / 16.0;
		TranMerc_dp = 35.e0 * TranMerc_a * (tn3 - tn4 + 11.e0 * tn5 / 16.e0) / 48.e0;
		TranMerc_ep = 315.e0 * TranMerc_a * (tn4 - tn5) / 512.e0;
		Geodetic2TransverseMercator(MAX_LAT, MAX_DELTA_LONG + Central_Meridian, &TranMerc_Delta_Easting, &TranMerc_Delta_Northing);
		Geodetic2TransverseMercator(0, MAX_DELTA_LONG + Central_Meridian, &TranMerc_Delta_Easting, &dummy_northing);
		TranMerc_Delta_Northing++;
		TranMerc_Delta_Easting++;

	} /* END OF if(!Error_Code) */
	return (Error_Code);
} /* END of SetTransverseMercatorPara  */

long Geodetic2TransverseMercator(double Latitude, double Longitude, double *Easting, double *Northing)
{
	double c; /* Cosine of latitude                          */
	double c2;
	double c3;
	double c5;
	double c7;
	double dlam; /* Delta longitude - Difference in Longitude       */
	double eta; /* constant - TranMerc_ebs *c *c                   */
	double eta2;
	double eta3;
	double eta4;
	double s; /* Sine of latitude                        */
	double sn; /* Radius of curvature in the prime vertical       */
	double t; /* Tangent of latitude                             */
	double tan2;
	double tan3;
	double tan4;
	double tan5;
	double tan6;
	double t1; /* Term in coordinate conversion formula - GP to Y */
	double t2; /* Term in coordinate conversion formula - GP to Y */
	double t3; /* Term in coordinate conversion formula - GP to Y */
	double t4; /* Term in coordinate conversion formula - GP to Y */
	double t5; /* Term in coordinate conversion formula - GP to Y */
	double t6; /* Term in coordinate conversion formula - GP to Y */
	double t7; /* Term in coordinate conversion formula - GP to Y */
	double t8; /* Term in coordinate conversion formula - GP to Y */
	double t9; /* Term in coordinate conversion formula - GP to Y */
	double tmd; /* True Meridional distance                        */
	double tmdo; /* True Meridional distance for latitude of origin */
	long Error_Code = TRANMERC_NO_ERROR;
	double temp_Origin;
	double temp_Long;

	if ((Latitude < -MAX_LAT) || (Latitude > MAX_LAT))
	{ /* Latitude out of range */
		Error_Code |= TRANMERC_LAT_ERROR;
	}
	if (Longitude > PI)
		Longitude -= (2 * PI);
	if ((Longitude < (TranMerc_Origin_Long - MAX_DELTA_LONG)) || (Longitude > (TranMerc_Origin_Long + MAX_DELTA_LONG)))
	{
		if (Longitude < 0)
			temp_Long = Longitude + 2 * PI;
		else
			temp_Long = Longitude;
		if (TranMerc_Origin_Long < 0)
			temp_Origin = TranMerc_Origin_Long + 2 * PI;
		else
			temp_Origin = TranMerc_Origin_Long;
		if ((temp_Long < (temp_Origin - MAX_DELTA_LONG)) || (temp_Long > (temp_Origin + MAX_DELTA_LONG)))
			Error_Code |= TRANMERC_LON_ERROR;
	}
	if (!Error_Code)
	{ /* no errors */

		/*
		 *  Delta Longitude
		 */
		dlam = Longitude - TranMerc_Origin_Long;

		if (fabs(dlam) > (9.0 * PI / 180))
		{ /* Distortion will result if Longitude is more than 9 degrees from the Central Meridian */
			Error_Code |= TRANMERC_LON_WARNING;
		}

		if (dlam > PI)
			dlam -= (2 * PI);
		if (dlam < -PI)
			dlam += (2 * PI);
		if (fabs(dlam) < 2.e-10)
			dlam = 0.0;

		s = sin(Latitude);
		c = cos(Latitude);
		c2 = c * c;
		c3 = c2 * c;
		c5 = c3 * c2;
		c7 = c5 * c2;
		t = tan(Latitude);
		tan2 = t * t;
		tan3 = tan2 * t;
		tan4 = tan3 * t;
		tan5 = tan4 * t;
		tan6 = tan5 * t;
		eta = TranMerc_ebs * c2;
		eta2 = eta * eta;
		eta3 = eta2 * eta;
		eta4 = eta3 * eta;

		/* radius of curvature in prime vertical */
		sn = SPHSN(Latitude);

		/* True Meridianal Distances */
		tmd = SPHTMD(Latitude);

		/*  Origin  */
		tmdo = SPHTMD (TranMerc_Origin_Lat);

		/* northing */
		t1 = (tmd - tmdo) * TranMerc_Scale_Factor;
		t2 = sn * s * c * TranMerc_Scale_Factor / 2.e0;
		t3 = sn * s * c3 * TranMerc_Scale_Factor * (5.e0 - tan2 + 9.e0 * eta + 4.e0 * eta2) / 24.e0;

		t4 = sn * s * c5 * TranMerc_Scale_Factor * (61.e0 - 58.e0 * tan2 + tan4 + 270.e0 * eta - 330.e0 * tan2 * eta + 445.e0 * eta2 + 324.e0 * eta3 - 680.e0 * tan2 * eta2 + 88.e0 * eta4 - 600.e0
				* tan2 * eta3 - 192.e0 * tan2 * eta4) / 720.e0;

		t5 = sn * s * c7 * TranMerc_Scale_Factor * (1385.e0 - 3111.e0 * tan2 + 543.e0 * tan4 - tan6) / 40320.e0;

		*Northing = TranMerc_False_Northing + t1 + pow(dlam, 2.e0) * t2 + pow(dlam, 4.e0) * t3 + pow(dlam, 6.e0) * t4 + pow(dlam, 8.e0) * t5;

		/* Easting */
		t6 = sn * c * TranMerc_Scale_Factor;
		t7 = sn * c3 * TranMerc_Scale_Factor * (1.e0 - tan2 + eta) / 6.e0;
		t8 = sn * c5 * TranMerc_Scale_Factor * (5.e0 - 18.e0 * tan2 + tan4 + 14.e0 * eta - 58.e0 * tan2 * eta + 13.e0 * eta2 + 4.e0 * eta3 - 64.e0 * tan2 * eta2 - 24.e0 * tan2 * eta3) / 120.e0;
		t9 = sn * c7 * TranMerc_Scale_Factor * (61.e0 - 479.e0 * tan2 + 179.e0 * tan4 - tan6) / 5040.e0;

		*Easting = TranMerc_False_Easting + dlam * t6 + pow(dlam, 3.e0) * t7 + pow(dlam, 5.e0) * t8 + pow(dlam, 7.e0) * t9;
	}
	return (Error_Code);
} /* END OF Geodetic2TransverseMercator */

long TransverseMercator2Geodetic(double Easting, double Northing, double *Latitude, double *Longitude)
{
	double c; /* Cosine of latitude                          */
	double de; /* Delta easting - Difference in Easting (Easting-Fe)    */
	double dlam; /* Delta longitude - Difference in Longitude       */
	double eta; /* constant - TranMerc_ebs *c *c                   */
	double eta2;
	double eta3;
	double eta4;
	double ftphi; /* Footpoint latitude                              */
	int i; /* Loop iterator                   */
	double s; /* Sine of latitude                        */
	double sn; /* Radius of curvature in the prime vertical       */
	double sr; /* Radius of curvature in the meridian             */
	double t; /* Tangent of latitude                             */
	double tan2;
	double tan4;
	double t10; /* Term in coordinate conversion formula - GP to Y */
	double t11; /* Term in coordinate conversion formula - GP to Y */
	double t12; /* Term in coordinate conversion formula - GP to Y */
	double t13; /* Term in coordinate conversion formula - GP to Y */
	double t14; /* Term in coordinate conversion formula - GP to Y */
	double t15; /* Term in coordinate conversion formula - GP to Y */
	double t16; /* Term in coordinate conversion formula - GP to Y */
	double t17; /* Term in coordinate conversion formula - GP to Y */
	double tmd; /* True Meridional distance                        */
	double tmdo; /* True Meridional distance for latitude of origin */
	long Error_Code = TRANMERC_NO_ERROR;

	if ((Easting < (TranMerc_False_Easting - TranMerc_Delta_Easting)) || (Easting > (TranMerc_False_Easting + TranMerc_Delta_Easting)))
	{ /* Easting out of range  */
		Error_Code |= TRANMERC_EASTING_ERROR;
	}
	if ((Northing < (TranMerc_False_Northing - TranMerc_Delta_Northing)) || (Northing > (TranMerc_False_Northing + TranMerc_Delta_Northing)))
	{ /* Northing out of range */
		Error_Code |= TRANMERC_NORTHING_ERROR;
	}

	if (!Error_Code)
	{
		/* True Meridional Distances for latitude of origin */
		tmdo = SPHTMD(TranMerc_Origin_Lat);

		/*  Origin  */
		tmd = tmdo + (Northing - TranMerc_False_Northing) / TranMerc_Scale_Factor;

		/* First Estimate */
		sr = SPHSR(0.e0);
		ftphi = tmd / sr;

		for (i = 0; i < 5; i++)
		{
			t10 = SPHTMD (ftphi);
			sr = SPHSR(ftphi);
			ftphi = ftphi + (tmd - t10) / sr;
		}

		/* Radius of Curvature in the meridian */
		sr = SPHSR(ftphi);

		/* Radius of Curvature in the meridian */
		sn = SPHSN(ftphi);

		/* Sine Cosine terms */
		s = sin(ftphi);
		c = cos(ftphi);

		/* Tangent Value  */
		t = tan(ftphi);
		tan2 = t * t;
		tan4 = tan2 * tan2;
		eta = TranMerc_ebs * pow(c, 2);
		eta2 = eta * eta;
		eta3 = eta2 * eta;
		eta4 = eta3 * eta;
		de = Easting - TranMerc_False_Easting;
		if (fabs(de) < 0.0001)
			de = 0.0;

		/* Latitude */
		t10 = t / (2.e0 * sr * sn * pow(TranMerc_Scale_Factor, 2));
		t11 = t * (5.e0 + 3.e0 * tan2 + eta - 4.e0 * pow(eta, 2) - 9.e0 * tan2 * eta) / (24.e0 * sr * pow(sn, 3) * pow(TranMerc_Scale_Factor, 4));
		t12 = t * (61.e0 + 90.e0 * tan2 + 46.e0 * eta + 45.E0 * tan4 - 252.e0 * tan2 * eta - 3.e0 * eta2 + 100.e0 * eta3 - 66.e0 * tan2 * eta2 - 90.e0 * tan4 * eta + 88.e0 * eta4 + 225.e0 * tan4
				* eta2 + 84.e0 * tan2 * eta3 - 192.e0 * tan2 * eta4) / (720.e0 * sr * pow(sn, 5) * pow(TranMerc_Scale_Factor, 6));
		t13 = t * (1385.e0 + 3633.e0 * tan2 + 4095.e0 * tan4 + 1575.e0 * pow(t, 6)) / (40320.e0 * sr * pow(sn, 7) * pow(TranMerc_Scale_Factor, 8));
		*Latitude = ftphi - pow(de, 2) * t10 + pow(de, 4) * t11 - pow(de, 6) * t12 + pow(de, 8) * t13;

		t14 = 1.e0 / (sn * c * TranMerc_Scale_Factor);

		t15 = (1.e0 + 2.e0 * tan2 + eta) / (6.e0 * pow(sn, 3) * c * pow(TranMerc_Scale_Factor, 3));

		t16 = (5.e0 + 6.e0 * eta + 28.e0 * tan2 - 3.e0 * eta2 + 8.e0 * tan2 * eta + 24.e0 * tan4 - 4.e0 * eta3 + 4.e0 * tan2 * eta2 + 24.e0 * tan2 * eta3) / (120.e0 * pow(sn, 5) * c * pow(
				TranMerc_Scale_Factor, 5));

		t17 = (61.e0 + 662.e0 * tan2 + 1320.e0 * tan4 + 720.e0 * pow(t, 6)) / (5040.e0 * pow(sn, 7) * c * pow(TranMerc_Scale_Factor, 7));

		/* Difference in Longitude */
		dlam = de * t14 - pow(de, 3) * t15 + pow(de, 5) * t16 - pow(de, 7) * t17;

		/* Longitude */
		(*Longitude) = TranMerc_Origin_Long + dlam;

		if ((fabs)(*Latitude) > (90.0 * PI / 180.0))
			Error_Code |= TRANMERC_NORTHING_ERROR;

		if ((*Longitude) > (PI))
		{
			*Longitude -= (2 * PI);
			if ((fabs)(*Longitude) > PI)
				Error_Code |= TRANMERC_EASTING_ERROR;
		}
		else if ((*Longitude) < (-PI))
		{
			*Longitude += (2 * PI);
			if ((fabs)(*Longitude) > PI)
				Error_Code |= TRANMERC_EASTING_ERROR;
		}

		if (fabs(dlam) > (9.0 * PI / 180) * cos(*Latitude))
		{ /* Distortion will result if Longitude is more than 9 degrees from the Central Meridian at the equator */
			/* and decreases to 0 degrees at the poles */
			/* As you move towards the poles, distortion will become more significant */
			Error_Code |= TRANMERC_LON_WARNING;
		}
	}
	return (Error_Code);
} /* END OF TransverseMercator2Geodetic */
