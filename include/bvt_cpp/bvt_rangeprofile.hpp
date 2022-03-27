/*
    This file has been generated by bvtidl.pl. DO NOT MODIFY!
*/

#ifndef __CPP_BVTRANGEPROFILE_HPP__
#define __CPP_BVTRANGEPROFILE_HPP__

#include <string>
#ifdef _WIN32
#   include <memory>
#else
#   include <cstddef>
#   if defined (__GLIBCXX__) && __cplusplus <= 199711L
#      include <tr1/memory>
       namespace std {
          using std::tr1::shared_ptr;
       }
#   else
#       include <memory>
#   endif
#endif
#include <bvt_cpp/bvt_retval.hpp>
#include <bvt_cpp/bvt_error.hpp>
#include <bvt_cpp/bvt_colorimage.hpp>

namespace BVTSDK
{

    class ColorImage;

    /// <summary>
    /// @warning RangeProfile functions will fail on a sonar with old firmware, or a file recorded from a sonar with old firmware.
    /// The RangeProfile interface provides a 1-D view of a single ping data. It consists of a vector of ranges from the sonar head to the sonic reflection point, indexed along the the bearing (theta) dimension.
    /// For each angle, the range and raw intensity of the return beam at that range is stored. There are a number of approaches to choosing the reflection point. This interface provides a settable minimum intensity threshold that must be crossed and a choosable algorithm, returning either the first (nearest) point that exceeds the threshold (THRESHOLD_POLICY_NEAREST) or the distance to sample of greatest intensity (THRESHOLD_POLICY_LARGEST).
    /// <summary>
    class RangeProfile
    {
    public:
    public: /*consider private*/
        RangeProfile(BVTRangeProfile p)
        {
            _owned = std::shared_ptr<BVTOpaqueRangeProfile>(p , BVTRangeProfile_Destroy );
        }
    public:
        ~RangeProfile()
        {
            
        }

    //public:
    //  RangeProfile(const RangeProfile &) {}
    //  RangeProfile& operator=(const RangeProfile &) {}
    public:
        /// SDK object pointer
        BVTRangeProfile Handle() const
        {
            return _owned.get();
        }
    private:
        std::shared_ptr<BVTOpaqueRangeProfile> _owned;

    public:
        /// <summary>
        /// Values greater than this indicate no range could be measured. 
        /// </summary>
        static const int MAX_RANGE = 999;

        /// <summary>
        /// Return the largest intensity greater than the threshold found along a beam. 
        /// </summary>
        static const int THRESHOLD_POLICY_LARGEST = 1;

        /// <summary>
        /// Return the nearest intensity greater than the threshold along a beam. 
        /// </summary>
        static const int THRESHOLD_POLICY_NEAREST = 2;

        //
        // Returns the number of range values stored for this ping.
        //
        // @param count range data entry count      
        int GetCount()
        {
            int count;
            int error_code = BVTRangeProfile_GetCount(_owned.get(), /* out */ &count);
            if (0 != error_code)
                throw SdkException(error_code);
            return count;
        }

        //
        // Returns the number of valid range values stored for this ping.
        //
        // @param count range data entry count      
        int GetValidCount()
        {
            int count;
            int error_code = BVTRangeProfile_GetValidCount(_owned.get(), /* out */ &count);
            if (0 != error_code)
                throw SdkException(error_code);
            return count;
        }

        //
        // Returns the intensity threshold used to populate this RangeProfile structure. The intensity threshold serves as a noise floor, below which no sample will be considered a candidate for the beam's RangeProfile point. 
        //
        // @param threshold raw intensity threshold     
        unsigned short GetIntensityThreshold()
        {
            unsigned short threshold;
            int error_code = BVTRangeProfile_GetIntensityThreshold(_owned.get(), /* out */ &threshold);
            if (0 != error_code)
                throw SdkException(error_code);
            return threshold;
        }

        //
        // Returns the resolution of the range values, in meters. The RangeProfile range value at a given bearing should be considered approximate to within +- resolution.
        //
        // @param resolution range resolution       
        double GetRangeResolution()
        {
            double resolution;
            int error_code = BVTRangeProfile_GetRangeResolution(_owned.get(), /* out */ &resolution);
            if (0 != error_code)
                throw SdkException(error_code);
            return resolution;
        }

