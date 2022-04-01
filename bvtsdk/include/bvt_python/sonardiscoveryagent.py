"""
This file was generated by bvtidl.pl.
Your changes will most likely be lost.
"""

from ctypes import *
import sys
import sdkerror


class SonarDiscoveryAgent(object):
    """
    The SonarDiscoveryAgent is used to discover any BlueView
    sonars resident on a network.
    """
    def __init__(self, handle=None):
        super(SonarDiscoveryAgent, self).__setattr__("_initialized", False)
        self._deleted = False
        if handle is None:
            self._handle = dll.BVTSonarDiscoveryAgent_Create()
        else:
            self._handle = handle
        super(SonarDiscoveryAgent, self).__setattr__("_initialized", True)

    def __del__(self):
        self._deleted = True
        dll.BVTSonarDiscoveryAgent_Destroy(self._handle)

    def __setattr__(self, name, value):
        """ Don't allow setting non-existent attributes on this class
        """
        if self._initialized and not hasattr(self, name):
            raise AttributeError("%s instance has no attribute '%s'" % (self.__class__.__name__, name))
        super(SonarDiscoveryAgent, self).__setattr__(name, value)

    def start(self):
        """
        Causes the SonarDiscoveryAgent to begin probing the network for sonars.
        """
        error_code = dll.BVTSonarDiscoveryAgent_Start(self._handle)
        if (0 != error_code):
            raise sdkerror.SDKError(error_code)

    def stop(self):
        """
        Causes the SonarDiscoveryAgent to stop probing for sonars.
        """
        error_code = dll.BVTSonarDiscoveryAgent_Stop(self._handle)
        if (0 != error_code):
            raise sdkerror.SDKError(error_code)

    @property
    def sonar_count(self):
        """
        Get the number of sonars discovered on the network.
        """
        sonar_count = c_int()
        error_code = dll.BVTSonarDiscoveryAgent_GetSonarCount(self._handle, byref(sonar_count))
        if (0 != error_code):
            raise sdkerror.SDKError(error_code)
        return sonar_count.value

    def get_sonar_info(self, sonar_index):
        """
        Returns the host IP address (as a null-terminated string) for the specified sonar.
        The sonar_index parameter is zero-based, that is, if @ref GetSonarCount returned a value of 3, then valid sonar_index values are 0, 1, and 2.
        A buffer size of 20 characters should be sufficient for the buffer.
        """
        buffer_size = 255
        buffer = create_string_buffer(buffer_size)
        error_code = dll.BVTSonarDiscoveryAgent_GetSonarInfo(self._handle, sonar_index, buffer, buffer_size)
        if (0 != error_code):
            raise sdkerror.SDKError(error_code)
        return (buffer.value)


    def get_handle(self):
        """
        SDK object pointer
        """
        return self._handle
if "win32" in sys.platform:
    dll_name = "bvtsdk4.dll"
elif "darwin" in sys.platform:
    dll_name = "libbvtsdk.dylib"
else:
    dll_name = "libbvtsdk.so"
dll = CDLL(dll_name)
dll.BVTSonarDiscoveryAgent_Create.restype = c_void_p
dll.BVTSonarDiscoveryAgent_Destroy.restype = None
dll.BVTSonarDiscoveryAgent_Destroy.argtypes = (c_void_p,)
dll.BVTSonarDiscoveryAgent_Start.restype = c_int
dll.BVTSonarDiscoveryAgent_Start.argtypes = (c_void_p, )
dll.BVTSonarDiscoveryAgent_Stop.restype = c_int
dll.BVTSonarDiscoveryAgent_Stop.argtypes = (c_void_p, )
dll.BVTSonarDiscoveryAgent_GetSonarCount.restype = c_int
dll.BVTSonarDiscoveryAgent_GetSonarCount.argtypes = (c_void_p, POINTER(c_int), )
dll.BVTSonarDiscoveryAgent_GetSonarInfo.restype = c_int
dll.BVTSonarDiscoveryAgent_GetSonarInfo.argtypes = (c_void_p, c_int, c_char_p, c_int, )
