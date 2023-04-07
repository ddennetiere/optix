#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "04";
	static const char MONTH[] = "04";
	static const char YEAR[] = "2023";
	static const char UBUNTU_VERSION_STYLE[] =  "23.04";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 2;
	static const long MINOR  = 3;
	static const long BUILD  = 455;
	static const long REVISION  = 455;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 522;
	#define RC_FILEVERSION 2,3,455,455
	#define RC_FILEVERSION_STRING "2, 3, 455, 455\0"
	static const char FULLVERSION_STRING [] = "2.3.455.455";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 318;
	

}
#endif //VERSION_H
