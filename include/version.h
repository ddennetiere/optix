#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "10";
	static const char MONTH[] = "10";
	static const char YEAR[] = "2022";
	static const char UBUNTU_VERSION_STYLE[] =  "22.10";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 2;
	static const long MINOR  = 1;
	static const long BUILD  = 687;
	static const long REVISION  = 687;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 2094;
	#define RC_FILEVERSION 2,1,687,687
	#define RC_FILEVERSION_STRING "2, 1, 687, 687\0"
	static const char FULLVERSION_STRING [] = "2.1.687.687";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 363;
	

}
#endif //VERSION_H
