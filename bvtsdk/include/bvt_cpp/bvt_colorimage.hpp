/*
    This file has been generated by bvtidl.pl. DO NOT MODIFY!
*/

#ifndef __CPP_BVTCOLORIMAGE_HPP__
#define __CPP_BVTCOLORIMAGE_HPP__

#include "bvt_imagetype.hpp"
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

namespace BVTSDK
{

    /// <summary>
    /// Store a color image.  The API is nearly identical to MagImage.  The main difference is the 
    /// pixel datatype. In ColorImage, each pixel is a single unsigned int. 
    /// - Byte 0: Red Value
    /// - Byte 1: Green Value
    /// - Byte 2: Blue Value
    /// - Byte 3: Alpha Value
    /// <summary>
    class ColorImage
    {
    public:
        ColorImage()
        {
            BVTColorImage p = BVTColorImage_Create();
            _owned = std::shared_ptr<BVTOpaqueColorImage>(p , BVTColorImage_Destroy );
        }
    public: /*consider private*/
        ColorImage(BVTColorImage p)
        {
            _owned = std::shared_ptr<BVTOpaqueColorImage>(p , BVTColorImage_Destroy );
        }
    public:
        ~ColorImage()
        {
            
        }

    //public:
    //  ColorImage(const ColorImage &) {}
    //  ColorImage& operator=(const ColorImage &) {}
    public:
        /// SDK object pointer
        BVTColorImage Handle() const
        {
            return _owned.get();
        }
    private:
        std::shared_ptr<BVTOpaqueColorImage> _owned;

    public:
        //
        // Return the value of the pixel at (row, col)
        //
        // @param row Requested row 
        // @param col Requested col 
        // @param pixel Value of the pixel at (row, col)        
        unsigned int GetPixel(int row, int col)
        {
            unsigned int pixel;
            int error_code = BVTColorImage_GetPixel(_owned.get(), row, col, /* out */ &pixel);
            if (0 != error_code)
                throw SdkException(error_code);
            return pixel;
        }

        //
        // Set the value of the pixel at (row, col)
        //
        // @param row Requested row 
        // @param col Requested col 
        // @param pixel Value of the pixel at (row, col)        
        void SetPixel(int row, int col, unsigned int pixel)
        {
            int error_code = BVTColorImage_SetPixel(_owned.get(), row, col, pixel);
            if (0 != error_code)
                throw SdkException(error_code);
        }

        //
        // Set the value of the pixel at (row, col)
        //
        // @param row Requested row 
        // @param col Requested col 
        // @param pixel Value of the pixel at (row, col)        
        bool TrySetPixel(int row, int col, unsigned int pixel)
        {
            int error_code = BVTColorImage_SetPixel(_owned.get(), row, col, pixel);
            if (0 != error_code)
            return true;
            {
                return false;
            }
        }

        //
        // Return a pointer to a row of pixels  
        //
        // @param row Requested row index 
        // @param rowPointer pointer to pointer to row      
        unsigned int * GetRow(int row)
        {
            unsigned int * rowPointer;
            int error_code = BVTColorImage_GetRow(_owned.get(), row, /* out */ &rowPointer);
            if (0 != error_code)
                throw SdkException(error_code);
            return rowPointer;
        }

        //
        // Return a pointer to the entire image.
        // The image or organized in Row-Major order (just like C/C++).
        //
        // @param bitsPointer pointer to pointer to image       
        unsigned int * GetBits()
        {
            unsigned int * bitsPointer;
            int error_code = BVTColorImage_GetBits(_owned.get(), /* out */ &bitsPointer);
            if (0 != error_code)
                throw SdkException(error_code);
            return bitsPointer;
        }

        //
        // Copy the raw image data to the user specified buffer. See GetBits for more info.
        //
        // @param data Pointer to a valid buffer 
        // @param len The size of the buffer pointed to by data in pixels NOT bytes.        
        void CopyBits(unsigned int* data, unsigned int len)
        {
            int error_code = BVTColorImage_CopyBits(_owned.get(), data, len);
            if (0 != error_code)
                throw SdkException(error_code);
        }