        //
        // Returns the resolution of the bearing (in degrees) of each RangeProfile range value. This is the difference in bearing between adjacent range values in the array.
        // <br>
        //
        // @param resolution bearing (angular) resolution       
        double GetBearingResolution()
        {
            double resolution;
            int error_code = BVTRangeProfile_GetBearingResolution(_owned.get(), /* out */ &resolution);
            if (0 != error_code)
                throw SdkException(error_code);
            return resolution;
        }

        //
        // Return the minimum angle for the sonar's imaging field of view.
        // This is the angle of the first range value, as all
        // angles are "left referenced." The angle is returned in degrees.
        // Note that this may not represent the actual physical field of view
        // of a particular sonar, but does represent the field of view of the
        // data being returned. Some outer angles may have range values
        // indicating they are out of range.
        //
        // @param minAngle minimum angle in field-of-view       
        float GetFOVMinAngle()
        {
            float minAngle;
            int error_code = BVTRangeProfile_GetFOVMinAngle(_owned.get(), /* out */ &minAngle);
            if (0 != error_code)
                throw SdkException(error_code);
            return minAngle;
        }

        //
        // Return the maximum angle for the sonar's imaging field of view.
        // This is the angle of the last range value, as all
        // angles are "left referenced." The angle is returned in degrees.
        // Note that this may not represent the actual physical field of view
        // of a particular sonar, but does represent the field of view of the
        // data being returned. Some outer angles may have range values
        // indicating they are out of range.
        //
        // @param maxAngle maximum angle in field-of-view       
        float GetFOVMaxAngle()
        {
            float maxAngle;
            int error_code = BVTRangeProfile_GetFOVMaxAngle(_owned.get(), /* out */ &maxAngle);
            if (0 != error_code)
                throw SdkException(error_code);
            return maxAngle;
        }

        //
        // Copies the range values into the user specified buffer. The
        // buffer must hold the entire number of ranges (See GetCount() above),
        // or an error is returned.
        //
        // @param ranges Pointer to a valid buffer of type float.
        // @param number_of_ranges Number of values the buffer can hold.        
        void CopyRangeValues(float* ranges, int number_of_ranges)
        {
            int error_code = BVTRangeProfile_CopyRangeValues(_owned.get(), ranges, number_of_ranges);
            if (0 != error_code)
                throw SdkException(error_code);
        }

        //
        // Copies the bearing values into the user specified buffer. The
        // buffer must hold the entire number of bearings (See GetCount() above),
        // or an error is returned.
        //
        // @param bearings Pointer to a valid buffer of type float.
        // @param number_of_bearings Number of values the buffer can hold.      
        void CopyBearingValues(float* bearings, int number_of_bearings)
        {
            int error_code = BVTRangeProfile_CopyBearingValues(_owned.get(), bearings, number_of_bearings);
            if (0 != error_code)
                throw SdkException(error_code);
        }

        //
        // Copies the intensity values into the user specified buffer. The
        // buffer must hold the entire number of intensities (See GetCount() above),
        // or an error is returned.
        //
        // @param intensities Pointer to a valid buffer of type float.
        // @param number_of_intensities Number of values the buffer can hold.       
        void CopyIntensityValues(unsigned short* intensities, int number_of_intensities)
        {
            int error_code = BVTRangeProfile_CopyIntensityValues(_owned.get(), intensities, number_of_intensities);
            if (0 != error_code)
                throw SdkException(error_code);
        }

        //
        // Returns the range from the sonar head, in meters, at a particular
        // index into the array. <br>
        // NOTE: Check all returned values for validity. If range > BVTRANGEPROFILE_MAX_RANGE
        // then the range at the given bearing (index) is not valid.
        // This is the result of either the nearest object at the given bearing was out of view of the sonar, or no return along that beam crossed the specified threshold.
        //
        // @param index index into the array of RangeProfile values  
        // @param range range at given index        
        float GetRangeValue(int index)
        {
            float range;
            int error_code = BVTRangeProfile_GetRangeValue(_owned.get(), index, /* out */ &range);
            if (0 != error_code)
                throw SdkException(error_code);
            return range;
        }

        //
        // Returns the intensity value at the specified index into the RangeProfile array. <br>
        //
        // @param index index into the array of RangeProfile values  
        // @param intensity raw intensity value         
        unsigned short GetIntensityValue(int index)
        {
            unsigned short intensity;
            int error_code = BVTRangeProfile_GetIntensityValue(_owned.get(), index, /* out */ &intensity);
            if (0 != error_code)
                throw SdkException(error_code);
            return intensity;
        }

