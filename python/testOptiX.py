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
    def toString(self):
        print("[",self.min,self.max,"]")
        
class ParamArray(Structure):
    _fields_ = [('dims', c_int64 * 2),
                ('data', POINTER(DOUBLE))]

class uData(Union):
    #_pack_ = 8
    _fields_ = [("value", DOUBLE),
                ("p_array", POINTER(ParamArray))]

class Parameter(Structure):
    #_pack_ = 8
    _anonymous_ = ("u",)
    _fields_ = [("u", uData),
                ("bounds", Bounds),
                ("multiplier", DOUBLE),
                ("type", INT),
                ("group", INT),
                ("flags", UINT),
                ]
    def __init__(self):
        self.array=None

    # 
    # np_array doit être un numpy ndarray
    # this function sets self.array, the array flag bit, and the required pointer and dims fields of the Parameter.paramArray sub structure
    def setArray(self, np_array):
        # self.array=ParamArray()
        # self.array.dims[0]=np_array.shape[1]
        # self.array.dims[1]=np_array.shape[0]
        # self.array.data=np_array.ctypes.data_as(POINTER(c_double))
        # self.p_array=pointer(self.array)
        if not isinstance(np_array, np.ndarray):
            print("a 'ndarray' argument was expected ,but the received argument type is ", type(np_array).__name__ )
            return
        if np_array.dtype.name ==  'float64':
            self.array=np_array
        else:
            self.array=np_array.astype('float64')
        pa=ParamArray()
        pa.dims[0]=self.array.shape[1]
        pa.dims[1]=self.array.shape[0]
        pa.data=self.array.ctypes.data_as(POINTER(c_double))
        self.p_array=pointer(pa)
        # pa peut être retrouvé par pa=p_array.contents
        self.flags=self.flags | 0x08
    


def Load_OptiX():
    global OptiX
    print("intialzing SR library")
    OptiX=cdll.LoadLibrary('D:/projets/projetsCB/OptiX/release/OptiX.dll')
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
                parameter.setArray(np.empty([dims[1],dims[0]]))
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
    loadSolemioFile("D:\\projets\\projetsCB\\OptiX\\solemio\\CASSIOPEE")
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
    param.setArray(np.array([[-10,10],[-5,5]]))
    setParameter(NPmirrorID,"surfaceLimits",param)
    param2=Parameter()
    getParameter(NPmirrorID,"surfaceLimits",param2)
    dumpArgParameter(param2)


# initialisation auto    
try:
    test=OptiX
    print(test, "  already initialized")
except NameError:    
    Load_OptiX()
    print("OptiX library initialized")
  