        //
        // Return the number of boundaries for this image. A boundary defines
        // the limit of data for a transducer, and is always a line.
        //
        // @param count The number of boundaries defined for this   image.      
        int GetBoundaryCount()
        {
            int count;
            int error_code = BVTColorImage_GetBoundaryCount(_owned.get(), /* out */ &count);
            if (0 != error_code)
                throw SdkException(error_code);
            return count;
        }

        //
        // Get the true line segment for the boundary at the specified index.
        //
        // @param index The zero-based index of the boundary. 
        // @param x0 The near x-coordinate in meters. 
        // @param y0 The near y-coordinate in meters. 
        // @param x1 The far x-coordinate in meters. 
        // @param y1 The far y-coordinate   in meters.      
        void GetBoundaryLineSegment(int index, float *x0, float *y0, float *x1, float *y1)
        {
            int error_code = BVTColorImage_GetBoundaryLineSegment(_owned.get(), index, x0, y0, x1, y1);
            if (0 != error_code)
                throw SdkException(error_code);
        }

        //
        // Get the approximate bearing angle for the boundary at the specified index. A boundary may not necessarily start at the origin, so this
        // approximation may be too imprecise for some applications.
        //
        // @param index the zero-based index of the boundary 
        // @param radians the approximate bearing in radians of this boundary       
        float GetBoundaryRadiansApproximate(int index)
        {
            float radians;
            int error_code = BVTColorImage_GetBoundaryRadiansApproximate(_owned.get(), index, /* out */ &radians);
            if (0 != error_code)
                throw SdkException(error_code);
            return radians;
        }

        //
        // Save the image in PPM (PortablePixMap) format. http://en.wikipedia.org/wiki/Netpbm_format
        //
        // @param file_name File name to save to        
        void SavePPM(const std::string & file_name)
        {
            int error_code = BVTColorImage_SavePPM(_owned.get(), file_name.c_str());
            if (0 != error_code)
                throw SdkException(error_code);
        }

        //
        // Return the height (in pixels) of this image
        //
        // @param height image height       
        int GetHeight()
        {
            int height;
            int error_code = BVTColorImage_GetHeight(_owned.get(), /* out */ &height);
            if (0 != error_code)
                throw SdkException(error_code);
            return height;
        }

        //
        // Return the width (in pixels) of this image
        //
        // @param width image width     
        int GetWidth()
        {
            int width;
            int error_code = BVTColorImage_GetWidth(_owned.get(), /* out */ &width);
            if (0 != error_code)
                throw SdkException(error_code);
            return width;
        }

        //
        // Return the max range (in meters) to a non-blank pixel in the image
        //
        // @param maxRange max range to a non-blank pixel in the image      
        float GetMaxRangeOfPixel()
        {
            float maxRange;
            int error_code = BVTColorImage_GetMaxRangeOfPixel(_owned.get(), /* out */ &maxRange);
            if (0 != error_code)
                throw SdkException(error_code);
            return maxRange;
        }

        //
        // 
        //
        // @param type Image type constant (IMAGETYPE_*)        
        enum ImageType::Enum GetImageType()
        {
            int type;
            int error_code = BVTColorImage_GetImageType(_owned.get(), /* out */ &type);
            if (0 != error_code)
                throw SdkException(error_code);
            return (enum ImageType::Enum) type;
        }

        //
        // Return the range resolution of this image.
        // The resolution is returned in meters per pixel row.
        //
        // @param resolution image range resolution     
        double GetRangeResolution()
        {
            double resolution;
            int error_code = BVTColorImage_GetRangeResolution(_owned.get(), /* out */ &resolution);
            if (0 != error_code)
                throw SdkException(error_code);
            return resolution;
        }

        //
        // Only valid for R-Theta images.
        // Returns the bearing resolution, in degrees per pixel column.
        //
        // @param resolution image bearing resolution       
        double GetBearingResolution()
        {
            double resolution;
            int error_code = BVTColorImage_GetBearingResolution(_owned.get(), /* out */ &resolution);
            if (0 != error_code)
                throw SdkException(error_code);
            return resolution;
        }