        //
        // Returns the bearing from the center of the sonar head, in degrees (positive is clockwise as viewed from above) at the given index into the RangeProfile array.
        //
        // @param index index into the array of RangeProfile values  
        // @param bearing bearing (angle) at given index        
        float GetBearingValue(int index)
        {
            float bearing;
            int error_code = BVTRangeProfile_GetBearingValue(_owned.get(), index, /* out */ &bearing);
            if (0 != error_code)
                throw SdkException(error_code);
            return bearing;
        }

        //
        // 
        //
        // @param points an array to be filled with points on the profile
        // @param number_of_points the number of points to return (shall not be greater then result of GetCount)        
        void CopyProfileValues(struct BVTProfilePoint * points, int number_of_points)
        {
            int error_code = BVTRangeProfile_CopyProfileValues(_owned.get(), points, number_of_points);
            if (0 != error_code)
                throw SdkException(error_code);
        }

        //
        // 
        //
        // @param points an array to be filled with valid points on the profile
        // @param number_of_points the number of points to return (shall not be greater then result of GetValidCount)       
        void CopyValidProfileValues(struct BVTProfilePoint * points, int number_of_points)
        {
            int error_code = BVTRangeProfile_CopyValidProfileValues(_owned.get(), points, number_of_points);
            if (0 != error_code)
                throw SdkException(error_code);
        }

        //
        // Returns the x position seen from the center of the sonar head.
        //
        // @param index index into the array of values  
        // @param x x at given index        
        float Get_X_Value(int index)
        {
            float x;
            int error_code = BVTRangeProfile_Get_X_Value(_owned.get(), index, /* out */ &x);
            if (0 != error_code)
                throw SdkException(error_code);
            return x;
        }

        //
        // Returns the y position seen from the center of the sonar head.
        //
        // @param index index into the array of values  
        // @param y y at given index        
        float Get_Y_Value(int index)
        {
            float y;
            int error_code = BVTRangeProfile_Get_Y_Value(_owned.get(), index, /* out */ &y);
            if (0 != error_code)
                throw SdkException(error_code);
            return y;
        }

        //
        // Returns the z position seen from the center of the sonar head.
        //
        // @param index index into the array of values  
        // @param z z at given index        
        float Get_Z_Value(int index)
        {
            float z;
            int error_code = BVTRangeProfile_Get_Z_Value(_owned.get(), index, /* out */ &z);
            if (0 != error_code)
                throw SdkException(error_code);
            return z;
        }

        //
        // Returns the time (in ms) relative to the first sub ping of the sonar head.
        //
        // @param index index into the array of values  
        // @param time time at given index      
        int GetTimeValue(int index)
        {
            int time;
            int error_code = BVTRangeProfile_GetTimeValue(_owned.get(), index, /* out */ &time);
            if (0 != error_code)
                throw SdkException(error_code);
            return time;
        }

        //
        // Returns a bool telling if the given index in the value array is valid
        //
        // @param index index into the array of values  
        // @param valid valid at given index        
        bool GetValidValue(int index)
        {
            int valid;
            int error_code = BVTRangeProfile_GetValidValue(_owned.get(), index, /* out */ &valid);
            if (0 != error_code)
                throw SdkException(error_code);
            return valid > 0;
        }

        //
        // Returns the X coordinate for the pixel in the passed ColorImage, which
        // maps to the range and bearing at the index passed. This allows placing
        // of the range data on a colorimage, easing analysis of the algorithm
        // used for thresholding.
        //
        // @param index index into the array of RangeProfile values  
        // @param image ColorImage object where the pixel coordinate is needed 
        // @param x pixel index in the X axis       
        int GetColorImagePixelX(int index, const ColorImage & image)
        {
            int x;
            int error_code = BVTRangeProfile_GetColorImagePixelX(_owned.get(), index, image.Handle(), /* out */ &x);
            if (0 != error_code)
                throw SdkException(error_code);
            return x;
        }

        //
        // Returns the Y coordinate for the pixel in the passed ColorImage which
        // maps to the range and bearing at the index passed. (see similar function,
        // above, for more details)
        //
        // @param index index into the array of RangeProfile values  
        // @param image ColorImage object where the pixel coordinate is needed 
        // @param y pixel index in the Y axis       
        int GetColorImagePixelY(int index, const ColorImage & image)
        {
            int y;
            int error_code = BVTRangeProfile_GetColorImagePixelY(_owned.get(), index, image.Handle(), /* out */ &y);
            if (0 != error_code)
                throw SdkException(error_code);
            return y;
        }

    };

}

#endif
