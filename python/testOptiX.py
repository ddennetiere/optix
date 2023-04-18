# -*- coding: utf-8 -*-
import ctypes, os
#from ctypes import cdll, windll, Structure
from ctypes.wintypes import *
from ctypes import *
#from ctypes import WINFUNCTYPE, Structure, pointer,byref, POINTER,  c_char_p, c_void_p, c_double, c_float
import numpy as np



global OptiX


class Bounds(Structure):
    _fields_ = [("min", DOUBLE),
                ("max", DOUBLE)]

    def __repr__(self):
        return f"Parameter bounds [{self.min}, {self.max}]"
        
class ParamArray(Structure):
    _fields_ = [('dims', c_int64 * 2),
                ('data', POINTER(DOUBLE))]

class uData(Union):
    #_pack_ = 8
    _fields_ = [("value", DOUBLE),
                ("p_array", POINTER(ParamArray))]

class Parameter(Structure):
    """
    C structure defining modifiable fields of optix optical element parameters. Note bounds type is Bounds. See Bounds
    docstring.
    """
    # _pack_ = 8
    _anonymous_ = ("u",)
    _fields_ = [("u", uData),
                ("bounds", Bounds),
                ("multiplier", DOUBLE),
                ("type", INT),
                ("group", INT),
                ("flags", UINT),
                ]

    def __init__(self):
        super().__init__()
        self._array = None

    @property
    def array(self):
        return self._array

    @array.setter
    def array(self, np_array):
        """
        np_array doit être un numpy ndarray
        this function sets self.array, the array flag bit, and the required pointer and dims fields of
        the Parameter.paramArray sub structure

        :param np_array: array value
        :type np_array: numpy.ndarray
        :return: None
        :rtype: Nonetype
        """
        if not isinstance(np_array, np.ndarray):
            print("a 'ndarray' argument was expected ,but the received argument type is ", type(np_array).__name__)
            return
        if np_array.dtype.name == 'float64':
            self._array = np_array
        else:
            self._array = np_array.astype('float64')
        pa = ParamArray()
        pa.dims[0] = self._array.shape[1]
        pa.dims[1] = self._array.shape[0]
        pa.data = self._array.ctypes.data_as(POINTER(c_double))
        self.p_array = pointer(pa)
        # pa peut être retrouvé par pa=p_array.contents
        self.flags |= 1 << 3

    def __repr__(self):
        if self.flags & 1 << 3:
            data_length = self.p_array.contents.dims[0]*self.p_array.contents.dims[1]
            data_address = self.p_array.contents.data.contents
            data = np.ctypeslib.as_array((ctypes.c_double * data_length).from_address(ctypes.addressof(data_address)))
            return f"Param object containing:\n" \
                   f"\t ParamArray dimension {self.p_array.contents.dims[0]}*{self.p_array.contents.dims[1]}, " \
                   f"@ {hex(ctypes.addressof(self.p_array))}\n" \
                   f"\t\twith data = {data} @ {hex(ctypes.addressof(data_address))}\n" \
                   f"\t Bounds {self.bounds}\n" \
                   f"\t multiplier {self.multiplier}\n" \
                   f"\t type {self.type}\n" \
                   f"\t group {self.group}\n" \
                   f"\t flags {self.flags}\n"
        else:
            return f"Param object containing:\n" \
                   f"\t Value {self.value}\n" \
                   f"\t Bounds {self.bounds}\n" \
                   f"\t multiplier {self.multiplier}\n" \
                   f"\t type {self.type}\n" \
                   f"\t group {self.group}\n" \
                   f"\t flags {self.flags}\n"
    


def Load_OptiX():
    global OptiX
    print("intialzing SR library")
    OptiX=cdll.LoadLibrary(r'D:\Dennetiere\optix\release/OptiX.dll')
    OptiX.LoadSolemioFile.restype=BYTE      # gcc bool match to ctypes BYTE 
    OptiX.LoadSolemioFile.argtypes=[LPCSTR]
    OptiX.GetOptiXLastError.restype=BYTE
    OptiX.GetOptiXLastError.argtypes=[LPCSTR, INT]
    OptiX.GetElementID.argtypes=[LPCSTR]
    OptiX.GetElementID.restype=HANDLE
    OptiX.GetNextElement.argtypes=[HANDLE]
    OptiX.GetNextElement.restype=HANDLE
    OptiX.Align.argtypes=[HANDLE, DOUBLE]
    OptiX.Align.restype=BYTE
    OptiX.ClearImpacts.argtypes=[HANDLE]
    OptiX.ClearImpacts.restype=BYTE
    OptiX.Generate.argtypes=[HANDLE, DOUBLE]
    OptiX.Generate.restype=INT
    OptiX.Radiate.argtypes=[HANDLE]
    OptiX.Radiate.restype=BYTE
    OptiX.GetParameterFlags.argtypes = [HANDLE, LPCSTR, POINTER(c_uint32)]
    OptiX.GetParameterFlags.restype = BYTE
    OptiX.GetParameter.argtypes = [HANDLE, LPCSTR, HANDLE]
    OptiX.GetParameter.restype = BYTE
    OptiX.GetArrayParameter.argtypes = [HANDLE, LPCSTR, HANDLE, c_size_t]
    OptiX.GetArrayParameter.restype = BYTE
    OptiX.GetParameterArrayDims.argtypes = [HANDLE, LPCSTR, POINTER(ARRAY(c_int64,2)) ]
    OptiX.GetParameterArrayDims.restype = BYTE
    # SetParameter(size_t elementID, const char* paramTag,  Parameter paramData)
    OptiX.SetParameter.argtypes = [HANDLE, LPCSTR, Parameter]
    OptiX.SetParameter.restype = BYTE
    OptiX.CreateElement.argtypes = [LPCSTR, LPCSTR]
    OptiX.CreateElement.restype = HANDLE
    OptiX.MemoryDump.argtypes = [c_voidp, c_uint64] 
    OptiX.DumpArgParameter.argtypes = [POINTER(Parameter)] 

