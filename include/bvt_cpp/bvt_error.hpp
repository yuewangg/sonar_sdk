/*
    This file has been generated by bvtidl.pl. DO NOT MODIFY!
*/

#ifndef __CPP_BVTERROR_HPP__
#define __CPP_BVTERROR_HPP__

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

namespace BVTSDK
{

    /// <summary>
    /// The Error object provides access to the SDK's error reporting 
    /// system.  This allows the user to map from an error number to 
    /// a human readable description of the error.
    /// <summary>
    class Error
    {
    public:
    public:
        //
        // Return a description of the error
        //
        // @param code Error code       
        static std::string GetString(int code)
        {
            return std::string(BVTError_GetString(code));
        }

        //
        // Return a the error number as a string.
        //
        // @param code Error code       
        static std::string GetName(int code)
        {
            return std::string(BVTError_GetName(code));
        }

    };

    class SdkException /* TODO: : std::exception */
    {
    public:
        SdkException(int returnCode) :
            _returnCode(returnCode),
            _errorName(Error::GetName(returnCode)),
            _errorMessage(Error::GetString(returnCode))
        {
        }

        int ReturnCode() const { return _returnCode; }
        const std::string ErrorName() const { return _errorName; }
        const std::string ErrorMessage() const { return _errorMessage; }

    private:
        const int _returnCode;
        const std::string _errorName;
        const std::string _errorMessage;
    };
}

#endif
