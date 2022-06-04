#ifndef UTM_H
#define UTM_H
#include "string.h"
double	GetDistanceLonLat(double long1, double lat1, double long2, double lat2);
double 	GetErrorLon(double long1, double lat1, double long2, double lat2);
double 	GetErrorLat(double long1, double lat1, double long2, double lat2);
//void 	GetDiffLat(double oriLon, double oriLat, double *aimLon, double *aimLat, double eastDis, double northDis);
float  	GetHeadingLonLat(double startLon, double startLat, double endLon, double endLat);
double 	GetDistanceEastNoth(double east1, double north1, double east2, double north2);
float  	GetHeadingEastNoth(double startEast, double startNorth, double endEast, double endNorth);

//long LonLat2UTM (double Latitude,double Longitude,  long   *Zone, char   *Hemisphere,
//                               double *Northing, double *Easting);
long LonLat2UTM (double Longitude,double Latitude,  long   *Zone, char   *Hemisphere,
                              double *Easting, double *Northing);

//long UTM2LonLat(long   Zone,  char   Hemisphere,   double Northing,  double Easting,
//                             double *Latitude,  double *Longitude);
long UTM2LonLat(long   Zone,  char   Hemisphere,  double Easting,   double Northing,
                             double *Longitude,  double *Latitude);

long SetTransverseMercatorPara(double a,  double f, double Origin_Latitude, double Central_Meridian,
                                        double False_Easting, double False_Northing, double Scale_Factor);

long Geodetic2TransverseMercator (double Latitude,double Longitude,double *Easting,double *Northing);

long TransverseMercator2Geodetic ( double Easting, double Northing, double *Latitude, double *Longitude);

#endif /* UTM_H */
