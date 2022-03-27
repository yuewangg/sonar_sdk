/*
    This file has been generated by bvtidl.pl. DO NOT MODIFY!
*/

#ifndef __CPP_BVTNETWORKSETTINGS_HPP__
#define __CPP_BVTNETWORKSETTINGS_HPP__

#include "bvt_addressmode.hpp"
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
    /// Various network-related parameters.
    /// \warning You should not modify these settings unless you carefully record the new
    /// parameters!
    /// \note A sonar will typically only apply these
    /// parameters once per power cycle, at boot.
    /// <summary>
    class NetworkSettings
    {
    public:
    public: /*consider private*/
        NetworkSettings(BVTNetworkSettings p)
        {
            _owned = std::shared_ptr<BVTOpaqueNetworkSettings>(p , BVTNetworkSettings_Destroy );
        }
    public:
        ~NetworkSettings()
        {
            
        }

    //public:
    //  NetworkSettings(const NetworkSettings &) {}
    //  NetworkSettings& operator=(const NetworkSettings &) {}
    public:
        /// SDK object pointer
        BVTNetworkSettings Handle() const
        {
            return _owned.get();
        }
    private:
        std::shared_ptr<BVTOpaqueNetworkSettings> _owned;

    public:
        //
        // Set the method by which the sonar obtains an IP address. 
        //
        // @param mode the method by which the sonar obtains an IP address.         
        void SetAddressMode(enum AddressMode::Enum mode)
        {
            int error_code = BVTNetworkSettings_SetAddressMode(_owned.get(), ((int) mode));
            if (0 != error_code)
                throw SdkException(error_code);
        }

        //
        // Get the method by which the sonar obtains an IP address.
        //
        // @param mode the method by which the sonar obtains an IP address.         
        enum AddressMode::Enum GetAddressMode()
        {
            int mode;
            int error_code = BVTNetworkSettings_GetAddressMode(_owned.get(), /* out */ &mode);
            if (0 != error_code)
                throw SdkException(error_code);
            return (enum AddressMode::Enum) mode;
        }

        //
        // Set the sonar's static IP address.
        // \attention Requires AddressMode to be STATIC
        //
        // @param address A valid IPv4 address that the sonar will use.         
        void SetIPv4Address(const std::string & address)
        {
            int error_code = BVTNetworkSettings_SetIPv4Address(_owned.get(), address.c_str());
            if (0 != error_code)
                throw SdkException(error_code);
        }

        //
        // Get the sonar's IP address.
        //
        // @param address A buffer to hold the sonar's IPv4 address. 
        // @param addressLength Size in bytes of the address buffer         
        std::string GetIPv4Address()
        {
            char address[256] = { 0 };
            int addressLength = 255;
            int error_code = BVTNetworkSettings_GetIPv4Address(_owned.get(), address, addressLength);
            if (0 != error_code)
                throw SdkException(error_code);
            return std::string(address);
        }

        //
        // Set the sonar's static subnet mask.
        // \attention Requires AddressMode to be STATIC
        //
        // @param mask A valid IPv4 subnet mask that the sonar will use.        
        void SetSubnetMask(const std::string & mask)
        {
            int error_code = BVTNetworkSettings_SetSubnetMask(_owned.get(), mask.c_str());
            if (0 != error_code)
                throw SdkException(error_code);
        }

        //
        // Get the sonar's subnet mask address.
        //
        // @param mask A buffer to hold the sonar's subnet mask. 
        // @param maskLength Size in bytes of the mask buffer       
        std::string GetSubnetMask()
        {
            char mask[256] = { 0 };
            int maskLength = 255;
            int error_code = BVTNetworkSettings_GetSubnetMask(_owned.get(), mask, maskLength);
            if (0 != error_code)
                throw SdkException(error_code);
            return std::string(mask);
        }

        //
        // Set the sonar's gateway.
        // \attention Requires AddressMode to be STATIC
        //
        // @param gateway A valid IPv4 gateway address that the sonar will use.         
        void SetGateway(const std::string & gateway)
        {
            int error_code = BVTNetworkSettings_SetGateway(_owned.get(), gateway.c_str());
            if (0 != error_code)
                throw SdkException(error_code);
        }

        //
        // Get the sonar's gateway address.
        //
        // @param gateway A buffer to hold the sonar's gateway address. 
        // @param gatewayLength Size in bytes of the gateway buffer         
        std::string GetGateway()
        {
            char gateway[256] = { 0 };
            int gatewayLength = 255;
            int error_code = BVTNetworkSettings_GetGateway(_owned.get(), gateway, gatewayLength);
            if (0 != error_code)
                throw SdkException(error_code);
            return std::string(gateway);
        }

        //
        // Set this to true if you want the sonar to launch a DHCP server at boot.
        // \warning You must not set this to true if the AddressMode is DHCP_CLIENT
        //
        // @param enable Set to true if the sonar should act as a DHCP server.      
        void SetDHCPServerEnabled(bool enable)
        {
            int error_code = BVTNetworkSettings_SetDHCPServerEnabled(_owned.get(), (enable ? 1 : 0));
            if (0 != error_code)
                throw SdkException(error_code);
        }

        //
        // If true the sonar will launch a DHCP server at boot.
        //
        // @param is_enabled Tue if the sonar will act as a DHCP server.        
        bool GetDHCPServerEnabled()
        {
            int is_enabled;
            int error_code = BVTNetworkSettings_GetDHCPServerEnabled(_owned.get(), /* out */ &is_enabled);
            if (0 != error_code)
                throw SdkException(error_code);
            return is_enabled > 0;
        }

        //
        // Set the sonar's IPv4 multicast address.
        // \attention This is experimental
        // \see GetPingMulticast
        //
        // @param address The sonar's IPv4 multicast address.       
        void SetIPv4MulticastAddress(const std::string & address)
        {
            int error_code = BVTNetworkSettings_SetIPv4MulticastAddress(_owned.get(), address.c_str());
            if (0 != error_code)
                throw SdkException(error_code);
        }

        //
        // Get the sonar's IPv4 multicast address.
        // \attention This is experimental
        // \see GetPingMulticast
        //
        // @param address A buffer to hold the sonar's IPv4 multicast address. 
        // @param addressLength Size in bytes of the address buffer         
        std::string GetIPv4MulticastAddress()
        {
            char address[256] = { 0 };
            int addressLength = 255;
            int error_code = BVTNetworkSettings_GetIPv4MulticastAddress(_owned.get(), address, addressLength);
            if (0 != error_code)
                throw SdkException(error_code);
            return std::string(address);
        }

    };

}

#endif
