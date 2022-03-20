#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "20";
	static const char MONTH[] = "03";
	static const char YEAR[] = "2022";
	static const char UBUNTU_VERSION_STYLE[] =  "22.03";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 2;
	static const long MINOR  = 0;
	static const long BUILD  = 142;
	static const long REVISION  = 142;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 1370;
	#define RC_FILEVERSION 2,0,142,142
	#define RC_FILEVERSION_STRING "2, 0, 142, 142\0"
	static const char FULLVERSION_STRING [] = "2.0.142.142";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 318;
	

}
#endif //VERSION_H
