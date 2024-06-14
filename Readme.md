OptiX Ray-tracing Library
=================================

OptiX is a ray-tracer library for X-ray optics.
All function are exposed in C form, though it is internally written in C++

June 14, 2024
-------------
V2.5

- Surface errors simulation:
  specific surface error API added to the library  (see doc)


June-july 2023
---------------

- wavefront computations completed (natural and Legendre polynomial interpolation)
- added new aperture API (acts on ray intensity)
- ray intensity includes polarization

March 2022  
--------------
V2.3 

New polynomial classes added : **REQUIRES last EIGEN version 3.4.0**
- Polynomial is a generic class providing intercept computations and fitting functions
- NaturalPolynomial is the specialization for natural polynomials
- *LegendrePolynomial, not yet available*

September 2022  
--------------
V2.1 

Coatings addedd to the element definition. 
A new LoadConfigurationFile function added to the interface to ease the definition of a system.
Language is keyword and tabulation structured. Se example file config.dat.
OptiXLastError is now set when SetParameter is called with an invalid parameter name.
Many doc updates including holographic grating parameter description.

March 29, 2022
---------------
V2.0

Coherent computation are now exposed  by the OptiX C-API with 2 new functions : WaveRadiate and GetPsf.
These functions requires a rather large number of parameters which are passed in structs.
Psf distributions can be calculated for a sequence of equally spaced planes around the estimated focus. They are returned in a four dimensions version of the ndArray struct (\see c_types.h) ). Please look at GetPsf documentation to allocate the storage space required

See interface document

In order to complete the coherent computation the transmission and reflectivity factors of the optical element must be included in the ray tracing. This will be the next extension and will be done by linking the the RefleX library presently under development. Only (simple or layer coated) mirror reflectivity is  presently available in RefleX. Gratings and Crystal will hopefully follow.

March 20, 2022
---------------
A new development branch was started on March10, 2022, the name of which is "coherence".
The object is to extract the wavefront shape from the ray tracing and inject it into Fourier computation to get the PSF.
Two functions added to the Surface class have been added to compute the OPD of the wavefront measured on a given surface with respect to a particular focus-point in the "Surface space", and then  compute the PSF in S and P polarizations. The source used in this ray tracing must be a monochromatic point source. However a waveRadiate function has been added to the sourceBase class which radiate a point source on a uniform aperture grid, without modifying the particular source properties
The Fourier computation involve evaluation Fourier transforms of irregularly spaced data. It therefore make use of the NFFT library : https://www-user.tu-chemnitz.de/~potts/nfft/index.php. The currently used version is  nfft-3.5.2-dll64-openmp  https://www-user.tu-chemnitz.de/~potts/nfft/download.php#windowsdll
After unzipping the available file, you should add the nfft directory  to your include and linker path.  With Codeblocks, add the paths to the search directories  and "libnfft3_threads-4" to the linker settings of the main OptiX target.

Version number was set to 2.0 at the start of the coherence branch. Last common version before forking is 1.6.721

February 10, 2022
-----------------
An Aperture API was added to the interface. It allows to define the obstruction/transmission of the aperture associated with each optical element. The aperture affects the intensity carried by a ray. It doesn't affect the ray path, which is computed though the transported intensity might be 0.

June 23, 2021:
-------------
New surface shape classes have been added:
ConicBaseCylinder  implements any conic based cylinder (elliptical, hyperbolic, and parabolic cylinders)
RevolutionQuadric implements  revolution quadrics (ellipsoid, hyperboloid, a paraboloid).
These shapes are only available with mirrors but could be used with gratings or films if really needed.

Compilation requires that ConicBaseCylinder.cpp and RevolutionQuadric.cpp are added to project configuration files for targets debug, release, test, and test(release)

WARNING - Starting from May 5, 2021  system can be saved in XML text format
 For this aim, it must now be linked to libxml2-2
 You can add libxml2.a to the linked libraries and addd path to libxml2-2\include\libxml2 for compilation,
 libxml2-2\lib for linking and ..\libxml2-2\bin for execution  (both last 2 in the Linker search  tab for CB users)

libxml2-2 for Window 64 bits can be downloaded from https://www.zlatkovic.com/libxml.en.html
current version is libxml2-2.9.3-win32-x86_64

At present it is only single treaded,
Parallel multhreading with openmp should be added later on.

For more details, see the documementation generated by Doxygen
