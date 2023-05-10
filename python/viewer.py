# -*- coding: utf-8 -*-

'''
This file is contains viewer functions to visualiz results from OptiX ray tracing

'''

__author__  = "François Polack"
__copyright__ = "2023, OptiX project"
__email__ = "francois.polack@synchrotron-soleil.fr"
__version__ = "0.1.0"

__pdoc__={}

import tkinter
from tkinter import filedialog
import time
import numpy as np
import array

from pathlib import Path
from ctypes import Structure, sizeof, c_float, c_char, c_byte, c_ubyte, c_short, c_ushort, c_int, c_uint, c_double
import matplotlib.pyplot as plt
import matplotlib.figure as fig

global diagram

class spotdiag(Structure):
    _pack_ =8
    _fields_ = [("vdim", c_uint),
                ("reserved", c_uint),
                ("counts", c_uint),
                ("lost", c_uint) ]
                
    def __init__(self):
        self.vmin=array.array('d')
        self.vmax=array.array('d')
        self.vmean=array.array('d')
        self.vsigma=array.array('d')
        self.data=None
        
    def load(self, filepath=''):
        if not Path(filepath).is_file():
            tkinter.Tk().withdraw()
            self.filepath=filedialog.askopenfilename(title="open a .sdg file", filetypes=(("spotdiag file","*.sdg"), ("all","*.*") ) )
        print("Opening:", self.filepath)
        
        with open(self.filepath,'rb') as file:
           file.readinto(self)
           print('spot dimension', self.vdim, '    number of spots', self.counts)
           self.vmin.fromfile(file,self.vdim)
           self.vmax.fromfile(file,self.vdim)
           self.vmean.fromfile(file,self.vdim)
           self.vsigma.fromfile(file,self.vdim)
           databuf=array.array('d')
           databuf.fromfile(file, self.vdim*self.counts)
           self.data=np.ndarray((self.counts,self.vdim), dtype=np.float64, buffer=databuf)
        for i in range(self.vdim):
            print(i, self.vmin[i], self.vmax[i], self.vmean[i], self.vsigma[i] )
    
    def view(self,x_axis='x', y_axis='y'):
        naxis=dict( x=0, y=1, xp=2, yp=3)
        xsize=201
        ysize=201
        
        xymap=np.zeros((xsize,ysize),dtype=int)
        xstep=(self.vmax[naxis[x_axis]]-self.vmin[naxis[x_axis]])/(xsize-1)
        x0=self.vmin[naxis[x_axis]]+xstep/2
        ystep=(self.vmax[naxis[y_axis]]-self.vmin[naxis[y_axis]])/(ysize-1)
        y0=self.vmin[1]+ystep/2
        for spot in range(self.counts):
            ix= int((self.data[spot,naxis[x_axis]]-x0)/xstep)
            iy= int((self.data[spot,naxis[y_axis]]-y0)/ystep)
            xymap[iy,ix]+=1


        xscale=np.linspace(x0, self.vmax[naxis[x_axis]]-xstep, num=xsize, endpoint=True)
        yscale=np.linspace(y0, self.vmax[naxis[y_axis]]-ystep, num=ysize, endpoint=True)

        (w, h)=fig.figaspect(xymap)
        #warnings.simplefilter('ignore', category=UserWarning)
        #plotfig=plt.figure(figsize=(w, h), constrained_layout=True)
        plotfig=plt.figure()
        #warnings.resetwarnings()

        plt.pcolormesh(xscale*1e3, yscale*1e3, xymap, cmap=plt.get_cmap('rainbow'),
                  vmin=0, vmax=np.max(xymap), shading='auto')
                 

        cbar= plt.colorbar(aspect=50)
        cbar.set_label("spot counts") #, rotation=270)
        if naxis[x_axis] <2 :
            plt.xlabel(x_axis+' (mm)')
        else:
            plt.xlabel(x_axis+' (mrad)')
        if naxis[y_axis] <2 :
            plt.ylabel(y_axis +' (mm)')
        else:
            plt.ylabel(y_axis +' (mrad)')
        plt.title("Map "+y_axis+" vs "+x_axis)

        return plotfig


if __name__ == "__main__":
    # initialisation auto  
     global diagram
     diagram=spotdiag()
     diagram.load()