        //
        // Retrieve the image row of the origin.
        // In most cases the origin row will be outside of the image boundaries (i.e., negative). The origin is the 'location' (in pixels) of the sonar head in image plane.
        //
        // @param row pixel row     
        int GetOriginRow()
        {
            int row;
            int error_code = BVTColorImage_GetOriginRow(_owned.get(), /* out */ &row);
            if (0 != error_code)
                throw SdkException(error_code);
            return row;
        }

        //
        // Retrieve the image column of the origin.
        // The origin is the 'location' (in pixels) of the
        // sonar head in image plane.
        //
        // @param column pixel column       
        int GetOriginCol()
        {
            int column;
            int error_code = BVTColorImage_GetOriginCol(_owned.get(), /* out */ &column);
            if (0 != error_code)
                throw SdkException(error_code);
            return column;
        }

        //
        // Retrieve the range (from the sonar head) of the specified pixel (in meters)
        //
        // @param row pixel row 
        // @param col pixel col 
        // @param range range to given pixel        
        double GetPixelRange(int row, int col)
        {
            double range;
            int error_code = BVTColorImage_GetPixelRange(_owned.get(), row, col, /* out */ &range);
            if (0 != error_code)
                throw SdkException(error_code);
            return range;
        }

        //
        // Retrieve the bearing relative to the sonar head of the specified pixel
        //
        // @param row pixel row 
        // @param col pixel col 
        // @param bearing bearing to given pixel        
        double GetPixelRelativeBearing(int row, int col)
        {
            double bearing;
            int error_code = BVTColorImage_GetPixelRelativeBearing(_owned.get(), row, col, /* out */ &bearing);
            if (0 != error_code)
                throw SdkException(error_code);
            return bearing;
        }

        //
        // Return the pixel coordinate at the specified physical location.
        // The returned coordinate may be outside this image's bounds.
        //
        // @param range range in meters 
        // @param bearing bearing in radians 
        // @param x pixel index in the X-axis 
        // @param y pixel index in the Y-axis       
        void GetPixelCoordinateAtRangeBearing(float range, float bearing, int *x, int *y)
        {
            int error_code = BVTColorImage_GetPixelCoordinateAtRangeBearing(_owned.get(), range, bearing, x, y);
            if (0 != error_code)
                throw SdkException(error_code);
        }

        //
        // Return the pixel coordinate at the specified physical location in the sonar's frame.
        // The returned coordinate may be outside this image's bounds.
        //
        // @param x_meters X distance in meters 
        // @param y_meters Y distance in meters 
        // @param x pixel index in the X-axis 
        // @param y pixel index in the Y-axis       
        void GetPixelCoordinateAtXY(float x_meters, float y_meters, int *x, int *y)
        {
            int error_code = BVTColorImage_GetPixelCoordinateAtXY(_owned.get(), x_meters, y_meters, x, y);
            if (0 != error_code)
                throw SdkException(error_code);
        }

        //
        // Return the minimum angle for the sonar's imaging field of view. 
        // The angle is returned in degrees and referenced with respect to sonar head boresight (clockwise-positive).
        //
        // @param angle min. angle      
        float GetFOVMinAngle()
        {
            float angle;
            int error_code = BVTColorImage_GetFOVMinAngle(_owned.get(), /* out */ &angle);
            if (0 != error_code)
                throw SdkException(error_code);
            return angle;
        }

        //
        // Return the maximum angle for the sonar's imaging field of view. 
        // The angle is returned in degrees and referenced with respect to sonar head boresight (clockwise-positive).
        //
        // @param angle max. angle      
        float GetFOVMaxAngle()
        {
            float angle;
            int error_code = BVTColorImage_GetFOVMaxAngle(_owned.get(), /* out */ &angle);
            if (0 != error_code)
                throw SdkException(error_code);
            return angle;
        }

    };

}

#endif