def Release_OptiX():
    global OptiX
    windll.kernel32.FreeLibrary.argtypes=[HMODULE]
    libhandle=OptiX._handle
    windll.kernel32.FreeLibrary(libhandle)
    del OptiX
    
def version():
    OptiX.Version()
    
def getElementID(name):
    ID=OptiX.GetElementID(name.encode())
    print(hex(ID))
    return ID
    
def getNextElement(name):
    ID=OptiX.GetElementID(name.encode())
    if ID :
        ID=OptiX.GetNextElement(ID)
    else:
        outputError("Invalid element name:")
    return ID
    
def outputError(message):
    buf = ctypes.create_string_buffer(256) # create a 128 byte buffer
    OptiX.GetOptiXLastError(buf,256)
    print(message)
    print(buf.value)
    
def loadSolemioFile(name):
    if OptiX.LoadSolemioFile(name.encode()) == 0 :
        outputError("Error loading Solemio file :")
        
def align(elemID, wavelength):
    if OptiX.Align(elemID, wavelength) == 0 :
        outputError("Alignment error:")

def clearImpacts(elemID):
    if OptiX.ClearImpacts(elemID) == 0 :
        outputError("Clear impacts error:")

def generate(elemID, wavelength):
    rays=OptiX.Generate(elemID, wavelength)
    return rays

def radiate(elemID):
    if OptiX.Radiate(elemID) == 0 :
        outputError("Radiate Error:")
        
def getParameterFlags(elemID, paramName, flags):
    if OptiX.GetParameterFlags(elemID, paramName.encode(), byref(flags)) == 0 :
        outputError("Get parameter Error:")
 
def getValueParameter(elemID, paramName, parameter):
    if OptiX.GetParameter(elemID, paramName.encode(), byref(parameter)) == 0 :
        outputError("Get parameter Error:")

def getParameterArrayDims(elemID, paramName, dims):
    if OptiX.GetParameterArrayDims(elemID, paramName.encode(), byref(dims) ) == 0 :
        outputError("Get parameter Error:")

def getArrayParameter(elemID, paramName, parameter, maxsize):
    if OptiX.GetArrayParameter(elemID, paramName.encode(), byref(parameter), maxsize) == 0 :
        outputError("Get parameter Error:")

def createElement(elemType, elemName):
    return OptiX.CreateElement(elemType.encode(), elemName.encode() )
    # validity of the pointer must be tested in the calling program

def dumpArgParameter(parameter):
    OptiX.DumpArgParameter(byref(parameter))

def memoryDump(address, size):
    OptiX.MemoryDump(address,size)

def getParameter(elemID,paramName, parameter):
    if not isinstance(parameter, Parameter) :
        print("parameter must be an instance of class Parameter")
        return
    flags=c_uint32()
    if OptiX.GetParameterFlags(elemID, paramName.encode(), byref(flags) ) != 0 :
        # print("flags=",flags)
        if flags.value & 0x8 :
            #the paracmeter contains an array
            dims=(c_int64 * 2)()
            if OptiX.GetParameterArrayDims(elemID, paramName.encode(), byref(dims) ) != 0 :
                parameter.array = np.empty([dims[1],dims[0]])
                if OptiX.GetArrayParameter(elemID, paramName.encode(), byref(parameter), dims[0]*dims[1] ) != 0 :
                    return
        else:
            # release the array data and unset flags array bit, if needed 
            parameter.array = None
            parameter.flags=0
            if OptiX.GetParameter(elemID, paramName.encode(), byref(parameter)) != 0 :
                return 
    outputError("Get parameter Error:")

def setParameter(elemID,paramName, parameter):
    if OptiX.SetParameter(elemID, paramName.encode(), parameter) == 0 :
        outputError("Get parameter Error:")


def test(wavelength):
    loadSolemioFile(r"D:\Dennetiere\optix\\solemio\\CASSIOPEE")
    source=getElementID("S_ONDUL1")
    align(source, wavelength)
    clearImpacts(source)
    print(generate(source, wavelength), " rays")
    radiate(source)
    
def paramTest():
    NPmirrorID=createElement("NaturalPolynomialMirror","NPmirror")
    param=Parameter()
    getParameter(NPmirrorID,"surfaceLimits",param)
    dumpArgParameter(param)
    param.array = np.array([[-10,10],[-5,5]])
    setParameter(NPmirrorID,"surfaceLimits",param)
    param2=Parameter()
    getParameter(NPmirrorID,"surfaceLimits",param2)
    dumpArgParameter(param2)
    param3=Parameter()
    getParameter(NPmirrorID,"coefficients",param3)
    dumpArgParameter(param3)
    param3.array = np.zeros((1,1))
    dumpArgParameter(param3)
    print(param3)
    setParameter(NPmirrorID,"coefficients",param3)
    


if __name__ == "__main__":
    # initialisation auto    
    try:
        test=OptiX
        print(test, "  already initialized")
    except NameError:    
        Load_OptiX()
        print("OptiX library initialized")
    paramTest()
    print("\n"*3)
    # test(1e-9)
  